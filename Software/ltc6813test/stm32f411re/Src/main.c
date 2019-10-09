/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "error.h"
#include "fenice_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEMPS_READ_INTERVAL 200
#define VOLTS_READ_INTERVAL 40
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

USART_HandleTypeDef husart2;

/* USER CODE BEGIN PV */
state_func_t *const state_table[BMS_NUM_STATES] = {
	do_state_init, do_state_idle,   do_state_precharge,
	do_state_on,   do_state_charge, do_state_halt};

transition_func_t *const transition_table[BMS_NUM_STATES][BMS_NUM_STATES] = {
	{NULL, to_idle, NULL, NULL, NULL, to_halt},		   // from init
	{NULL, NULL, to_precharge, NULL, NULL, to_halt},   // from idle
	{NULL, to_idle, NULL, to_on, to_charge, to_halt},  // from precharge
	{NULL, to_idle, NULL, NULL, NULL, to_halt},		   // from on
	{NULL, NULL, NULL, NULL, NULL, to_halt}};		   // from halt

BMS_STATE_T state = BMS_INIT;
state_global_data_t data;

USART_HandleTypeDef husart2;
DMA_HandleTypeDef hdma_adc1;
SPI_HandleTypeDef hspi1;

uint32_t timer_precharge = 0;
uint32_t timer_volts = 0;
uint32_t timer_temps = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
BMS_STATE_T do_state_init(state_global_data_t *data) {
	error_init(&data->can_error);

	data->error = ERROR_OK;

	pack_init(&(data->pack));

#if CHARGING > 0
	HAL_Delay(10000);
	data->bms.precharge_bypass = true;
	return BMS_PRECHARGE;
#endif
	return BMS_IDLE;
}

void to_idle(state_global_data_t *data) {}

BMS_STATE_T do_state_idle(state_global_data_t *data) { return BMS_IDLE; }

void to_precharge(state_global_data_t *data) {
	// Precharge
	timer_precharge = HAL_GetTick();
}

BMS_STATE_T do_state_precharge(state_global_data_t *data) { return BMS_ON; }

void to_charge(state_global_data_t *data) {
	// send ready to charge message (TBD)
}

BMS_STATE_T do_state_charge(state_global_data_t *data) { return BMS_CHARGE; }

void to_on(state_global_data_t *data) {}

BMS_STATE_T do_state_on(state_global_data_t *data) { return BMS_ON; }

void to_halt(state_global_data_t *data) {}

BMS_STATE_T do_state_halt(state_global_data_t *data) { return BMS_HALT; }

BMS_STATE_T run_state(BMS_STATE_T state, state_global_data_t *data) {
	BMS_STATE_T new_state = state_table[state](data);

	if (data->error != ERROR_OK) {
		new_state = BMS_HALT;
	}

	transition_func_t *transition = transition_table[state][new_state];

	if (transition) {
		transition(data);
	}

	return new_state;
}

void check_timers(state_global_data_t *data) {
	uint32_t tick = HAL_GetTick();

	// Read and send temperatures
	if (tick - timer_temps >= TEMPS_READ_INTERVAL) {
		timer_temps = tick;

		// read_temps(data);
		ER_CHK(&data->error);

		// Delay voltage measurement to avoid interferences
		timer_volts = HAL_GetTick() - (VOLTS_READ_INTERVAL / 2);
	}

	// Read and send voltages and current
	if (tick - timer_volts >= VOLTS_READ_INTERVAL) {
		timer_volts = tick;

		read_volts(data);
		ER_CHK(&data->error);
	}

End:;
}

/**
 * @brief Runs all the checks mandated by the rules
 * @details It runs voltage and current measurements
 */
void read_volts(state_global_data_t *data) {
	// Voltages
	WARNING_T warning = WARN_OK;

	data->error_index =
		pack_update_voltages(&hspi1, &data->pack, &warning, &data->error);
	ER_CHK(&data->error);

	if (warning != WARN_OK) {
	}

	// Current
	pack_update_current(&data->pack.current, &data->error);
	ER_CHK(&data->error);

	// Update total values

End:;
}

void read_temps(state_global_data_t *data) {
	// Temperatures
	data->error_index =
		pack_update_temperatures(&hspi1, &data->pack, &data->error);
	ER_CHK(&data->error);

	// Check for not healthy cells
	uint8_t volts[PACK_MODULE_COUNT];
	uint8_t num_cells = pack_check_voltage_drops(&data->pack, volts);

	uint8_t i;
	for (i = 0; i < num_cells; i++) {
	}

End:;
}
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
  MX_SPI1_Init();
  MX_USART2_Init();

  /* Initialize interrupts */
  MX_NVIC_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		data.error = error_check_fatal(&data.can_error, HAL_GetTick());
		ER_CHK(&data.error);

		check_timers(&data);
		ER_CHK(&data.error);

		data.error_index = pack_check_errors(&data.pack, &data.error);
		ER_CHK(&data.error);

	End:

		state = run_state(state, &data);

		// switch (can_rx.StdId) {}

		// Check precharge timeout
		/*if (bms.status == BMS_PRECHARGE) {
			switch (bms_precharge_check(&bms)) {
				case BMS_ON:
					// Used when bypassing precharge
					can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_ON, 8);
					HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);

					break;
				case BMS_OFF:
					// Precharge timed out
					HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);
					can_send_warning(&hcan, WARN_PRECHARGE_FAIL, 0);
					break;
				case BMS_PRECHARGE:
					// If precharge is still running, send the bus
		voltage
					// request
					if (HAL_GetTick() - timer_precharge >= 20) {
						timer_precharge = HAL_GetTick();
						can_send(&hcan, CAN_ID_OUT_INVERTER_L,
								 CAN_MSG_INVERTER_VOLTAGE, 8);
					}
					break;
				default:
					break;
			}
		}*/
	}
	return 0;
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
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USART2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  husart2.Instance = USART2;
  husart2.Init.BaudRate = 115200;
  husart2.Init.WordLength = USART_WORDLENGTH_8B;
  husart2.Init.StopBits = USART_STOPBITS_1;
  husart2.Init.Parity = USART_PARITY_NONE;
  husart2.Init.Mode = USART_MODE_TX_RX;
  husart2.Init.CLKPolarity = USART_POLARITY_LOW;
  husart2.Init.CLKPhase = USART_PHASE_1EDGE;
  husart2.Init.CLKLastBit = USART_LASTBIT_DISABLE;
  if (HAL_USART_Init(&husart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI1_CS_Pin */
  GPIO_InitStruct.Pin = SPI1_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI1_CS_GPIO_Port, &GPIO_InitStruct);

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
	/* User can add his own implementation to report the HAL error return
	 * state
	 */

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
	/* User can add his own implementation to report the file name and line
	   number, tex: printf("Wrong parameters value: file %s on line %d\r\n",
	   file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
