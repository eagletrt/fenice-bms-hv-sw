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

#include "bal_fsm.h"
#include "bms_fsm.h"
#include "energy.h"
#include "error/error.h"
#include "feedback.h"
#include "pack.h"
#include "soc.h"
#include "usart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: don't count manually
#define N_COMMANDS 12

cli_command_func_t _cli_volts;
cli_command_func_t _cli_volts_all;
cli_command_func_t _cli_temps;
cli_command_func_t _cli_temps_all;
cli_command_func_t _cli_status;
cli_command_func_t _cli_balance;
cli_command_func_t _cli_soc;
cli_command_func_t _cli_errors;
cli_command_func_t _cli_ts;
cli_command_func_t _cli_current;
cli_command_func_t _cli_dmesg;
cli_command_func_t _cli_reset;
cli_command_func_t _cli_help;
cli_command_func_t _cli_taba;

const char *bms_state_names[BMS_NUM_STATES] =
    {[BMS_IDLE] = "idle", [BMS_PRECHARGE] = "precharge", [BMS_ON] = "run", [BMS_HALT] = "halt"};

const char *bal_state_names[BAL_NUM_STATES] =
    {[BAL_OFF] = "off", [BAL_COMPUTE] = "computing", [BAL_DISCHARGE] = "discharging", [BAL_COOLDOWN] = "cooldown"};

const char *error_names[ERROR_NUM_ERRORS] = {
    [ERROR_CELL_UNDER_VOLTAGE]    = "under-voltage",
    [ERROR_CELL_OVER_VOLTAGE]     = "over-voltage",
    [ERROR_CELL_OVER_TEMPERATURE] = "over-temperature",
    [ERROR_OVER_CURRENT]          = "over-current",
    [ERROR_CAN]                   = "CAN",
    [ERROR_ADC_INIT]              = "ADC init",
    [ERROR_ADC_TIMEOUT]           = "ADC timeout",
    [ERROR_INT_VOLTAGE_MISMATCH]  = "internal voltage mismatch",
    [ERROR_CELLBOARD_COMM]        = "cellboard communication",
    [ERROR_CELLBOARD_INTERNAL]    = "cellboard internal",
    [ERROR_FEEDBACK]              = "feedback",
    [ERROR_EEPROM_COMM]           = "EEPROM communication"};

char const *const feedback_names[FEEDBACK_N] = {
    [FEEDBACK_VREF_POS]          = "VREF",
    [FEEDBACK_FROM_TSMS_POS]     = "FROM TSMS",
    [FEEDBACK_TO_TSMS_POS]       = "TO TSMS",
    [FEEDBACK_FROM_SHUTDOWN_POS] = "FROM SHUTDOWN",
    [FEEDBACK_LATCH_IMD_POS]     = "LATCH IMD",
    [FEEDBACK_LATCH_BMS_POS]     = "LATCH BMS",
    [FEEDBACK_IMD_FAULT_POS]     = "IMD FAULT",
    [FEEDBACK_BMS_FAULT_POS]     = "BMS FAULT",
    [FEEDBACK_TSAL_HV_POS]       = "TSAL OVER 60V",
    [FEEDBACK_AIR_POSITIVE_POS]  = "AIR POSITIVE",
    [FEEDBACK_AIR_NEGATIVE_POS]  = "AIR NEGATIVE",
    [FEEDBACK_PC_END_POS]        = "PRE-CHARGE END",
    [FEEDBACK_RELAY_LV_POS]      = "RELAY LV",
    [FEEDBACK_IMD_SHUTDOWN_POS]  = "IMD SHUTDOWN",
    [FEEDBACK_BMS_SHUTDOWN_POS]  = "BMS SHUTDOWN",
    [FEEDBACK_TS_ON_POS]         = "TS ON"};

char *command_names[N_COMMANDS] =
    {"volt", "temp", "status", "errors", "ts", "bal", "soc", "current", "dmesg", "reset", "?", "\ta"};

cli_command_func_t *commands[N_COMMANDS] = {
    &_cli_volts,
    &_cli_temps,
    &_cli_status,
    &_cli_errors,
    &_cli_ts,
    &_cli_balance,
    &_cli_soc,
    &_cli_current,
    &_cli_dmesg,
    &_cli_reset,
    &_cli_help,
    &_cli_taba};

cli_t cli_bms;
bool dmesg_ena = true;

void cli_bms_init() {
    cli_bms.uart           = &CLI_UART;
    cli_bms.cmds.functions = commands;
    cli_bms.cmds.names     = command_names;
    cli_bms.cmds.count     = N_COMMANDS;

    char init[94];
    sprintf(
        init,
        "\r\n\n********* Fenice BMS *********\r\n"
        " build: %s @ %s\r\n\n type ? for commands\r\n\n",
        __DATE__,
        __TIME__);

    strcat(init, cli_ps);

    cli_init(&cli_bms);
    cli_print(&cli_bms, init, strlen(init));
}

void cli_bms_debug(char *text, size_t length) {
    if (dmesg_ena) {
        char out[3000] = {'\0'};
        // add prefix
        sprintf(out, "[%.3f] ", (float)HAL_GetTick() / 1000);

        strcat(out, text);
        length += strlen(out);

        // add suffix
        sprintf(out + length, "\r\n");
        length += 2;

        cli_print(&cli_bms, out, length);
    }
}

void _cli_volts(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "") == 0) {
        voltage_t max = pack_get_max_voltage();
        voltage_t min = pack_get_min_voltage();
        sprintf(
            out,
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
    } else if (strcmp(argv[1], "all") == 0) {
        _cli_volts_all(argc, &argv[1], out);
    } else {
        sprintf(
            out,
            "Unknown parameter: %s\r\n"
            "valid parameters:\r\n"
            "- all: returns voltages for all cells\r\n",
            argv[1]);
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

        sprintf(out + strlen(out), "[%3u %-.3fv] ", i, (float)pack_get_voltages()[i] / 10000);
    }

    sprintf(out + strlen(out), "\r\n");
}

void _cli_temps(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "") == 0) {
        sprintf(
            out,
            "average.....%.1f C\r\nmax.........%2u "
            "C\r\nmin.........%2u C\r\n"
            "delta.......%2u C\r\n",
            (float)pack_get_mean_temperature() / 10,
            pack_get_max_temperature(),
            pack_get_min_temperature(),
            pack_get_max_temperature() - pack_get_min_temperature());
    } else if (strcmp(argv[1], "all") == 0) {
        _cli_temps_all(argc, &argv[1], out);
    } else {
        sprintf(
            out,
            "Unknown parameter: %s\r\n"
            "valid parameters:\r\n"
            "- all: returns temperature for all cells\r\n",
            argv[1]);
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
#define n_items 3

    char thresh[5] = {'\0'};
    itoa((float)bal_get_threshold() / 10, thresh, 10);

    char er_count[3] = {'\0'};
    itoa(error_count(), er_count, 10);

    // TODO: Fix this
    const char *values[n_items][2] = {
        {"BMS state", bms_state_names[fsm_get_state(bms.fsm)]},
        {"error count", er_count},
        {"balancing state", bal_state_names[fsm_get_state(bal.fsm)]}};
    //{"BMS state", (char *)fsm_bms.state_names[fsm_bms.current_state]}, {"error
    // count", er_count}, {"balancing", bal}, {"balancing threshold", thresh}};

    out[0] = '\0';
    for (uint8_t i = 0; i < n_items; i++) {
        sprintf(
            out + strlen(out),
            "%s%s%s\r\n",
            values[i][0],
            "........................" + strlen(values[i][0]),
            values[i][1]);
    }
}

void _cli_balance(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "on") == 0) {
        fsm_trigger_event(bal.fsm, EV_BAL_START);
        sprintf(out, "enabling balancing\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        fsm_trigger_event(bal.fsm, EV_BAL_STOP);
        sprintf(out, "disabling balancing\r\n");
    } else if (strcmp(argv[1], "thr") == 0) {
        if (argv[2] != NULL) {
            voltage_t thresh = atoi(argv[2]) * 10;
            if (thresh <= BAL_MAX_VOLTAGE_THRESHOLD) {
                bal_set_threshold(thresh);
            }
        }
        sprintf(out, "balancing threshold is %u mV\r\n", bal_get_threshold() / 10);
    } else {
        sprintf(
            out,
            "Unknown parameter: %s\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n"
            "- thr <millivolts>\r\n",
            argv[1]);
    }
}
void _cli_soc(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "reset") == 0) {
        energy_reset_soc();
        sprintf(out, "Resetting energy meter\r\n");
    } else {
        sprintf(
            out,
            "SoC: %.2f %%\r\n"
            "Energy: %.1f Wh\r\n"
            "Energy total: %.1f Wh\r\n",
            energy_get_soc(),
            energy_get_energy_last_charge(),
            energy_get_energy_total());
    }
}

void _cli_errors(uint16_t argc, char **argv, char *out) {
    uint16_t count = error_count();
    error_t errors[count];
    error_dump(errors);

    uint32_t now = HAL_GetTick();
    sprintf(out, "total %u\r\n", count);
    for (uint16_t i = 0; i < count; i++) {
        sprintf(
            out + strlen(out),
            "\r\nid..........%i (%s)\r\n"
            "timestamp...T+%lu (%lums ago)\r\n"
            "offset......%u\r\n"
            "state.......%s\r\n",
            errors[i].id,
            error_names[errors[i].id],
            errors[i].timestamp,
            now - errors[i].timestamp,
            errors[i].offset,
            errors[i].state == STATE_WARNING ? "warning" : "fatal");
    }
}

void _cli_ts(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "on") == 0) {
        fsm_trigger_event(bms.fsm, BMS_EV_TS_ON);
        sprintf(out, "triggered TS ON event\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
        sprintf(out, "triggered TS OFF event\r\n");
    } else {
        sprintf(
            out,
            "Unknown parameter: %s\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n",
            argv[1]);
    }
}

void _cli_current(uint16_t argc, char **argv, char *out) {
    if (argc == 0) {
        sprintf(
            out,
            "Hall 50A:\t%.1fA\r\n"
            "Hall 300A:\t%.1fA\r\n"
            "Shunt:\t\t%.1fA\r\n",
            current_get_current_from_sensor(CURRENT_SENSOR_50),
            current_get_current_from_sensor(CURRENT_SENSOR_300),
            current_get_current_from_sensor(CURRENT_SENSOR_SHUNT));
    } else if (strcmp(argv[1], "zero") == 0) {
        current_zero();
        sprintf(out, "Current zeroed\r\n");
    } else {
        sprintf(
            out,
            "Unknown parameter: %s\r\n\n"
            "valid parameters:\r\n"
            "- zero: zeroes the hall-sensor measurement\r\n",
            argv[1]);
    }
}

void _cli_dmesg(uint16_t argc, char **argv, char *out) {
    dmesg_ena = !dmesg_ena;
    sprintf(out, "dmesg output is %s\r\n", dmesg_ena ? "enabled" : "disabled");
}

void _cli_reset(uint16_t argc, char **argv, char *out) {
    HAL_NVIC_SystemReset();
}

void _cli_taba(uint16_t argc, char **argv, char *out) {
    sprintf(
        out,
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