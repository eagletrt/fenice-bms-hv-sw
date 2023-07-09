/**
 * @file		cli_bms.h
 * @brief		cli instance for bms	
 *
 * @date		Mar 29,2020
 * 
 * @author      Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author      Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef CLI_BMS_H
#define CLI_BMS_H

#include "stm32f4xx_hal.h"
#include "cli.h"
#include "config.h"

// Special characters
#define SIGTERM "\003"
#define ESCAPE "\033"

#define STR_PREFIX ESCAPE "["
#define STR_INFIX ";"
#define STR_SUFFIX "M"

#define NORMAL "0"

// Foreground
#define BLACK_FG   "30"
#define RED_FG     "31"
#define GREEN_FG   "32"
#define YELLOW_FG  "33"
#define BLUE_FG    "34"
#define MAGENTA_FG "35"
#define CYAN_FG    "36"
#define WHITE_FG   "37"

// Background
#define BLACK_BG   "40"
#define RED_BG     "41"
#define GREEN_BG   "42"
#define YELLOW_BG  "43"
#define BLUE_BG    "44"
#define MAGENTA_BG "45"
#define CYAN_BG    "46"
#define WHITE_BG   "47"

// Text style
#define BOLD "1"
#define DULL "2"
#define UNDERLINE "4"

/** @brief Concat style and color into a string that will be interpreted by the terminal */
#define MAGIC(STYLE, COLOR) STR_PREFIX STYLE STR_INFIX COLOR STR_SUFFIX

/** @brief Apply terminal color to a string */
#define COL(STRING, COLOR) MAGIC(NORMAL, COLOR) STRING MAGIC(NORMAL, NORMAL)
/** @brief Apply terminal sytle to a string */
#define STYLE(STRING, STYLE) MAGIC(STYLE, NORMAL) STRING MAGIC(NORMAL, NORMAL)
/** @brief Apply terminal color and style to a string */
#define STYLE_AND_COL(STRING, STYLE, COLOR) MAGIC(STYLE, COLOR) STRING MAGIC(NORMAL, NORMAL)

typedef enum {
    CLI_VOLT,
    CLI_TEMP,
    CLI_STATUS,
    CLI_BAL,
    CLI_SOC,
    CLI_ERROR,
    CLI_TS,
    CLI_CURRENT,
    CLI_DMESG,
    CLI_RESET,
    CLI_IMD,
    CLI_CAN,
    CLI_FEEDBACK,
    CLI_WATCH,
    CLI_DISTR,
    CLI_FANS,
    CLI_PACK,
    CLI_HELP,
    CLI_SIGTERM,
    CLI_TABA,
    CLI_SBORAT,

    CLI_COMMAND_COUNT
} CLI_COMMAND;

typedef enum {
    CLI_HELP_NAME,
    CLI_HELP_SYNOPSIS,
    CLI_HELP_DESCRIPTION,

    CLI_HELP_SECTION_COUNT
} CLI_HELP_SECTION;


/** @brief Initialize the BMS CLI*/
void cli_bms_init();

/**
 * @brief Print debug messages to the CLI
 * @details If dmesg option is disabled no messages are printed
 * 
 * @param text The string to print
 */
void cli_bms_debug(char * text);

void _cli_timer_handler(TIM_HandleTypeDef * htim);
void cli_watch_flush_handler();

#endif