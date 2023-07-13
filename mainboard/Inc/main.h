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
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CONNS_DETECTION_Pin GPIO_PIN_13
#define CONNS_DETECTION_GPIO_Port GPIOC
#define MUX_A3_Pin GPIO_PIN_14
#define MUX_A3_GPIO_Port GPIOC
#define MUX_A2_Pin GPIO_PIN_15
#define MUX_A2_GPIO_Port GPIOC
#define MUX_A1_Pin GPIO_PIN_0
#define MUX_A1_GPIO_Port GPIOC
#define MUX_A0_Pin GPIO_PIN_1
#define MUX_A0_GPIO_Port GPIOC
#define ITS_CH2_Pin GPIO_PIN_2
#define ITS_CH2_GPIO_Port GPIOC
#define ITS_CH1_Pin GPIO_PIN_3
#define ITS_CH1_GPIO_Port GPIOC
#define PROBING_3V3_Pin GPIO_PIN_0
#define PROBING_3V3_GPIO_Port GPIOA
#define FB_SD_IMD_Pin GPIO_PIN_1
#define FB_SD_IMD_GPIO_Port GPIOA
#define FB_SD_BMS_Pin GPIO_PIN_2
#define FB_SD_BMS_GPIO_Port GPIOA
#define SD_IN_Pin GPIO_PIN_3
#define SD_IN_GPIO_Port GPIOA
#define MUX_IN_Pin GPIO_PIN_4
#define MUX_IN_GPIO_Port GPIOA
#define ADC_SCK_Pin GPIO_PIN_5
#define ADC_SCK_GPIO_Port GPIOA
#define ADC_MISO_Pin GPIO_PIN_6
#define ADC_MISO_GPIO_Port GPIOA
#define ADC_MOSI_Pin GPIO_PIN_7
#define ADC_MOSI_GPIO_Port GPIOA
#define ADC_CS_Pin GPIO_PIN_4
#define ADC_CS_GPIO_Port GPIOC
#define AIRN_OFF_Pin GPIO_PIN_5
#define AIRN_OFF_GPIO_Port GPIOC
#define SD_OUT_Pin GPIO_PIN_0
#define SD_OUT_GPIO_Port GPIOB
#define FANS_PWM_Pin GPIO_PIN_1
#define FANS_PWM_GPIO_Port GPIOB
#define IMD_PWM_Pin GPIO_PIN_2
#define IMD_PWM_GPIO_Port GPIOB
#define FANS_DETECT_Pin GPIO_PIN_10
#define FANS_DETECT_GPIO_Port GPIOB
#define EEPROM_SCK_Pin GPIO_PIN_13
#define EEPROM_SCK_GPIO_Port GPIOB
#define EEPROM_MISO_Pin GPIO_PIN_14
#define EEPROM_MISO_GPIO_Port GPIOB
#define EEPROM_MOSI_Pin GPIO_PIN_15
#define EEPROM_MOSI_GPIO_Port GPIOB
#define EEPROM_CS_Pin GPIO_PIN_6
#define EEPROM_CS_GPIO_Port GPIOC
#define EEPROM_HOLD_Pin GPIO_PIN_7
#define EEPROM_HOLD_GPIO_Port GPIOC
#define MONITOR_EN_Pin GPIO_PIN_8
#define MONITOR_EN_GPIO_Port GPIOC
#define MONITOR_CS_Pin GPIO_PIN_9
#define MONITOR_CS_GPIO_Port GPIOC
#define BUZZER_PWM_Pin GPIO_PIN_8
#define BUZZER_PWM_GPIO_Port GPIOA
#define USART_TX_Pin GPIO_PIN_9
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_10
#define USART_RX_GPIO_Port GPIOA
#define CAN_CAR_RX_Pin GPIO_PIN_11
#define CAN_CAR_RX_GPIO_Port GPIOA
#define CAN_CAR_TX_Pin GPIO_PIN_12
#define CAN_CAR_TX_GPIO_Port GPIOA
#define ADC_INT_Pin GPIO_PIN_15
#define ADC_INT_GPIO_Port GPIOA
#define BMS_FAULT_Pin GPIO_PIN_10
#define BMS_FAULT_GPIO_Port GPIOC
#define MONITOR_MISO_Pin GPIO_PIN_11
#define MONITOR_MISO_GPIO_Port GPIOC
#define MONITOR_MOSI_Pin GPIO_PIN_12
#define MONITOR_MOSI_GPIO_Port GPIOC
#define AIRP_OFF_Pin GPIO_PIN_2
#define AIRP_OFF_GPIO_Port GPIOD
#define MONITOR_SCK_Pin GPIO_PIN_3
#define MONITOR_SCK_GPIO_Port GPIOB
#define CAN_CELL_RX_Pin GPIO_PIN_5
#define CAN_CELL_RX_GPIO_Port GPIOB
#define CAN_CELL_TX_Pin GPIO_PIN_6
#define CAN_CELL_TX_GPIO_Port GPIOB
#define PRECHARGE_Pin GPIO_PIN_7
#define PRECHARGE_GPIO_Port GPIOB
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
