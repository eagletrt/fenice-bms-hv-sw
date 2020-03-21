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

#include "fdcan.h"
#include "gpio.h"
#include "spi.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "../../fenice_config.h"
#include "cli.h"
#include "error/error.h"
#include "si8900.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TEMPS_READ_INTERVAL 40
#define VOLTS_READ_INTERVAL 10
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
state_func_t *const state_table[BMS_NUM_STATES] = {
	do_state_init, do_state_idle, do_state_precharge,
	do_state_on, do_state_charge, do_state_halt};

transition_func_t *const transition_table[BMS_NUM_STATES][BMS_NUM_STATES] = {
	{NULL, to_idle, to_precharge, NULL, NULL, to_halt},	 // from init
	{NULL, NULL, to_precharge, NULL, NULL, to_halt},	 // from idle
	{NULL, to_idle, NULL, to_on, to_charge, to_halt},	 // from precharge
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
	[ERROR_ADC_INIT] = "adc init",
	[ERROR_ADC_TIMEOUT] = "adc timeout",
	[ERROR_OK] = "ok"};

const char *bool_names[2] = {"false", "true"};

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
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

BMS_STATE_T do_state_init(state_global_data_t *data) {
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

	data->error = error_verify(HAL_GetTick());

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
	}

	// Read and send voltages and current
	if (tick - timer_volts >= VOLTS_READ_INTERVAL) {
		timer_volts = tick;

		read_volts(data);

		si8900_read_channel(&huart3, SI8900_AIN0, &data->pack.adc_voltage);
		si8900_read_channel(&huart3, SI8900_AIN1, &data->pack.ext_voltage);
	}

	if (data->balancing.enable && tick - timer_bal >= BAL_CYCLE_LENGTH + 5000) {
		timer_bal = tick;
		if (!pack_balance_cells(&hspi1, &data->pack, &data->balancing)) {
			data->balancing.enable = false;
			cli_print("turning balancing off\r\n", 23);
		}
	}
}

/**
 * @brief Runs all the checks mandated by the rules
 * @details It runs voltage and current measurements
 */
void read_volts(state_global_data_t *data) {
	// Voltages

	pack_update_voltages(&hspi1, &data->pack);

	// Current
	pack_update_current(&data->pack.current);

	// Update total values
}

void read_temps(state_global_data_t *data) {
	// Temperatures
	pack_update_temperatures(&hspi1, &data->pack);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {
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
	MX_FDCAN1_Init();
	MX_SPI1_Init();
	MX_SPI2_Init();
	MX_USART2_UART_Init();
	MX_USART3_UART_Init();

	/* Initialize interrupts */
	MX_NVIC_Init();
	/* USER CODE BEGIN 2 */

	cli_init(&cli, &huart2);

	if (si8900_init(&huart3)) {
		cli_print("SI8900 INITIALIZED\r\n", 20);
	} else {
		cli_print("SI8900 ERROR\r\n", 14);
	}

	data.hspi = &hspi1;

	data.balancing.threshold = BAL_MAX_VOLTAGE_THRESHOLD;
	data.balancing.slot_time = 2;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		//state = run_state(state, &data);
		//check_timers(&data);

		cli_loop(&data, state);
	}
	return 0;
	/* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
	RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

	/** Configure the main internal regulator output voltage 
  */
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
	/** Initializes the CPU, AHB and APB busses clocks 
  */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
	RCC_OscInitStruct.PLL.PLLN = 85;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
	RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB busses clocks 
  */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_8) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the peripherals clocks 
  */
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_USART3 | RCC_PERIPHCLK_FDCAN;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
	PeriphClkInit.FdcanClockSelection = RCC_FDCANCLKSOURCE_PCLK1;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void) {
	/* FDCAN1_IT0_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
	/* FDCAN1_IT1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
	/* USART2_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return
	 * state
	 */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line
	   number, tex: printf("Wrong parameters value: file %s on line %d\r\n",
	   file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
