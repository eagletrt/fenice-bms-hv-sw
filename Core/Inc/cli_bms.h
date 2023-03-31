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

#include "cli.h"
#include "fenice_config.h"

#define CLI_UART huart1

#define NORMAL_COLOR           "\033[0m"
#define RED_BG(S)              "\033[0;41m" S NORMAL_COLOR
#define YELLOW_BG(S)           "\033[0;43m" S NORMAL_COLOR
#define CYAN_BG(S)             "\033[0;46m" S NORMAL_COLOR
#define RED_BG_ON_YELLOW_FG(S) "\033[0;31;43m" S NORMAL_COLOR

extern cli_t cli_bms;

void cli_bms_init();

/**
 * @brief Print messages in the cli if dmesg is enabled
 */
void cli_bms_debug(char *text, size_t length);
void _cli_timer_handler(TIM_HandleTypeDef *htim);
void cli_watch_flush_handler();

#endif