/**
 * @file cli_bms.c
 * @brief cli instance for bms
 *
 * @date Mar 29,2020
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Simone Ruffini [simone.ruffini@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "cli_bms.h"

#include "measures.h"
#include "bal.h"
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
#include "soc.h"
#include "usart.h"
#include "bms/bms_network.h"
#include "internal_voltage.h"
#include "cell_voltage.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CELLBOARD_DISTR_ADDR 0x50
#define CELLBOARD_DISTR_VER  0x01

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

const char * bms_state_names[NUM_STATES] = {
    [STATE_INIT] = "init",
    [STATE_IDLE] = "idle",
    [STATE_WAIT_AIRN_CLOSE] = "airn_close",
    [STATE_WAIT_TS_PRECHARGE] = "precharge",
    [STATE_WAIT_AIRP_CLOSE] = "airn_close",
    [STATE_TS_ON] = "ts_on",
    [STATE_FATAL_ERROR] = "fault"
};

const char *bal_state_names[2] = { "off", "discharging" };

const char *error_names[ERROR_COUNT] = {
    [ERROR_CELL_LOW_VOLTAGE]       = "low-voltage",
    [ERROR_CELL_UNDER_VOLTAGE]     = "under-voltage",
    [ERROR_CELL_OVER_VOLTAGE]      = "over-voltage",
    [ERROR_CELL_OVER_TEMPERATURE]  = "over-temperature",
    [ERROR_CELL_HIGH_TEMPERATURE]  = "high-temperature",
    [ERROR_OVER_CURRENT]           = "over-current",
    [ERROR_CAN_COMM]               = "CAN communication",
    [ERROR_VOLTAGE_MISMATCH]       = "internal voltage mismatch",
    [ERROR_CELLBOARD_COMM]         = "cellboard communication",
    [ERROR_CELLBOARD_INTERNAL]     = "cellboard internal",
    [ERROR_CONNECTOR_DISCONNECTED] = "connection error",
    [ERROR_FEEDBACK]               = "feedback",
    [ERROR_FEEDBACK_CIRCUITRY]     = "feedback_circuitry",
    [ERROR_EEPROM_COMM]            = "EEPROM communication",
    [ERROR_EEPROM_WRITE]           = "EEPROM write"};

char const *const feedback_names[FEEDBACK_N] = {
    [FEEDBACK_IMPLAUSIBILITY_DETECTED_POS]  = "FEEDBACK_IMPLAUSBILITY_DETECTED",
    [FEEDBACK_IMD_COCKPIT_POS]              = "FEEDBACK_IMD_COCKPIT",
    [FEEDBACK_TSAL_GREEN_FAULT_LATCHED_POS] = "FEEDBACK_TSAL_GREEN_FAULT_LATCHED",
    [FEEDBACK_BMS_COCKPIT_POS]              = "FEEDBACK_BMS_COCKPIT",
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
    [FEEDBACK_SD_BMS_POS]                   = "FEEDBACK_SD_BMS",
    [FEEDBACK_SD_IMD_POS]                   = "FEEDBACK_SD_IMD",
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
bool dmesg_ena = true;

config_t cellboard_distribution;


void cli_bms_init() {
    // Update cellboard distrbution in the EEPROM
    uint8_t cell_distr_default[CELLBOARD_COUNT] = { 0, 1, 2, 3, 4, 5 };
    config_init(&cellboard_distribution, CELLBOARD_DISTR_ADDR, CELLBOARD_DISTR_VER, cell_distr_default, 6);

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
        float tick    = (float)HAL_GetTick() / 1000;
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
        float max = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_max());
        float min = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_min());
        float sum = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_sum());
        float avg = CONVERT_VALUE_TO_VOLTAGE(cell_voltage_get_avg());
        sprintf(
            out,
            "vts+........%.2f V\r\n"
            "vts-........%.2f V\r\n"
            "vbat........%.2f V\r\n"
            "vshunt......%.2f V\r\n"
            "sum.........%.2f V\r\n"
            "average.....%.2f V\r\n"
            "max.........%.3f V\r\n"
            "min.........%.3f V\r\n"
            "delta.......%.3f V\r\n",
            CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsp()),
            CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_tsn()),
            CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_bat()),
            CONVERT_VALUE_TO_INTERNAL_VOLTAGE(internal_voltage_get_shunt()),
            sum,
            avg,
            max,
            min,
            max - min
        );
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
    sprintf(out, "     MIN      MAX      AVG\r\n");
    for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
        sprintf(out + strlen(out), "%d    %.2fV    %.2fV    %.2fV\r\n",
            i + 1,
            CONVERT_VALUE_TO_VOLTAGE(cell_volts.min[i]),
            CONVERT_VALUE_TO_VOLTAGE(cell_volts.max[i]),
            CONVERT_VALUE_TO_VOLTAGE(cell_volts.avg[i]));
    }
    sprintf(out + strlen(out), "\r\n");
    /*
    voltage_t * cells = cell_voltage_get_cells();

    voltage_t max = cell_voltage_get_max();
    voltage_t min = cell_voltage_get_min();

    float cell_v, total_power = 0.0f, segment_sum = 0.0f;
    for (uint8_t i = 0; i < PACK_CELL_COUNT; i++) {
        cell_v = (float)cells[i] / 10000;
        if (i % LTC6813_CELL_COUNT == 0) {
            if (i != 0) {
                sprintf(out + strlen(out), "Segment total: %.2fV", segment_sum);
                segment_sum = 0;
            }
            sprintf(out + strlen(out), "\r\n%-3d", i / LTC6813_CELL_COUNT);
        } else if (i % (LTC6813_CELL_COUNT / 3) == 0 && i > 0) {
            sprintf(out + strlen(out), "\r\n%-3s", "");
        }
        segment_sum += cell_v;

        bool is_cell_selected = (cells[i / CELLBOARD_CELL_COUNT] & (1 << i % CELLBOARD_CELL_COUNT));

        if (!bal_is_balancing()) {
            if (cells[i] == max)
                sprintf(out + strlen(out), RED_BG("[%3u %-.3fV]") " ", i, cell_v);
            else if (cells[i] == min)
                sprintf(out + strlen(out), CYAN_BG("[%3u %-.3fV]") " ", i, cell_v);
            else
                sprintf(out + strlen(out), "[%3u %-.3fV] ", i, cell_v);
        } else {
            if (cells[i] == max) {
                if (is_cell_selected) {
                    total_power += cell_v * cell_v;
                    sprintf(
                        out + strlen(out),
                        RED_BG_ON_YELLOW_FG("[%3u %-.3fV %.2fW]") " ",
                        i,
                        cell_v,
                        DISCHARGE_DUTY_CYCLE * cell_v * cell_v / DISCHARGE_R);
                } else
                    sprintf(out + strlen(out), RED_BG("[%3u %-.3fV 0.00W]") " ", i, cell_v);
            } else if (is_cell_selected) {
                total_power += cell_v * cell_v;
                sprintf(
                    out + strlen(out),
                    YELLOW_BG("[%3u %-.3fV %.2fW]") " ",
                    i,
                    cell_v,
                    DISCHARGE_DUTY_CYCLE * cell_v * cell_v / DISCHARGE_R);
            } else if (cells[i] == min)
                sprintf(out + strlen(out), CYAN_BG("[%3u %-.3fV 0.00W]") " ", i, cell_v);
            else
                sprintf(out + strlen(out), "[%3u %-.3fV 0.00W] ", i, cell_v);
        }
    }
    sprintf(out + strlen(out), "Segment total: %.2fV", segment_sum);
    total_power /= DISCHARGE_R;

    if (!bal_is_balancing()) {
        cell_v           = max / 10000;
        float max_soc    = 0.0862 * cell_v * cell_v * cell_v - 1.4260 * cell_v * cell_v + 6.0088 * cell_v - 6.551;
        cell_v           = (min + bal_get_threshold() - 10) / 10000;
        float target_soc = 0.0862 * cell_v * cell_v * cell_v - 1.4260 * cell_v * cell_v + 6.0088 * cell_v - 6.551;

        float ETA = ((target_soc - max_soc) * CELL_CAPACITY * 4) / (max / 10000 / DISCHARGE_R);

        sprintf(out + strlen(out), "\r\nETA: %.1fh\r\n", ETA / DISCHARGE_DUTY_CYCLE);
        sprintf(out + strlen(out), "Pwr: %3.1fW\r\n", total_power * DISCHARGE_DUTY_CYCLE);
    } else
        sprintf(out + strlen(out), "\r\n");
    */
}

void _cli_temps(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "") == 0) {
        float min = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_min());
        float max = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_max());
        float avg = CONVERT_VALUE_TO_TEMPERATURE(temperature_get_average());
        sprintf(
            out,
            "average.....%.2f °C\r\nmax.........%.2f "
            "C\r\nmin.........%.2f °C\r\n"
            "delta.......%.2f °C\r\n",
            avg,
            max,
            min,
            max - min);
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
/*
void _cli_temps_all(uint16_t argc, char **argv, char *out) {
    out[0]                  = '\0';
    temperature_t *temp_all = temperature_get_all();
    uint8_t * distr = bms_get_cellboard_distribution();
    uint8_t ids[CELLBOARD_COUNT] = { 0 };

    for (uint8_t i = 0; i < CELLBOARD_COUNT; i++)
        ids[distr[i]] = i;

    sprintf(out + strlen(out), "   ");
    for (size_t i = 0; i < TEMP_SENSORS_PER_STRIP; i++)
        sprintf(out + strlen(out), "         %d  ", i);

    
    for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
        const size_t cellboard = i / TEMP_SENSOR_COUNT;
        const size_t strip = i % TEMP_STRIPS_PER_BUS;

        if (i % TEMP_SENSOR_COUNT == 0)
            sprintf(out + strlen(out), "\r\n\r\n%-3d", cellboard);
        else if (i % (TEMP_SENSOR_COUNT / TEMP_STRIPS_PER_BUS) == 0 && i > 0)
            sprintf(out + strlen(out), "\r\n%-3s", "");

        sprintf(out + strlen(out), "[%3u %2u °C] ", i, (uint8_t)(temp_all[ids[strip]] / 2.56 - 20));
    }

    sprintf(out + strlen(out), "\r\n");
}
*/
void _cli_temps_all(uint16_t argc, char **argv, char *out) {
    sprintf(out, "     MIN         MAX         AVG\r\n");
    for (size_t i = 0; i < CELLBOARD_COUNT; i++) {
        sprintf(out + strlen(out), "%d   %6.2f°C    %6.2f°C    %6.2f°C\r\n",
            i,
            CONVERT_VALUE_TO_TEMPERATURE(cell_temps.min[i]),
            CONVERT_VALUE_TO_TEMPERATURE(cell_temps.max[i]),
            CONVERT_VALUE_TO_TEMPERATURE(cell_temps.avg[i]));
    }
    sprintf(out + strlen(out), "\r\n");/*
    out[0]                  = '\0';
    temperature_t *temp_all = temperature_get_all();

    for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {

        if (i % TEMP_SENSOR_COUNT == 0)
            sprintf(out + strlen(out), "\r\n\r\n%-3d", i / TEMP_SENSOR_COUNT);
        else if (i % (TEMP_SENSOR_COUNT / 4) == 0 && i > 0)
            sprintf(out + strlen(out), "\r\n%-3s", "");

        sprintf(out + strlen(out), "[%3u %2u °C] ", i, (uint8_t)(temp_all[i] / 2.55 - 20));
    }

    sprintf(out + strlen(out), "\r\n");
    */
}

void _cli_status(uint16_t argc, char **argv, char *out) {
#define n_items 6

    char thresh[5] = {'\0'};
    itoa((float)bal_get_threshold() / 10, thresh, 10);

    char running_errors[4] = { 0 };
    char expired_errors[4] = { 0 };
    itoa(error_running_count(), running_errors, 10);
    itoa(error_expired_count(), expired_errors, 10);

    char handcart_connected[13] = { '\0' };
    if (is_handcart_connected)
        strncpy(handcart_connected, "connected", 10);
    else
        strncpy(handcart_connected, "disconnected", 13);

    const char *values[n_items][2] = {
        {"BMS state", bms_state_names[fsm_get_state()]},
        {"Running errors", running_errors},
        {"Expired errors", expired_errors},
        {"CAN forwarding", can_is_forwarding() ? "true" : "false"},
        {"Balancing state", bal_state_names[bal_is_balancing()]},
        {"Handcart status", handcart_connected}
    };
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

// TODO: Balancing actions
void _cli_balance(uint16_t argc, char **argv, char *out) {
    strcpy(out, "Work in progress...\n\0");
    
    if (strcmp(argv[1], "on") == 0) {
        // if (argc > 2)
        //     bal.target = atoi(argv[2]);
        bal_start();
        sprintf(out, "enabling balancing\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        bal_stop();
        sprintf(out, "disabling balancing\r\n");
    } else if (strcmp(argv[1], "thr") == 0) {
        if (argv[2] != NULL) {
            voltage_t thresh = atoff(argv[2]) * 10;
            if (thresh <= BAL_MAX_VOLTAGE_THRESHOLD) {
                bal_set_threshold(thresh);
            }
        }
        sprintf(out, "balancing threshold is %.2f mV\r\n", bal_get_threshold() / 10.0);
    } else if (strcmp(argv[1], "test") == 0) {
        /*
        CAN_TxHeaderTypeDef tx_header;
        uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];
        uint8_t board;
        uint32_t cells = 0;

        tx_header.ExtId = 0;
        tx_header.IDE   = CAN_ID_STD;
        tx_header.RTR   = CAN_RTR_DATA;
        tx_header.StdId = BMS_SET_BALANCING_STATUS_FRAME_ID;

        board = atoi(argv[2]);

        if (argc < 4) {
            sprintf(out, "wrong number of parameters\r\n");
            return;
        }

        sprintf(out, "testing cells [");

        uint8_t c;
        for (uint8_t i = 3; i < argc; ++i) {
            c = atoi(argv[i]);
            cells |= 1 << c;
            sprintf(out + strlen(out), "%u,", c);
        }


        tx_header.DLC = bms_balancing_pack(buffer, &raw, BMS_BALANCING_BYTE_SIZE);
        can_send(&BMS_CAN, buffer, &tx_header);

        sprintf(out + strlen(out) - 1, "]\r\non board %d\r\n", board);
        */
    } else if (argc < 2) {
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

// TODO: Print errors to cli
void _cli_errors(uint16_t argc, char **argv, char *out) {
    /*
    uint32_t now = HAL_GetTick();
    ErrorUtilsRunningInstance * errors[397U] = { 0 };
    sprintf(out, "Running %u\r\nExpired %u\r\n", error_running_count(), error_expired_count());
    size_t error_count = error_dump(errors);
    for (size_t i = 0; i < error_count; ++i) {
        if (errors[i]->string_instance) {
            sprintf(
                out + strlen(out),
                "\r\n"
                "Type.......%lu\r\n"
                "Instance...%s\r\n"
                "Timestamp..T+%lu (%lums ago)\r\n"
                "Running....%s\r\n"
                "Expired....%s\r\n",
                errors[i]->error,
                errors[i]->instance.s,
                errors[i]->timestamp,
                now - errors[i]->timestamp,
                errors[i]->is_running ? "true" : "false",
                errors[i]->is_expired ? "true" : "false"
            );
        }
        else {
            sprintf(
                out + strlen(out),
                "\r\n"
                "Type.......%lu\r\n"
                "Instance...%lu\r\n"
                "Timestamp..T+%lu (%lums ago)\r\n"
                "Running....%s\r\n"
                "Expired....%s\r\n",
                errors[i]->error,
                errors[i]->instance.i,
                errors[i]->timestamp,
                now - errors[i]->timestamp,
                errors[i]->is_running ? "true" : "false",
                errors[i]->is_expired ? "true" : "false"
            );
        }
    }
    */
    /*
    *out           = 0;
    uint16_t count = error_count();
    error_t errors[500] = { 0 };
    error_dump(errors);

    volatile uint32_t now = HAL_GetTick();
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
    */
}

void _cli_ts(uint16_t argc, char **argv, char *out) {
    if (strcmp(argv[1], "on") == 0) {
        set_ts_request.is_new = true;
        set_ts_request.next_state = STATE_WAIT_AIRN_CLOSE;
        sprintf(out, "triggered TS ON event\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        set_ts_request.is_new = true;
        set_ts_request.next_state = STATE_IDLE;
        sprintf(out, "triggered TS OFF event\r\n");
    } else if (argc < 2) {
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
    } else if (argc < 2) {
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
    for (uint8_t i = 0; i < N_COMMANDS - 3; i++) {
        sprintf(out + strlen(out), "- %s\r\n", cli_bms.cmds.names[i]);
    }
}

void _cli_imd(uint16_t argc, char **argv, char *out) {
    sprintf(
        out,
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
            sprintf(
                out,
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
            sprintf(out, "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
            return;
        }
        *divider_index = '\0';

        if (strlen(divider_index + 1) % 2 != 0) {
            sprintf(out, "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
            return;
        }

        CAN_TxHeaderTypeDef header = {
            .StdId = strtoul(argv[2], NULL, 16), .DLC = strlen(divider_index + 1) / 2, .ExtId = 0, .RTR = 0, .IDE = 0};
        *((uint64_t *)payload) = (uint64_t)strtoull(divider_index + 1, NULL, 16);
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
    feedback_feed_t f[FEEDBACK_N];
    feedback_get_all_states(f);
    *out = '\0';
    for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
        sprintf(
            out + strlen(out),
            "%02d - %-40s: %s, %.4f, %s\n\r",
            i,
            feedback_names[i],
            f[i].real_state == FEEDBACK_STATE_H   ? "1"
            : f[i].real_state == FEEDBACK_STATE_L ? "0"
                                             : "E",
            f[i].voltage,
            f[i].cur_state == FEEDBACK_STATE_H   ? "1"
            : f[i].cur_state == FEEDBACK_STATE_L ? "0"
                                             : "E");
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
        sprintf(out, "\r\n");
    } else {
        sprintf(out, "^C\r\n");
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
            sprintf(watch_buf + strlen(watch_buf), "%s ", argv[i]);
        }
        cli_watch_execute_cmd            = 1;
        watch_buf[strlen(watch_buf) - 1] = '\0';
        __HAL_TIM_SetAutoreload(&HTIM_CLI, TIM_MS_TO_TICKS(&HTIM_CLI, interval));
        __HAL_TIM_CLEAR_IT(&HTIM_CLI, TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(&HTIM_CLI);
    }

    sprintf(out, "\033[2J\033H");
}

void _cli_timer_handler(TIM_HandleTypeDef *htim) {
    if (cli_watch_flush_tx == 0)
        cli_watch_execute_cmd = 1;
    cli_watch_flush_tx = 1;
}

char tx_buf[4096]   = {'\0'};
char print_buf[256] = {'\0'};
char *save_ptr      = NULL;
void cli_watch_flush_handler() {
    char *argv[BUF_SIZE] = {NULL};
    uint16_t argc;
    char *to_print;

    if (cli_watch_flush_tx == 0)
        return;

    if (cli_watch_execute_cmd == 1) {
        sprintf(
            tx_buf,
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
                sprintf(tx_buf, "Command not found\r\n");
                *watch_buf = '\0';
                HAL_TIM_Base_Stop_IT(&HTIM_CLI);
                HAL_UART_Transmit(&CLI_UART, (uint8_t *)tx_buf, strlen(tx_buf), 100);
                return;
            }
        }

        sprintf(tx_buf + strlen(tx_buf), "\r\nPress CTRL+C to stop\r\n\033[J");

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

uint8_t * bms_get_cellboard_distribution() {
    return (uint8_t *)config_get(&cellboard_distribution);
}
void bms_set_cellboard_distribution(uint8_t distribution[static 6]) {
    config_set(&cellboard_distribution, (void *)distribution);
    config_write(&cellboard_distribution);
}
void _cli_cellboard_distribution(uint16_t argc, char **argv, char *out) {
    *out = '\0';
    if (argc == 1) {
        uint8_t *distr = bms_get_cellboard_distribution();
        for (uint8_t i = 0; i < CELLBOARD_COUNT; ++i) {
            sprintf(out + strlen(out), "%u ", distr[i]);
        }
        sprintf(out + strlen(out), "\r\n");
    } else if (argc == 7) {
        uint8_t distr[CELLBOARD_COUNT];
        for (uint8_t i = 1; i < argc; ++i) {
            uint8_t pos = atoi(argv[i]);
            if (pos < 0 || pos > 5) {
                sprintf(out + strlen(out), "Index out of range: %u\r\n", pos);
                return;
            }

            for (uint8_t j = 0; j < i - 1; ++j) {
                if (pos == distr[j]) {
                    sprintf(out + strlen(out), "Non puoi ripetere la stessa cella piu' di una volta\r\n");
                    return;
                }
            }

            distr[i - 1] = pos;
        }

        bms_set_cellboard_distribution(distr);
        sprintf(out + strlen(out), "Distribuzione delle cellboard aggiornata con successo\r\n");
    } else {
        sprintf(
            out,
            "Invalid sintax\r\n"
            "Available commands are:\r\n"
            "\t-<empty>: show the current cell distribution\r\n"
            "\t-<cellboard index at p0> ... <cellboard index at p5>: set a new cellboard distribution\r\n");
    }
}

void _cli_fans(uint16_t argc, char **argv, char *out) {
    if (argc == 2) {
        if (strcmp(argv[1], "auto") == 0) {
            fans_set_override(false);
            sprintf(out, "Fans speed set to auto\r\n");
        }
        else if (strcmp(argv[1], "off") == 0) {
            fans_set_override(true);
            fans_set_speed(0);
            sprintf(out, "Fans turned off\r\n");
        } else {
            uint8_t perc = atoi(argv[1]);
            if (perc <= 100) {
                fans_set_override(true);
                fans_set_speed(perc / 100.0);
                sprintf(out, "Fans speed set to %d\r\n", perc);
            } else {
                sprintf(out, "Invalid perc value: %d\r\nIt must be between 0 and 100", perc);
            }
        }
    } else {
        sprintf(out, "Invalid sintax.\r\n Use this way: fans <perc>\r\n");
    }
}

void _cli_pack(uint16_t argc, char **argv, char *out) {
    if (argc == 1) {
        sprintf(out,
            "AIR- status:      %s\r\n"
            "AIR+ status:      %s\r\n"
            "Precharge status: %s\r\n"
            "Fault status:     %s\r\n",
            (pack_get_airn_off() == AIRN_ON_VALUE ? "closed" : "open"),
            (pack_get_airp_off() == AIRP_ON_VALUE ? "closed" : "open"),
            (pack_get_precharge() == PRECHARGE_ON_VALUE ? "on" : "off"),
            (pack_get_fault() == BMS_FAULT_ON_VALUE ? "on" : "off"));
    }
    else if (argc == 3) {
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
        else if (strcmp(argv[1], "fault") == 0) {
            if (strcmp(argv[2], "on") == 0)
                value = BMS_FAULT_ON_VALUE;
            else
                value = BMS_FAULT_OFF_VALUE;
            pack_set_fault(value);
        }

        sprintf(out, "%s set %s\r\n", argv[1], argv[2]);
    } else if (argc == 2) {
    }
}