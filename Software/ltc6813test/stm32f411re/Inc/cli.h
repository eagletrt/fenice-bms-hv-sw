#ifndef CLI_H
#define CLI_H
#include <stdbool.h>
#include "fsm.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_ll_usart.h"

#define BUF_SIZE 255
#define N_COMMANDS 5

typedef char *cli_state_func_t(state_global_data_t *data, BMS_STATE_T state);

typedef struct buffer_t {
	char buffer[BUF_SIZE];
	uint8_t index;
	bool complete;
} buffer_t;

typedef struct cli_t {
	UART_HandleTypeDef *uart;

	buffer_t rx;

	cli_state_func_t *states[N_COMMANDS];
} cli_t;

void cli_init(cli_t *cli, UART_HandleTypeDef *uart);
void cli_loop(cli_t *cli, state_global_data_t *data, BMS_STATE_T state);
void cli_char_receive(cli_t *cli);
#endif