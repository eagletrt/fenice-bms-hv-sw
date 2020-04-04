/**
 * @file		cli.c
 * @brief		This file contains the functions to create the cli
 *
 * @date		Oct 24, 2019
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "common/cli.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HIST_UP 1
#define HIST_DOWN 0

void cli_buf_init(buffer_t *buf) {
	buf->index = 0;
	buf->buffer[0] = '\0';
}

void cli_init(cli_t *cli) {
	cli_buf_init(&cli->current_command);

	list_init(cli->history);

	//the history always has a top void buffer (use case: when going down history this buffer will be used
	//for ther partial command written befor going into history)
	buffer_t first_void_buffer;
	cli_buf_init(&first_void_buffer);
	list_insert(&cli->history, &first_void_buffer, sizeof(buffer_t));

	cli->complete = false;
	cli->receive = true;
	cli->escaping = BUF_SIZE;
	cli->current_history = cli->history;
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

node_t *_cli_get_next_history_node(node_t *history, bool direction, node_t *current_history) {
	//if we are starting at default position (aftert a write in cli)
	//or the history is empty: in that case current_history has to be necessary NULL
	if (current_history == NULL)
		return history;

	//otherwise check the direction and boundaries (the list grows by next pointers)
	if (direction == HIST_DOWN && current_history->prev != NULL) {
		return current_history->prev;
	} else if (direction == HIST_UP && current_history->next != NULL) {
		return current_history->next;
	}

	//default condition if the direction is non existant (wrong value)
	return current_history;
}
void cli_handle_escape(cli_t *cli) {
	if (cli->current_command.buffer[cli->current_command.index - 1] == '[') {
		cli->escaping = BUF_SIZE;
		uint8_t direction;

		if (cli->current_command.buffer[cli->current_command.index] == 'A') {  // UP
			direction = HIST_UP;
		} else if (cli->current_command.buffer[cli->current_command.index] == 'B') {  // DOWN
			direction = HIST_DOWN;
		} else {
			// Unknown escape sequence
			return;
		}
		cli->current_history = _cli_get_next_history_node(cli->history, direction, cli->current_history);
		if (cli->current_history == NULL) {
			// No history
			return;
		}

		//-----------------------COMMENT OF THE YEAR------------------------//
		//																	//
		//																	//
		//			 Naaaaaah, dovrebbe essere cosa da poco (lel)			//
		//																	//
		//																	//
		//------------------------------------------------------------------//

		// TODO: translate this
		// si crea la stringa che elimina il vecchio comando stampato in seriale, in pratica si buttano tanti spazi quanti erano i caratteri del vecchio comando
		char eraser[BUF_SIZE];
		sprintf(eraser, "%-*c\r", cli->current_command.index + PS_SIZE, '\r');

		//------------------------------------------------------------------//
		//																	//
		//				cosa e' cli_ps  il comando della history? 			//
		// 		è il pisellino che c'è all'inizio di ogni riga, il "> "		//
		//																	//
		//------------------------------------------------------------------//

		strcat(eraser, cli_ps);

		//N.B. qui bisogna stare attenti perche' data e' un puntatore mentre current_command no quinndi
		//se ci sono errori poterbbero essere qui
		memcpy(&(cli->current_command), cli->current_history->data, sizeof(buffer_t));

		strcat(eraser, cli->current_command.buffer);

		HAL_UART_Transmit(cli->uart, (uint8_t *)eraser, strlen(eraser), 500);
	}
	return;
}

void cli_loop(cli_t *cli) {
	if (cli->receive) {
		cli->receive = false;
		cli_char_receive(cli);
		HAL_UART_Receive_IT(cli->uart, (uint8_t *)&cli->input_buf, 1);
	}

	if (cli->complete) {
		cli->complete = false;

		if (cli->escaping < BUF_SIZE) {
			cli_handle_escape(cli);
			return;
		}

		// Clean the buffer from backspaces
		cli->current_command.index = cli_clean(cli->current_command.buffer);

		// TODO: Make this better
		char tx_buf[3000] = "?\r\n";

		// Check which command corresponds with the buffer
		for (uint8_t i = 0; i < cli->commands.count; i++) {
			if (strncmp(cli->current_command.buffer, cli->commands.names[i], strlen(cli->commands.names[i])) == 0) {
				cli->commands.functions[i](cli->current_command.buffer, tx_buf);
			}
		}

		buffer_t *last_buf = (buffer_t *)list_get_nth(cli->history, 1);

		// Check if last history entry is equal to the current
		bool comp = last_buf == NULL || strcmp(cli->current_command.buffer, last_buf->buffer) != 0;
		if (cli->current_command.index > 0 && comp) {
			// If the last command wasn't empty, we save it to history.

			//the command is not directly added to the history but instead the first node on top
			//is modified since this one is always empty (the purpose of this node is descirbed in cli_init)
			memcpy(cli->history->data, &cli->current_command, sizeof(buffer_t));

			//afeter the node has been changed a new void node has to be placed on top ho the history
			buffer_t top_history_virgin_buffer;
			cli_buf_init(&top_history_virgin_buffer);
			list_insert(&cli->history, &top_history_virgin_buffer, sizeof(buffer_t));

			//old style, remove void_buffer insert in cli_init
			//list_insert(&cli->history, &cli->current_command, sizeof(buffer_t));
		}

		// Prepare current_command for the next command
		cli_buf_init(&cli->current_command);

		cli->current_history = cli->history;
		//use if no void history node is used
		//cli->current_history = NULL;

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
			cli->escaping = cli->current_command.index;
		} else if (cli->input_buf == '\r' || cli->input_buf == '\n') {
			cli->complete = true;
			cli->current_command.buffer[cli->current_command.index] = '\0';

			HAL_UART_Transmit(cli->uart, (uint8_t *)"\r\n", 2, 10);

			return;
		}

		cli->current_command.buffer[cli->current_command.index] = cli->input_buf;

		if (cli->escaping < BUF_SIZE && cli->current_command.index > cli->escaping + 1) {
			cli->complete = true;
			return;
		}
		cli->current_command.index++;

		// Echo the last char
		// TODO: CHeck for echo
		//if (cli->escaping == BUF_SIZE && CLI_ECHO) {
		if (cli->escaping == BUF_SIZE) {
			HAL_UART_Transmit(cli->uart, (uint8_t *)&cli->input_buf, 1, 10);
		}

		if (cli->current_command.index == BUF_SIZE) {
			cli->complete = true;
		}
	}
}