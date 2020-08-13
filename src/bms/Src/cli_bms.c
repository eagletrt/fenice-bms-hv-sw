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

const char *error_names[ERROR_NUM_ERRORS] = {
	[ERROR_LTC_PEC_ERROR] = "PEC",
	[ERROR_CELL_UNDER_VOLTAGE] = "under-voltage",
	[ERROR_CELL_OVER_VOLTAGE] = "over-voltage",
	[ERROR_CELL_OVER_TEMPERATURE] = "over-temperature",
	[ERROR_OVER_CURRENT] = "over-current",
	[ERROR_CAN] = "CAN",
	[ERROR_ADC_INIT] = "adc init",
	[ERROR_ADC_TIMEOUT] = "adc timeout"};

char const *const feedback_names[FEEDBACK_N] = {
	[FEEDBACK_VREF_POS] = "VREF",
	[FEEDBACK_FROM_TSMS_POS] = "FROM TSMS",
	[FEEDBACK_TO_TSMS_POS] = "TO TSMS",
	[FEEDBACK_FROM_SHUTDOWN_POS] = "FROM SHUTDOWN",
	[FEEDBACK_LATCH_IMD_POS] = "LATCH IMD",
	[FEEDBACK_LATCH_BMS_POS] = "LATCH BMS",
	[FEEDBACK_IMD_FAULT_POS] = "IMD FAULT",
	[FEEDBACK_BMS_FAULT_POS] = "BMS FAULT",
	[FEEDBACK_TSAL_HV_POS] = "TSAL OVER 60V",
	[FEEDBACK_AIR_POSITIVE_POS] = "AIR POSITIVE",
	[FEEDBACK_AIR_NEGATIVE_POS] = "AIR NEGATIVE",
	[FEEDBACK_PC_END_POS] = "PRE-CHARGE END",
	[FEEDBACK_RELAY_LV_POS] = "RELAY LV",
	[FEEDBACK_IMD_SHUTDOWN_POS] = "IMD SHUTDOWN",
	[FEEDBACK_BMS_SHUTDOWN_POS] = "BMS SHUTDOWN",
	[FEEDBACK_TS_ON_POS] = "TS ON"};

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

	char *bal = bool_names[balancing.enable];

	char thresh[5] = {'\0'};
	itoa((float)balancing.threshold / 10, thresh, 10);

	char er_count[3] = {'\0'};
	itoa(error_count(), er_count, 10);

	// TODO: Fix this
	char *values[n_items][2] = {
		{"BMS state", ""}, {"error count", er_count}, {"balancing", bal}, {"balancing threshold", thresh}};
	//{"BMS state", (char *)fsm_bms.state_names[fsm_bms.current_state]}, {"error count", er_count}, {"balancing", bal}, {"balancing threshold", thresh}};

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
	uint16_t count = error_count();
	error_t errors[count];
	error_dump(errors);

	sprintf(out, "total %u\r\n", count);
	for (uint16_t i = 0; i < count; i++) {
		sprintf(out + strlen(out),
				"\r\ntype........%s\r\n"
				"timestamp...%lu (%lums ago)\r\n"
				"offset......%u\r\n"
				"state.......%u\r\n",
				error_names[errors[i].state], errors[i].timestamp, HAL_GetTick() - errors[i].timestamp, errors[i].offset, errors[i].state);
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