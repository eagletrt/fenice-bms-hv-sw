/**
 * @file		ltc6813_utils.h
 * @brief		This file contains utilities for improving LTC6813
 * 				communications
 *
 * @date		Nov 16, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */
#pragma once

#include "can_comms.h"
#include "cellboard_config.h"
#include "ltc6813.h"
#include "bms_network.h"

#include <inttypes.h>
#include <main.h>

/**
 * @brief	Polls all the registers of the LTC6813 and updates the cell array
 * @details It executes multiple rdcv requests to the LTCs and saves the values
 * 					in the voltage variable of the CELL_Ts.
 *
 * 					1     CMD0    8     CMD1      16      32
 * 					|- - - - - - -|- - - - - - - -|- ... -|
 * 					1 0 0 0 0 0 0 0 0 0 0 0 X X X X  PEC
 * 					 Address |             |  Reg  |
 * 					  (BRD)
 *
 * @param	spi		The SPI configuration structure
 */
size_t ltc6813_read_voltages(SPI_HandleTypeDef *hspi, voltage_t *volts);

///**
// * @brief		Checks that voltage is between its thresholds.
// *
// * @param		volts		The voltage
// * @param		error		The error return code
// */
//void ltc6813_check_voltage(voltage_t volts, uint8_t index);
//
///**
// * @brief		Checks that temperature is between its thresholds.
// *
// * @param		temp		The temperature
// * @param		error		The error return code
// */
//void ltc6813_check_temperature(uint16_t temps, uint8_t index);

void ltc6813_build_dcc(uint32_t cells, uint8_t cfgar[8], uint8_t cfgbr[8]);
void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, uint32_t cells, int dcto);

/**
 * @brief	This function is used to convert the 2 byte raw data from the LTC68xx to a 16 bit unsigned integer
 *
 * @param 	v_data	Raw data bytes
 *
 * @returns	Voltage [mV]
 */
#define ltc6813_convert_voltage(v_data) (*((voltage_t *)(v_data)))
