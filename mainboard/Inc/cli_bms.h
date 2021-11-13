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

extern cli_t cli_bms;

void cli_bms_init();

/**
 * @brief Print messages in the cli if dmesg is enabled
 */
void cli_bms_debug(char *text, size_t length);

#endif