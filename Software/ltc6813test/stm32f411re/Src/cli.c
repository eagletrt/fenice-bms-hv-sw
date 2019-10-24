#include "cli.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *cli_commands[N_COMMANDS] = {"volts", "temps", "status", "taba",
										"?"};

char *_cli_volts(state_global_data_t *data, BMS_STATE_T state) {
	char tmp[BUF_SIZE];

	sprintf(tmp, "Total: %5.2f V\tMax: %4.2f V\tMin: %4.2f V\n",
			(float)data->pack.total_voltage / 10000,
			(float)data->pack.max_voltage / 10000,
			(float)data->pack.min_voltage / 10000);

	char *out = tmp;
	return out;
}

char *_cli_temps(state_global_data_t *data, BMS_STATE_T state) {
	char tmp[BUF_SIZE];

	sprintf(tmp, "Average: %4.1f °C\tMax: %4.1f °C\tMin: %4.1f °C\n",
			(float)data->pack.avg_temperature / 100,
			(float)data->pack.max_temperature / 100,
			(float)data->pack.min_temperature / 100);

	char *out = tmp;
	return out;
}

char *_cli_status(state_global_data_t *data, BMS_STATE_T state) {
	char *values[3][3] = {{"BMS state", (char *)bms_state_names[state]},
						  {"Global error", (char *)error_names[data->error]},
						  {"Global error index", NULL}};

	char val[4];
	itoa(data->error_index, val, 10);
	values[2][1] = val;

	char tmp[BUF_SIZE] = "\0";

	for (uint8_t i = 0; i < 3; i++) {
		sprintf(tmp + strlen(tmp), "%s%s%s\n", values[i][0],
				"........................" + strlen(values[i][0]),
				values[i][1]);
	}

	char *out = tmp;
	return out;
}

char *_cli_taba(state_global_data_t *data, BMS_STATE_T state) {
	return " #######    #    ######     #    ######     #    ####### ####### "
		   "######  \n"
		   "    #      # #   #     #   # #   #     #   # #      #    #       # "
		   "    # \n"
		   "    #     #   #  #     #  #   #  #     #  #   #     #    #       # "
		   "    # \n"
		   "    #    #     # ######  #     # ######  #     #    #    #####   # "
		   "    # \n"
		   "    #    ####### #     # ####### #   #   #######    #    #       # "
		   "    # \n"
		   "    #    #     # #     # #     # #    #  #     #    #    #       # "
		   "    # \n"
		   "    #    #     # ######  #     # #     # #     #    #    ####### "
		   "######  \n"
		   "                                                                   "
		   "      \n"

		;
}

char *_cli_help(state_global_data_t *data, BMS_STATE_T state) {
	char tmp[BUF_SIZE] = "Command list:\n";
	for (uint8_t i = 0; i < N_COMMANDS; i++) {
		sprintf(tmp + strlen(tmp), "- %s\n", cli_commands[i]);
	}
	char *out = tmp;
	return out;
}

void cli_init(cli_t *cli, UART_HandleTypeDef *uart) {
	cli->uart = uart;
	cli->rx.complete = false;
	cli->rx.index = 0;
	cli->states[0] = &_cli_volts;
	cli->states[1] = &_cli_temps;
	cli->states[2] = &_cli_status;
	cli->states[3] = &_cli_taba;
	cli->states[4] = &_cli_help;

	LL_USART_EnableIT_RXNE(cli->uart->Instance);
	LL_USART_EnableIT_ERROR(cli->uart->Instance);
	LL_USART_EnableIT_TXE(cli->uart->Instance);

	char init[BUF_SIZE];
	sprintf(init,
			"\n\n********* Fenice BMS *********\n"
			" Build: %s @ %s\n\n type ? for commands\n\n"
			"> ",
			__DATE__, __TIME__);

	HAL_UART_Transmit(cli->uart, (uint8_t *)init, strlen(init), 100);
}

void cli_loop(cli_t *cli, state_global_data_t *data, BMS_STATE_T state) {
	if (cli->rx.complete) {
		cli->rx.complete = false;

		// char tmp[BUF_SIZE] = "?\n";
		char *tmp = "?\n";

		for (uint8_t i = 0; i < N_COMMANDS; i++) {
			if (strcmp(cli->rx.buffer, cli_commands[i]) == 0) {
				tmp = cli->states[i](data, state);
			}
		}

		for (uint8_t i = 0; i < BUF_SIZE; i++) {
			cli->rx.buffer[i] = '\0';
		}

		HAL_UART_Transmit(cli->uart, (uint8_t *)tmp, strlen(tmp), 100);

		HAL_UART_Transmit(cli->uart, (uint8_t *)"> ", 2, 100);
	}
}

// Interrupt callback
void cli_char_receive(cli_t *cli) {
	char rx_char = LL_USART_ReceiveData8(USART2);

	if (rx_char == '\r' || rx_char == '\n') {
		// Check new line
		cli->rx.complete = true;
		cli->rx.index = 0;
		return;
	} else if (rx_char == '\b' && cli->rx.index > 0) {
		// Check backspace
		cli->rx.buffer[--cli->rx.index] = '\0';
	} else {
		// Add to buffer
		cli->rx.buffer[cli->rx.index] = rx_char;
		cli->rx.index++;
	}

	/* Check if reception is completed (expected nb of bytes has been
	 * received)
	 */
	if (cli->rx.index == BUF_SIZE) {
		cli->rx.complete = true;
	}
}