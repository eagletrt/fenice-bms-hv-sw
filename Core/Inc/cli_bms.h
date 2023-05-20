/**
 * @file cli_bms.h
 * @brief cli instance for bms
 *
 * @date Mar 29,2020
 *
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Simone Ruffini [simone.ruffini@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@stduenti.unitn.it]
 */

#ifndef CLI_BMS_H
#define CLI_BMS_H

#include "bms_config.h"

#include "micro-libs/cli/cli.h"

#define NORMAL_COLOR           "\033[0m"
#define RED_BG(S)              "\033[0;41m" S NORMAL_COLOR
#define YELLOW_BG(S)           "\033[0;43m" S NORMAL_COLOR
#define CYAN_BG(S)             "\033[0;46m" S NORMAL_COLOR
#define RED_BG_ON_YELLOW_FG(S) "\033[0;31;43m" S NORMAL_COLOR

typedef enum {
    CLI_COMMAND_VOLTS,
    CLI_COMMAND_TEMPS,
    CLI_COMMAND_STATUS,
    CLI_COMMAND_BALANCE,
    CLI_COMMAND_SOC,
    CLI_COMMAND_ERRORS,
    CLI_COMMAND_TS,
    CLI_COMMAND_CURRENT,
    CLI_COMMAND_DMESG,
    CLI_COMMAND_RESET,
    CLI_COMMAND_IMD,
    CLI_COMMAND_CAN_FORWARD,
    CLI_COMMAND_FEEDBACKS,
    CLI_COMMAND_WATCH,
    CLI_COMMAND_CELLBOARD_DISTRIBUTION,
    CLI_COMMAND_FANS,
    CLI_COMMAND_PACK,
    CLI_COMMAND_HELP,
    CLI_COMMAND_SIGTERM,
    CLI_COMMAND_TABA,
    CLI_COMMAND_SBORAT,
    CLI_COMMAND_COUNT
} CLI_COMMAND;

extern cli_t cli_bms;

/** @brief Initialize the BMS Command Line Interface */
void cli_bms_init();
/**
 * @brief Print messages in the cli if dmesg is enabled 
 * @details For debugging purposes
 */
void cli_bms_debug(char * text, size_t length);
/**
 * @brief Handler function for the CLI timer
 * 
 * @param htim The timer instance
 */
void _cli_timer_handler(TIM_HandleTypeDef * htim);
/** @brief Run selected command passing all the arguments */
void cli_watch_flush_handler();

#endif