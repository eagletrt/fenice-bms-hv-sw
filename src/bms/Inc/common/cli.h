/**
 * @file		cli.h
 * @brief		This file contains the functions to create the cli
 *
 * @date		Oct 24, 2019
 *
 * @author	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author	Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef CLI_H
#define CLI_H
#include <inttypes.h>
#include <stdbool.h>

#include "common/llist.h"
#include "stm32g4xx_hal.h"

#define BUF_SIZE 64
#define PS_SIZE 2
// TODO: Enforce this limit
#define HISTORY_MAX_SIZE 100

typedef void cli_command_func_t(char *cmd, char *out);

typedef struct buffer {
	uint8_t index;
	char buffer[BUF_SIZE];
} buffer_t;

typedef struct commands {
	cli_command_func_t **functions;	 // Array of pointers to command functions
	char **names;					 // Array of command names
	uint8_t count;					 // Number of commands
} commands_t;

typedef struct cli_t {
	UART_HandleTypeDef *uart;

	char input_buf;	 // Input byte

	node_t *history;		  // stream (history)
	node_t *current_history;  // Currently "selected" history item

	buffer_t current_command;

	bool receive;	   // True if a byte has been received
	bool complete;	   // True if the current command has been \n'd
	uint8_t escaping;  // index at which escaping started

	commands_t commands;
} cli_t;

static const char *bool_names[2] = {"false", "true"};
static const char cli_ps[PS_SIZE] = "> ";

void cli_buf_init(buffer_t *buf);
void cli_init(cli_t *cli);
void cli_print(cli_t *cli, char *text, size_t length);
void cli_loop(cli_t *cli);
void cli_handle_interrupt(cli_t *cli);
void cli_char_receive(cli_t *cli);
#endif