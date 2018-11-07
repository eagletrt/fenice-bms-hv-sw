/*
 * ltc1380.c

 *
 *  Created on: 30 ott 2018
 *      Author: Utente
 */
#include "ltc1380.h"

typedef struct
{
	int8_t address_byte;
	int8_t command_byte;
} LTC1380_COMMAND_TYPE;

// Commands an LTC1380 mux to connect one channel to its output
void LTC1380_Set_Channel(int8_t  board_num, int8_t mux_num, int8_t channel_num)
{
    LTC1380_COMMAND_TYPE command;

    if((NUM_MUXES <= mux_num) ||
       (LTC1380_NUM_CHANNELS <= channel_num))
    {
        return;
    }

    // Build command to control the mux
    command.address_byte = (LTC1380_BASE_ADDRESS | mux_num) << 1;
    command.command_byte = LTC1380_EN_BIT | channel_num;

    // Send command to control the mux
   // LTC1380_CONFIG_I2C_WRITE(board_num, &command, sizeof(command), LTC1380_BAUD_RATE);
    LTC6804s_wrcomm(SPI_HandleTypeDef *hspi1);
    // Wait for the channel to connect
    delay_us(LTC1380_TON);

    return;
}

// Commands an LTC1380 mux to disconnect all channels from its output
void LTC1380_All_Off(int8_t board_num, int8_t mux_num)
{
    LTC1380_COMMAND_TYPE command;

    if(LTC1380_NUM_CHANNELS <= mux_num)
    {
        return;
    }

    // Build command to control the mux
    command.address_byte = (LTC1380_BASE_ADDRESS | mux_num) << 1;
    command.command_byte = ~LTC1380_EN_BIT;

    // Send command to control the mux
    LTC1380_CONFIG_I2C_WRITE(board_num, &command, sizeof(command), LTC1380_BAUD_RATE);

    return;
}

