/**
 * @file		adctemp.c
 * @brief   A library for reading ADC128D818 ADC Temperatures from NTC termistor
 *
 * @date    Jul 7, 2021
 * @author  Francesco Osti [francesco.osti-1@studenti.unitn.it]
 */

/* Includes ------------------------------------------------------------------*/
#include "adctemp.h"

void ADCTEMP_powerOn_Bank(uint8_t bank) {
    if (bank & 0x01)
        HAL_GPIO_WritePin(ADCTEMP_GPIO_PORT_BANK_0, ADCTEMP_GPIO_PIN_BANK_0, GPIO_PIN_SET);
    if (bank & 0x02)
        HAL_GPIO_WritePin(ADCTEMP_GPIO_PORT_BANK_1, ADCTEMP_GPIO_PIN_BANK_1, GPIO_PIN_SET);
}

void ADCTEMP_powerOff_Bank(uint8_t bank) {
    if (bank & 0x01)
        HAL_GPIO_WritePin(ADCTEMP_GPIO_PORT_BANK_0, ADCTEMP_GPIO_PIN_BANK_0, GPIO_PIN_RESET);
    if (bank & 0x02)
        HAL_GPIO_WritePin(ADCTEMP_GPIO_PORT_BANK_1, ADCTEMP_GPIO_PIN_BANK_1, GPIO_PIN_RESET);
}

ADCTEMP_StateTypeDef ADCTEMP_init_ADC(I2C_HandleTypeDef *interface, uint8_t address, uint8_t monitoring_mode) {
    //turn on ADC bank if is not done before
    ADCTEMP_powerOn_Bank((address > ADCTEMP_CELL_2_ADR) ? ADCTEMP_BANK_0 : ADCTEMP_BANK_1);
    //ADCTEMP_powerOn_Bank(ADCTEMP_BANK_ALL);

    uint8_t DataReg[1];

    //reset configuration delete old junk
    DataReg[0] = 0x80U;
    HAL_I2C_Mem_Write(interface, address, 0x00U, 1, DataReg, 1U, 10U);

    //check if ADC is ready
    HAL_I2C_Mem_Read(interface, address, 0x0CU, 1, DataReg, 1U, 10U);
    if (DataReg[0] && 0x02U)
        return ADCTEMP_STATE_NOT_READY;

    //disable IN6 input that is unutilized
    DataReg[0] = 0x40U;
    HAL_I2C_Mem_Write(interface, address, 0x08U, 1, DataReg, 1U, 10U);

    //interrupt masking: no masking, all input can cause interrupt
    DataReg[0] = 0x00U;
    HAL_I2C_Mem_Write(interface, address, 0x03U, 1, DataReg, 1U, 10U);

    //set conversion mode LOWPOWER or CONTINIOUS
    DataReg[0] = (monitoring_mode >> 1) && 0x01U;
    HAL_I2C_Mem_Write(interface, address, 0x07U, 1, DataReg, 1U, 10U);

    //select Mode 00 (7 input + internal temperature sensor) and external reference voltage
    DataReg[0] = 0x01U;
    HAL_I2C_Mem_Write(interface, address, 0x0BU, 1, DataReg, 1U, 10U);

    //enable interrupt pin and start acquisition if monitoring_mode is ENABLED
    DataReg[0] = 0x02U + (monitoring_mode && 0x01U);
    HAL_I2C_Mem_Write(interface, address, 0x00U, 1, DataReg, 1U, 10U);

    //check if device is present
    HAL_I2C_Mem_Read(interface, address, 0x3EU, 1, DataReg, 1U, 10U);
    return (DataReg[0] == 0x01U) ? ADCTEMP_STATE_OK : ADCTEMP_STATE_ERROR;
}

ADCTEMP_StateTypeDef ADCTEMP_start_Conversion(I2C_HandleTypeDef *interface, uint8_t address) {
    uint8_t DataReg[1];

    HAL_I2C_Mem_Read(interface, address, 0x0CU, 1, DataReg, 1U, 10U);
    if (DataReg[0] && 0x01U)
        return ADCTEMP_STATE_BUSY;

    DataReg[0] = 0x01U;
    HAL_I2C_Mem_Write(interface, address, 0x09U, 1, DataReg, 1U, 10U);
    return ADCTEMP_STATE_OK;
}

ADCTEMP_StateTypeDef ADCTEMP_is_Busy(I2C_HandleTypeDef *interface, uint8_t address) {
    uint8_t DataReg[1];

    HAL_I2C_Mem_Read(interface, address, 0x0CU, 1, DataReg, 1U, 10U);
    if (DataReg[0] && 0x01U)
        return ADCTEMP_STATE_BUSY;
    else
        return ADCTEMP_STATE_OK;
}

ADCTEMP_StateTypeDef ADCTEMP_read_Raw(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, uint16_t *out) {
    uint8_t Buffer[2] = {0};
    if (HAL_I2C_Mem_Read(interface, address, sensor + 0x20U, 1U, (uint8_t *)Buffer, 2U, 10U) != HAL_OK) {
        return ADCTEMP_STATE_ERROR;
    }

    if (sensor == ADCTEMP_INTERNAL_TEMP_REG)
        *out = (((int16_t)((int8_t)Buffer[0])) << 1) + (((uint16_t)Buffer[1]) >> 7);
    else
        *out = (((uint16_t)Buffer[0]) << 4) + (((uint16_t)Buffer[1]) >> 4);

    return ADCTEMP_STATE_OK;
}

//conversion formula constants see jupyter-notebook file
//these are specific to the choice of the termistor and bias resistor
#define ADCTEMP_CONST_a 1.2702004494185367e+02
#define ADCTEMP_CONST_b -8.522206763788798e-02
#define ADCTEMP_CONST_c 3.134789783702983e-05
#define ADCTEMP_CONST_d -6.014808784849526e-09
#define ADCTEMP_CONST_e 3.012146376392016e-13

ADCTEMP_StateTypeDef ADCTEMP_read_Temp(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, float *temp) {
    uint16_t rawData           = 0;
    ADCTEMP_StateTypeDef state = ADCTEMP_read_Raw(interface, address, sensor, &rawData);
    if (state != ADCTEMP_STATE_OK) {
        return state;
    }

    //internal termperature conversion
    if (sensor == ADCTEMP_INTERNAL_TEMP_REG) {
        *temp = (((int16_t)rawData) * 0.5F);
    } else {
        //conversion formula see jupyter-notebook file
        float val  = rawData * 1.0;
        float val2 = val * val;
        float val3 = val2 * val;
        float val4 = val2 * val2;
        *temp =
            (float)(ADCTEMP_CONST_a + ADCTEMP_CONST_b * val + ADCTEMP_CONST_c * val2 + ADCTEMP_CONST_d * val3 + ADCTEMP_CONST_e * val4);
    }

    return ADCTEMP_STATE_OK;
}

void ADCTEMP_set_Upper_Limit(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, uint8_t limit) {
    uint8_t DataReg[1];
    DataReg[0] = limit;
    HAL_I2C_Mem_Write(interface, address, 0x2AU + (sensor << 1), 1, DataReg, 1U, 10U);
}

void ADCTEMP_set_Lower_Limit(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, uint8_t limit) {
    uint8_t DataReg[1];
    DataReg[0] = limit;
    HAL_I2C_Mem_Write(interface, address, 0x2AU + (sensor << 1) + 1, 1, DataReg, 1U, 10U);
}

uint8_t ADCTEMP_read_Interrupt_Status(I2C_HandleTypeDef *interface, uint8_t address) {
    uint8_t Buffer[1];
    HAL_I2C_Mem_Read(interface, address, 0x01U, 1U, (uint8_t *)Buffer, 1U, 10U);
    return Buffer[0];
}

#define ADCTEMP_CONST_INV_a 219.4338943886704
#define ADCTEMP_CONST_INV_b -1.6740551743831218
#define ADCTEMP_CONST_INV_c -0.03838410999621445
#define ADCTEMP_CONST_INV_d 0.0005527605291832662
#define ADCTEMP_CONST_INV_e -1.9277343480702197e-06

uint8_t _ADCTEMP_temp_to_int(float val) {
    float val2 = val * val;
    float val3 = val * val2;
    float val4 = val2 * val2;

    return (
        uint8_t)(ADCTEMP_CONST_INV_a + ADCTEMP_CONST_INV_b * val + ADCTEMP_CONST_INV_c * val2 + ADCTEMP_CONST_INV_d * val3 + ADCTEMP_CONST_INV_e * val4);
}

void ADCTEMP_set_Temperature_Limit(
    I2C_HandleTypeDef *interface,
    uint8_t address,
    uint8_t sensor,
    float high_limit,
    float low_limit) {
    if (sensor == ADCTEMP_INTERNAL_TEMP_REG) {
        ADCTEMP_set_Upper_Limit(interface, address, ADCTEMP_INTERNAL_TEMP_REG, (int)high_limit);
        ADCTEMP_set_Lower_Limit(interface, address, ADCTEMP_INTERNAL_TEMP_REG, (int)low_limit);
    } else {
        //high and low are inverted beacause voltage decrease when temperature increase
        ADCTEMP_set_Upper_Limit(interface, address, sensor, _ADCTEMP_temp_to_int(low_limit));
        ADCTEMP_set_Lower_Limit(interface, address, sensor, _ADCTEMP_temp_to_int(high_limit));
    }
}
