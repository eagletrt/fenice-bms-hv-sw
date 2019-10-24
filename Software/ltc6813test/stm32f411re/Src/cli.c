#include "cli.h"
#include <stdio.h>
#include <string.h>

const char *cli_commands[N_COMMANDS] = {"volts", "temps", "status", "?"};

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
	char tmp[BUF_SIZE];
	sprintf(tmp, "State: %s\nError: %s\nError index: %i\n",
			bms_state_names[state], error_names[data->error],
			data->error_index);

	char *out = tmp;
	return out;
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
	cli->states[3] = &_cli_help;

	LL_USART_EnableIT_RXNE(cli->uart->Instance);
	LL_USART_EnableIT_ERROR(cli->uart->Instance);
	LL_USART_EnableIT_TXE(cli->uart->Instance);
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

		// if (strcmp(cli->rx.buffer, "?") == 0) {
		// 	tmp = "Command list:\n- ?\n";
		// } else if (strcmp(cli->rx.buffer, "volts") == 0) {
		// 	char t[BUF_SIZE];
		// 	sprintf(t, "Total: %.2f V\tMax: %.2f V\tMin: %.2f V\n",
		// 			(float)data->pack.total_voltage / 10000,
		// 			(float)data->pack.max_voltage / 10000,
		// 			(float)data->pack.min_voltage / 10000);
		// 	tmp = t;
		// } else {
		// 	tmp = "?\n";
		// }

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