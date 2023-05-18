/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CONNs_DETECTION_Pin GPIO_PIN_13
#define CONNs_DETECTION_GPIO_Port GPIOC
#define MUX_A3_Pin GPIO_PIN_14
#define MUX_A3_GPIO_Port GPIOC
#define MUX_A2_Pin GPIO_PIN_15
#define MUX_A2_GPIO_Port GPIOC
#define MUX_A1_Pin GPIO_PIN_0
#define MUX_A1_GPIO_Port GPIOC
#define MUX_A0_Pin GPIO_PIN_1
#define MUX_A0_GPIO_Port GPIOC
#define HALL300_ADC_Pin GPIO_PIN_2
#define HALL300_ADC_GPIO_Port GPIOC
#define HAL50_ADC_Pin GPIO_PIN_3
#define HAL50_ADC_GPIO_Port GPIOC
#define PROBING_3V3_Pin GPIO_PIN_0
#define PROBING_3V3_GPIO_Port GPIOA
#define SD_BMS_FB_ADC_Pin GPIO_PIN_1
#define SD_BMS_FB_ADC_GPIO_Port GPIOA
#define SD_IMD_FB_ADC_Pin GPIO_PIN_2
#define SD_IMD_FB_ADC_GPIO_Port GPIOA
#define SD_IN_ADC_Pin GPIO_PIN_3
#define SD_IN_ADC_GPIO_Port GPIOA
#define MUX_OUTPUT_Pin GPIO_PIN_4
#define MUX_OUTPUT_GPIO_Port GPIOA
#define ADC_SPI_SCK_Pin GPIO_PIN_5
#define ADC_SPI_SCK_GPIO_Port GPIOA
#define ADC_SPI_MISO_Pin GPIO_PIN_6
#define ADC_SPI_MISO_GPIO_Port GPIOA
#define ADC_SPI_MOSI_Pin GPIO_PIN_7
#define ADC_SPI_MOSI_GPIO_Port GPIOA
#define ADC_CS_Pin GPIO_PIN_4
#define ADC_CS_GPIO_Port GPIOC
#define AIRN_OFF_Pin GPIO_PIN_5
#define AIRN_OFF_GPIO_Port GPIOC
#define SD_OUT_ADC_Pin GPIO_PIN_0
#define SD_OUT_ADC_GPIO_Port GPIOB
#define FAN_PWM_Pin GPIO_PIN_1
#define FAN_PWM_GPIO_Port GPIOB
#define IMD_PWM_Pin GPIO_PIN_2
#define IMD_PWM_GPIO_Port GPIOB
#define FAN_DETECT_Pin GPIO_PIN_10
#define FAN_DETECT_GPIO_Port GPIOB
#define EEPROM_SPI_SCK_Pin GPIO_PIN_13
#define EEPROM_SPI_SCK_GPIO_Port GPIOB
#define EEPROM_SPI_MISO_Pin GPIO_PIN_14
#define EEPROM_SPI_MISO_GPIO_Port GPIOB
#define EEPROM_SPI_MOSI_Pin GPIO_PIN_15
#define EEPROM_SPI_MOSI_GPIO_Port GPIOB
#define EEPROM_CS_Pin GPIO_PIN_6
#define EEPROM_CS_GPIO_Port GPIOC
#define EEPROM_HOLD_Pin GPIO_PIN_7
#define EEPROM_HOLD_GPIO_Port GPIOC
#define MONITOR_SPI_EN_Pin GPIO_PIN_8
#define MONITOR_SPI_EN_GPIO_Port GPIOC
#define MONITOR_SPI_CS_Pin GPIO_PIN_9
#define MONITOR_SPI_CS_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOA
#define ADC_INT_Pin GPIO_PIN_15
#define ADC_INT_GPIO_Port GPIOA
#define BMS_FAULT_Pin GPIO_PIN_10
#define BMS_FAULT_GPIO_Port GPIOC
#define MONITOR_SPI_MISO_Pin GPIO_PIN_11
#define MONITOR_SPI_MISO_GPIO_Port GPIOC
#define MONITOR_SPI_MOSI_Pin GPIO_PIN_12
#define MONITOR_SPI_MOSI_GPIO_Port GPIOC
#define AIRP_OFF_Pin GPIO_PIN_2
#define AIRP_OFF_GPIO_Port GPIOD
#define MONITOR_SPI_SCK_Pin GPIO_PIN_3
#define MONITOR_SPI_SCK_GPIO_Port GPIOB
#define PRECHARGE_Pin GPIO_PIN_7
#define PRECHARGE_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_8
#define LED_2_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_9
#define LED_1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
