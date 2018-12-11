/*
 * ltc68xx.h
 *
 *  Created on: 23 nov 2018
 *      Author: Utente
 */

#ifndef LTC68XX_H_
#define LTC68XX_H_

#include "stm32f3xx_hal.h"


#define MD_422HZ_1KHZ 0
#define MD_27KHZ_14KHZ 1
#define MD_7KHZ_3KHZ 2
#define MD_26HZ_2KHZ 3

#define ADC_OPT_ENABLED 1
#define ADC_OPT_DISABLED 0

#define CELL_CH_ALL 0
#define CELL_CH_1and7 1
#define CELL_CH_2and8 2
#define CELL_CH_3and9 3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

#define SELFTEST_1 1
#define SELFTEST_2 2

#define AUX_CH_ALL 0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6

#define STAT_CH_ALL 0
#define STAT_CH_SOC 1
#define STAT_CH_ITEMP 2
#define STAT_CH_VREGA 3
#define STAT_CH_VREGD 4

#define DCP_DISABLED 0
#define DCP_ENABLED 1

#define PULL_UP_CURRENT 1
#define PULL_DOWN_CURRENT 0

#define CELL_CHANNELS 12
#define AUX_CHANNELS 6
#define STAT_CHANNELS 4
#define CELL 1
#define AUX 2
#define STAT 3




uint8_t spi_read_byte(uint8_t tx_dat,SPI_HandleTypeDef hspi1);
//int osal_DataLength( char *pString );
//void spi_write_read(uint8_t tx_Data[],	// Array of data to be written on SPI port
//			        uint8_t tx_len, 	// Length of the tx data arry
//			        uint8_t rx_data[8],	// Input: array that will store the data read by the SPI port
//			        uint8_t rx_len,	// Option: number of bytes to be read from the SPI port
//					SPI_HandleTypeDef hspi1
//			        );
uint16_t pec15(uint8_t len,uint8_t data[], uint16_t crcTable[]);
void wakeup_idle1(SPI_HandleTypeDef hspi1);
//void spi_write_array(uint8_t len, 		// Option: Number of bytes to be written on the SPI port
//                     uint8_t data[] ,	// Array of bytes to be written on the SPI port
//					 SPI_HandleTypeDef hspi1
//			         );
//void wakeup_sleep(SPI_HandleTypeDef hspi1);
//void max_min_voltages(uint16_t cell_codes[108], uint16_t max_vol, uint16_t min_vol, float average_vol);
//
//void array_voltages(uint16_t voltages[9], uint8_t cell_data[32]);
//void array_temp_odd(uint16_t temp[9], uint8_t cell_data[9]);
//void array_temp_even(uint16_t temp[9], uint8_t cell_data[9]);
//void max_ave_temp(uint16_t cell_codes[108], uint16_t max_temp, float average_temp);
//float convert_temp(uint16_t volt);
//uint16_t total_pack_voltage(uint16_t cell_codes[108]);
//void ltc6804_rdcv_temp(uint8_t ic_n,				// Number of the current ic
//				  uint8_t total_ic, 		// The number of ICs in the
//			      uint8_t rx_data[9],			// An array of the unparsed cell codes
//				  SPI_HandleTypeDef hspi1
//
//			          );
//void ltc6804_rdcv_reg(uint8_t ic_n,			// Number of the current ic
//					  uint8_t total_ic, 	// The number of ICs in the
//			          uint8_t data[] ,		// An array of the unparsed cell codes
//					  SPI_HandleTypeDef hspi1
//					  );
//void ltc6804_stop_temp(uint8_t MD, 		//!< ADC Conversion Mode
//                  	  	  	  uint8_t DCP, 		//!< Controls if Discharge is permitted during conversion
//							  uint8_t CH ,		//!< Sets which Cell channels are converted
//							  SPI_HandleTypeDef hspi1
//			      	  	  	  );
//void ltc6804_address_temp_odd(uint8_t MD, 		//!< ADC Conversion Mode
//                  	  	  	  uint8_t DCP, 		//!< Controls if Discharge is permitted during conversion
//							  uint8_t CH ,		//!< Sets which Cell channels are converted
//							  SPI_HandleTypeDef hspi1
//			      	  	  	  );
//void ltc6804_address_temp_even(uint8_t MD, 		//!< ADC Conversion Mode
//                  	  	  	  uint8_t DCP, 		//!< Controls if Discharge is permitted during conversion
//							  uint8_t CH ,		//!< Sets which Cell channels are converted
//							  SPI_HandleTypeDef hspi1
//			      	  	  	  );
//void ltc6804_adcv_temp(uint8_t MD, 		//!< ADC Conversion Mode
//                  uint8_t DCP, 		//!< Controls if Discharge is permitted during conversion
//                  uint8_t CH ,		//!< Sets which Cell channels are converted
//				  SPI_HandleTypeDef hspi1
//			      );
void ltc6804_adcv(uint8_t DCP, SPI_HandleTypeDef *hspi);
uint32_t ltc6804_pollAdc(SPI_HandleTypeDef hspi1);
uint16_t convert_voltage(uint8_t v_data[8]);
void ltc6804_rdcv_voltages(uint8_t ic_n, uint16_t cell_voltages[12][2], SPI_HandleTypeDef hspi1);
void ltc6804_adax(uint8_t chg, SPI_HandleTypeDef *hspi1);
void ltc6804_rdaux(uint8_t ic_n, uint16_t gpio_voltages[6][2],SPI_HandleTypeDef *hspi1);
	void LTC6804s_I2CMUX(SPI_HandleTypeDef hspi1,uint8_t ch);
	void ltc6804_command(SPI_HandleTypeDef *hspi);
	void ltc6804_rdstat(SPI_HandleTypeDef hspi1,uint16_t cell_voltages[12][2] );
	void ltc6804_adstat( SPI_HandleTypeDef *hspi);
//	void ltc6804_rdcv_voltages(uint8_t ic_n, uint16_t cell_voltages[108][2], SPI_HandleTypeDef *hspi);
#endif /* LTC68XX_H_ */