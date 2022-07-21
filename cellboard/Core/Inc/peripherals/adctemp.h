/**
 * @file		adctemp.h
 * @brief   A library for reading ADC128D818 ADC Temperatures from NTC termistor
 *
 * @date    Jul 7, 2021
 * @author  Francesco Osti [francesco.osti-1@studenti.unitn.it]
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ADCTEMP_H
#define ADCTEMP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define ADCTEMP_GPIO_PORT_BANK_0 GPIOC  ///< PORT for enable of bank 0 power
#define ADCTEMP_GPIO_PORT_BANK_1 GPIOC  ///< PORT for enable of bank 0 power
#define ADCTEMP_GPIO_PORT_INT    GPIOA  ///< PORT for enable of bank 0 power

#define ADCTEMP_GPIO_PIN_BANK_0 GPIO_PIN_15  ///< GPIO for enable of bank 0 power
#define ADCTEMP_GPIO_PIN_BANK_1 GPIO_PIN_14  ///< GPIO for enable of bank 1 power
#define ADCTEMP_GPIO_PIN_INT    GPIO_PIN_8   ///< GPIO for interrupt

typedef enum {
    ADCTEMP_STATE_ERROR     = 0xFFU,  ///< error in initialization
    ADCTEMP_STATE_OK        = 0x00U,  ///< initialization is OK
    ADCTEMP_STATE_BUSY      = 0x01U,  ///< ADC is converting an input
    ADCTEMP_STATE_NOT_READY = 0x02U   ///< ADC on startup, initialization not available

} ADCTEMP_StateTypeDef;

#define ADCTEMP_MONITORING_LOWPOWER   0x01U  ///< enable monitoring in low power mode (~750ms for reading all channels)
#define ADCTEMP_MONITORING_CONTINIOUS 0x03U  ///< enable monitoring in continious mode (~12ms per channel)
#define ADCTEMP_MONITORING_DISABLED   0x00U  ///< disable monitoring, measurement only on request

//definition of ADC adresses
#define ADCTEMP_CELL_1_ADR 0x1DU << 1  ///< address of ADC 0 (J14, BANK_0)
#define ADCTEMP_CELL_2_ADR 0x1EU << 1  ///< address of ADC 1 (J13, BANK_0)
#define ADCTEMP_CELL_3_ADR 0x1FU << 1  ///< address of ADC 2 (J15, BANK_0)
#define ADCTEMP_CELL_4_ADR 0x2DU << 1  ///< address of ADC 3 (J11, BANK_1)
#define ADCTEMP_CELL_5_ADR 0x2EU << 1  ///< address of ADC 4 (J12, BANK_1)
#define ADCTEMP_CELL_6_ADR 0x2FU << 1  ///< address of ADC 5 (J16, BANK_1)

//definition of ADC bank
#define ADCTEMP_BANK_0   0x01U  ///< bank 0 (J14, J13, J15)
#define ADCTEMP_BANK_1   0x02U  ///< bank 1 (J11, J12, J16)
#define ADCTEMP_BANK_ALL 0x03U  ///< all the ADC

//definition of internal temperature sensor and ADC input register
#define ADCTEMP_INPUT_1_REG       0x00U  ///< input 1 of ADC register (numerated as in PCB)
#define ADCTEMP_INPUT_2_REG       0x01U  ///< input 2 of ADC register (numerated as in PCB)
#define ADCTEMP_INPUT_3_REG       0x02U  ///< input 3 of ADC register (numerated as in PCB)
#define ADCTEMP_INPUT_4_REG       0x03U  ///< input 4 of ADC register (numerated as in PCB)
#define ADCTEMP_INPUT_5_REG       0x04U  ///< input 5 of ADC register (numerated as in PCB)
#define ADCTEMP_INPUT_6_REG       0x05U  ///< input 6 of ADC register (numerated as in PCB)
#define ADCTEMP_INTERNAL_TEMP_REG 0x07U  ///< internal temperature sensor register

//definition of internal temperature sensor and ADC input interrupt status mask
#define ADCTEMP_INPUT_1_INT       0x01U  ///< input 1 of ADC interrupt mask (numerated as in PCB)
#define ADCTEMP_INPUT_2_INT       0x02U  ///< input 2 of ADC interrupt mask  (numerated as in PCB)
#define ADCTEMP_INPUT_3_INT       0x04U  ///< input 3 of ADC interrupt mask  (numerated as in PCB)
#define ADCTEMP_INPUT_4_INT       0x08U  ///< input 4 of ADC interrupt mask  (numerated as in PCB)
#define ADCTEMP_INPUT_5_INT       0x10U  ///< input 5 of ADC interrupt mask  (numerated as in PCB)
#define ADCTEMP_INPUT_6_INT       0x20U  ///< input 6 of ADC interrupt mask  (numerated as in PCB)
#define ADCTEMP_INTERNAL_TEMP_INT 0x80U  ///< internal temperature sensor interrupt mask

/*!
   \brief turn on the selected bank
   \param bank (ADCTEMP_BANK_*)
*/
void ADCTEMP_powerOn_Bank(uint8_t bank);

/*!
   \brief turn off the selected bank
   \param bank (ADCTEMP_BANK_*)
*/
void ADCTEMP_powerOff_Bank(uint8_t bank);

/*!
   \brief Initialize the selected ADC
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param operation mode (ADCTEMP_MONITORING_*)
   \return status
*/
ADCTEMP_StateTypeDef ADCTEMP_init_ADC(I2C_HandleTypeDef *interface, uint8_t address, uint8_t monitoring_mode);

/*!
   \brief start conversion in ADCTEMP_MONITORING_DISABLED mode
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
*/
ADCTEMP_StateTypeDef ADCTEMP_start_Conversion(I2C_HandleTypeDef *interface, uint8_t address);

/*!
   \brief check if ADC is BUSY or OK
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \return status
*/
ADCTEMP_StateTypeDef ADCTEMP_is_Busy(I2C_HandleTypeDef *interface, uint8_t address);

/*!
   \brief Read 12bit raw data from ADC from the selected channel
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param number of ADC input from 0 to 6 or ADCTEMP_INTERNAL_TEMP
   \param out ADC data
   \return status
*/
ADCTEMP_StateTypeDef ADCTEMP_read_Raw(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, uint16_t *out);

/*!
   \brief temperature from ADC from the selected channel
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param number of ADC input from 0 to 6 or ADCTEMP_INTERNAL_TEMP
   \param temp temperature in Celsius degrees
   \return status
*/
ADCTEMP_StateTypeDef ADCTEMP_read_Temp(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, float *temp);

/*!
   \brief set upper limit of ADC input value that activate interrupt
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param number of ADC input from 0 to 6 or ADCTEMP_INTERNAL_TEMP
   \param high limit value
*/
void ADCTEMP_set_Upper_Limit(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, uint8_t limit);

/*!
   \brief set lower limit of ADC input value that activate interrupt
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param number of ADC input from 0 to 6 or ADCTEMP_INTERNAL_TEMP
   \param low limit value
*/
void ADCTEMP_set_Lower_Limit(I2C_HandleTypeDef *interface, uint8_t address, uint8_t sensor, uint8_t limit);

/*!
   \brief set hysteresis of internal ADC temperature sensor that activate interrupt
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param use only ADCTEMP_INTERNAL_TEMP_REG
   \param hysteresis value
*/
#define ADCTEMP_set_Hysteresis_Limit ADCTEMP_set_Lower_Limit

/*!
   \brief read on which of inputs has generated interrupt and clear interrupt register and pin on ADC
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \return inputs status (mask with ADCTEMP_INPUT_*_INT to get single input status)
*/
uint8_t ADCTEMP_read_Interrupt_Status(I2C_HandleTypeDef *interface, uint8_t address);

/*!
   \brief set upper limit of ADC input value that activate interrupt
   \param I2C interface
   \param ADC address (ADCTEMP_CELL_*_ADR)
   \param number of ADC input from 0 to 6 or ADCTEMP_INTERNAL_TEMP
   \param high limit temperature in degrees
   \param low limit temperature, or hysteresys for ADCTEMP_INTERNAL_TEMP_REG, in degrees
*/
void ADCTEMP_set_Temperature_Limit(
    I2C_HandleTypeDef *interface,
    uint8_t address,
    uint8_t sensor,
    float high_limit,
    float low_limit);

#ifdef __cplusplus
}
#endif

#endif /* ADCTEMP_H */

/* *****END OF FILE*** */
