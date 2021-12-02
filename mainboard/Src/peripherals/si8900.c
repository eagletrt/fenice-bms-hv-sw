/**
 * @file		si8900.c
 * @brief		This file contains the functions to read bus and total pack
 * 				voltages
 *
 * @date		Gen 09, 2020
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@tutanota.com]
 */

#include "peripherals/si8900.h"

#include "error/error.h"
#include "mainboard_config.h"
#include "usart.h"

#include <stdlib.h>
#include <string.h>

bool si8900_ready = false;

bool si8900_init(UART_HandleTypeDef *huart, GPIO_TypeDef *reset_gpio, uint16_t reset_pin) {
    uint8_t recv = 0;
    uint8_t tx   = 0xAA;

    bool timeout      = false;
    bool code_receive = false;
    bool code_confirm = false;

    HAL_GPIO_WritePin(ADC_SIN_GPIO_Port, ADC_SIN_Pin, GPIO_PIN_SET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(ADC_SIN_GPIO_Port, ADC_SIN_Pin, GPIO_PIN_RESET);

    uint32_t time = HAL_GetTick();

    HAL_UART_Transmit(huart, &tx, 1, 10);

    while ((!code_receive || !code_confirm) && !timeout) {
        HAL_UART_Receive(huart, &recv, 1, 10);
        if (recv == 0x55) {
            if (code_receive) {
                code_confirm = true;
                si8900_ready = true;
            }
            code_receive = true;
        } else {
            code_receive = false;
            code_confirm = false;
        }
        HAL_UART_Transmit(huart, &tx, 1, 10);

        timeout = (HAL_GetTick() - time) >= SI8900_INIT_TIMEOUT;
    }

    if (timeout) {
        error_set(ERROR_ADC_INIT, 0, HAL_GetTick());
        return false;
    }
    error_reset(ERROR_ADC_INIT, 0);
    return true;
}

bool si8900_read_channel(UART_HandleTypeDef *huart, SI8900_CHANNEL ch, uint16_t *voltage) {
    if (si8900_ready) {
        uint8_t conf = si8900_cnfg_0 | (ch << 4);

        HAL_UART_Transmit(huart, &conf, 1, 1);

        uint8_t recv, count = 0;
        while (recv != conf) {
            HAL_UART_Receive(huart, &recv, 1, 1);
            if (count++ > 5) {
                error_set(ERROR_ADC_TIMEOUT, 0, HAL_GetTick());
                return false;
            }
        }

        uint8_t data[2] = {0};
        HAL_UART_Receive(huart, data, 2, 1);
        *voltage = si8900_convert_voltage(data);

        error_reset(ERROR_ADC_TIMEOUT, 0);
        return true;
    }
    return false;
}

uint16_t si8900_convert_voltage(uint8_t adc_hl[2]) {
    // MSB | LSB
    uint16_t dig = ((adc_hl[0] & 0b00001111) << 6) | ((adc_hl[1] & 0b01111110) >> 1);

    return ((SI8900_VREF * (float)dig) / 1024) * 100;
}