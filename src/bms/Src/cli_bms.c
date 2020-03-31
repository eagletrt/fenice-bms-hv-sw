/**
 * @file		cli_bms.c
 * @brief		cli instance for bms	
 *
 * @date		Mar 29,2020
 * 
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "cli_bms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bal.h"
#include "error/error.h"
#include "fsm_bms.h"
#include "pack_data.h"
#include "usart.h"

#define N_COMMANDS 9

void _cli_volts(char *cmd, char *out);
void _cli_volts_all(char *cmd, char *out);
void _cli_temps(char *cmd, char *out);
void _cli_temps_all(char *cmd, char *out);
void _cli_status(char *cmd, char *out);
void _cli_balance(char *cmd, char *out);
void _cli_errors(char *cmd, char *out);
void _cli_taba(char *cmd, char *out);
void _cli_help(char *cmd, char *out);

char *command_names[N_COMMANDS] = {
	"volts", "volts all", "temps", "temps all", "status", "errors", "bal", "?", "\ta"};

cli_command_func_t *commands[N_COMMANDS] = {
	&_cli_volts, &_cli_volts_all, &_cli_temps, &_cli_temps_all,
	&_cli_status, &_cli_errors, &_cli_balance, &_cli_help, &_cli_taba};

cli_t cli_bms;

void cli_bms_init() {
	cli_bms.uart = &huart2;
	cli_bms.commands.functions = commands;
	cli_bms.commands.names = command_names;
	cli_bms.commands.count = N_COMMANDS;

	char init[94];
	sprintf(init,
			"\r\n\n********* Fenice BMS *********\r\n"
			" build: %s @ %s\r\n\n type ? for commands\r\n\n",
			__DATE__, __TIME__);

	strcat(init, cli_ps);

	cli_init(&cli_bms);
	cli_print(&cli_bms, init, strlen(init));
}

void _cli_volts(char *cmd, char *out) {
	sprintf(out,
			"bus.......%.2f V\r\nadc.......%.2f V\r\ntotal.....%.2f "
			"V\r\nmax.......%.3f V\r\nmin.......%.3f V"
			"\r\ndelta.....%.3f V\r\n",
			(float)pd_get_bus_voltage() / 100,
			(float)pd_get_adc_voltage() / 100,
			(float)pd_get_total_voltage() / 10000,
			(float)pd_get_max_voltage() / 10000,
			(float)pd_get_min_voltage() / 10000,
			(float)(pd_get_max_voltage() - pd_get_min_voltage()) / 10000);
}

void _cli_volts_all(char *cmd, char *out) {
	out[0] = '\0';

	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		if (i % LTC6813_CELL_COUNT == 0) {
			sprintf(out + strlen(out), "\r\n%-3d", i / LTC6813_CELL_COUNT);
		} else if (i % (LTC6813_CELL_COUNT / 2) == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}

		sprintf(out + strlen(out), "[%3u %-.3fv] ", i,
				(float)pd_get_voltage(i) / 10000);
	}

	sprintf(out + strlen(out), "\r\n");
}

void _cli_temps(char *cmd, char *out) {
	sprintf(out,
			"average.....%.1f C\r\nmax.........%2u "
			"C\r\nmin.........%2u C\r\n"
			"delta.......%2u C\r\n",
			(float)pd_get_avg_temperature() / 10, pd_get_max_temperature(),
			pd_get_min_temperature(),
			pd_get_max_temperature() - pd_get_min_temperature());
}

void _cli_temps_all(char *cmd, char *out) {
	out[0] = '\0';

	// TODO: Update temperatures in main cycle
	//uint8_t temps[PACK_TEMP_COUNT];
	//pack_update_temperatures_all(data->hspi, temps);

	for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
		if (i % TEMP_SENSOR_COUNT == 0) {
			sprintf(out + strlen(out), "\r\n%-3d", i / TEMP_SENSOR_COUNT);
		} else if (i % (TEMP_SENSOR_COUNT / 2) == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}
		sprintf(out + strlen(out), "[%3u %2uc] ", i, pd_get_temperature(i));
	}

	sprintf(out + strlen(out), "\r\n");
}

void _cli_status(char *cmd, char *out) {
#define n_items 4

	char *bal = balancing.enable ? "true" : "false";

	char thresh[5] = {'\0'};
	itoa((float)balancing.threshold / 10, thresh, 10);

	char er_count[3] = {'\0'};
	itoa(error_count(), er_count, 10);

	char *values[n_items][3] = {
		{"BMS state", (char *)fsm_bms.state_names[fsm_bms.current_state]}, {"error count", er_count}, {"balancing", bal}, {"balancing threshold", thresh}};

	out[0] = '\0';
	for (uint8_t i = 0; i < n_items; i++) {
		sprintf(out + strlen(out), "%s%s%s\r\n", values[i][0],
				"........................" + strlen(values[i][0]),
				values[i][1]);
	}
}

void _cli_balance(char *cmd, char *out) {
	uint8_t cmd_len = strlen("bal");

	if (strcmp(cmd + cmd_len, " tog") == 0) {
		balancing.enable = !balancing.enable;

		sprintf(out, "setting balancing to %u\r\n", balancing.enable);
	} else if (strncmp(cmd + cmd_len, " thr", 4) == 0) {
		balancing.threshold = atoi(cmd + cmd_len + 5) * 10;

		sprintf(out, "setting balancing threshold to %u mV\r\n",
				balancing.threshold / 10);
	}
}

void _cli_errors(char *cmd, char *out) {
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

void _cli_taba(char *cmd, char *out) {
	sprintf(out,
			" #######    #    ######     #    ######     #    ####### ####### ######  \r\n"
			"    #      # #   #     #   # #   #     #   # #      #    #       #     # \r\n"
			"    #     #   #  #     #  #   #  #     #  #   #     #    #       #     # \r\n"
			"    #    #     # ######  #     # ######  #     #    #    #####   #     # \r\n"
			"    #    ####### #     # ####### #   #   #######    #    #       #     # \r\n"
			"    #    #     # #     # #     # #    #  #     #    #    #       #     # \r\n"
			"    #    #     # ######  #     # #     # #     #    #    ####### ######  \r\n");
}

void _cli_help(char *cmd, char *out) {
	sprintf(out, "command list:\r\n");
	for (uint8_t i = 0; i < N_COMMANDS - 1; i++) {
		sprintf(out + strlen(out), "- %s\r\n", cli_bms.commands.names[i]);
	}
}