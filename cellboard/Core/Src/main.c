/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bal_fsm.h"
#include "can_comms.h"
#include "ltc6813_utils.h"
#include "temp.h"
#include "volt.h"

#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t cellboard_index = 0;
uint32_t volt_timer     = 0;
uint32_t temp_timer     = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM2_Init();
  MX_CAN1_Init();
  MX_I2C1_Init();
  MX_SPI1_Init();
  MX_SPI3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */

    cellboard_index = HAL_GPIO_ReadPin(ADDRESS_2_GPIO_Port, ADDRESS_2_Pin) |
                      HAL_GPIO_ReadPin(ADDRESS_1_GPIO_Port, ADDRESS_1_Pin) << 1 |
                      HAL_GPIO_ReadPin(ADDRESS_0_GPIO_Port, ADDRESS_0_Pin) << 2;

    temp_init();

    //set temperature limits ( 0 - 60 )
    temp_set_limits(1.0, CELL_MAX_TEMPERATURE);

    // Blink led to signal cellboard index
    for (uint8_t i = 0; i < cellboard_index; i++) {
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
        HAL_Delay(150);
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
        HAL_Delay(250);
    }
    HAL_Delay(300);
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

    bal_fsm_init();

    CAN_FilterTypeDef filter;
    filter.FilterMode       = CAN_FILTERMODE_IDMASK;
    filter.FilterIdLow      = 0 << 5;                 // Take all ids from 0
    filter.FilterIdHigh     = ((1U << 11) - 1) << 5;  // to 2^11 - 1
    filter.FilterMaskIdHigh = 0 << 5;                 // Don't care on can id bits
    filter.FilterMaskIdLow  = 0 << 5;                 // Don't care on can id bits
    /* HAL considers IdLow and IdHigh not as just the ID of the can message but
        as the combination of: 
        STDID + RTR + IDE + 4 most significant bits of EXTID
    */
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterBank           = 0;
    filter.FilterScale          = CAN_FILTERSCALE_16BIT;
    filter.FilterActivation     = ENABLE;

    HAL_CAN_ConfigFilter(&BMS_CAN, &filter);
    HAL_CAN_ActivateNotification(&BMS_CAN, CAN_IT_ERROR | CAN_IT_RX_FIFO0_MSG_PENDING );
    HAL_CAN_Start(&BMS_CAN);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

    while (1) {
        if (HAL_GetTick() - volt_timer >= VOLT_MEASURE_INTERVAL - VOLT_MEASURE_TIME) {
            volt_start_measure();
        }

        if (HAL_GetTick() - volt_timer >= VOLT_MEASURE_INTERVAL) {
            volt_timer = HAL_GetTick();
            volt_read();

            can_send(ID_VOLTAGES_0);
            CAN_WAIT(&BMS_CAN);
            can_send(ID_VOLTAGES_1);
            CAN_WAIT(&BMS_CAN);
            can_send(ID_VOLTAGES_2);
            CAN_WAIT(&BMS_CAN);
            can_send(ID_VOLTAGES_3);
            CAN_WAIT(&BMS_CAN);
            can_send(ID_VOLTAGES_4);
            CAN_WAIT(&BMS_CAN);
            can_send(ID_VOLTAGES_5);

            char buf[5000] = {'\0'};
            uint16_t min   = volt_get_min();

            for (uint8_t i = 0; i < CELLBOARD_CELL_COUNT; i++) {
                sprintf(buf + strlen(buf), "%3u %-.3fV", i, (float)voltages[i] / 10000);

                for (size_t k = 0; k < bal.cells_length; k++) {
                    if (bal.cells[k] == i) {
                        sprintf(buf + strlen(buf), " D");
                        break;
                    }
                }
                if (min == i) {
                    sprintf(buf + strlen(buf), " M");
                }
                sprintf(buf + strlen(buf), "\r\n");
                //if (i % LTC6813_REG_CELL_COUNT == 0) {
                //    sprintf(buf + strlen(buf), "\r\n");
                //}
                //sprintf(buf + strlen(buf), "[%3u %-.3f V] ", i, (float)voltages[i] / 10000);
            }

            can_send(ID_BOARD_STATUS);

            sprintf(buf + strlen(buf), "Threshold: %d mV", BAL_MAX_VOLTAGE_THRESHOLD / 10);
            sprintf(buf + strlen(buf), "\r\nBAL: %li", fsm_get_state(bal.fsm));
            if (errors != 0) {
                sprintf(buf + strlen(buf), "\r\nERRORS: %x", errors);
            }
            sprintf(buf + strlen(buf), "\r\n\n");
            HAL_UART_Transmit(&CLI_UART, (uint8_t *)buf, strlen(buf), 500);
            HAL_Delay(200);
            fsm_trigger_event(bal.fsm, EV_BAL_CHECK_TIMER);
        }

        if (HAL_GetTick() - temp_timer >= TEMP_MEASURE_INTERVAL) {
            temp_timer = HAL_GetTick();

            temp_measure_all();
            can_send(ID_TEMP_STATS);
        }

        fsm_run(bal.fsm);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1) {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
