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

enum ltc6813_i2c_ctrl {
    I2C_READ             = 1,
    I2C_WRITE            = 0,
    I2C_START            = 0b01100000,
    I2C_STOP             = 0b00010000,
    I2C_BLANK            = 0b00000000,
    I2C_NO_TRANSMIT      = 0b01110000,
    I2C_MASTER_ACK       = 0b00000000,
    I2C_MASTER_NACK      = 0b00001000,
    I2C_MASTER_NACK_STOP = 0b00001001
};

size_t ltc6813_read_voltages(SPI_HandleTypeDef *hspi, voltage_t *volts) {
    uint8_t cmd[4];
    uint16_t cmd_pec;
    uint8_t data[8];  //[8 * CELLBOARD_COUNT];

    cmd[0] = 0;  // Broadcast

    uint8_t count = 0;  // counts the cells
    for (uint8_t reg = 0; reg < LTC6813_REG_COUNT; reg++) {
        cmd[1]  = (uint8_t)rdcv_cmd[reg];
        cmd_pec = ltc6813_pec15(2, cmd);
        cmd[2]  = (uint8_t)(cmd_pec >> 8);
        cmd[3]  = (uint8_t)(cmd_pec);

        ltc6813_wakeup_idle(hspi);

        ltc6813_enable_cs(hspi);
        if (HAL_SPI_Transmit(hspi, cmd, 4, 100) != HAL_OK) {
            // goto End;
        }
        HAL_SPI_Receive(hspi, data, LTC6813_REG_CELL_COUNT * 2 + 2, 400);

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
                    ltc6813_convert_voltage(&data[sizeof(voltage_t) * cell]);  //&data[sizeof(voltage_t) * cell]);
                //ltc6813_check_voltage(volts[index], index);

                count++;
            }
        } else {
            ERROR_SET(ERROR_LTC_COMM);
        }
        //}
    }

    return count;
}

void ltc6813_build_dcc(uint16_t indexes[], uint16_t size, uint8_t cfgar[8], uint8_t cfgbr[8]) {
    for (uint16_t i = 0; i < size; i++) {
        if (indexes[i] < 8) {
            cfgar[4] |= dcc[indexes[i]];
        } else if (indexes[i] >= 8 && indexes[i] < 12) {
            cfgar[5] |= dcc[indexes[i]];
        } else if (indexes[i] >= 12 && indexes[i] < 16) {
            cfgbr[0] |= dcc[indexes[i]];
        } else if (indexes[i] >= 16 && indexes[i] < 18) {
            cfgbr[1] |= dcc[indexes[i]];
        }
    }

    uint16_t pec = ltc6813_pec15(6, cfgar);
    cfgar[6]     = (uint8_t)(pec >> 8);
    cfgar[7]     = (uint8_t)(pec);

    pec      = ltc6813_pec15(6, cfgbr);
    cfgbr[6] = (uint8_t)(pec >> 8);
    cfgbr[7] = (uint8_t)(pec);
}

void ltc6813_set_balancing(SPI_HandleTypeDef *hspi, uint16_t *indexes, uint16_t size, int dcto) {
    uint8_t cfgar[8] = {0};
    uint8_t cfgbr[8] = {0};

    // cfgbr[i][1] += 0b00001000; // set DTMEN
    cfgar[0] += 0b00000010;  // set DTEN
    cfgar[5] += dcto << 4;   // Set timer
    ltc6813_build_dcc(indexes, size, cfgar, cfgbr);

    ltc6813_wakeup_idle(hspi);

    ltc6813_wrcfg(hspi, WRCFGA, cfgar);
    ltc6813_wrcfg(hspi, WRCFGB, cfgbr);
}

uint16_t ltc6813_convert_voltage(uint8_t v_data[]) {
    return v_data[0] + (v_data[1] << 8);
}
