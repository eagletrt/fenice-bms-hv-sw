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
#include "error/error.h"
#include "feedback.h"
#include "pack/pack.h"
#include "pack/temperature.h"
#include "pack/voltage.h"
#include "soc.h"
#include "usart.h"
#include "can.h"
#include "can_comm.h"
#include "imd.h"
#include "fans_buzzer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: don't count manually
#define N_COMMANDS 16

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
cli_command_func_t _cli_imd;
cli_command_func_t _cli_can_forward;
cli_command_func_t _cli_feedbacks;
cli_command_func_t _cli_help;
cli_command_func_t _cli_taba;
cli_command_func_t _cli_sborat;

const char *bms_state_names[BMS_NUM_STATES] =
    {[BMS_IDLE] = "idle", [BMS_PRECHARGE] = "precharge", [BMS_ON] = "run", [BMS_FAULT] = "fault"};

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
    [ERROR_FEEDBACK_CIRCUITRY]    = "feedback_circuitry",
    [ERROR_EEPROM_COMM]           = "EEPROM communication",
    [ERROR_EEPROM_WRITE]          = "EEPROM write"};

char const *const feedback_names[FEEDBACK_N] = {
    [FEEDBACK_TSAL_GREEN_FAULT_POS]=            "FEEDBACK_TSAL_GREEN_FAULT",
    [FEEDBACK_IMD_LATCHED_POS]=                 "FEEDBACK_IMD_LATCHED",
    [FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS]=    "FEEDBACK_TSAL_GREEN_FAULT_LATCHED",
    [FEEDBACK_BMD_LATCHED_POS]=                 "FEEDBACK_BMD_LATCHED",
    [FEEDBACK_EXT_LATCHED_POS]=                 "FEEDBACK_EXT_LATCHED",
    [FEEDBACK_TSAL_GREEN_POS]=                  "FEEDBACK_TSAL_GREEN",
    [FEEDBACK_TS_OVER_60V_STATUS_POS]=          "FEEDBACK_TS_OVER_60V_STATUS",
    [FEEDBACK_AIRN_STATUS_POS]=                 "FEEDBACK_AIRN_STATUS",
    [FEEDBACK_AIRP_STATUS_POS]=                 "FEEDBACK_AIRP_STATUS",
    [FEEDBACK_AIRP_GATE_POS]=                   "FEEDBACK_AIRP_GATE",
    [FEEDBACK_AIRN_GATE_POS]=                   "FEEDBACK_AIRN_GATE",
    [FEEDBACK_PRECHARGE_STATUS_POS]=            "FEEDBACK_PRECHARGE_STATUS",
    [FEEDBACK_TSP_OVER_60V_STATUS_POS]=         "FEEDBACK_TSP_OVER_60V_STATUS",
    [FEEDBACK_CHECK_MUX_POS]=                   "FEEDBACK_CHECK_MUX",
    [FEEDBACK_SD_IN_POS]=                       "FEEDBACK_SD_IN",
    [FEEDBACK_SD_OUT_POS]=                      "FEEDBACK_SD_OUT",
    [FEEDBACK_RELAY_SD_POS]=                    "FEEDBACK_RELAY_SD",
    [FEEDBACK_IMD_FAULT_POS]=                   "FEEDBACK_IMD_FAULT",
    [FEEDBACK_SD_END_POS]=                      "FEEDBACK_SD_END"
};

char *command_names[N_COMMANDS] =
    {"volt", "temp", "status", "errors", "ts", "bal", "soc", "current", "dmesg", "reset", "imd", "can_forward", "feedbacks", "?", "\ta", "sbor@"};

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
    &_cli_imd,
    &_cli_can_forward,
    &_cli_feedbacks,
    &_cli_help,
    &_cli_taba,
    &_cli_sborat};

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
        char out[300] = {'\0'};
        float tick = (float)HAL_GetTick() / 1000;
        // add prefix
        sprintf(out, "[%.2f] ", tick);

        strcat(out, text);
        length += strlen(out);

        // add suffix
        sprintf(out + length, "\r\n> ");
        length += 4;

        cli_print(&cli_bms, out, length);
    }
}

void _cli_volts(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "") == 0) {
        voltage_t max = voltage_get_cell_max();
        voltage_t min = voltage_get_cell_min();
        sprintf(
            out,
            "bus.......%.2f V\r\n"
            "internal..%.2f V\r\n"
            "average...%.2f V\r\n"
            "max.......%.3f V\r\n"
            "min.......%.3f V\r\n"
            "delta.....%.3f V\r\n",
            (float)voltage_get_bus() / 100,
            (float)voltage_get_internal() / 100,
            (float)voltage_get_internal() / PACK_CELL_COUNT / 100,
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
    voltage_t *cells = voltage_get_cells();
    for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
        if (i % LTC6813_CELL_COUNT == 0) {
            sprintf(out + strlen(out), "\r\n%-3d", i / LTC6813_CELL_COUNT);
        } else if (i % (LTC6813_CELL_COUNT / 2) == 0 && i > 0) {
            sprintf(out + strlen(out), "\r\n%-3s", "");
        }

        sprintf(out + strlen(out), "[%3u %-.3fv] ", i, (float)cells[i] / 10000);
    }

    sprintf(out + strlen(out), "\r\n");
}

void _cli_temps(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "") == 0) {
        sprintf(
            out,
            "average.....%.2f °C\r\nmax.........%.2f "
            "C\r\nmin.........%.2f °C\r\n"
            "delta.......%.2f °C\r\n",
            (float)temperature_get_average() / 10,
            temperature_get_max() / 4.0f,
            temperature_get_min() / 4.0f,
            (temperature_get_max() - temperature_get_min()) / 4.0f);
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
    temperature_t *temp_all = temperature_get_all();

    for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
        if (i % TEMP_SENSOR_COUNT == 0) {
            sprintf(out + strlen(out), "\r\n%-3d", i / TEMP_SENSOR_COUNT);
        } else if (i % (TEMP_SENSOR_COUNT / 4) == 0 && i > 0) {
            sprintf(out + strlen(out), "\r\n%-3s", "");
        }
        sprintf(out + strlen(out), "[%3u %2u °C] ", i, temp_all[i] / 4);
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
    } else if(strcmp(argv[1], "test") == 0) {
        CAN_TxHeaderTypeDef tx_header;
        uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];
        uint8_t board;
        bms_balancing_cells cells = bms_balancing_cells_default;
        
        tx_header.ExtId = 0;
        tx_header.IDE   = CAN_ID_STD;
        tx_header.RTR   = CAN_RTR_DATA;
        tx_header.StdId = ID_BALANCING;

        board = atoi(argv[2]);

        if(argc < 4) {
            sprintf(out, "wrong number of parameters\r\n");
            return;
        }

        sprintf(out, "testing cells [");

        uint8_t c;
        for(uint8_t i=3; i<argc; ++i) {
            c = atoi(argv[i]);   
            setBit(cells, c, 1);
            sprintf(out + strlen(out), "%u,", c);
        }

        tx_header.DLC = serialize_bms_BALANCING(buffer, board, cells);
        can_send(&BMS_CAN, buffer, &tx_header);

        sprintf(out + strlen(out) - 1, "]\r\non board %d\r\n", board);
    } else if(argc < 2) {
        sprintf(
            out,
            "Invalid number of parameters.\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n"
            "- thr <millivolts>\r\n"
            "- test <board> <cell0 cell1 ... cellN>\r\n");
    } else {
        sprintf(
            out,
            "Unknown parameter: %s\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n"
            "- thr <millivolts>\r\n"
            "- test <board> <cell0 cell1 ... cellN>\r\n",
            argv[0]);
    }
}
void _cli_soc(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "reset") == 0) {
        soc_reset_soc();
        sprintf(out, "Resetting energy meter\r\n");
    } else {
        sprintf(
            out,
            "SoC: %.2f %%\r\n"
            "Energy: %.1f Wh\r\n"
            "Energy total: %.1f Wh\r\n",
            soc_get_soc(),
            soc_get_energy_last_charge(),
            soc_get_energy_total());
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
    } else if(argc < 2) {
        sprintf(
            out,
            "Invalid number of parameters.\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n");
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
    if (argc == 1) {
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
    } else if(argc < 2) {
        sprintf(
            out,
            "Invalid number of parameters.\r\n\n"
            "valid parameters:\r\n"
            "- zero: zeroes the hall-sensor measurement\r\n");
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
    for (uint8_t i = 0; i < N_COMMANDS - 2; i++) {
        sprintf(out + strlen(out), "- %s\r\n", cli_bms.cmds.names[i]);
    }
}

void _cli_imd(uint16_t argc, char **argv, char *out) {
    sprintf(out,
     "fault:        %u\r\n"
     "IMD status:   %u\r\n"
     "details:      %d\r\n"
     "duty cycle:   %.2f%%\r\n"
     "frequency:    %uHz\r\n"
     "period:       %ums\r\n",
     imd_is_fault(),
     (uint8_t)imd_get_state(),
     imd_get_details(),
     imd_get_duty_cycle_percentage(),
     imd_get_freq(),
     imd_get_period());
}

void _cli_can_forward(uint16_t argc, char **argv, char *out) {
    if(argc == 3) {
        CAN_HandleTypeDef *can;
        if(!strcmp(argv[1], "bms")) {
            can = &BMS_CAN;
        } else if(!strcmp(argv[1], "primary")) {
            can = &CAR_CAN;
        } else {
            sprintf(
                out,
                "Unknown parameter: %s\r\n\n"
                "valid parameters:\r\n"
                "- bms: bms internal network\r\n"
                "- primary: car primary network\r\n",
                argv[1]);
            return;
        }
        char *divider_index = strchr(argv[2], '#');
        uint8_t payload[CAN_MAX_PAYLOAD_LENGTH] = {0};

        if(divider_index == NULL) {
            sprintf(out, "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
            return;
        }
        *divider_index = '\0';

        if(strlen(divider_index+1) % 2 != 0) {
            sprintf(out, "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
            return;
        }

        CAN_TxHeaderTypeDef header = {
            .StdId = strtoul(argv[2], NULL, 16),
            .DLC = strlen(divider_index+1)/2,
            .ExtId = 0,
            .RTR = 0,
            .IDE = 0
        };
        *((uint64_t*)payload) = (uint64_t)strtoull(divider_index+1, NULL, 16);
        can_send(can, payload, &header);
        *out = '\0';
    } else {
            sprintf(
                out,
                "Invalid command format\r\n\n"
                "valid format:\r\n"
                "can_forward <network> <id>#<payload>\r\n");
    }
}

void _cli_feedbacks(uint16_t argc, char **argv, char *out) {
    FEEDBACK_STATE f[FEEDBACK_N];
    feedback_get_feedback_states(f);
    *out = '\0';
    for(uint8_t i=0; i<FEEDBACK_N; ++i) {
        sprintf(out + strlen(out), "%02d - %s: %s\n\r", i, feedback_names[i],
            f[i] == FEEDBACK_STATE_H ? "1" :
            f[i] == FEEDBACK_STATE_L ? "0" :
            "error");
    }
}

void _cli_sborat(uint16_t argc, char **argv, char *out) {
    BUZ_sborati(&HTIM_PWM);
    *out = '\0';
}