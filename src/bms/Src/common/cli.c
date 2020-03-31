/**
 * @file		cli.c
 * @brief		This file contains the functions to create the cli
 *
 * @date		Oct 24, 2019
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cli_buf_init(buffer_t *buf) {
	buf->index = 0;
	buf->buffer[0] = '\0';
}

void cli_init(cli_t *cli) {
	//cli->input_buf = '\0';

	cli->history_index = 0;
	cli->complete = false;
	cli->receive = true;
	cli->escaping = BUF_SIZE;

	cli_buf_init(&cli->current_command);

	list_init(cli->history);
}

uint8_t cli_clean(char *cmd) {
	uint8_t cursor = 0;

	for (uint8_t i = 0; i < BUF_SIZE; i++) {
		if (cmd[i] == '\b') {
			// Check backspace
			if (cursor > 0) {
				cursor--;
				cmd[cursor] = '\0';
			}
		} else if (cmd[i] == '\033') {
			cmd[cursor] = '\0';
			return cursor;
		} else {
			// Add to buffer
			cmd[cursor] = cmd[i];

			if (cmd[i] == '\0') {
				// Exit if done
				return cursor;
			}
			cursor++;
		}
	}
	return cursor;
}

void cli_print(cli_t *cli, char *text, size_t length) {
	HAL_UART_Transmit(cli->uart, (uint8_t *)text, length, 500);
}

void cli_handle_escape(cli_t *cli) {
	uint8_t list_n = list_count(cli->history);

	if (cli->current_command->buffer[cli->current_command->index - 1] == '[') {
		cli->escaping = BUF_SIZE;
		uint8_t h_i;  // Index of history to be displayed

		if (cli->current_command->buffer[cli->current_command->index] == 'A' &&
			cli->history_index < list_n - 1) {	// UP

			h_i = cli->history_index + 1;
		} else if (cli->current_command->buffer[cli->current_command->index] == 'B' &&
				   cli->history_index > 1) {  // DOWN

			h_i = cli->history_index - 1;
		} else {
			return;
		}

		buffer_t *hist = (buffer_t *)list_get_nth(cli->history, h_i);
		cli->history_index = h_i;

		memcpy(cli->current_command->buffer, hist->buffer, sizeof(char) * BUF_SIZE);

		char eraser[BUF_SIZE];
		sprintf(eraser, "%-*c\r", cli->current_command->index + PS_SIZE, '\r');

		strcat(eraser, cli_ps);
		strcat(eraser, cli->current_command->buffer);

		HAL_UART_Transmit(cli->uart, (uint8_t *)eraser, strlen(eraser), 500);

		cli->current_command->index = hist->index;
	}
	return;
}

void cli_loop(cli_t *cli) {
	if (cli->receive) {
		cli->receive = false;
		cli_char_receive();
		HAL_UART_Receive_IT(cli->uart, (uint8_t *)&cli->input_buf, 1);
	}

	if (cli->complete) {
		cli->complete = false;

		if (cli->escaping < BUF_SIZE) {
			cli_handle_escape(cli);
			return;
		}

		// Clean the buffer from backspaces
		cli->current_command->index = cli_clean(cli->current_command->buffer);

		// TODO: Make this better
		char tx_buf[3000] = "?\r\n";

		// Check which command corresponds with the buffer
		for (uint8_t i = 0; i < N_COMMANDS; i++) {
			if (strncmp(cli->current_command->buffer, cli_commands[i], strlen(cli_commands[i])) == 0) {
				cli->states[i](cli->current_command->buffer, tx_buf);
			}
		}

		buffer_t *last_buf = (buffer_t *)list_get_nth(cli->history, 1);

		// Check if last history entry is equal to the current
		bool comp = last_buf == NULL || strcmp(cli->current_command->buffer, last_buf->buffer) != 0;
		if (cli->current_command->index > 0 && comp) {
			// If the last command wasn't empty, we save it to history.

			list_insert(&cli->history, &cli->current_command, sizeof(buffer_t));
		}

		// Prepare current_command for the next command
		cli_buf_init(cli->current_command);

		cli->history_index = 0;

		HAL_UART_Transmit(cli->uart, (uint8_t *)tx_buf, strlen(tx_buf), 200);
		HAL_UART_Transmit(cli->uart, (uint8_t *)cli_ps, PS_SIZE, 100);
	}
}

void cli_handle_interrupt(cli_t *cli) {
	cli->receive = true;
}

// Interrupt callback
void cli_char_receive(cli_t *cli) {
	if (cli->input_buf != '\0') {
		if (cli->input_buf == '\033') {	 // Arrow
			cli->escaping = cli->current_command->index;
		} else if (cli->input_buf == '\r' || cli->input_buf == '\n') {
			cli->complete = true;
			cli->current_command->buffer[cli->current_command->index] = '\0';

			HAL_UART_Transmit(cli->uart, (uint8_t *)"\r\n", 2, 10);

			return;
		}

		cli->current_command->buffer[cli->current_command->index] = cli->input_buf;

		if (cli->escaping < BUF_SIZE && cli->current_command->index > cli->escaping + 1) {
			cli->complete = true;
			return;
		}
		cli->current_command->index++;

		// Echo the last char
		if (cli->escaping == BUF_SIZE && CLI_ECHO) {
			HAL_UART_Transmit(cli->uart, (uint8_t *)&cli->input_buf, 1, 10);
		}

		if (cli->current_command->index == BUF_SIZE) {
			cli->complete = true;
		}
	}
}