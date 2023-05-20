/**
 * @file cli_bms.c
 * @brief cli instance for bms
 *
 * @date Mar 29,2020
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Simone Ruffini [simone.ruffini@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@stduenti.unitn.it]
 */

#include "cli_bms.h"

#include <string.h>

#include "can/lib/bms/c/network.h"

#include "bms_fsm.h"
#include "bal_fsm.h"
#include "error.h"
#include "imd.h"

#include "pack/internal_voltage.h"
#include "pack/cell_voltage.h"
#include "pack/temperature.h"
#include "pack/current.h"

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

cli_command_func_t * commands[] = {
    [CLI_COMMAND_VOLTS]                  = &_cli_volts,
    [CLI_COMMAND_TEMPS]                  = &_cli_temps,
    [CLI_COMMAND_STATUS]                 = &_cli_status,
    [CLI_COMMAND_BALANCE]                = &_cli_balance,
    [CLI_COMMAND_SOC]                    = &_cli_soc,
    [CLI_COMMAND_ERRORS]                 = &_cli_errors,
    [CLI_COMMAND_TS]                     = &_cli_ts,
    [CLI_COMMAND_CURRENT]                = &_cli_current,
    [CLI_COMMAND_DMESG]                  = &_cli_dmesg,
    [CLI_COMMAND_RESET]                  = &_cli_reset,
    [CLI_COMMAND_IMD]                    = &_cli_imd,
    [CLI_COMMAND_CAN_FORWARD]            = &_cli_can_forward,
    [CLI_COMMAND_FEEDBACKS]              = &_cli_feedbacks,
    [CLI_COMMAND_WATCH]                  = &_cli_watch,
    [CLI_COMMAND_CELLBOARD_DISTRIBUTION] = &_cli_cellboard_distribution,
    [CLI_COMMAND_FANS]                   = &_cli_fans,
    [CLI_COMMAND_PACK]                   = &_cli_pack,
    [CLI_COMMAND_HELP]                   = &_cli_help,
    [CLI_COMMAND_SIGTERM]                = &_cli_sigterm,
    [CLI_COMMAND_TABA]                   = &_cli_taba,
    [CLI_COMMAND_SBORAT]                 = &_cli_sborat
};

const char * bms_state_names[] = {
    [BMS_IDLE]      = "idle",
    [BMS_PRECHARGE] = "precharge",
    [BMS_ON]        = "run",
    [BMS_FAULT]     = "fault"
};

const char * bal_state_names[] = {
    [BAL_OFF]       = "off",
    [BAL_COMPUTE]   = "computing",
    [BAL_DISCHARGE] = "discharging",
    [BAL_COOLDOWN]  = "cooldown"
};

const char * error_names[] = {
    // Cells
    [ERROR_CELL_LOW_VOLTAGE]      = "low-voltage",
    [ERROR_CELL_UNDER_VOLTAGE]    = "under-voltage",
    [ERROR_CELL_OVER_VOLTAGE]     = "over-voltage",
    [ERROR_CELL_HIGH_TEMPERATURE] = "high-temperature",
    [ERROR_CELL_OVER_TEMPERATURE] = "over-temperature",

    // Internal
    [ERROR_OVER_CURRENT]          = "over-current",
    [ERROR_INT_VOLTAGE_MISMATCH]  = "internal voltage mismatch",
    
    // COMMs
    [ERROR_CAN]                   = "CAN",
    [ERROR_BMS_MONITOR_COMM]      = "BMS monitor communication",
    [ERROR_EEPROM_COMM]           = "EEPROM communication",
    [ERROR_ADC_COMM]              = "ADC communication",

    // EEPROM
    [ERROR_EEPROM_WRITE]          = "EEPROM write",

    // Other
    [ERROR_FEEDBACK]              = "feedback",
    [ERROR_FEEDBACK_CIRCUITRY]    = "feedback_circuitry"
};

const char * feedback_names[] = {
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
    [FEEDBACK_SD_END_POS]                   = "FEEDBACK_SD_END"
};

const char * imd_state_names[] = {
    [IMD_SC]            = "IMD_SHORT_CIRCUIT",
    [IMD_NORMAL]        = "IMD_NORMAL",
    [IMD_UNDER_VOLTAGE] = "IMD_UNDER_VOLTAGE",
    [IMD_START_MEASURE] = "IMD_START_MEASURE",
    [IMD_DEVICE_ERROR]  = "IMD_DEVICE_ERROR",
    [IMD_EARTH_FAULT]   = "IMD_EARTH_FAULT"
};

const char * command_names[] = {
    [CLI_COMMAND_VOLTS]                  = "volt",
    [CLI_COMMAND_TEMPS]                  = "temp",
    [CLI_COMMAND_STATUS]                 = "status",
    [CLI_COMMAND_BALANCE]                = "bal",
    [CLI_COMMAND_SOC]                    = "soc",
    [CLI_COMMAND_ERRORS]                 = "errors",
    [CLI_COMMAND_TS]                     = "ts",
    [CLI_COMMAND_CURRENT]                = "current",
    [CLI_COMMAND_DMESG]                  = "dmesg",
    [CLI_COMMAND_RESET]                  = "reset",
    [CLI_COMMAND_IMD]                    = "imd",
    [CLI_COMMAND_CAN_FORWARD]            = "can_forward",
    [CLI_COMMAND_FEEDBACKS]              = "feedbacks",
    [CLI_COMMAND_WATCH]                  = "watch",
    [CLI_COMMAND_CELLBOARD_DISTRIBUTION] = "cell_distr",
    [CLI_COMMAND_FANS]                   = "fans",
    [CLI_COMMAND_PACK]                   = "pack",
    [CLI_COMMAND_HELP]                   = "?",
    [CLI_COMMAND_SIGTERM]                = "\003",
    [CLI_COMMAND_TABA]                   = "\ta",
    [CLI_COMMAND_SBORAT]                 = "sbor@"
};

cli_t cli_bms;
bool dmesg_enabled = true;

char watch_buf[BUF_SIZE] = { '\0' };
uint8_t cli_watch_flush_tx = 0;
uint8_t cli_watch_execute_cmd = 0;
char tx_buf[4096] = { '\0' };
char print_buf[256] = { '\0' };
char * save_ptr = NULL;


/**
 * @brief Print all voltages to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_volts_all(uint16_t argc, char ** argv, char * out) {
    out[0] = '\0';
    // Read cell voltages
    voltage_t cells[CELL_COUNT];
    size_t max_index, min_index;
    float voltage, total_power = 0.0f, segment_sum = 0.0f;

    // Get voltage values
    cell_voltage_measure(cells);
    cell_voltage_get_min(&min_index);
    cell_voltage_get_max(&max_index);
    
    for (size_t i = 0; i < CELL_COUNT; i++) {
        voltage = (float)cells[i] / 10000;

        // Print each voltage value
        if (i % LTC_CELL_COUNT == 0) {
            if (i != 0) {
                sprintf(out + strlen(out), "Segment total: %.2fV", segment_sum);
                segment_sum = 0;
            }
            sprintf(out + strlen(out), "\r\n%-3d", i / LTC_CELL_COUNT);
        } else if (i % (LTC_CELL_COUNT / 3) == 0 && i > 0) {
            sprintf(out + strlen(out), "\r\n%-3s", "");
        }
        segment_sum += voltage;

        if (fsm_get_state(bal.fsm) == BAL_OFF) {
            if (i == max_index)
                sprintf(out + strlen(out), RED_BG("[%3u %-.3fV]") " ", i, voltage);
            else if (i == min_index)
                sprintf(out + strlen(out), CYAN_BG("[%3u %-.3fV]") " ", i, voltage);
            else
                sprintf(out + strlen(out), "[%3u %-.3fV] ", i, voltage);
        } else {
            if (i == max_index) {
                if (CANLIB_BITTEST(bal.cells[i / CELLBOARD_CELL_COUNT], i % CELLBOARD_CELL_COUNT)) {
                    total_power += voltage * voltage;
                    sprintf(
                        out + strlen(out),
                        RED_BG_ON_YELLOW_FG("[%3u %-.3fV %.2fW]") " ",
                        i,
                        voltage,
                        DISCHARGE_DUTY_CYCLE * voltage * voltage / DISCHARGE_R);
                } else {
                    sprintf(out + strlen(out), RED_BG("[%3u %-.3fV 0.00W]") " ", i, voltage);
                }
            } else if (CANLIB_BITTEST(bal.cells[i / CELLBOARD_CELL_COUNT], i % CELLBOARD_CELL_COUNT)) {
                total_power += voltage * voltage;
                sprintf(
                    out + strlen(out),
                    YELLOW_BG("[%3u %-.3fV %.2fW]") " ",
                    i,
                    voltage,
                    DISCHARGE_DUTY_CYCLE * voltage * voltage / DISCHARGE_R);
            } else if (i == min_index) {
                sprintf(out + strlen(out), CYAN_BG("[%3u %-.3fV 0.00W]") " ", i, voltage);
            } else {
                sprintf(out + strlen(out), "[%3u %-.3fV 0.00W] ", i, voltage);
            }
        }
    }
    sprintf(out + strlen(out), "Segment total: %.2fV", segment_sum);
    total_power /= DISCHARGE_R;

    if (fsm_get_state(bal.fsm) != BAL_OFF) {
        voltage = (float)cells[max_index] / 10000;
        float max_soc = 0.0862 * voltage * voltage * voltage - 1.4260 * voltage * voltage + 6.0088 * voltage - 6.551;
        voltage = ((float)cells[min_index] + bal_get_threshold() - 10) / 10000;
        float target_soc = 0.0862 * voltage * voltage * voltage - 1.4260 * voltage * voltage + 6.0088 * voltage - 6.551;

        float ETA = ((target_soc - max_soc) * CELL_CAPACITY * 4) / ((float)cells[max_index] / 10000 / DISCHARGE_R);

        sprintf(out + strlen(out), "\r\nETA: %.1fh\r\n", ETA / DISCHARGE_DUTY_CYCLE);
        sprintf(out + strlen(out), "Pwr: %3.1fW\r\n", total_power * DISCHARGE_DUTY_CYCLE);
    } else
        sprintf(out + strlen(out), "\r\n");
}
/**
 * @brief Print voltages information to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
// TODO: Check voltage values to print
void _cli_volts(uint16_t argc, char ** argv, char * out) {
    if (strcmp(argv[1], "") == 0) {
        voltage_t min = cell_voltage_get_min(NULL);
        voltage_t max = cell_voltage_get_max(NULL);
        sprintf(
            out,
            "vts.........%.2f V\r\n"
            "vbat........%.2f V\r\n"
            "shunt.......%.2f V\r\n"
            "tsn.........%.2f V\r\n"
            "max.........%.3f V\r\n"
            "min.........%.3f V\r\n"
            "average.....%.2f V\r\n"
            "delta.......%.3f V\r\n",
            internal_voltage_get_vts(),
            internal_voltage_get_vbat(),
            internal_voltage_get_shunt(),
            internal_voltage_get_tsn(),
            (float)max / 10000,
            (float)min / 10000,
            cell_voltage_get_avg(),
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
/**
 * @brief Print all temperatures information to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_temps_all(uint16_t argc, char ** argv, char * out) {
    out[0]                  = '\0';
    temperature_t temp_all[PACK_TEMP_COUNT] = { 0 };
    // TODO: Handle errors
    cell_temperature_measure(temp_all);

    for (uint8_t i = 0; i < PACK_TEMP_COUNT; i++) {
        if (i % TEMP_SENSOR_COUNT == 0)
            sprintf(out + strlen(out), "\r\n%-3d", i / TEMP_SENSOR_COUNT);
        else if (i % (TEMP_SENSOR_COUNT / 4) == 0 && i > 0)
            sprintf(out + strlen(out), "\r\n%-3s", "");
        sprintf(out + strlen(out), "[%3u %2u °C] ", i, (uint8_t)(temp_all[i] / 2.56f - 20f));
    }

    sprintf(out + strlen(out), "\r\n");
}
/**
 * @brief Print temperatures information to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_temps(uint16_t argc, char ** argv, char * out) {
    if (strcmp(argv[1], "") == 0) {
        float max = temperature_get_max(),
              min = temperature_get_min();
        sprintf(
            out,
            "average.....%.2f °C\r\nmax.........%.2f "
            "C\r\nmin.........%.2f °C\r\n"
            "delta.......%.2f °C\r\n",
            temperature_get_avg() / 10f,
            max / 2.56f - 20f,
            min / 2.56f - 20f,
            (max - min) / 2.56f - 20f);
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
/**
 * @brief Print the status of the BMS to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_status(uint16_t argc, char ** argv, char * out) {
#define n_items 3

    char thresh[5] = { '\0' };
    itoa((float)bal_get_threshold() / 10, thresh, 10);

    char er_count[3] = { '\0' };
    itoa(error_count(), er_count, 10);

    const char * values[n_items][2] = {
        { "BMS state", bms_state_names[fsm_get_state(bms.fsm)] },
        { "error count", er_count },
        { "balancing state", bal_state_names[fsm_get_state(bal.fsm)] }
    };

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
/**
 * @brief Control balancing behaviour from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_balance(uint16_t argc, char ** argv, char * out) {
    if (strcmp(argv[1], "on") == 0) {
        if (argc > 2)
            bal.target = atoi(argv[2]);
        fsm_trigger_event(bal.fsm, EV_BAL_START);
        sprintf(out, "enabling balancing\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        fsm_trigger_event(bal.fsm, EV_BAL_STOP);
        sprintf(out, "disabling balancing\r\n");
    } else if (strcmp(argv[1], "thr") == 0) {
        if (argv[2] != NULL) {
            voltage_t thresh = atoff(argv[2]) * 10;
            if (thresh <= BAL_MAX_VOLTAGE_THRESHOLD)
                bal_set_threshold(thresh);
        }
        sprintf(out, "balancing threshold is %.2f mV\r\n", bal_get_threshold() / 10.0);
    } else if (strcmp(argv[1], "test") == 0) {
        CAN_TxHeaderTypeDef tx_header;
        uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];
        uint8_t board;
        bms_BalancingCells cells = bms_BalancingCells_DEFAULT;

        tx_header.ExtId = 0;
        tx_header.IDE   = CAN_ID_STD;
        tx_header.RTR   = CAN_RTR_DATA;
        tx_header.StdId = bms_id_BALANCING;

        board = atoi(argv[2]);

        if (argc < 4) {
            sprintf(out, "wrong number of parameters\r\n");
            return;
        }

        sprintf(out, "testing cells [");

        uint8_t c;
        for (uint8_t i = 3; i < argc; ++i) {
            c = atoi(argv[i]);
            CANLIB_BITSET(cells, c);
            sprintf(out + strlen(out), "%u,", c);
        }

        tx_header.DLC = bms_BALANCING_SIZE;
        bms_serialize_BALANCING(buffer, cells, board);
        // TODO: Remove internal CAN communication
        can_send(&BMS_CAN, buffer, &tx_header);

        sprintf(out + strlen(out) - 1, "]\r\non board %d\r\n", board);
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
/**
 * @brief Print info about the State Of Charge (SOC) to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_soc(uint16_t argc, char ** argv, char * out) {
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
/**
 * @brief Print information about errors to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of argumens
 * @param out The output message
 */
// TODO: Replace old error functions
void _cli_errors(uint16_t argc, char ** argv, char * out) {
    out[0] = '\0';
    size_t count = error_count();
    error_t errors[100];
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
}
/**
 * @brief Control TS status from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_ts(uint16_t argc, char ** argv, char * out) {
    if (strcmp(argv[1], "on") == 0) {
        fsm_trigger_event(bms.fsm, BMS_EV_TS_ON);
        sprintf(out, "triggered TS ON event\r\n");
    } else if (strcmp(argv[1], "off") == 0) {
        fsm_trigger_event(bms.fsm, BMS_EV_TS_OFF);
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
/**
 * @brief Print current information to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The arrays of arguments
 * @param out The output message
 */
void _cli_current(uint16_t argc, char ** argv, char * out) {
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
/**
 * @brief Print "dmesg" status to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_dmesg(uint16_t argc, char **argv, char *out) {
    dmesg_enabled = !dmesg_enabled;
    sprintf(out, "dmesg output is %s\r\n", dmesg_enabled ? "enabled" : "disabled");
}
/**
 * @brief Reset the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output array
 */
void _cli_reset(uint16_t argc, char ** argv, char * out) {
    HAL_NVIC_SystemReset();
}
/**
 * @brief GET TABARATED!!!
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_taba(uint16_t argc, char ** argv, char * out) {
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
/**
 * @brief Print CLI usage to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_help(uint16_t argc, char ** argv, char * out) {
    sprintf(out, "command list:\r\n");
    for (size_t i = 0; i < N_COMMANDS - 3; i++)
        sprintf(out + strlen(out), "- %s\r\n", cli_bms.cmds.names[i]);
}
/**
 * @brief Print IMD information to the CLI
 * 
 * @param argc The number of argument
 * @param argv The array of argument
 * @param out The output message
 */
void _cli_imd(uint16_t argc, char ** argv, char * out) {
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
/**
 * @brief Send CAN data from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_can_forward(uint16_t argc, char ** argv, char * out) {
    if (argc != 3) {
        sprintf(
            out,
            "Invalid command format\r\n\n"
            "valid format:\r\n"
            "can_forward <network> <id>#<payload>\r\n");
        return;
    }

    char * divider_index = strchr(argv[2], '#');
    uint8_t payload[CAN_MAX_PAYLOAD_LENGTH] = { 0 };

    if (divider_index == NULL) {
        sprintf(out, "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
        return;
    }
    divider_index[0] = '\0';
    if (strlen(divider_index + 1) % 2 != 0) {
        sprintf(out, "Errore di formattazione: ID#PAYLOAD\n\rPAYLOAD is hex encoded and is MSB...LSB\r\n");
        return;
    }

    CAN_TxHeaderTypeDef header = {
        .StdId = strtoul(argv[2], NULL, 16),
        .DLC = strlen(divider_index + 1) / 2,
        .ExtId = 0,
        .RTR = 0,
        .IDE = 0
    
    };
    *((uint64_t *)payload) = (uint64_t)strtoull(divider_index + 1, NULL, 16);
    can_send(&CAR_CAN, payload, &header);
    out[0] = '\0';
}
/**
 * @brief Print feedbacks information to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_feedbacks(uint16_t argc, char ** argv, char * out) {
    feedback_feed_t f[FEEDBACK_N];
    feedback_get_feedback_states(f);
    out[0] = '\0';
    for (size_t i = 0; i < FEEDBACK_N; ++i) {
        sprintf(
            out + strlen(out),
            "%02d - %s: %s, %.4f\n\r",
            i,
            feedback_names[i],
            f[i].state == FEEDBACK_STATE_H ? "1" : (
                f[i].state == FEEDBACK_STATE_L ? "0" : "E"),
            f[i].voltage
        );
    }
}
/**
 * @brief Something went 'sborato'!!
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_sborat(uint16_t argc, char ** argv, char * out) {
    BUZ_sborati(&HTIM_PWM);
    out[0] = '\0';
}
/**
 * @brief Send an interrupt signal (SIGTERM, Ctrl^C) to the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out 
 */
void _cli_sigterm(uint16_t argc, char ** argv, char * out) {
    if (HTIM_CLI.State == HAL_TIM_STATE_BUSY) {
        HAL_TIM_Base_Stop_IT(&HTIM_CLI);
        watch_buf[0] = '\0';
        sprintf(out, "\r\n");
    } else
        sprintf(out, "^C\r\n");
}
/**
 * @brief Control status of the timer from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_watch(uint16_t argc, char ** argv, char * out) {
    watch_buf[0] = '\0';
    if (!strcmp(argv[1], "stop"))
        HAL_TIM_Base_Stop_IT(&HTIM_CLI);
    else {
        uint16_t interval = atoi(argv[1]);
        if (interval == 0)
            interval = 500;
        for (size_t i = 2; i < argc; ++i)
            sprintf(watch_buf + strlen(watch_buf), "%s ", argv[i]);
        
        cli_watch_execute_cmd = 1;
        watch_buf[strlen(watch_buf) - 1] = '\0';
        __HAL_TIM_SetAutoreload(&HTIM_CLI, TIM_MS_TO_TICKS(&HTIM_CLI, interval));
        __HAL_TIM_CLEAR_IT(&HTIM_CLI, TIM_IT_UPDATE);
        HAL_TIM_Base_Start_IT(&HTIM_CLI);
    }

    sprintf(out, "\033[2J\033H");
}
/**
 * @brief Control cellboard distribution from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_cellboard_distribution(uint16_t argc, char ** argv, char * out) {
    out[0] = '\0';
    if (argc == 1) {
        uint8_t * distr = bms_get_cellboard_distribution();
        for (size_t i = 0; i < CELLBOARD_COUNT; ++i)
            sprintf(out + strlen(out), "%u ", distr[i]);
        sprintf(out + strlen(out), "\r\n");
    } else if (argc == 7) {
        uint8_t distr[CELLBOARD_COUNT];
        for (size_t i = 1; i < argc; ++i) {
            uint8_t pos = atoi(argv[i]);
            if (pos < 0 || pos > 5) {
                sprintf(out + strlen(out), "Index out of range: %u\r\n", pos);
                return;
            }

            for (size_t j = 0; j < i - 1; ++j) {
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
/**
 * @brief Control the fans from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_fans(uint16_t argc, char ** argv, char * out) {
    if (argc == 2) {
        if (!strcmp(argv[1], "off")) {
            fans_set_speed(0);
            sprintf(out, "Fans turned off\r\n");
        } else {
            uint8_t perc = atoi(argv[1]);
            if (perc <= 100) {
                fans_set_speed(perc / 100.0);
                sprintf(out, "Fans speed set to %d\r\n", perc);
            } else
                sprintf(out, "Invalid perc value: %d\r\nIt must be between 0 and 100", perc);
        }
    } else
        sprintf(out, "Invalid sintax.\r\n Use this way: fans <perc>\r\n");
}
/**
 * @brief Control the battery pack from the CLI
 * 
 * @param argc The number of arguments
 * @param argv The array of arguments
 * @param out The output message
 */
void _cli_pack(uint16_t argc, char ** argv, char * out) {
    if (argc == 3) {
        uint8_t value;
        if (!strcmp(argv[1], "airn")) {
            if (!strcmp(argv[2], "on"))
                value = AIRN_ON_VALUE;
            else
                value = AIRN_OFF_VALUE;
            pack_set_airn_off(value);
        } else if (!strcmp(argv[1], "airp")) {
            if (!strcmp(argv[2], "on"))
                value = AIRP_ON_VALUE;
            else
                value = AIRP_OFF_VALUE;
            pack_set_airp_off(value);
        } else if (!strcmp(argv[1], "precharge")) {
            if (!strcmp(argv[2], "on"))
                value = PRECHARGE_ON_VALUE;
            else
                value = PRECHARGE_OFF_VALUE;
            pack_set_precharge(value);
        }

        sprintf(out, "%s setted %s\r\n", argv[1], argv[2]);
    }
}

void cli_bms_init() {
    cli_bms.uart           = &UART_CLI;
    cli_bms.cmds.functions = commands;
    cli_bms.cmds.names     = command_names;
    cli_bms.cmds.count     = CLI_COMMAND_COUNT;

    char init[94] = { '\0' };
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
void cli_bms_debug(char * text, size_t length) {
    if (dmesg_enabled) {
        char out[300] = { '\0' };
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
void _cli_timer_handler(TIM_HandleTypeDef * htim) {
    if (cli_watch_flush_tx == 0)
        cli_watch_execute_cmd = 1;
    cli_watch_flush_tx = 1;
}
void cli_watch_flush_handler() {
    char * argv[BUF_SIZE] = { NULL };
    uint16_t argc;
    char * to_print;

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
        for (size_t i = 0; i < N_COMMANDS; i++) {
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
        for (size_t i = 0; i < argc - 1; ++i)
            watch_buf[strlen(watch_buf)] = ' ';
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
