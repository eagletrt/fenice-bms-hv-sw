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

#include "adc124s021.h"
#include "bal_fsm.h"
#include "bms_fsm.h"
#include "can.h"
#include "can_comm.h"
#include "error/error.h"
#include "fans_buzzer.h"
#include "feedback.h"
#include "imd.h"
#include "mainboard_config.h"
#include "pack/pack.h"
#include "pack/temperature.h"
#include "pack/voltage.h"
#include "soc.h"
#include "spi.h"
#include "usart.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// TODO: don't count manually
#define N_COMMANDS 21

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
cli_command_func_t _cli_watch;
cli_command_func_t _cli_cellboard_distribution;
cli_command_func_t _cli_fans;
cli_command_func_t _cli_pack;
cli_command_func_t _cli_help;
cli_command_func_t _cli_sigterm;
cli_command_func_t _cli_taba;
cli_command_func_t _cli_sborat;

const char *bms_state_names[BMS_NUM_STATES] =
    {[BMS_IDLE] = "idle", [BMS_PRECHARGE] = "precharge", [BMS_ON] = "run", [BMS_FAULT] = "fault"};

const char *bal_state_names[BAL_NUM_STATES] =
    {[BAL_OFF] = "off", [BAL_COMPUTE] = "computing", [BAL_DISCHARGE] = "discharging", [BAL_COOLDOWN] = "cooldown"};

const char *error_names[ERROR_NUM_ERRORS] = {
    [ERROR_CELL_LOW_VOLTAGE]      = "low-voltage",
    [ERROR_CELL_UNDER_VOLTAGE]    = "under-voltage",
    [ERROR_CELL_OVER_VOLTAGE]     = "over-voltage",
    [ERROR_CELL_OVER_TEMPERATURE] = "over-temperature",
    [ERROR_CELL_HIGH_TEMPERATURE] = "high-temperature",
    [ERROR_OVER_CURRENT]          = "over-current",
    [ERROR_CAN]                   = "CAN",
    [ERROR_INT_VOLTAGE_MISMATCH]  = "internal voltage mismatch",
    [ERROR_CELLBOARD_COMM]        = "cellboard communication",
    [ERROR_CELLBOARD_INTERNAL]    = "cellboard internal",
    [ERROR_FEEDBACK]              = "feedback",
    [ERROR_FEEDBACK_CIRCUITRY]    = "feedback_circuitry",
    [ERROR_EEPROM_COMM]           = "EEPROM communication",
    [ERROR_EEPROM_WRITE]          = "EEPROM write"};

char const *const feedback_names[FEEDBACK_N] = {
    [FEEDBACK_TSAL_GREEN_FAULT_POS]         = "FEEDBACK_TSAL_GREEN_FAULT",
    [FEEDBACK_IMD_LATCHED_POS]              = "FEEDBACK_IMD_LATCHED",
    [FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS] = "FEEDBACK_TSAL_GREEN_FAULT_LATCHED",
    [FEEDBACK_BMS_LATCHED_POS]              = "FEEDBACK_BMS_LATCHED",
    [FEEDBACK_EXT_LATCHED_POS]              = "FEEDBACK_EXT_LATCHED",
    [FEEDBACK_TSAL_GREEN_POS]               = "FEEDBACK_TSAL_GREEN",
    [FEEDBACK_TS_OVER_60V_STATUS_POS]       = "FEEDBACK_TS_OVER_60V_STATUS",
    [FEEDBACK_AIRN_STATUS_POS]              = "FEEDBACK_AIRN_STATUS",
    [FEEDBACK_AIRP_STATUS_POS]              = "FEEDBACK_AIRP_STATUS",
    [FEEDBACK_AIRP_GATE_POS]                = "FEEDBACK_AIRP_GATE",
    [FEEDBACK_AIRN_GATE_POS]                = "FEEDBACK_AIRN_GATE",
    [FEEDBACK_PRECHARGE_STATUS_POS]         = "FEEDBACK_PRECHARGE_STATUS",
    [FEEDBACK_TSP_OVER_60V_STATUS_POS]      = "FEEDBACK_TSP_OVER_60V_STATUS",
    [FEEDBACK_CHECK_MUX_POS]                = "FEEDBACK_CHECK_MUX",
    [FEEDBACK_SD_IN_POS]                    = "FEEDBACK_SD_IN",
    [FEEDBACK_SD_OUT_POS]                   = "FEEDBACK_SD_OUT",
    [FEEDBACK_RELAY_SD_POS]                 = "FEEDBACK_RELAY_SD",
    [FEEDBACK_IMD_FAULT_POS]                = "FEEDBACK_IMD_FAULT",
    [FEEDBACK_SD_END_POS]                   = "FEEDBACK_SD_END"};

char const *const imd_state_names[IMD_STATES_N] = {
    [IMD_SC]            = "IMD_SHORT_CIRCUIT",
    [IMD_NORMAL]        = "IMD_NORMAL",
    [IMD_UNDER_VOLTAGE] = "IMD_UNDER_VOLTAGE",
    [IMD_START_MEASURE] = "IMD_START_MEASURE",
    [IMD_DEVICE_ERROR]  = "IMD_DEVICE_ERROR",
    [IMD_EARTH_FAULT]   = "IMD_EARTH_FAULT"};

char *command_names[N_COMMANDS] = {"volt",       "temp",  "status", "errors", "ts",          "bal",       "soc",
                                   "current",    "dmesg", "reset",  "imd",    "can_forward", "feedbacks", "watch",
                                   "cell_distr", "fans",  "pack",   "?",      "\003",        "\ta",       "sbor@"};

cli_command_func_t *commands[N_COMMANDS] = {
    &_cli_volts,   &_cli_temps,       &_cli_status,    &_cli_errors,  &_cli_ts,
    &_cli_balance, &_cli_soc,         &_cli_current,   &_cli_dmesg,   &_cli_reset,
    &_cli_imd,     &_cli_can_forward, &_cli_feedbacks, &_cli_watch,   &_cli_cellboard_distribution,
    &_cli_fans,    &_cli_pack,        &_cli_help,      &_cli_sigterm, &_cli_taba,
    &_cli_sborat};

cli_t cli_bms;
bool dmesg_ena = false;

void cli_bms_init() {
    cli_bms.uart           = &CLI_UART;
    cli_bms.cmds.functions = commands;
    cli_bms.cmds.names     = command_names;
    cli_bms.cmds.count     = N_COMMANDS;

    char init[94];
    snprintf(
        init,
        94,
        "\r\n\n********* Fenice BMS *********\r\n"
        " build: %s @ %s\r\n\n type ? for commands\r\n\n",
        __DATE__,
        __TIME__);

    strcat(init, cli_ps);

    cli_init(&cli_bms);
    cli_print(&cli_bms, init, strlen(init));
}

void cli_bms_debug(char *text) {
    if (dmesg_ena) {
        char out[256] = {'\0'};

        snprintf(out, 256, "[%.2f] %s\r\n> ", (float)HAL_GetTick() / 1000, text);

        cli_print(&cli_bms, out, strlen(out));
    }
}

void _cli_volts(uint16_t argc, char **argv, char *out) {
    float adc_vals[2]    = {0.0};
    ADC124S021_CH chs[2] = {ADC124_BUS_CHANNEL, ADC124_INTERNAL_CHANNEL};
    adc124s021_read_channels(&SPI_ADC124S, chs, 2, adc_vals);
    if (strcmp(argv[1], "") == 0) {
        voltage_t max = voltage_get_cell_max(NULL);
        voltage_t min = voltage_get_cell_min(NULL);
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "vts_p.......%.2f V --> %.4f\r\n"
            "vbat_adc....%.2f V --> %.4f\r\n"
            "vbat_sum....%.2f V\r\n"
            "average.....%.2f V\r\n"
            "max.........%.3f V\r\n"
            "min.........%.3f V\r\n"
            "delta.......%.3f V\r\n",
            voltage_get_vts_p(),
            adc_vals[0],
            voltage_get_vbat_adc(),
            adc_vals[1],
            voltage_get_vbat_sum(),
            voltage_get_vbat_adc() / PACK_CELL_COUNT,
            (float)max / 10000,
            (float)min / 10000,
            (float)(max - min) / 10000);
    } else if (strcmp(argv[1], "all") == 0) {
        _cli_volts_all(argc, &argv[1], out);
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Unknown parameter: %s\r\n"
            "valid parameters:\r\n"
            "- all: returns voltages for all cells\r\n",
            argv[1]);
    }
}

void _cli_volts_all(uint16_t argc, char **argv, char *out) {
    out[0]           = '\0';
    voltage_t *cells = voltage_get_cells();
    uint8_t max_index, min_index;
    float cell_v, total_power = 0.0f, segment_sum = 0.0f;
    voltage_get_cell_max(&max_index);
    voltage_get_cell_min(&min_index);
    for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
        cell_v = (float)cells[i] / 10000;
        if (i % LTC6813_CELL_COUNT == 0) {
            if (i != 0) {
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "Segment total: %.2fV", segment_sum);
                segment_sum = 0;
            }
            snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n%-3d", i / LTC6813_CELL_COUNT);
        } else if (i % (LTC6813_CELL_COUNT / 3) == 0 && i > 0) {
            snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n%-3s", "");
        }
        segment_sum += cell_v;

        if (fsm_get_state(bal.fsm) == BAL_OFF) {
            if (i == max_index)
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), RED_BG("[%3u %-.3fV]") " ", i, cell_v);
            else if (i == min_index)
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), CYAN_BG("[%3u %-.3fV]") " ", i, cell_v);
            else
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "[%3u %-.3fV] ", i, cell_v);
        } else {
            if (i == max_index) {
                if (CANLIB_BITTEST(bal.cells[i / CELLBOARD_CELL_COUNT], i % CELLBOARD_CELL_COUNT)) {
                    total_power += cell_v * cell_v;
                    snprintf(
                        out + strlen(out),
                        CLI_TX_BUF_LEN - strlen(out),
                        RED_BG_ON_YELLOW_FG("[%3u %-.3fV %.2fW]") " ",
                        i,
                        cell_v,
                        DISCHARGE_DUTY_CYCLE * cell_v * cell_v / DISCHARGE_R);
                } else {
                    snprintf(
                        out + strlen(out), CLI_TX_BUF_LEN - strlen(out), RED_BG("[%3u %-.3fV 0.00W]") " ", i, cell_v);
                }
            } else if (CANLIB_BITTEST(bal.cells[i / CELLBOARD_CELL_COUNT], i % CELLBOARD_CELL_COUNT)) {
                total_power += cell_v * cell_v;
                snprintf(
                    out + strlen(out),
                    CLI_TX_BUF_LEN - strlen(out),
                    YELLOW_BG("[%3u %-.3fV %.2fW]") " ",
                    i,
                    cell_v,
                    DISCHARGE_DUTY_CYCLE * cell_v * cell_v / DISCHARGE_R);
            } else if (i == min_index) {
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), CYAN_BG("[%3u %-.3fV 0.00W]") " ", i, cell_v);
            } else {
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "[%3u %-.3fV 0.00W] ", i, cell_v);
            }
        }
    }
    snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "Segment total: %.2fV", segment_sum);
    total_power /= DISCHARGE_R;

    if (fsm_get_state(bal.fsm) != BAL_OFF) {
        cell_v           = (float)cells[max_index] / 10000;
        float max_soc    = CELL_CAPACITY - soc_volt_to_capacity(cell_v);
        cell_v           = ((float)cells[min_index] + bal_get_threshold()) / 10000;
        float target_soc = CELL_CAPACITY - soc_volt_to_capacity(cell_v);

        float ETA = ((target_soc - max_soc) * CELL_CAPACITY * 4) / ((float)cells[max_index] / 10000 / DISCHARGE_R);

        snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\nETA: %.1fh\r\n", ETA / DISCHARGE_DUTY_CYCLE);
        snprintf(
            out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "Pwr: %3.1fW\r\n", total_power * DISCHARGE_DUTY_CYCLE);
    } else
        snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n");
}

void _cli_temps(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "") == 0) {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "average.....%.2f °C\r\nmax.........%.2f "
            "C\r\nmin.........%.2f °C\r\n"
            "delta.......%.2f °C\r\n",
            (float)temperature_get_average() / 10,
            temperature_get_max() / 2.56 - 20,
            temperature_get_min() / 2.56 - 20,
            (temperature_get_max() - temperature_get_min()) / 2.56 - 20);
    } else if (strcmp(argv[1], "all") == 0) {
        _cli_temps_all(argc, &argv[1], out);
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Unknown parameter: %s\r\n"
            "valid parameters:\r\n"
            "- all: returns temperature for all cells\r\n",
            argv[1]);
    }
}

void _cli_temps_all(uint16_t argc, char **argv, char *out) {
    out[0]                  = '\0';
    temperature_t *temp_all = temperature_get_all();

    for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
        if (i % TEMP_SENSOR_COUNT == 0) {
            snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n%-3d", i / TEMP_SENSOR_COUNT);
        } else if (i % (TEMP_SENSOR_COUNT / 4) == 0 && i > 0) {
            snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n%-3s", "");
        }
        snprintf(
            out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "[%3u %2u °C] ", i, (uint8_t)(temp_all[i] / 2.56 - 20));
    }

    snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n");
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
        snprintf(
            out + strlen(out),
            CLI_TX_BUF_LEN - strlen(out),
            "%s%s%s\r\n",
            values[i][0],
            "........................" + strlen(values[i][0]),
            values[i][1]);
    }
}

void _cli_balance(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "on") == 0) {
        if (argc > 2)
            bal.target = atoi(argv[2]);
        fsm_trigger_event(bal.fsm, EV_BAL_START);
        snprintf(out, CLI_TX_BUF_LEN, "enabling balancing\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        fsm_trigger_event(bal.fsm, EV_BAL_STOP);
        snprintf(out, CLI_TX_BUF_LEN, "disabling balancing\r\n");
    } else if (strcmp(argv[1], "thr") == 0) {
        if (argv[2] != NULL) {
            voltage_t thresh = atoff(argv[2]) * 10;
            if (thresh <= BAL_MAX_VOLTAGE_THRESHOLD) {
                bal_set_threshold(thresh);
            }
        }
        snprintf(out, CLI_TX_BUF_LEN, "balancing threshold is %.2f mV\r\n", bal_get_threshold() / 10.0);
    } else if (strcmp(argv[1], "test") == 0) {
        CAN_TxHeaderTypeDef tx_header;
        uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];
        uint8_t board;
        bms_BalancingCells cells = bms_BalancingCells_DEFAULT;

        tx_header.ExtId = 0;
        tx_header.IDE   = CAN_ID_STD;
        tx_header.RTR   = CAN_RTR_DATA;
        tx_header.StdId = bms_ID_BALANCING;

        board = atoi(argv[2]);

        if (argc < 4) {
            snprintf(out, CLI_TX_BUF_LEN, "wrong number of parameters\r\n");
            return;
        }

        snprintf(out, CLI_TX_BUF_LEN, "testing cells [");

        uint8_t c;
        for (uint8_t i = 3; i < argc; ++i) {
            c = atoi(argv[i]);
            CANLIB_BITSET(cells, c);
            snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "%u,", c);
        }

        tx_header.DLC = bms_serialize_BALANCING(buffer, board, cells);
        can_send(&BMS_CAN, buffer, &tx_header);

        snprintf(out + strlen(out) - 1, CLI_TX_BUF_LEN - strlen(out) - 1, "]\r\non board %d\r\n", board);
    } else if (argc < 2) {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Invalid number of parameters.\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n"
            "- thr <millivolts>\r\n"
            "- test <board> <cell0 cell1 ... cellN>\r\n");
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
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
        snprintf(out, CLI_TX_BUF_LEN, "Resetting energy meter\r\n");
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "SoC: %.2f %%\r\n"
            "Energy total: %.1f Wh\r\n",
            soc_get_soc(),
            soc_get_energy_total());
    }
}

void _cli_errors(uint16_t argc, char **argv, char *out) {
    *out           = 0;
    uint16_t count = error_count();
    error_t errors[100];
    error_dump(errors);

    uint32_t now = HAL_GetTick();
    snprintf(out, CLI_TX_BUF_LEN, "total %u\r\n", count);
    for (uint16_t i = 0; i < count; i++) {
        snprintf(
            out + strlen(out),
            CLI_TX_BUF_LEN - strlen(out),
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
        snprintf(out, CLI_TX_BUF_LEN, "triggered TS ON event\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
        snprintf(out, CLI_TX_BUF_LEN, "triggered TS OFF event\r\n");
    } else if (argc < 2) {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Invalid number of parameters.\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n");
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Unknown parameter: %s\r\n\n"
            "valid parameters:\r\n"
            "- on\r\n"
            "- off\r\n",
            argv[1]);
    }
}

void _cli_current(uint16_t argc, char **argv, char *out) {
    if (argc == 1) {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Hall 50A:\t%.1fA\r\n"
            "Hall 300A:\t%.1fA\r\n"
            "Shunt:\t\t%.1fA\r\n",
            current_get_current_from_sensor(CURRENT_SENSOR_50),
            current_get_current_from_sensor(CURRENT_SENSOR_300),
            current_get_current_from_sensor(CURRENT_SENSOR_SHUNT));
    } else if (strcmp(argv[1], "zero") == 0) {
        current_zero();
        snprintf(out, CLI_TX_BUF_LEN, "Current zeroed\r\n");
    } else if (argc < 2) {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Invalid number of parameters.\r\n\n"
            "valid parameters:\r\n"
            "- zero: zeroes the hall-sensor measurement\r\n");
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Unknown parameter: %s\r\n\n"
            "valid parameters:\r\n"
            "- zero: zeroes the hall-sensor measurement\r\n",
            argv[1]);
    }
}

void _cli_dmesg(uint16_t argc, char **argv, char *out) {
    dmesg_ena = !dmesg_ena;
    snprintf(out, CLI_TX_BUF_LEN, "dmesg output is %s\r\n", dmesg_ena ? "enabled" : "disabled");
}

void _cli_reset(uint16_t argc, char **argv, char *out) {
    HAL_NVIC_SystemReset();
}

void _cli_taba(uint16_t argc, char **argv, char *out) {
    snprintf(
        out,
        CLI_TX_BUF_LEN,
        " #######    #    ######     #    ######     #    ####### ####### ######  \r\n"
        "    #      # #   #     #   # #   #     #   # #      #    #       #     # \r\n"
        "    #     #   #  #     #  #   #  #     #  #   #     #    #       #     # \r\n"
        "    #    #     # ######  #     # ######  #     #    #    #####   #     # \r\n"
        "    #    ####### #     # ####### #   #   #######    #    #       #     # \r\n"
        "    #    #     # #     # #     # #    #  #     #    #    #       #     # \r\n"
        "    #    #     # ######  #     # #     # #     #    #    ####### ######  \r\n");
}

void _cli_help(uint16_t argc, char **argv, char *out) {
    snprintf(out, CLI_TX_BUF_LEN, "command list:\r\n");
    for (uint8_t i = 0; i < N_COMMANDS - 3; i++) {
        snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "- %s\r\n", cli_bms.cmds.names[i]);
    }
}

void _cli_imd(uint16_t argc, char **argv, char *out) {
    snprintf(
        out,
        CLI_TX_BUF_LEN,
        "fault:        %u\r\n"
        "IMD status:   %s\r\n"
        "details:      %ld\r\n"
        "duty cycle:   %.2f%%\r\n"
        "frequency:    %uHz\r\n"
        "period:       %ums\r\n",
        imd_is_fault(),
        imd_state_names[imd_get_state()],
        imd_get_details(),
        imd_get_duty_cycle_percentage(),
        imd_get_freq(),
        imd_get_period());
}

void _cli_can_forward(uint16_t argc, char **argv, char *out) {
    if (argc == 3) {
        CAN_HandleTypeDef *can;
        if (!strcmp(argv[1], "bms")) {
            can = &BMS_CAN;
        } else if (!strcmp(argv[1], "primary")) {
            can = &CAR_CAN;
        } else {
            snprintf(
                out,
                CLI_TX_BUF_LEN,
                "Unknown parameter: %s\r\n\n"
                "valid parameters:\r\n"
                "- bms: bms internal network\r\n"
                "- primary: car primary network\r\n",
                argv[1]);
            return;
        }
        char *divider_index                     = strchr(argv[2], '#');
        uint8_t payload[CAN_MAX_PAYLOAD_LENGTH] = {0};

        if (divider_index == NULL) {
            snprintf(
                out,
                CLI_TX_BUF_LEN,
                "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
            return;
        }
        *divider_index = '\0';

        if (strlen(divider_index + 1) % 2 != 0) {
            snprintf(
                out,
                CLI_TX_BUF_LEN,
                "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
            return;
        }

        CAN_TxHeaderTypeDef header = {
            .StdId = strtoul(argv[2], NULL, 16), .DLC = strlen(divider_index + 1) / 2, .ExtId = 0, .RTR = 0, .IDE = 0};
        *((uint64_t *)payload) = (uint64_t)strtoull(divider_index + 1, NULL, 16);
        can_send(can, payload, &header);
        *out = '\0';
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Invalid command format\r\n\n"
            "valid format:\r\n"
            "can_forward <network> <id>#<payload>\r\n");
    }
}

void _cli_feedbacks(uint16_t argc, char **argv, char *out) {
    feedback_feed_t f[FEEDBACK_N];
    feedback_get_feedback_states(f);
    *out = '\0';
    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
        snprintf(
            out + strlen(out),
            CLI_TX_BUF_LEN - strlen(out),
            "%02d - %s: %s, %.4f\n\r",
            i,
            feedback_names[i],
            f[i].state == FEEDBACK_STATE_H   ? "1"
            : f[i].state == FEEDBACK_STATE_L ? "0"
                                             : "E",
            f[i].voltage);
    }
}

void _cli_sborat(uint16_t argc, char **argv, char *out) {
    BUZ_sborati(&HTIM_PWM);
    *out = '\0';
}

char watch_buf[BUF_SIZE]      = {'\0'};
uint8_t cli_watch_flush_tx    = 0;
uint8_t cli_watch_execute_cmd = 0;

void _cli_sigterm(uint16_t argc, char **argv, char *out) {
    if (HTIM_CLI.State == HAL_TIM_STATE_BUSY) {
        HAL_TIM_Base_Stop_IT(&HTIM_CLI);
        *watch_buf = '\0';
        snprintf(out, CLI_TX_BUF_LEN, "\r\n");
    } else {
        snprintf(out, CLI_TX_BUF_LEN, "^C\r\n");
    }
}

void _cli_watch(uint16_t argc, char **argv, char *out) {
    *watch_buf = '\0';
    if (!strcmp(argv[1], "stop")) {
        HAL_TIM_Base_Stop_IT(&HTIM_CLI);
    } else {
        uint16_t interval = atoi(argv[1]);
        if (interval == 0)
            interval = 500;
        for (uint8_t i = 2; i < argc; ++i) {
            snprintf(watch_buf + strlen(watch_buf), BUF_SIZE - strlen(out), "%s ", argv[i]);
        }
        cli_watch_execute_cmd            = 1;
        watch_buf[strlen(watch_buf) - 1] = '\0';
        __HAL_TIM_SetAutoreload(&HTIM_CLI, TIM_MS_TO_TICKS(&HTIM_CLI, interval));
        __HAL_TIM_CLEAR_IT(&HTIM_CLI, TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(&HTIM_CLI);
    }

    snprintf(out, CLI_TX_BUF_LEN, "\033[2J\033H");
}

void _cli_timer_handler(TIM_HandleTypeDef *htim) {
    if (cli_watch_flush_tx == 0)
        cli_watch_execute_cmd = 1;
    cli_watch_flush_tx = 1;
}

#define CLI_BMS_PRINT_BUF_LEN (CLI_TX_BUF_LEN / 4)

char tx_buf[CLI_TX_BUF_LEN]           = {'\0'};
char print_buf[CLI_BMS_PRINT_BUF_LEN] = {'\0'};
char *save_ptr                        = NULL;
void cli_watch_flush_handler() {
    char *argv[BUF_SIZE] = {NULL};
    uint16_t argc;
    char *to_print;

    if (cli_watch_flush_tx == 0)
        return;

    if (cli_watch_execute_cmd == 1) {
        snprintf(
            tx_buf,
            CLI_TX_BUF_LEN,
            "\033[HExecuting %s every %.0fms\033[K\r\n[%.2f]\033[K\r\n",
            watch_buf,
            TIM_TICKS_TO_MS(&HTIM_CLI, __HAL_TIM_GetAutoreload(&HTIM_CLI)),
            HAL_GetTick() / 1000.0);

        argc = _cli_get_args(watch_buf, argv);

        // Check which command corresponds with the buffer
        for (uint16_t i = 0; i < N_COMMANDS; i++) {
            //size_t len = strlen(cli->cmds.names[i]);

            if (strcmp(argv[0], command_names[i]) == 0) {
                commands[i](argc, argv, tx_buf + strlen(tx_buf));
                break;
            }

            if (i == N_COMMANDS - 1) {
                snprintf(tx_buf, CLI_TX_BUF_LEN, "Command not found\r\n");
                *watch_buf = '\0';
                HAL_TIM_Base_Stop_IT(&HTIM_CLI);
                HAL_UART_Transmit(&CLI_UART, (uint8_t *)tx_buf, strlen(tx_buf), 100);
                return;
            }
        }

        snprintf(tx_buf + strlen(tx_buf), CLI_TX_BUF_LEN - strlen(tx_buf), "\r\nPress CTRL+C to stop\r\n\033[J");

        // restore watch_buf after splitting it in _cli_get_args
        for (uint16_t i = 0; i < argc - 1; ++i) {
            watch_buf[strlen(watch_buf)] = ' ';
        }
        to_print = strtok_r(tx_buf, "\r\n", &save_ptr);

        snprintf(print_buf, sizeof(print_buf), "%s\r\n", to_print);

        HAL_UART_Transmit_IT(&CLI_UART, (uint8_t *)print_buf, strlen(print_buf));
        cli_watch_execute_cmd = 0;
    } else {
        if (CLI_UART.gState != HAL_UART_STATE_BUSY_TX) {
            to_print = strtok_r(NULL, "\r\n", &save_ptr);

            if (to_print != NULL) {
                snprintf(print_buf, sizeof(print_buf), "%s\033[K\r\n", to_print);
                HAL_UART_Transmit_IT(&CLI_UART, (uint8_t *)print_buf, strlen(print_buf));
            } else {
                cli_watch_flush_tx = 0;
            }
        }
    }
}

void _cli_cellboard_distribution(uint16_t argc, char **argv, char *out) {
    *out = '\0';
    if (argc == 1) {
        uint8_t *distr = bms_get_cellboard_distribution();
        for (uint8_t i = 0; i < CELLBOARD_COUNT; ++i) {
            snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "%u ", distr[i]);
        }
        snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "\r\n");
    } else if (argc == 7) {
        uint8_t distr[CELLBOARD_COUNT];
        for (uint8_t i = 1; i < argc; ++i) {
            uint8_t pos = atoi(argv[i]);
            if (pos < 0 || pos > 5) {
                snprintf(out + strlen(out), CLI_TX_BUF_LEN - strlen(out), "Index out of range: %u\r\n", pos);
                return;
            }

            for (uint8_t j = 0; j < i - 1; ++j) {
                if (pos == distr[j]) {
                    snprintf(
                        out + strlen(out),
                        CLI_TX_BUF_LEN - strlen(out),
                        "Non puoi ripetere la stessa cella piu' di una volta\r\n");
                    return;
                }
            }

            distr[i - 1] = pos;
        }

        bms_set_cellboard_distribution(distr);
        snprintf(
            out + strlen(out),
            CLI_TX_BUF_LEN - strlen(out),
            "Distribuzione delle cellboard aggiornata con successo\r\n");
    } else {
        snprintf(
            out,
            CLI_TX_BUF_LEN,
            "Invalid sintax\r\n"
            "Available commands are:\r\n"
            "\t-<empty>: show the current cell distribution\r\n"
            "\t-<cellboard index at p0> ... <cellboard index at p5>: set a new cellboard distribution\r\n");
    }
}

void _cli_fans(uint16_t argc, char **argv, char *out) {
    if (argc == 2) {
        if (!strcmp(argv[1], "off")) {
            fans_override       = 1;
            fans_override_value = 0;
            snprintf(out, CLI_TX_BUF_LEN, "Fans turned off\r\n");
        } else if (!strcmp(argv[1], "auto")) {
            fans_override       = 0;
            fans_override_value = 0;
        } else {
            uint8_t perc = atoi(argv[1]);
            if (perc <= 100) {
                fans_override       = 1;
                fans_override_value = perc / 100.0;
                snprintf(out, CLI_TX_BUF_LEN, "Fans speed set to %d\r\n", perc);
            } else {
                snprintf(out, CLI_TX_BUF_LEN, "Invalid perc value: %d\r\nIt must be between 0 and 100", perc);
            }
        }
    } else {
        snprintf(out, CLI_TX_BUF_LEN, "Invalid sintax.\r\n Use this way: fans <perc>\r\n");
    }
}

void _cli_pack(uint16_t argc, char **argv, char *out) {
    if (argc == 3) {
        uint8_t value;
        if (!strcmp(argv[1], "airn")) {
            if (!strcmp(argv[2], "on")) {
                value = AIRN_ON_VALUE;
            } else {
                value = AIRN_OFF_VALUE;
            }
            pack_set_airn_off(value);
        } else if (!strcmp(argv[1], "airp")) {
            if (!strcmp(argv[2], "on")) {
                value = AIRP_ON_VALUE;
            } else {
                value = AIRP_OFF_VALUE;
            }
            pack_set_airp_off(value);
        } else if (!strcmp(argv[1], "precharge")) {
            if (!strcmp(argv[2], "on")) {
                value = PRECHARGE_ON_VALUE;
            } else {
                value = PRECHARGE_OFF_VALUE;
            }
            pack_set_precharge(value);
        }

        snprintf(out, CLI_TX_BUF_LEN, "%s setted %s\r\n", argv[1], argv[2]);
    } else if (argc == 2) {
    }
}