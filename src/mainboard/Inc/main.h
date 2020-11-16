/* USER CODE BEGIN Header */
/**
 * @file		main.h
 *
 * @date		Oct 9, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "error/error.h"
#include "pack.h"
#include "stm32g4xx_ll_usart.h"
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
void check_timers();
void read_volts();
void read_temps();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define ANALOG_DATA_Pin GPIO_PIN_13
#define ANALOG_DATA_GPIO_Port GPIOC
#define IP_LOW_Pin GPIO_PIN_14
#define IP_LOW_GPIO_Port GPIOC
#define IP_HIGH_Pin GPIO_PIN_15
#define IP_HIGH_GPIO_Port GPIOC
#define CS_EEPROM_Pin GPIO_PIN_0
#define CS_EEPROM_GPIO_Port GPIOF
#define CARD_INSERT_Pin GPIO_PIN_1
#define CARD_INSERT_GPIO_Port GPIOF
#define MUX_A2_Pin GPIO_PIN_0
#define MUX_A2_GPIO_Port GPIOA
#define MUX_A3_Pin GPIO_PIN_1
#define MUX_A3_GPIO_Port GPIOA
#define CS_LTC_Pin GPIO_PIN_4
#define CS_LTC_GPIO_Port GPIOA
#define ADC_SIN_Pin GPIO_PIN_0
#define ADC_SIN_GPIO_Port GPIOB
#define MUX_A1_Pin GPIO_PIN_1
#define MUX_A1_GPIO_Port GPIOB
#define MUX_A0_Pin GPIO_PIN_2
#define MUX_A0_GPIO_Port GPIOB
#define CS_SD_Pin GPIO_PIN_12
#define CS_SD_GPIO_Port GPIOB
#define BMS_FAULT_Pin GPIO_PIN_9
#define BMS_FAULT_GPIO_Port GPIOA
#define TS_ON_Pin GPIO_PIN_10
#define TS_ON_GPIO_Port GPIOA
#define PC_ENDED_Pin GPIO_PIN_11
#define PC_ENDED_GPIO_Port GPIOA
#define PWM_IMD_Pin GPIO_PIN_12
#define PWM_IMD_GPIO_Port GPIOA
#define HOLD_Pin GPIO_PIN_15
#define HOLD_GPIO_Port GPIOA
#define CHARGE_Pin GPIO_PIN_5
#define CHARGE_GPIO_Port GPIOB
#define LED_2_Pin GPIO_PIN_6
#define LED_2_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_7
#define LED_1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
