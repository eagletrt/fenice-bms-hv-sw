/**
 * @file		cli.h
 * @brief		This file contains the functions to create the cli
 *
 * @date		Oct 24, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef CLI_H
#define CLI_H
#include <stdbool.h>
#include "fsm.h"
#include "stm32g4xx_hal.h"

#define BUF_SIZE 255
#define HISTORY_LENGTH 100
#define N_COMMANDS 9
#define PS1_SIZE 2
typedef void cli_state_func_t(char *cmd, state_global_data_t *data,
							  BMS_STATE_T state, char *out);

typedef struct buffer_t {
	uint8_t index;
	char *buffer;
} buffer_t;

typedef struct history_t {
	buffer_t *list;
	uint8_t index;
	uint8_t showing;

} history_t;

typedef struct cli_t {
	UART_HandleTypeDef *uart;

	buffer_t rx;
	bool complete;
	bool echo;
	uint8_t escaping;

	history_t history;

	cli_state_func_t *states[N_COMMANDS];
} cli_t;

static const char ps1[PS1_SIZE] = "> ";
extern cli_t cli;

void cli_init(cli_t *cli, UART_HandleTypeDef *uart);
void cli_print(char *text, size_t length);
void cli_loop(state_global_data_t *data, BMS_STATE_T state);
void cli_char_receive();
#endif