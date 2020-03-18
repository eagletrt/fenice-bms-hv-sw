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
#include "fsm.h"
#include "pack.h"
#include "stm32f4xx_ll_usart.h"
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
BMS_STATE_T do_state_init(state_global_data_t *data);
BMS_STATE_T do_state_idle(state_global_data_t *data);
BMS_STATE_T do_state_precharge(state_global_data_t *data);
BMS_STATE_T do_state_on(state_global_data_t *data);
BMS_STATE_T do_state_charge(state_global_data_t *data);
BMS_STATE_T do_state_halt(state_global_data_t *data);

void to_idle(state_global_data_t *data);
void to_precharge(state_global_data_t *data);
void to_on(state_global_data_t *data);
void to_charge(state_global_data_t *data);
void to_halt(state_global_data_t *data);

void check_timers(state_global_data_t *data);
void read_volts(state_global_data_t *data);
void read_temps(state_global_data_t *data);

void UART_CharReception_Callback(void);
void UART_TXEmpty_Callback(void);
void UART_CharTransmitComplete_Callback(void);
void UART_Error_Callback(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CUR_LOW_Pin GPIO_PIN_0
#define CUR_LOW_GPIO_Port GPIOC
#define CUR_HIGH_Pin GPIO_PIN_1
#define CUR_HIGH_GPIO_Port GPIOC
#define CS_LTC_Pin GPIO_PIN_4
#define CS_LTC_GPIO_Port GPIOA
#define TS_ON_Pin GPIO_PIN_12
#define TS_ON_GPIO_Port GPIOB
#define BMS_FAULT_Pin GPIO_PIN_13
#define BMS_FAULT_GPIO_Port GPIOB
#define IMD_Pin GPIO_PIN_14
#define IMD_GPIO_Port GPIOB
#define LED_1_Pin GPIO_PIN_9
#define LED_1_GPIO_Port GPIOC
#define LED_2_Pin GPIO_PIN_8
#define LED_2_GPIO_Port GPIOA
#define LED_3_Pin GPIO_PIN_9
#define LED_3_GPIO_Port GPIOA
#define EEPROM_WC_Pin GPIO_PIN_10
#define EEPROM_WC_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
