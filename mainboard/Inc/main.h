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
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "error/error.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void check_timers();
void read_volts();
void read_temps();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MUX_A3_Pin GPIO_PIN_14
#define MUX_A3_GPIO_Port GPIOC
#define MUX_A2_Pin GPIO_PIN_15
#define MUX_A2_GPIO_Port GPIOC
#define MUX_A1_Pin GPIO_PIN_0
#define MUX_A1_GPIO_Port GPIOC
#define MUX_A0_Pin GPIO_PIN_1
#define MUX_A0_GPIO_Port GPIOC
#define ITS_CH1_Pin GPIO_PIN_2
#define ITS_CH1_GPIO_Port GPIOC
#define ITS_CH2_Pin GPIO_PIN_3
#define ITS_CH2_GPIO_Port GPIOC
#define FB_SD_END_Pin GPIO_PIN_0
#define FB_SD_END_GPIO_Port GPIOA
#define FB_RELAY_SD_Pin GPIO_PIN_1
#define FB_RELAY_SD_GPIO_Port GPIOA
#define FB_RELAY_SD_EXTI_IRQn EXTI1_IRQn
#define FB_IMD_FAULT_Pin GPIO_PIN_2
#define FB_IMD_FAULT_GPIO_Port GPIOA
#define FB_IMD_FAULT_EXTI_IRQn EXTI2_IRQn
#define MUX_IN_Pin GPIO_PIN_4
#define MUX_IN_GPIO_Port GPIOA
#define CS_ADC_Pin GPIO_PIN_4
#define CS_ADC_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_0
#define BUZZER_GPIO_Port GPIOB
#define FANS_Pin GPIO_PIN_1
#define FANS_GPIO_Port GPIOB
#define IMD_PWM_Pin GPIO_PIN_2
#define IMD_PWM_GPIO_Port GPIOB
#define HANDCART_Pin GPIO_PIN_12
#define HANDCART_GPIO_Port GPIOB
#define EEPROM_CS_Pin GPIO_PIN_6
#define EEPROM_CS_GPIO_Port GPIOC
#define EEPROM_HOLD_Pin GPIO_PIN_7
#define EEPROM_HOLD_GPIO_Port GPIOC
#define CARD_CS_Pin GPIO_PIN_8
#define CARD_CS_GPIO_Port GPIOC
#define CARD_OK_Pin GPIO_PIN_9
#define CARD_OK_GPIO_Port GPIOC
#define BMS_FAULT_Pin GPIO_PIN_10
#define BMS_FAULT_GPIO_Port GPIOC
#define PRECHARGE_Pin GPIO_PIN_11
#define PRECHARGE_GPIO_Port GPIOC
#define AIRN_OFF_Pin GPIO_PIN_12
#define AIRN_OFF_GPIO_Port GPIOC
#define AIRP_OFF_Pin GPIO_PIN_2
#define AIRP_OFF_GPIO_Port GPIOD
#define TS_ON_Pin GPIO_PIN_4
#define TS_ON_GPIO_Port GPIOB
#define CAN_CELL_RX_Pin GPIO_PIN_5
#define CAN_CELL_RX_GPIO_Port GPIOB
#define CAN_CELL_TX_Pin GPIO_PIN_6
#define CAN_CELL_TX_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_8
#define LED2_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_9
#define LED1_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
