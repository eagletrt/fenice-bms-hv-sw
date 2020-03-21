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
#include <inttypes.h>
#include <stdbool.h>

#include "common/list.h"
#include "fsm.h"
#include "stm32g4xx_hal.h"

#define BUF_SIZE 64
#define HISTORY_LENGTH 100
#define N_COMMANDS 9
#define PS_SIZE 2

typedef void cli_state_func_t(char *cmd, state_global_data_t *data,
							  BMS_STATE_T state, char *out);

typedef struct buffer_t {
	uint8_t index;
	char buffer[BUF_SIZE];
} buffer_t;

typedef struct cli_t {
	UART_HandleTypeDef *uart;

	char input_buf;	 // Input byte

	node_t *buffers;  // list of input buffers

	uint8_t history_index;

	bool receive;	   // True if a byte has been received
	bool complete;	   // True if the current command has been \n'd
	uint8_t escaping;  // index at which escaping started

	cli_state_func_t *states[N_COMMANDS];
} cli_t;

static const char ps[PS_SIZE] = "> ";
extern cli_t cli;

void cli_buf_init(buffer_t *buf);
void cli_init(cli_t *cli, UART_HandleTypeDef *uart);
void cli_print(char *text, size_t length);
void cli_loop(state_global_data_t *data, BMS_STATE_T state);
void cli_handle_interrupt();
void cli_char_receive();
#endif