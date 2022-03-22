/**
 * @file		ltc6813_utils.c
 * @brief		This file contains utilities for improving LTC6813
 * 				communications
 *
 * @date		Nov 16, 2019
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "peripherals/ltc6813_utils.h"

#include "main.h"

#include <math.h>

size_t ltc6813_read_voltages(SPI_HandleTypeDef *hspi, voltage_t *volts) {
    uint8_t cmd[4];
    uint16_t cmd_pec;
    uint8_t data[8];  //[8 * CELLBOARD_COUNT];

    cmd[0] = 0;  // Broadcast

    if(ltc6813_poll_convertion(hspi, 10) == HAL_TIMEOUT) {
        ERROR_SET(ERROR_LTC_COMM);
        return 0;
    }

    uint8_t count = 0;  // counts the cells
    for (uint8_t reg = 0; reg < LTC6813_REG_COUNT; reg++) {
        cmd[1]  = (uint8_t)rdcv_cmd[reg];
        cmd_pec = ltc6813_pec15(2, cmd);
        cmd[2]  = (uint8_t)(cmd_pec >> 8);
        cmd[3]  = (uint8_t)(cmd_pec);

        ltc6813_wakeup_idle(hspi);

        ltc6813_enable_cs(hspi);
        if (HAL_SPI_Transmit(hspi, cmd, 4, 10) != HAL_OK) {
            ERROR_SET(ERROR_LTC_COMM);
            return 0;
        }
        HAL_SPI_Receive(hspi, data, LTC6813_REG_CELL_COUNT * 2 + 2, 100);

        ltc6813_disable_cs(hspi);

#if LTC6813_EMU == 1
        // Writes 3.6v to each cell

        uint8_t emu_i;
        for (emu_i = 0; emu_i < LTC6813_REG_CELL_COUNT * 2; emu_i++) {
            // 36000
            data[emu_i]   = 0b10100000;
            data[++emu_i] = 0b10001100;
        }
        uint16_t emu_pec = ltc6813_pec15(6, data);
        data[6]          = (uint8_t)(emu_pec >> 8);
        data[7]          = (uint8_t)emu_pec;
#endif

        //for (uint8_t ltc = 0; ltc < CELLBOARD_COUNT; ltc++) {
        if (ltc6813_pec15(6, data) == (uint16_t)(data[6] * 256 + data[7])) {
            ERROR_UNSET(ERROR_LTC_COMM);

            // For every cell in the register
            for (uint8_t cell = 0; cell < LTC6813_REG_CELL_COUNT; cell++) {
                // offset by register (3 slots) + cell //+ offset by ltc (18 slots)
                uint16_t index = (reg * LTC6813_REG_CELL_COUNT) + cell;  //+(ltc * CELLBOARD_CELL_COUNT) ;
                
                volts[index] = 
                    ltc6813_convert_voltage(data + (sizeof(voltage_t) * cell));  //&data[sizeof(voltage_t) * cell]);
                //ltc6813_check_voltage(volts[index], index);
            
                // Check if measured voltage is above max voltage or under min voltage
                if(volts[index] > CELL_MAX_VOLTAGE || volts[index] < CELL_MIN_VOLTAGE){
                    ERROR_SET(ERROR_LTC_COMM);
                    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
                }
                
                count++;
            }
        } else {
            ERROR_SET(ERROR_LTC_COMM);
        }
        //}
    }

    return count;
}

void ltc6813_build_dcc(bms_balancing_cells cells, uint8_t cfgar[8], uint8_t cfgbr[8]) {
    for(uint8_t i = 0; i < LTC6813_CELL_COUNT; ++i) {
        if(getBit(cells, i)) {
            if (i < 8) {
                cfgar[4] |= dcc[i];
            } else if (i >= 8 && i < 12) {
                cfgar[5] |= dcc[i];
            } else if (i >= 12 && i < 16) {
                cfgbr[0] |= dcc[i];
            } else if (i >= 16 && i < 18) {
                cfgbr[1] |= dcc[i];
            }
        }
    }

    uint16_t pec = ltc6813_pec15(6, cfgar);
    cfgar[6]     = (uint8_t)(pec >> 8);
    cfgar[7]     = (uint8_t)(pec);

    pec      = ltc6813_pec15(6, cfgbr);
    cfgbr[6] = (uint8_t)(pec >> 8);
    cfgbr[7] = (uint8_t)(pec);
}

void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, bms_balancing_cells cells, int dcto) {
    uint8_t cfgar[8] = {0};
    uint8_t cfgbr[8] = {0};

    // cfgbr[i][1] += 0b00001000; // set DTMEN
    cfgar[0] += 0b00000010;  // set DTEN
    cfgar[5] += dcto << 4;   // Set timer
    ltc6813_build_dcc(cells, cfgar, cfgbr);

    ltc6813_wakeup_idle(hspi);

    ltc6813_wrcfg(hspi, WRCFGA, cfgar);
    ltc6813_wrcfg(hspi, WRCFGB, cfgbr);
}