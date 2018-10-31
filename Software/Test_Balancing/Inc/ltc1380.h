/*
 * ltc1380.h
 *
 *  Created on: 30 ott 2018
 *      Author: Utente
 */


#include "stm32f4xx_hal.h"
#include <stdlib.h>

#ifndef LTC1380_H_
#define LTC1380_H_

#define LTC1380_NUM_CHANNELS            8   //! Num addresses and channels from datasheet Description on page 1
#define LTC1380_BAUD_RATE               100 //! in kHz, Max SMBus Operating Frequency (fSMB from datasheet page 3)

// I2C address and format from datasheet page 8
#define LTC1380_BASE_ADDRESS               0x48

#define NUM_MUXES               2           // The number of LTC1380 Muxes on a cellboard
// command byte format from datasheet table 2
#define LTC1380_EN_BIT                      0x08
#define LTC1380_CHANNEL_MASK                0x07

// Timing characteristics from datasheet page 3
#define LTC1380_TON                         2        // in us, max value




//! Commands an LTC1380 mux to connect one channel to its output.
//! @return void
void LTC1380_Set_Channel(int8_t board_num,            //!< The logical address for the PCB containing this LTC1380 IC.
                         int8_t mux_num,              //!< The number for the LTC1380 IC, must be less than LTC1380_CONFIG_NUM_ICS_PER_ADDRESS.
						 int8_t channel_num           //!< The channel number to set for the LTC1380 IC.
                         );

//! Commands an LTC1380 mux to disconnect all channels from its output.
//! @return void
void LTC1380_All_Off(int8_t board_num,                //!< The logical address for the PCB containing this LTC1380 IC.,
		             int8_t mux_num                   //!< The number for the LTC1380 IC, must be less than LTC1380_CONFIG_NUM_ICS_PER_ADDRESS.
                     );

#endif /* LTC1380_H_ */
