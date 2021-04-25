/**
 * @file		cli_bms.c
 * @brief		cli instance for bms	
 *
 * @date		Mar 29,2020
 * 
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#include "cli_bms.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bal.h"
#include "error/error.h"
#include "fsm_bms.h"
#include "usart.h"

#define N_COMMANDS 8

cli_command_func_t _cli_volts;
cli_command_func_t _cli_volts_all;
cli_command_func_t _cli_temps;
cli_command_func_t _cli_temps_all;
cli_command_func_t _cli_status;
cli_command_func_t _cli_balance;
cli_command_func_t _cli_errors;
cli_command_func_t _cli_dmesg;
cli_command_func_t _cli_help;
cli_command_func_t _cli_taba;

const char *state_names[BMS_NUM_STATES] = {
	[BMS_INIT] = "init",
	[BMS_SET_TS_OFF] = "ts off",
	[BMS_IDLE] = "idle",
	[BMS_PRECHARGE_START] = "precharge start",
	[BMS_PRECHARGE] = "precharge",
	[BMS_PRECHARGE_END] = "precharge end",
	[BMS_RUN] = "run",
	[BMS_CHARGE] = "charge",
	[BMS_TO_HALT] = "to halt",
	[BMS_HALT] = "halt"};

const char *error_names[ERROR_NUM_ERRORS] = {
	[ERROR_LTC_PEC_ERROR] = "LTC PEC mismatch",
	[ERROR_CELL_UNDER_VOLTAGE] = "under-voltage",
	[ERROR_CELL_OVER_VOLTAGE] = "over-voltage",
	[ERROR_CELL_OVER_TEMPERATURE] = "over-temperature",
	[ERROR_OVER_CURRENT] = "over-current",
	[ERROR_CAN] = "CAN",
	[ERROR_ADC_INIT] = "adc init",
	[ERROR_ADC_TIMEOUT] = "adc timeout",
	[ERROR_INT_VOLTAGE_MISMATCH] = "internal voltage mismatch",
	[ERROR_FEEDBACK_HARD] = "hard feedback",
	[ERROR_FEEDBACK_SOFT] = "soft feedback"};

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
	"volt", "temp", "status", "errors", "bal", "dmesg", "?", "\ta"};

cli_command_func_t *commands[N_COMMANDS] = {
	&_cli_volts, &_cli_temps, &_cli_status, &_cli_errors, &_cli_balance, &_cli_dmesg, &_cli_help, &_cli_taba};

cli_t cli_bms;
bool dmesg_ena = false;

void cli_bms_init() {
	cli_bms.uart = &huart2;
	cli_bms.cmds.functions = commands;
	cli_bms.cmds.names = command_names;
	cli_bms.cmds.count = N_COMMANDS;

	char init[94];
	sprintf(init,
			"\r\n\n********* Fenice BMS *********\r\n"
			" build: %s @ %s\r\n\n type ? for commands\r\n\n",
			__DATE__, __TIME__);

	strcat(init, cli_ps);

	cli_init(&cli_bms);
	cli_print(&cli_bms, init, strlen(init));
}

void cli_bms_debug(char *text, size_t length) {
	if (dmesg_ena) {
		char out[3000] = {'\0'};
		// add prefix
		sprintf(out, "[%014lu] ", HAL_GetTick());

		strcat(out, text);
		length += strlen(out);

		// add suffix
		sprintf(out + length, "\r\n");
		length += 2;

		cli_print(&cli_bms, out, length);
	}
}

void _cli_volts(uint16_t argc, char **argv, char *out) {
	if (strcmp(argv[1], "all") == 0) {
		_cli_volts_all(argc, &argv[1], out);
	} else {
		voltage_t max = pack_get_max_voltage();
		voltage_t min = pack_get_min_voltage();
		sprintf(out,
				"bus.......%.2f V\r\n"
				"internal..%.2f V\r\n"
				"average...%.2f V\r\n"
				"max.......%.3f V\r\n"
				"min.......%.3f V\r\n"
				"delta.....%.3f V\r\n",
				(float)pack_get_bus_voltage() / 100,
				(float)pack_get_int_voltage() / 100,
				(float)pack_get_int_voltage() / PACK_CELL_COUNT / 100,
				(float)max / 10000,
				(float)min / 10000,
				(float)(max - min) / 10000);
	}
}

void _cli_volts_all(uint16_t argc, char **argv, char *out) {
	out[0] = '\0';

	for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
		if (i % LTC6813_CELL_COUNT == 0) {
			sprintf(out + strlen(out), "\r\n%-3d", i / LTC6813_CELL_COUNT);
		} else if (i % (LTC6813_CELL_COUNT / 2) == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}

		sprintf(out + strlen(out), "[%3u %-.3fv] ", i,
				(float)pack_get_voltages()[i] / 10000);
	}

	sprintf(out + strlen(out), "\r\n");
}

void _cli_temps(uint16_t argc, char **argv, char *out) {
	if (strcmp(argv[1], "all") == 0) {
		_cli_temps_all(argc, &argv[1], out);
	} else {
		sprintf(out,
				"average.....%.1f C\r\nmax.........%2u "
				"C\r\nmin.........%2u C\r\n"
				"delta.......%2u C\r\n",
				(float)pack_get_mean_temperature() / 10, pack_get_max_temperature(),
				pack_get_min_temperature(),
				pack_get_max_temperature() - pack_get_min_temperature());
	}
}

void _cli_temps_all(uint16_t argc, char **argv, char *out) {
	out[0] = '\0';

	for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
		if (i % TEMP_SENSOR_COUNT == 0) {
			sprintf(out + strlen(out), "\r\n%-3d", i / TEMP_SENSOR_COUNT);
		} else if (i % (TEMP_SENSOR_COUNT / 2) == 0 && i > 0) {
			sprintf(out + strlen(out), "\r\n%-3s", "");
		}
		sprintf(out + strlen(out), "[%3u %2uc] ", i, pack_get_temperatures()[i]);
	}

	sprintf(out + strlen(out), "\r\n");
}

void _cli_status(uint16_t argc, char **argv, char *out) {
#define n_items 4

	bal_handle balancing = pack_get_balancing();

	const char *bal = bool_names[balancing.enable];

	char thresh[5] = {'\0'};
	itoa((float)balancing.threshold / 10, thresh, 10);

	char er_count[3] = {'\0'};
	itoa(error_count(), er_count, 10);

	// TODO: Fix this
	const char *values[n_items][2] = {
		{"BMS state", state_names[fsm_bms.current_state]}, {"error count", er_count}, {"balancing", bal}, {"balancing threshold", thresh}};
	//{"BMS state", (char *)fsm_bms.state_names[fsm_bms.current_state]}, {"error count", er_count}, {"balancing", bal}, {"balancing threshold", thresh}};

	out[0] = '\0';
	for (uint8_t i = 0; i < n_items; i++) {
		sprintf(out + strlen(out), "%s%s%s\r\n", values[i][0],
				"........................" + strlen(values[i][0]),
				values[i][1]);
	}
}

void _cli_balance(uint16_t argc, char **argv, char *out) {
	bal_handle balancing = pack_get_balancing();
	if (strcmp(argv[1], "tog") == 0) {
		balancing.enable = !balancing.enable;

		sprintf(out, "setting balancing to %u\r\n", balancing.enable);
	} else if (strcmp(argv[1], "thr") == 0) {
		balancing.threshold = atoi(argv[2]) * 10;

		sprintf(out, "setting balancing threshold to %u mV\r\n",
				balancing.threshold / 10);
	} else {
		sprintf(out, "Unknown parameter: %s\r\n", argv[1]);
	}
}

void _cli_errors(uint16_t argc, char **argv, char *out) {
	uint16_t count = error_count();
	error_t errors[count];
	error_dump(errors);

	uint32_t now = HAL_GetTick();
	sprintf(out, "total %u\r\n", count);
	for (uint16_t i = 0; i < count; i++) {
		sprintf(out + strlen(out),
				"\r\nid..........%i (%s)\r\n"
				"timestamp...T+%lu (%lums ago)\r\n"
				"offset......%u\r\n"
				"state.......%u\r\n",
				errors[i].id, error_names[errors[i].id], errors[i].timestamp, now - errors[i].timestamp, errors[i].offset, errors[i].state);
	}
}

void _cli_dmesg(uint16_t argc, char **argv, char *out) {
	dmesg_ena = !dmesg_ena;
	sprintf(out, "dmesg output is %s\r\n", dmesg_ena ? "enabled" : "disabled");
}

void _cli_taba(uint16_t argc, char **argv, char *out) {
	sprintf(out,
			" #######    #    ######     #    ######     #    ####### ####### ######  \r\n"
			"    #      # #   #     #   # #   #     #   # #      #    #       #     # \r\n"
			"    #     #   #  #     #  #   #  #     #  #   #     #    #       #     # \r\n"
			"    #    #     # ######  #     # ######  #     #    #    #####   #     # \r\n"
			"    #    ####### #     # ####### #   #   #######    #    #       #     # \r\n"
			"    #    #     # #     # #     # #    #  #     #    #    #       #     # \r\n"
			"    #    #     # ######  #     # #     # #     #    #    ####### ######  \r\n");
}

void _cli_help(uint16_t argc, char **argv, char *out) {
	sprintf(out, "command list:\r\n");
	for (uint8_t i = 0; i < N_COMMANDS - 1; i++) {
		sprintf(out + strlen(out), "- %s\r\n", cli_bms.cmds.names[i]);
	}
}