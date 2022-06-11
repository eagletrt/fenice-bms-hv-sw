/**
 * @file	adc124s021.h
 * @brief	adc124s021 communication interface
 *
 * @date	Mar 12, 2022
 * @author	Federico Carb0ne [federico.carbone@studenti.unitn.it]
 */

#include "main.h"
#include "pack/voltage.h"

#define ADC124_BUS_CHANNEL      ADC124S_CH2
#define ADC124_INTERNAL_CHANNEL ADC124S_CH3
#define ADC124_SHUNT_CHANNEL    ADC124S_CH4

typedef enum { ADC124S_CH1 = 0x00, ADC124S_CH2, ADC124S_CH3, ADC124S_CH4 } ADC124S021_CH;

voltage_t adc124S021_read_channel(SPI_HandleTypeDef *spi, ADC124S021_CH channel);

bool adc124s021_read_channels(SPI_HandleTypeDef *spi, ADC124S021_CH *channels, uint8_t ch_number, float *data_out);