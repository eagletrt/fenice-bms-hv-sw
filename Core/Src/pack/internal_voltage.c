/**
 * @file internal_voltage.c
 * @brief Functions to manage internal voltages in the pack
 * 
 * @date Apr 01, 2023
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#include "mainboard_config.h"
#include "pack/internal_voltage.h"
#include "peripherals/max22530.h"

struct internal_voltage {
    float vts;
    float vbat;
    float shunt;
    float tsn;
};

// Singleton
static struct internal_voltage voltage;

void internal_voltage_init() {
    voltage.vts   = 0;
    voltage.vbat  = 0;
    voltage.shunt = 0;
    voltage.tsn   = 0;
}

HAL_StatusTypeDef internal_voltage_measure() {
    MAX22530_CH channels[] = {
        MAX22530_VTS_CHANNEL,
        MAX22530_VBATT_CHANNEL,
        MAX22530_SHUNT_CHANNEL,
        MAX22530_TSN_CHANNEL
    };

    float voltages[4];
    HAL_StatusTypeDef status = max22530_read_channels(&SPI_MAX22530,
        ADC_CS_GPIO_Port,
        ADC_CS_Pin,
        channels,
        4,
        voltages);

    if (status == HAL_OK) {
        voltage.vts   = INTERNAL_VOLTAGE_ADC_TO_VOLTAGE(voltages[0]);
        voltage.vbat  = INTERNAL_VOLTAGE_ADC_TO_VOLTAGE(voltages[1]);
        voltage.shunt = INTERNAL_VOLTAGE_ADC_TO_VOLTAGE(voltages[2]);
        voltage.tsn   = INTERNAL_VOLTAGE_ADC_TO_VOLTAGE(voltages[3]);
    }
    return status;
}

// TODO: Add errors (SCS?)
float internal_voltage_get_vts() {
    return voltage.vts;
}
float internal_voltage_get_vbat() {
    return voltage.vbat;
}
float internal_voltage_get_shunt() {
    return voltage.shunt;
}
float internal_voltage_get_tsn() {
    return voltage.tsn;
}
