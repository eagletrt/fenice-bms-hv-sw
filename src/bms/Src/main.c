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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "cli.h"
#include "error.h"
#include "fenice_config.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEMPS_READ_INTERVAL 500
#define VOLTS_READ_INTERVAL 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
CAN_HandleTypeDef hcan1;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
state_func_t *const state_table[BMS_NUM_STATES] = {
	do_state_init, do_state_idle,   do_state_precharge,
	do_state_on,   do_state_charge, do_state_halt};

transition_func_t *const transition_table[BMS_NUM_STATES][BMS_NUM_STATES] = {
	{NULL, to_idle, to_precharge, NULL, NULL, to_halt},  // from init
	{NULL, NULL, to_precharge, NULL, NULL, to_halt},	 // from idle
	{NULL, to_idle, NULL, to_on, to_charge, to_halt},	// from precharge
	{NULL, to_idle, NULL, NULL, NULL, to_halt},			 // from on
	{NULL, NULL, NULL, NULL, NULL, to_halt}};			 // from halt

const char *bms_state_names[BMS_NUM_STATES] = {[BMS_IDLE] = "init",
											   [BMS_IDLE] = "idle",
											   [BMS_PRECHARGE] = "pre-charge",
											   [BMS_ON] = "on",
											   [BMS_CHARGE] = "charge",
											   [BMS_HALT] = "halt"};

const char *error_names[ERROR_NUM_ERRORS] = {
	[ERROR_LTC_PEC_ERROR] = "PEC",
	[ERROR_CELL_UNDER_VOLTAGE] = "under-voltage",
	[ERROR_CELL_OVER_VOLTAGE] = "over-voltage",
	[ERROR_CELL_OVER_TEMPERATURE] = "over-temperature",
	[ERROR_OVER_CURRENT] = "over-current",
	[ERROR_CAN] = "CAN",
	[ERROR_OK] = "ok"};

BMS_STATE_T state = BMS_INIT;
state_global_data_t data;

cli_t cli;

DMA_HandleTypeDef hdma_adc1;

uint32_t timer_precharge = 0;
uint32_t timer_volts = 0;
uint32_t timer_temps = 0;
uint32_t timer_bal = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_CAN1_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

BMS_STATE_T do_state_init(state_global_data_t *data) {
	error_init(&data->can_error);

	data->error = ERROR_OK;

	pack_init(&(data->pack));
	return BMS_IDLE;
}

void to_idle(state_global_data_t *data) {
	// bms_set_ts_off(&data->bms);
	// HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);
	// can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_OFF, 8);
}

BMS_STATE_T do_state_idle(state_global_data_t *data) {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_ON) {
	// 		// TS On
	// 		if (data->can_rx.Data[1] == 0x01) {
	// 			// Charge command
	// 			data->bms.precharge_bypass = true;
	// 		}
	// 		return BMS_PRECHARGE;
	// 	}
	// }

	return BMS_IDLE;
}

void to_precharge(state_global_data_t *data) {
	// Precharge
	// bms_precharge_start(&data->bms);
	// timer_precharge = HAL_GetTick();
}

BMS_STATE_T do_state_precharge(
	state_global_data_t *data) {  // Check for incoming voltage
	// if (data->can_rx.StdId == CAN_ID_IN_INVERTER_L) {
	// 	if (data->can_rx.Data[0] == CAN_IN_BUS_VOLTAGE) {
	// 		uint16_t bus_voltage = 0;

	// 		bus_voltage = data->can_rx.Data[2] << 8;
	// 		bus_voltage += data->can_rx.Data[1];
	// 		bus_voltage /= 31.99;

	// 		if (bus_voltage >= data->pack.total_voltage / 10000 * 0.95) {
	// 			bms_precharge_end(&data->bms);
	// 			return BMS_ON;
	// 		}
	// 	}
	// }

	// switch (bms_precharge_check(&(data)->bms)) {
	// 	case PRECHARGE_SUCCESS:
	// 		// Used when bypassing precharge

	// 		return BMS_CHARGE;
	// 		break;

	// 	case PRECHARGE_FAILURE:
	// 		// Precharge timed out

	// 		can_send_warning(&hcan, WARN_PRECHARGE_FAIL, 0);

	// 		return BMS_IDLE;
	// 		break;

	// 	case PRECHARGE_WAITING:
	// 		// If precharge is still running, send the bus voltage request

	// 		if (HAL_GetTick() - timer_precharge >= 20) {
	// 			timer_precharge = HAL_GetTick();

	// 			can_send(&hcan, CAN_ID_OUT_INVERTER_L, CAN_MSG_BUS_VOLTAGE, 8);
	// 		}
	// 		break;
	// }
	return BMS_PRECHARGE;
}

void to_charge(state_global_data_t *data) {
	// send ready to charge message (TBD)
}

BMS_STATE_T do_state_charge(state_global_data_t *data) {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_OFF) {
	// 		return BMS_IDLE;
	// 	}
	// }

	return BMS_CHARGE;
}

void to_on(state_global_data_t *data) {
	// bms_precharge_end(&data->bms);
	// HAL_CAN_ConfigFilter(&hcan, &CAN_FILTER_NORMAL);
	// can_send(&hcan, CAN_ID_BMS, CAN_MSG_TS_ON, 8);
}

BMS_STATE_T do_state_on(state_global_data_t *data) {
	// if (data->can_rx.StdId == CAN_ID_ECU) {
	// 	if (data->can_rx.Data[0] == CAN_IN_TS_OFF) {
	// 		return BMS_IDLE;
	// 	}
	// }

	return BMS_ON;
}

void to_halt(state_global_data_t *data) {
	// bms_set_ts_off(&data->bms);
	// bms_set_fault(&data->bms);

	// can_send_error(&hcan, data->error, data->error_index, &data->pack);
}

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

		read_temps(data);
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

	if (data->balancing.enable && tick - timer_bal >= BAL_CYCLE_LENGTH + 5000) {
		timer_bal = tick;
		if (!pack_balance_cells(&hspi1, &data->pack, &data->balancing,
								&data->error)) {
			data->balancing.enable = false;
			cli_print("turning balancing off\r\n", 23);
		}
	}

End:;
}

/**
 * @brief Runs all the checks mandated by the rules
 * @details It runs voltage and current measurements
 */
void read_volts(state_global_data_t *data) {
	// Voltages
	warning_t warning = WARN_OK;

	data->error_index =
		pack_update_voltages(&hspi1, &data->pack, &warning, &data->error);
	ER_CHK(&data->error);

	if (warning != WARN_OK) {
	}

	// Current
	// pack_update_current(&data->pack.current, &data->error);
	ER_CHK(&data->error);

	// Update total values

End:;
}

void read_temps(state_global_data_t *data) {
	// Temperatures
	data->error_index =
		pack_update_temperatures(&hspi1, &data->pack, &data->error);
	ER_CHK(&data->error);

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
  MX_USART2_UART_Init();
  MX_CAN1_Init();
  MX_I2C1_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
	cli_init(&cli, &huart2);

	error_init(&data.can_error);
	data.error = ERROR_OK;

	data.balancing.threshold = BAL_MAX_VOLTAGE_THRESHOLD;
	data.balancing.slot_time = 2;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		state = run_state(state, &data);

		check_timers(&data);
		ER_CHK(&data.error);

		data.error = error_check_fatal(&data.can_error, HAL_GetTick());
		ER_CHK(&data.error);

		data.error_index = pack_check_errors(&data.pack, &data.error);
		ER_CHK(&data.error);

	End:
		cli_loop(&data, state);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 128;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN1_Init(void)
{

  /* USER CODE BEGIN CAN1_Init 0 */

  /* USER CODE END CAN1_Init 0 */

  /* USER CODE BEGIN CAN1_Init 1 */

  /* USER CODE END CAN1_Init 1 */
  hcan1.Instance = CAN1;
  hcan1.Init.Prescaler = 5;
  hcan1.Init.Mode = CAN_MODE_NORMAL;
  hcan1.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan1.Init.TimeSeg1 = CAN_BS1_7TQ;
  hcan1.Init.TimeSeg2 = CAN_BS2_1TQ;
  hcan1.Init.TimeTriggeredMode = DISABLE;
  hcan1.Init.AutoBusOff = DISABLE;
  hcan1.Init.AutoWakeUp = DISABLE;
  hcan1.Init.AutoRetransmission = ENABLE;
  hcan1.Init.ReceiveFifoLocked = DISABLE;
  hcan1.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN1_Init 2 */

  /* USER CODE END CAN1_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

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
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_128;
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
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
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
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, CS_LTC_Pin|LED_1_Pin|LED_2_Pin|LED_3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, CS_ADC_Pin|PCHARGE_END_Pin|TS_ON_Pin|WC_EEPROM_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC13 PC0 PC1 PC2 
                           PC3 PC4 PC5 PC7 
                           PC10 CURRENT_HIGH_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2 
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_7 
                          |GPIO_PIN_10|CURRENT_HIGH_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA1 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_LTC_Pin LED_1_Pin LED_2_Pin LED_3_Pin */
  GPIO_InitStruct.Pin = CS_LTC_Pin|LED_1_Pin|LED_2_Pin|LED_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB10 
                           PB3 PB4 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_10 
                          |GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_ADC_Pin PCHARGE_END_Pin TS_ON_Pin WC_EEPROM_Pin */
  GPIO_InitStruct.Pin = CS_ADC_Pin|PCHARGE_END_Pin|TS_ON_Pin|WC_EEPROM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : SHUTDOWN_STATUS_Pin */
  GPIO_InitStruct.Pin = SHUTDOWN_STATUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(SHUTDOWN_STATUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : BMS_FAULT_Pin */
  GPIO_InitStruct.Pin = BMS_FAULT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(BMS_FAULT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_INSERT_Pin CHARGE_Pin CURRENT_LOW_Pin */
  GPIO_InitStruct.Pin = SD_INSERT_Pin|CHARGE_Pin|CURRENT_LOW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PD2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/**
 * @brief  Rx Transfer completed callback
 * @note   This example shows a simple way to report end of IT Rx transfer,
 * and you can add your own implementation.
 * @retval None
 */
void UART_CharReception_Callback(void) {
	/* Read Received character. RXNE flag is cleared by reading of DR
	 * register
	 */

	cli_char_receive();
}

/**
 * @brief  Function called for achieving next TX Byte sending
 * @retval None
 */
void UART_TXEmpty_Callback(void) {
	// if (tx_buf.index == (ubSizeToSend - 1)) {
	/* Disable TXE interrupt */
	LL_USART_DisableIT_TXE(USART2);

	/* Enable TC interrupt */
	LL_USART_EnableIT_TC(USART2);
	//}

	/* Fill DR with a new char */
	// LL_USART_TransmitData8(USART2, aTxStartMessage[rx_buf.index++]);
}

/**
 * @brief  Function called at completion of last byte transmission
 * @retval None
 */
void UART_CharTransmitComplete_Callback(void) {
	// if (tx_buf.index == sizeof(aTxStartMessage)) {
	// 	tx_buf.index = 0;

	// 	/* Disable TC interrupt */
	LL_USART_DisableIT_TC(USART2);

	// 	/* Set Tx complete boolean to 1 */
	// 	tx_complete = 1;
	// }
}

/**
 * @brief  UART error callbacks
 * @note   This example shows a simple way to report transfer error, and you
 * can add your own implementation.
 * @retval None
 */
void UART_Error_Callback(void) {
	__IO uint32_t sr_reg;

	/* Disable USARTx_IRQn */
	NVIC_DisableIRQ(USART2_IRQn);

	/* Error handling example :
	  - Read USART SR register to identify flag that leads to IT raising
	  - Perform corresponding error handling treatment according to flag
	*/
	sr_reg = LL_USART_ReadReg(USART2, SR);
	if (sr_reg & LL_USART_SR_NE) {
		/* Turn LED2 off: Transfer error in reception/transmission process
		 */
		// BSP_LED_Off(LED2);
	} else {
		/* Turn LED2 off: Transfer error in reception/transmission process
		 */
		// BSP_LED_Off(LED2);
	}
}
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
