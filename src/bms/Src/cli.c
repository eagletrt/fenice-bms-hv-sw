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
#include "error/error.h"
#include "pack.h"

const char *cli_commands[N_COMMANDS] = {
	"volts", "volts all", "temps", "temps all", "status", "errors", "bal", "?", "\ta"};

void _cli_volts(char *cmd, state_global_data_t *data, BMS_STATE_T state,
				char *out) {
	sprintf(out,
			"bus.......%.2f V\r\nadc.......%.2f V\r\ntotal.....%.2f "
			"V\r\nmax.......%.3f V\r\nmin.......%.3f V"
			"\r\ndelta.....%.3f V\r\n",
			(float)data->pack.adc_voltage / 100,
			(float)data->pack.ext_voltage / 100,
			(float)data->pack.total_voltage / 10000,
			(float)data->pack.max_voltage / 10000,
			(float)data->pack.min_voltage / 10000,
			(float)(data->pack.max_voltage - data->pack.min_voltage) / 10000);
}

void _cli_volts_all(char *cmd, state_global_data_t *data, BMS_STATE_T state,
					char *out) {
	out[0] = '\0';

	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		if (i % LTC6813_CELL_COUNT == 0) {
			sprintf(out + strlen(out), "\r\n%-3d", i / LTC6813_CELL_COUNT);
		} else if (i % (LTC6813_CELL_COUNT / 2) == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}

		sprintf(out + strlen(out), "[%3u %-.3fv] ", i,
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

	uint8_t temps[PACK_TEMP_COUNT];
	pack_update_temperatures_all(data->hspi, temps);

	for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
		if (i % TEMP_SENSOR_COUNT == 0) {
			sprintf(out + strlen(out), "\r\n%-3d", i / TEMP_SENSOR_COUNT);
		} else if (i % (TEMP_SENSOR_COUNT / 2) == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}
		sprintf(out + strlen(out), "[%3u %2uc] ", i, temps[i]);
	}

	sprintf(out + strlen(out), "\r\n");
}
void _cli_status(char *cmd, state_global_data_t *data, BMS_STATE_T state,
				 char *out) {
#define n_items 4

	char *bal = data->balancing.enable ? "true" : "false";

	char thresh[5] = {'\0'};
	itoa((float)data->balancing.threshold / 10, thresh, 10);

	char er_count[3] = {'\0'};
	itoa(error_count(), er_count, 10);

	char *values[n_items][3] = {
		{"BMS state", (char *)bms_state_names[state]},
		{"error count", er_count},
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

void _cli_errors(char *cmd, state_global_data_t *data, BMS_STATE_T state, char *out) {
	uint8_t count = error_count();
	error_status_t errors[count];
	error_dump(errors);

	sprintf(out, "total %u\r\n", count);
	for (uint8_t i = 0; i < count; i++) {
		sprintf(out + strlen(out),
				"\r\ntype........%s\r\n"
				"timestamp...%lu (%lums ago)\r\n"
				"offset......%u\r\n"
				"active......%s\r\n"
				"fatal.......%s\r\n"
				"count.......%lu\r\n",
				error_names[errors[i].type], errors[i].time_stamp, HAL_GetTick() - errors[i].time_stamp, errors[i].offset, bool_names[errors[i].active], bool_names[errors[i].fatal], errors[i].count);
	}
}

void _cli_taba(char *cmd, state_global_data_t *data, BMS_STATE_T state,
			   char *out) {
	sprintf(out,
			" #######    #    ######     #    ######     #    ####### ####### ######  \r\n"
			"    #      # #   #     #   # #   #     #   # #      #    #       #     # \r\n"
			"    #     #   #  #     #  #   #  #     #  #   #     #    #       #     # \r\n"
			"    #    #     # ######  #     # ######  #     #    #    #####   #     # \r\n"
			"    #    ####### #     # ####### #   #   #######    #    #       #     # \r\n"
			"    #    #     # #     # #     # #    #  #     #    #    #       #     # \r\n"
			"    #    #     # ######  #     # #     # #     #    #    ####### ######  \r\n");
}

void _cli_help(char *cmd, state_global_data_t *data, BMS_STATE_T state,
			   char *out) {
	sprintf(out, "command list:\r\n");
	for (uint8_t i = 0; i < N_COMMANDS - 1; i++) {
		sprintf(out + strlen(out), "- %s\r\n", cli_commands[i]);
	}
}

void cli_buf_init(buffer_t *buf) {
	buf->index = 0;
}

void cli_init(cli_t *cli, UART_HandleTypeDef *uart) {
	cli->uart = uart;

	cli->input_buf = '\0';

	cli->history_index = 0;
	cli->complete = false;
	cli->receive = true;
	cli->escaping = BUF_SIZE;

	list_init(cli->buffers);

	buffer_t buf;
	cli_buf_init(&buf);

	list_insert(&cli->buffers, &buf, sizeof(buffer_t));

	cli_state_func_t *temp[N_COMMANDS] = {
		&_cli_volts, &_cli_volts_all, &_cli_temps, &_cli_temps_all,
		&_cli_status, &_cli_errors, &_cli_balance, &_cli_help, &_cli_taba};
	memcpy(cli->states, temp, sizeof(cli->states));

	char init[94];
	sprintf(init,
			"\r\n\n********* Fenice BMS *********\r\n"
			" build: %s @ %s\r\n\n type ? for commands\r\n\n",
			__DATE__, __TIME__);

	strcat(init, ps);

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
	buffer_t *cur_buffer = (buffer_t *)cli.buffers->data;
	uint8_t list_n = list_count(cli.buffers);

	if (cur_buffer->buffer[cur_buffer->index - 1] == '[') {
		cli.escaping = BUF_SIZE;
		uint8_t h_i;  // Index of history to be displayed

		if (cur_buffer->buffer[cur_buffer->index] == 'A' &&
			cli.history_index < list_n - 1) {  // UP

			h_i = cli.history_index + 1;
		} else if (cur_buffer->buffer[cur_buffer->index] == 'B' &&
				   cli.history_index > 1) {	 // DOWN

			h_i = cli.history_index - 1;
		} else {
			return;
		}

		buffer_t *hist = (buffer_t *)list_get_nth(cli.buffers, h_i);
		cli.history_index = h_i;

		memcpy(cur_buffer->buffer, hist->buffer, sizeof(char) * BUF_SIZE);

		char eraser[BUF_SIZE];
		sprintf(eraser, "%-*c\r", cur_buffer->index + PS_SIZE, '\r');

		strcat(eraser, ps);
		strcat(eraser, cur_buffer->buffer);

		HAL_UART_Transmit(cli.uart, (uint8_t *)eraser, strlen(eraser), 500);

		cur_buffer->index = hist->index;
	}
	return;
}

void cli_loop(state_global_data_t *data, BMS_STATE_T state) {
	buffer_t *cur_buffer = (buffer_t *)cli.buffers->data;

	if (cli.receive) {
		cli.receive = false;
		cli_char_receive();
	}

	if (cli.complete) {
		cli.complete = false;

		if (cli.escaping < BUF_SIZE) {
			cli_handle_escape(cli);
			return;
		}

		// Clean the buffer from backspaces
		cur_buffer->index = cli_clean(cur_buffer->buffer);

		// TODO: Make this better
		char tx_buf[3000] = "?\r\n";

		// Check which command corresponds with the buffer
		for (uint8_t i = 0; i < N_COMMANDS; i++) {
			if (strncmp(cur_buffer->buffer, cli_commands[i], strlen(cli_commands[i])) == 0) {
				cli.states[i](cur_buffer->buffer, data, state, tx_buf);
			}
		}

		buffer_t *last_buf = (buffer_t *)list_get_nth(cli.buffers, 1);

		// Check if last history entry is equal to the current
		bool comp = last_buf == NULL || strcmp(cur_buffer->buffer, last_buf->buffer) != 0;
		if (cur_buffer->index > 0 && comp) {
			// If the last command wasn't empty, we save it to history.
			// To do that we simply add an empty buffer before it so that it will not be overwritten by the next command

			buffer_t tmp_buf;
			cli_buf_init(&tmp_buf);

			list_insert(&cli.buffers, &tmp_buf, sizeof(buffer_t));
		} else {
			// We need to reuse the last buffer
			cur_buffer->index = 0;
			cur_buffer->buffer[0] = '\0';
		}

		cli.history_index = 0;

		// cur_buffer might not be current anymore
		((buffer_t *)(cli.buffers->data))->buffer[0] = '\0';

		HAL_UART_Transmit(cli.uart, (uint8_t *)tx_buf, strlen(tx_buf), 200);
		HAL_UART_Transmit(cli.uart, (uint8_t *)ps, PS_SIZE, 100);
	}
}

void cli_handle_interrupt() {
	cli.receive = true;
}

// Interrupt callback
void cli_char_receive() {
	HAL_UART_Receive_IT(cli.uart, (uint8_t *)&cli.input_buf, 1);

	if (cli.input_buf != '\0') {
		buffer_t *cur_buffer = (buffer_t *)cli.buffers->data;

		if (cli.input_buf == '\033') {	// Arrow
			cli.escaping = cur_buffer->index;
		} else if (cli.input_buf == '\r' || cli.input_buf == '\n') {
			cli.complete = true;
			cur_buffer->buffer[cur_buffer->index] = '\0';

			HAL_UART_Transmit(cli.uart, (uint8_t *)"\r\n", 2, 10);

			return;
		}

		cur_buffer->buffer[cur_buffer->index] = cli.input_buf;

		if (cli.escaping < BUF_SIZE && cur_buffer->index > cli.escaping + 1) {
			cli.complete = true;
			return;
		}
		cur_buffer->index++;

		// Echo the last char
		if (cli.escaping == BUF_SIZE && CLI_ECHO) {
			HAL_UART_Transmit(cli.uart, (uint8_t *)&cli.input_buf, 1, 10);
		}

		if (cur_buffer->index == BUF_SIZE) {
			cli.complete = true;
		}
	}
}