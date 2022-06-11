/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <inttypes.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern uint8_t cellboard_index;
//ADC I2C addresses
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define POWER_SENS_2_Pin       GPIO_PIN_14
#define POWER_SENS_2_GPIO_Port GPIOC
#define POWER_SENS_1_Pin       GPIO_PIN_15
#define POWER_SENS_1_GPIO_Port GPIOC
#define ADDRESS_2_Pin          GPIO_PIN_1
#define ADDRESS_2_GPIO_Port    GPIOA
#define ADDRESS_1_Pin          GPIO_PIN_2
#define ADDRESS_1_GPIO_Port    GPIOA
#define ADDRESS_0_Pin          GPIO_PIN_3
#define ADDRESS_0_GPIO_Port    GPIOA
#define EEPROM_CS_Pin          GPIO_PIN_4
#define EEPROM_CS_GPIO_Port    GPIOA
#define LTC_SCK_Pin            GPIO_PIN_5
#define LTC_SCK_GPIO_Port      GPIOA
#define LTC_MISO_Pin           GPIO_PIN_6
#define LTC_MISO_GPIO_Port     GPIOA
#define LTC_MOSI_Pin           GPIO_PIN_7
#define LTC_MOSI_GPIO_Port     GPIOA
#define LTC_CS_Pin             GPIO_PIN_0
#define LTC_CS_GPIO_Port       GPIOB
#define EEPROM_HOLD_Pin        GPIO_PIN_1
#define EEPROM_HOLD_GPIO_Port  GPIOB
#define ADC_nINT_Pin           GPIO_PIN_8
#define ADC_nINT_GPIO_Port     GPIOA
#define LED_Pin                GPIO_PIN_15
#define LED_GPIO_Port          GPIOA
#define EEPROM_SCK_Pin         GPIO_PIN_3
#define EEPROM_SCK_GPIO_Port   GPIOB
#define EEPROM_MISO_Pin        GPIO_PIN_4
#define EEPROM_MISO_GPIO_Port  GPIOB
#define EEPROM_MOSI_Pin        GPIO_PIN_5
#define EEPROM_MOSI_GPIO_Port  GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
