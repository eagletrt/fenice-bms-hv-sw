/**
 * @file		cli.c
 * @brief		This file contains the functions to create the cli
 *
 * @date		Oct 24, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bal.h"
#include "pack.h"

const char *cli_commands[N_COMMANDS] = {
	"volts", "volts all", "temps", "temps all", "status", "bal", "?", "\ta"};

void _cli_volts(char *cmd, state_global_data_t *data, BMS_STATE_T state,
				char *out) {
	sprintf(out,
			"total.....%.2f V\r\nmax.......%.3f V\r\nmin.......%.3f V"
			"\r\ndelta.....%.3f V\r\n",
			(float)data->pack.total_voltage / 10000,
			(float)data->pack.max_voltage / 10000,
			(float)data->pack.min_voltage / 10000,
			(float)(data->pack.max_voltage - data->pack.min_voltage) / 10000);
}

void _cli_volts_all(char *cmd, state_global_data_t *data, BMS_STATE_T state,
					char *out) {
	out[0] = '\0';

	for (uint8_t i = 0; i < PACK_MODULE_COUNT; i++) {
		if (i % LTC6813_CELL_COUNT == 0) {
			sprintf(out + strlen(out), "%-3d", i % LTC6813_CELL_COUNT);
		} else if (i % 9 == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}
		sprintf(out + strlen(out), "[%2u %-.3fv] ", i % LTC6813_CELL_COUNT,
				(float)data->pack.voltages[i] / 10000);
	}

	sprintf(out + strlen(out), "\r\n");
}

void _cli_temps(char *cmd, state_global_data_t *data, BMS_STATE_T state,
				char *out) {
	sprintf(out,
			"average.....%.1f C\r\nmax.........%2u "
			"C\r\nmin.........%2u C\r\n"
			"delta.......%2u C\r\n",
			(float)data->pack.avg_temperature / 10, data->pack.max_temperature,
			data->pack.min_temperature,
			data->pack.max_temperature - data->pack.min_temperature);
}

void _cli_temps_all(char *cmd, state_global_data_t *data, BMS_STATE_T state,
					char *out) {
	out[0] = '\0';

	uint8_t temps[LTC6813_TEMP_COUNT * LTC6813_COUNT];
	pack_update_temperatures_all(data->hspi, temps);

	for (uint8_t i = 0; i < LTC6813_TEMP_COUNT * LTC6813_COUNT; i++) {
		sprintf(out + strlen(out), "[%3u] %2uc ", i, temps[i]);

		if ((i + 1) % 9 == 0) {
			sprintf(out + strlen(out), "\r\n");
		}
	}
}
void _cli_status(char *cmd, state_global_data_t *data, BMS_STATE_T state,
				 char *out) {
#define n_items 5

	char error_i[4] = {'\0'};
	itoa(data->error_index, error_i, 10);

	char *bal = data->balancing.enable ? "true" : "false";

	char thresh[5] = {'\0'};
	itoa((float)data->balancing.threshold / 10, thresh, 10);

	char *values[n_items][3] = {
		{"BMS state", (char *)bms_state_names[state]},
		{"global error", (char *)error_names[data->error]},
		{"global error index", error_i},
		{"balancing", bal},
		{"balancing threshold", thresh}};

	out[0] = '\0';
	for (uint8_t i = 0; i < n_items; i++) {
		sprintf(out + strlen(out), "%s%s%s\r\n", values[i][0],
				"........................" + strlen(values[i][0]),
				values[i][1]);
	}
}

void _cli_balance(char *cmd, state_global_data_t *data, BMS_STATE_T state,
				  char *out) {
	uint8_t cmd_len = strlen("bal");

	if (strcmp(cmd + cmd_len, " tog") == 0) {
		data->balancing.enable = !data->balancing.enable;

		sprintf(out, "setting balancing to %u\r\n", data->balancing.enable);
	} else if (strncmp(cmd + cmd_len, " thr", 4) == 0) {
		data->balancing.threshold = atoi(cmd + cmd_len + 5) * 10;

		sprintf(out, "setting balancing threshold to %u mV\r\n",
				data->balancing.threshold / 10);
	}
}

void _cli_taba(char *cmd, state_global_data_t *data, BMS_STATE_T state,
			   char *out) {
	sprintf(out,
			" #######    #    ######     #    ######     #    ####### ####### "
			"######  \r\n"
			"    #      # #   #     #   # #   #     #   # #      #    #       "
			"# "
			"    # \r\n"
			"    #     #   #  #     #  #   #  #     #  #   #     #    #       "
			"# "
			"    # \r\n"
			"    #    #     # ######  #     # ######  #     #    #    #####   "
			"# "
			"    # \r\n"
			"    #    ####### #     # ####### #   #   #######    #    #       "
			"# "
			"    # \r\n"
			"    #    #     # #     # #     # #    #  #     #    #    #       "
			"# "
			"    # \r\n"
			"    #    #     # ######  #     # #     # #     #    #    ####### "
			"######  \r\n");
}

void _cli_help(char *cmd, state_global_data_t *data, BMS_STATE_T state,
			   char *out) {
	sprintf(out, "command list:\r\n");
	for (uint8_t i = 0; i < N_COMMANDS - 1; i++) {
		sprintf(out + strlen(out), "- %s\r\n", cli_commands[i]);
	}
}

void cli_init(cli_t *cli, UART_HandleTypeDef *uart) {
	cli->uart = uart;
	cli->complete = false;
	cli->echo = true;
	cli->rx.index = 0;
	cli->rx.buffer = (char *)malloc(BUF_SIZE);

	cli->escaping = 255;

	cli->history.list = (buffer_t *)malloc(sizeof(buffer_t));

	cli_state_func_t *temp[N_COMMANDS] = {
		&_cli_volts,  &_cli_volts_all, &_cli_temps, &_cli_temps_all,
		&_cli_status, &_cli_balance,   &_cli_help,  &_cli_taba};
	memcpy(cli->states, temp, sizeof(cli->states));

	LL_USART_EnableIT_RXNE(cli->uart->Instance);
	LL_USART_EnableIT_ERROR(cli->uart->Instance);
	LL_USART_EnableIT_TXE(cli->uart->Instance);

	char init[BUF_SIZE];
	sprintf(init,
			"\r\n\n********* Fenice BMS *********\r\n"
			" build: %s @ %s\r\n\n type ? for commands\r\n\n"
			"> ",
			__DATE__, __TIME__);

	HAL_UART_Transmit(cli->uart, (uint8_t *)init, strlen(init), 100);
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

void cli_print(char *text, size_t length) {
	HAL_UART_Transmit(cli.uart, (uint8_t *)text, length, 500);
}

void cli_handle_escape() {
	if (cli.rx.buffer[cli.rx.index - 1] == '[') {
		cli.escaping = 255;
		uint8_t h_i;  // To be displayed history index

		if (cli.rx.buffer[cli.rx.index] == 'A' &&
			cli.history.showing > 0) {  // UP

			h_i = cli.history.showing - 1;
		} else if (cli.rx.buffer[cli.rx.index] == 'B' &&
				   cli.history.showing < cli.history.index) {  // DOWN

			h_i = cli.history.showing + 1;
		} else {
			return;
		}

		buffer_t *hist = &cli.history.list[h_i];
		cli.history.showing = h_i;

		// strcpy(cli->rx.buffer, hist->buffer);

		memcpy(cli.rx.buffer, hist->buffer, sizeof(char) * BUF_SIZE);

		char eraser[BUF_SIZE];
		sprintf(eraser, "%-*c\r", cli.rx.index + PS1_SIZE, '\r');

		strcat(eraser, ps1);
		strcat(eraser, cli.rx.buffer);

		HAL_UART_Transmit(cli.uart, (uint8_t *)eraser, strlen(eraser), 500);

		cli.rx.index = hist->index;
	}
	return;
}

void cli_loop(state_global_data_t *data, BMS_STATE_T state) {
	if (cli.complete) {
		cli.complete = false;

		if (cli.escaping < 255) {
			cli_handle_escape(cli);
			return;
		}

		cli.rx.index = cli_clean(cli.rx.buffer);

		if (cli.rx.index > 0) {  // Add to history

			cli.history.list = realloc(
				cli.history.list, (cli.history.index + 1) * sizeof(buffer_t));

			cli.history.list[cli.history.index].buffer =
				(char *)malloc(sizeof(char) * BUF_SIZE);

			// strcpy(cli.history.list[cli.history.index].buffer,
			// cli.rx.buffer);

			// cli.history.list[cli.history.index].index = cli.rx.index;

			memcpy(cli.history.list[cli.history.index].buffer, cli.rx.buffer,
				   sizeof(char) * BUF_SIZE);

			cli.history.list[cli.history.index].index = cli.rx.index;

			cli.history.index++;
			cli.history.showing = cli.history.index;
		}

		cli.rx.index = 0;

		char buf[2000] = "?\r\n";

		for (uint8_t i = 0; i < N_COMMANDS; i++) {
			if (strncmp(cli.rx.buffer, cli_commands[i],
						strlen(cli_commands[i])) == 0) {
				cli.states[i](cli.rx.buffer, data, state, buf);
			}
		}

		cli.rx.buffer[0] = '\0';

		HAL_UART_Transmit(cli.uart, (uint8_t *)buf, strlen(buf), 200);
		HAL_UART_Transmit(cli.uart, (uint8_t *)ps1, PS1_SIZE, 100);
	}
}

// Interrupt callback
void cli_char_receive() {
	uint8_t rx_char = LL_USART_ReceiveData8(USART2);

	if (rx_char == '\033') {  // Arrow
		cli.escaping = cli.rx.index;
	} else if (rx_char == '\r' || rx_char == '\n') {
		cli.complete = true;
		cli.rx.buffer[cli.rx.index] = '\0';

		HAL_UART_Transmit(cli.uart, (uint8_t *)"\r\n", 2, 200);

		return;
	}

	// Add to buffer
	cli.rx.buffer[cli.rx.index] = rx_char;

	if (cli.escaping < 255 && cli.rx.index > cli.escaping + 1) {
		cli.complete = true;
		return;
	}

	cli.rx.index++;

	if (cli.escaping == 255 && cli.echo) {
		HAL_UART_Transmit(cli.uart, &rx_char, 1, 50);
	}

	if (cli.rx.index == BUF_SIZE) {
		cli.complete = true;
	}
}