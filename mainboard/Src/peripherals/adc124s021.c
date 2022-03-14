#include "adc124s021.h"

#define ADC124S021_VREF 3.33f
#define ADC124S021_CONV_VALUE_TO_VOLTAGE_T(x) (100*(x)*ADC124S021_VREF/4096)

void _adc124s021_cs_enable(SPI_HandleTypeDef *spi, GPIO_TypeDef *port, uint16_t pin) {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    while(spi->State != HAL_SPI_STATE_READY);
}

void _adc124s021_cs_disable(SPI_HandleTypeDef *spi, GPIO_TypeDef *port, uint16_t pin) {
    while(spi->State != HAL_SPI_STATE_READY);
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

voltage_t adc124S021_read_channel(SPI_HandleTypeDef *spi, ADC124S021_CH channel) {
    uint8_t cmd[4];
    uint16_t rcv = 0;
    cmd[1] = cmd[2] = cmd[3] = 0xFF;
    cmd[0] = channel << 3;

    _adc124s021_cs_enable(spi, CS_ADC_GPIO_Port, CS_ADC_Pin);
    HAL_SPI_TransmitReceive(spi, cmd, cmd, 4, 100);
    _adc124s021_cs_disable(spi, CS_ADC_GPIO_Port, CS_ADC_Pin);

    rcv |= (uint16_t)cmd[2] << 8;
    rcv |= cmd[3];

    return ADC124S021_CONV_VALUE_TO_VOLTAGE_T(rcv);
}

bool adc124s021_read_channels(SPI_HandleTypeDef *spi, ADC124S021_CH *channels, uint8_t ch_number, voltage_t *data_out) {
    uint8_t cmd[10], i;
    uint16_t rcv;
    cmd[1] = cmd[3] = cmd[5] = cmd[7] = cmd[8] = cmd[9] = 0XFF;

    if(ch_number > 4) return false;

    for(i = 0; i<ch_number; ++i) {
        cmd[i*2] = channels[i] << 3;
    }

    _adc124s021_cs_enable(spi, CS_ADC_GPIO_Port, CS_ADC_Pin);
    HAL_SPI_TransmitReceive(spi, cmd, cmd, ch_number*2 + 2, 100);
    _adc124s021_cs_disable(spi, CS_ADC_GPIO_Port, CS_ADC_Pin);

    for(i = 1; i<=ch_number; ++i) {
        rcv = 0;
        rcv |= (uint16_t)cmd[i*2] << 8;
        rcv |= cmd[i*2 + 1];
        data_out[i-1] = ADC124S021_CONV_VALUE_TO_VOLTAGE_T(rcv);
    }

    return true;
}