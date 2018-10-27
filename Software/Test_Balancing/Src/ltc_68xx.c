/**
 ******************************************************************************
 * @file	ltc_68xx.c
 * @author	Dona, Riki, Gregor e Davide
 * @brief	This file contains all the functions for the LTC68xx battery
 * 			management
 ******************************************************************************
 */

#include "ltc_68xx.h"

uint16_t crcTable[256] ={0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,
    0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
    0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
    0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
    0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
    0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
    0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d,
    0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
    0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
    0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
    0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
    0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
    0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
    0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
    0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
    0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
    0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
    0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
    0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
    0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
    0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
    0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
    0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
    };

/**
  * @brief		This function is used to calculate the PEC value
  * @param		Length of the data array
  * @param		Array of data
  * @param		CRC table
  * @retval		16 bit unsigned integer containing the two PEC bytes
  */
uint16_t pec15(uint8_t len,uint8_t data[],uint16_t crcTable[] ){

    uint16_t remainder,address;
	remainder = 16;					// PEC seed
	for (int i = 0; i < len; i++){

			address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
			remainder = (remainder << 8 ) ^ crcTable[address];

	}
    return (remainder*2);//The CRC15 has a 0 in the LSB so the final value must be multiplied by 2

}

/**
  * @brief		This function is used to convert the 2 byte raw data from the
  * 			LTC68xx to a 16 bit unsigned integer
  * @param 		Raw data bytes
  * @retval		Voltage read from the LTC68xx
  */
uint16_t convert_voltage(uint8_t v_data[]){

	return v_data[0] + (v_data[1] << 8);

}

/**
  * @brief		This function converts a voltage data from the zener sensor
  * 			to a temperature
  * @param		Voltage read from the LTC68xx
  * @retval 	Temperature of the cell multiplied by 100
  */
uint16_t convert_temp(uint16_t volt){

	float voltf = volt*0.0001;
	float temp;
	temp = -225.7*voltf*voltf*voltf + 1310.6 * voltf*voltf -2594.8*voltf + 1767.8;
	return (uint16_t)(temp*100);

}

/**
  * @brief		Wakes up all the devices connected to the isoSPI bus
  * @param		hspi pointer to a SPI_HandleTypeDef structure that contains
  * 			the configuration information for SPI module.
  */
void wakeup_idle(SPI_HandleTypeDef *hspi){

	uint8_t data = 0xFF;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, &data, 1, 1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

}

/**
  * @brief		Monitors voltages and temperatures of the battery pack
  * @param		Array of voltages
  * @param		Array of temperatures
  * @param		Pointer to pack voltage
  * @param		Pointer to minimum voltage
  * @param		Pointer to maximum voltage
  * @param		Pointer to average voltage
  * @param		Pointer to maximum temperature
  * @param		Pointer to average temperature
  * @param		Pointer to current
  * @retval		Battery status
  */
PackStateTypeDef status(uint16_t cell_voltages[108][2],
						uint16_t cell_temperatures[108][2],
					    uint32_t *pack_v,
					    uint16_t *pack_t,
					    uint16_t *max_t,
					    int32_t current,
					    uint8_t *cell,
					    uint16_t *value){


	    uint32_t sum_t = 0;

	  	uint32_t pack_v_temp=0;
	  	uint32_t max_t_temp=0;

	for(int i = 0; i < 108; i++){
		if(cell_temperatures[i][0] > max_t_temp)
		max_t_temp = cell_temperatures[i][0];
		sum_t += cell_temperatures[i][0];
		pack_v_temp += cell_voltages[i][0];
		if(cell_voltages[1][1] > 10 || cell_temperatures[i][1] > 10){

			*cell = i;
			return DATA_NOT_UPDATED;

		}
		if(cell_voltages[i][0] < 25000){

			*cell = i;
			*value = cell_voltages[i][0];
			return UNDER_VOLTAGE;

		}
		if(cell_voltages[i][0] > 42250){

			*cell = i;
			*value = cell_voltages[i][0];
			return OVER_VOLTAGE;

		}
		if(cell_temperatures[i][0] > 7000 || cell_temperatures[i][0] == 0 ){

			*cell = i;
			*value = cell_temperatures[i][0];
			return OVER_TEMPERATURE;

		}
	}
	*pack_t = sum_t / 108;
	if (*pack_t > 6500){

		*cell = 0,
		*value = *pack_t;
		return PACK_OVER_TEMPERATURE;

	}
	*max_t=max_t_temp;
	*pack_v=pack_v_temp;
	return PACK_OK;
}

/**
 * @brief		Reads the data form the LTC68xx and updates the cell temperatures
 * @param		Number of the LTC68xx to read the data from
 * @param		uint8_t that indicates if we are reading even or odd temperatures
 * @param		Array of temperatures
 * @param		hspi pointer to a SPI_HandleTypeDef structure that contains
 * 				the configuration information for SPI module.
 */
void ltc6804_rdcv_temp(uint8_t ic_n,
					   uint8_t parity,
					   uint16_t cell_temperatures[108][2],
					   SPI_HandleTypeDef *hspi){

	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8];
	cmd[0] = (uint8_t)0x80 + 8*ic_n;

	wakeup_idle(hspi);

	// ---- Celle 1, 2, 3
	cmd[1] = (uint8_t)0x04;
	cmd_pec = pec15(2, cmd, crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		if(parity==0){

			cell_temperatures[ic_n*9+1][0] = convert_temp(convert_voltage(&data[2]));
			cell_temperatures[ic_n*9+1][1] = 0;

		}
		else{

			cell_temperatures[ic_n*9][0] = convert_temp(convert_voltage(&data[0]));
			cell_temperatures[ic_n*9+2][0] = convert_temp(convert_voltage(&data[4]));
			cell_temperatures[ic_n*9][1] = 0;
			cell_temperatures[ic_n*9+2][1] = 0;

		}

	}
	else{

		if(parity==0)
			cell_temperatures[ic_n*9+1][1]++;
		else{

			cell_temperatures[ic_n*9][1]++;
			cell_temperatures[ic_n*9+2][1]++;

		}

	}

	// ---- Celle 4, 5, /
	cmd[1] = 0x06;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		if(parity==0){

			cell_temperatures[ic_n*9+3][0] = convert_temp(convert_voltage(&data[0]));
			cell_temperatures[ic_n*9+3][1] = 0;

		}
		else{

			cell_temperatures[ic_n*9+4][0] = convert_temp(convert_voltage(&data[2]));
			cell_temperatures[ic_n*9+4][1] = 0;

		}

	}
	else{

		if(parity==0)
			cell_temperatures[ic_n*9+3][1]++;
		else
			cell_temperatures[ic_n*9+4][1]++;

	}

	// ---- Celle 6, 7, 8
	cmd[1] = 0x08;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	// CS LOW
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		if(parity==0){

			cell_temperatures[ic_n*9+5][0] = convert_temp(convert_voltage(&data[0]));
			cell_temperatures[ic_n*9+7][0] = convert_temp(convert_voltage(&data[4]));
			cell_temperatures[ic_n*9+5][1] = 0;
			cell_temperatures[ic_n*9+7][1] = 0;

		}
		else{

			cell_temperatures[ic_n*9+6][0] = convert_temp(convert_voltage(&data[2]));
			cell_temperatures[ic_n*9+6][1] = 0;

		}

	}
	else{

		if(parity==0){

			cell_temperatures[ic_n*9+5][1]++;
			cell_temperatures[ic_n*9+7][1]++;

		}
		else
			cell_temperatures[ic_n*9+6][1]++;

	}

	// ---- Celle 9, /, /
	if(parity==1){

		cmd[1] = 0x0A;
		cmd_pec = pec15(2, cmd,crcTable);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);
		// CS LOW
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_SPI_Transmit(hspi, cmd, 4, 100);
		HAL_SPI_Receive(hspi, data, 8, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
		if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

			cell_temperatures[ic_n*9+8][0] = convert_temp(convert_voltage(&data[0]));
			cell_temperatures[ic_n*9+8][1] = 0;

		}
		else
			cell_temperatures[ic_n*9+8][1]++;

	}
	return;

}

/**
 * @brief		Reads the data form the LTC68xx and updates the cell voltages
 * @param		Number of the LTC68xx to read the data from
 * @param		Array of voltages
 * @param		hspi pointer to a SPI_HandleTypeDef structure that contains
 * 				the configuration information for SPI module.
 */
void ltc6804_rdcv_voltages(uint8_t ic_n, uint16_t cell_voltages[108][2], SPI_HandleTypeDef *hspi){

	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8];
	cmd[0] = (uint8_t)0x80 + 8*ic_n;
	wakeup_idle(hspi);

	// ---- Celle 1, 2, 3
	cmd[1] = (uint8_t)0x04;
	cmd_pec = pec15(2, cmd, crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9][0] = convert_voltage(data);
		cell_voltages[ic_n*9+1][0] = convert_voltage(&data[2]);
		cell_voltages[ic_n*9+2][0] = convert_voltage(&data[4]);
		cell_voltages[ic_n*9][1] = 0;
		cell_voltages[ic_n*9+1][1] = 0;
		cell_voltages[ic_n*9+2][1] = 0;

	}
	else{

		cell_voltages[ic_n*9][1]++;
		cell_voltages[ic_n*9+1][1]++;
		cell_voltages[ic_n*9+2][1]++;

	}

	// ---- Celle 4, 5, /
	cmd[1] = 0x06;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9+3][0] = convert_voltage(data);
		cell_voltages[ic_n*9+4][0] = convert_voltage(&data[2]);
		cell_voltages[ic_n*9+3][1] = 0;
		cell_voltages[ic_n*9+4][1] = 0;

	}
	else{

		cell_voltages[ic_n*9+3][1]++;
		cell_voltages[ic_n*9+4][1]++;

	}

	// ---- Celle 6, 7, 8
	cmd[1] = 0x08;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9+5][0] = convert_voltage(data);
		cell_voltages[ic_n*9+6][0] = convert_voltage(&data[2]);
		cell_voltages[ic_n*9+7][0] = convert_voltage(&data[4]);
		cell_voltages[ic_n*9+5][1] = 0;
		cell_voltages[ic_n*9+6][1] = 0;
		cell_voltages[ic_n*9+7][1] = 0;


	}
	else{

		cell_voltages[ic_n*9+5][1]++;
		cell_voltages[ic_n*9+6][1]++;
		cell_voltages[ic_n*9+7][1]++;

	}

	// ---- Celle 9, /, /
	cmd[1] = 0x0A;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi, cmd, 4, 100);
	HAL_SPI_Receive(hspi, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9+8][0] = convert_voltage(data);
		cell_voltages[ic_n*9+8][1] = 0;

	}
	else
		cell_voltages[ic_n*9+8][1]++;
	return;

}

/**
 * @brief		Enable or disable the temperature measurement
 * @param		1 to start temperature measurement and 0 to stop it
 * @param		uint8_t that indicates if we are reading even or odd temperatures
 * @param		hspi pointer to a SPI_HandleTypeDef structure that contains
 * 				the configuration information for SPI module.
 */
void ltc6804_command_temperatures(uint8_t start, uint8_t parity, SPI_HandleTypeDef *hspi1){

	uint8_t cmd[4];
	uint8_t cfng[8];
	uint16_t cmd_pec;
	cmd[0] = 0x00;
	cmd[1] = 0x01;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
	cfng[0] = 0x00;
	cfng[1] = 0x00;
	cfng[2] = 0x00;
	cfng[3] = 0x00;
	if (start == 1)
	{
		if (parity == 0){

			cfng[4] = 0x4A;
			cfng[5] = 0x01;

		}
		else{

			cfng[4] = 0x95;
			cfng[5] = 0x02;

		}

	}
	else{

		cfng[4] = 0x00;
		cfng[5] = 0x00;

	}
	cmd_pec = pec15(6, cfng, crcTable);
	cfng[6] = (uint8_t)(cmd_pec >> 8);
	cfng[7] = (uint8_t)(cmd_pec);

    wakeup_idle(hspi1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4,10);
	HAL_SPI_Transmit(hspi1, cfng, 8,10);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

}

/**
 * @brief		Starts the LTC68xx ADC voltage conversion
 * @param		DCP: 0 to read voltages and 1 to read temperatures
 * @param		hspi pointer to a SPI_HandleTypeDef structure that contains
 * 				the configuration information for SPI module.
 */
void ltc6804_adcv(uint8_t DCP, SPI_HandleTypeDef *hspi1){
//0110 0011
	uint8_t cmd[4];
	uint16_t cmd_pec;
	cmd[0] = (uint8_t)0x03;  //0000 0011
	cmd[1] = (uint8_t)0x60 + DCP * 16; //0110 0000
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    wakeup_idle(hspi1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4,100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

}
/*
int ltc6894_adstat( LTC6804_StatusGroupSelection chst)
{
     uint8_t cmd[4];
     uint16_t cmd_pec;
     uint8_t bit;

     if ((md <= MD_FILTERED)&&(md >= MD_FAST))
     {
        if (chst <= CHST_VD)
        {
            bit = (md & 0x02) >> 1;
            cmd[0] = bit + 0x04;
            bit = (md & 0x01) << 7;
            cmd[1] = bit + 0x60 + 0x08 + chst;

            cmd_pec = pec15(2, cmd,crcTable);

            cmd[2] = (uint8_t)(cmd_pec >> 8);
            cmd[3] = (uint8_t)cmd_pec;

            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
            	HAL_SPI_Transmit(hspi1, cmd, 4,100);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

            return 0;
        }
     }

     return -1;
}


void ltc6804_rdstatA(uint8_t ic_n,SPI_HandleTypeDef *hspi, uint16_t cell_voltages[108][2]){

	uint8_t data[8];
	 uint8_t cmd[4];
	 uint16_t cmd_pec;

	 wakeup_idle(hspi);

	 cmd[0] = 0x00;
	 cmd[1]=0x10;
	 cmd_pec = pec15(2, cmd,crcTable);

     cmd[2] = (uint8_t)(cmd_pec >> 8);
     cmd[3] = (uint8_t)cmd_pec;

//
//	 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
//	 	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
//	 HAL_SPI_Receive(hspi1, data, 8, 100);
//	 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

 	// ---- Celle 1, 2, 3
 	cmd[1] = (uint8_t)0x04;
 	cmd_pec = pec15(2, cmd, crcTable);
 	cmd[2] = (uint8_t)(cmd_pec >> 8);
 	cmd[3] = (uint8_t)(cmd_pec);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
 	HAL_SPI_Transmit(hspi, cmd, 4, 100);
 	HAL_SPI_Receive(hspi, data, 8, 100);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
 	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

 		cell_voltages[ic_n*9][0] = convert_voltage(data);
 		cell_voltages[ic_n*9+1][0] = convert_voltage(&data[2]);
 		cell_voltages[ic_n*9+2][0] = convert_voltage(&data[4]);
 		cell_voltages[ic_n*9][1] = 0;
 		cell_voltages[ic_n*9+1][1] = 0;
 		cell_voltages[ic_n*9+2][1] = 0;

 	}
 	else{

 		cell_voltages[ic_n*9][1]++;
 		cell_voltages[ic_n*9+1][1]++;
 		cell_voltages[ic_n*9+2][1]++;

 	}

 	// ---- Celle 4, 5, /
 	cmd[1] = 0x06;
 	cmd_pec = pec15(2, cmd,crcTable);
 	cmd[2] = (uint8_t)(cmd_pec >> 8);
 	cmd[3] = (uint8_t)(cmd_pec);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
 	HAL_SPI_Transmit(hspi, cmd, 4, 100);
 	HAL_SPI_Receive(hspi, data, 8, 100);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
 	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

 		cell_voltages[ic_n*9+3][0] = convert_voltage(data);
 		cell_voltages[ic_n*9+4][0] = convert_voltage(&data[2]);
 		cell_voltages[ic_n*9+3][1] = 0;
 		cell_voltages[ic_n*9+4][1] = 0;

 	}
 	else{

 		cell_voltages[ic_n*9+3][1]++;
 		cell_voltages[ic_n*9+4][1]++;

 	}

 	// ---- Celle 6, 7, 8
 	cmd[1] = 0x08;
 	cmd_pec = pec15(2, cmd,crcTable);
 	cmd[2] = (uint8_t)(cmd_pec >> 8);
 	cmd[3] = (uint8_t)(cmd_pec);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
 	HAL_SPI_Transmit(hspi, cmd, 4, 100);
 	HAL_SPI_Receive(hspi, data, 8, 100);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
 	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

 		cell_voltages[ic_n*9+5][0] = convert_voltage(data);
 		cell_voltages[ic_n*9+6][0] = convert_voltage(&data[2]);
 		cell_voltages[ic_n*9+7][0] = convert_voltage(&data[4]);
 		cell_voltages[ic_n*9+5][1] = 0;
 		cell_voltages[ic_n*9+6][1] = 0;
 		cell_voltages[ic_n*9+7][1] = 0;


 	}
 	else{

 		cell_voltages[ic_n*9+5][1]++;
 		cell_voltages[ic_n*9+6][1]++;
 		cell_voltages[ic_n*9+7][1]++;

 	}

 	// ---- Celle 9, /, /
 	cmd[1] = 0x0A;
 	cmd_pec = pec15(2, cmd,crcTable);
 	cmd[2] = (uint8_t)(cmd_pec >> 8);
 	cmd[3] = (uint8_t)(cmd_pec);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
 	HAL_SPI_Transmit(hspi, cmd, 4, 100);
 	HAL_SPI_Receive(hspi, data, 8, 100);
 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
 	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

 		cell_voltages[ic_n*9+8][0] = convert_voltage(data);
 		cell_voltages[ic_n*9+8][1] = 0;

 	}
 	else
 		cell_voltages[ic_n*9+8][1]++;
 	return;

}
*/
/*
void ltc6804_rdstatB(uint8_t ic_n, SPI_HandleTypeDef *hspi1, uint16_t aux_codes[][6]){┬
	{
	uint8_t GPIO_IN_REG;
		uint8_t data[8];
		 uint8_t cmd[4];
		 uint16_t cmd_pec;
		 uint16_t parsed_aux;
		 uint8_t gpio_reg,current_ic,current_gpio;
		 uint8_t data_counter = 0;
		 wakeup_idle(hspi1);

		 cmd[0] = 0x00;
		 cmd[1]=0x12;
		 cmd_pec = pec15(2, cmd,crcTable);

	     cmd[2] = (uint8_t)(cmd_pec >> 8);
	     cmd[3] = (uint8_t)cmd_pec;


		 HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
		 	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
		 HAL_SPI_Receive(hspi1, data, 8, 100);
		 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

		 	  for (ic_n = 0; ic_n < total_ic; ic_n++)
		 		        {
		 		            for (current_gpio = 0; current_gpio < GPIO_IN_REG; current_gpio++)
		 		            {
		 		                parsed_aux = data[data_counter] + ((uint16_t)data[data_counter+1] << 8);

		 		                aux_codes[ic_n][current_gpio + ((gpio_reg-1)*GPIO_IN_REG)] = parsed_aux;

		 		                data_counter += 2;
		 		            }

		 		            received_pec = ((uint16_t)data[data_counter] << 8) + data[data_counter + 1];

		 		            data_pec = LTC6804_Pec15_Calc(BYTE_IN_GRPREG, &data[current_ic * NUM_RX_BYTE]);

		 		            if (received_pec != data_pec)
		 		            	return -1;

		 		            data_counter += 2;
		 		        }
		 		    }

				pec_error = 0;
				}
				else
				{
				pec_error = LTC6804_ReadRawData_Reg(rd_cmd, total_ic, data);

				for (current_ic = 0; current_ic < total_ic; current_ic++)
				{
						for (current_gpio = 0; current_gpio < GPIO_IN_REG; current_gpio++)
							{
									parsed_aux = data[data_counter] + ((uint16_t)data[data_counter+1] << 8);


										if (rd_cmd == LTC6804_RDSTATA)
				{
					aux_codes[current_ic][current_gpio] = parsed_aux;
				}
				else
				{
					aux_codes[current_ic][current_gpio + GPIO_IN_REG] =  parsed_aux;
				}
				data_counter += 2;
				}

				received_pec = ((uint16_t)data[data_counter] << 8) + data[data_counter + 1];

				data_pec = LTC6804_Pec15_Calc(BYTE_IN_GRPREG, &data[current_ic * NUM_RX_BYTE]);

				if (received_pec != data_pec)return -1;

				data_counter += 2;
				}

				pec_error = 0;
				}

				return (pec_error);
				}



	}

*/
/*
LTC6804_ReadRawData_Reg(LTC6804_ReadRegister_Command rd_cmd, uint8_t n_ic,){

	{
	    //const uint8_t REG_LEN = 8;
	    uint8_t cmd[4];
	    uint16_t cmd_pec;
	    uint8_t data[8];
	    if ((rd_cmd <= LTC6804_RDSTATB)&&(rd_cmd >= LTC6804_RDCVA) && (total_ic <= TOTAL_IC)&&(total_ic >= 1) && (r_data != NULL))
	    {
	        switch (rd_cmd)
	        {
	            case LTC6804_RDCVA:{cmd[0] = 0x00;cmd[1] = 0x04;
	            }break;

	            case LTC6804_RDCVB:{cmd[0] = 0x00;cmd[1] = 0x06;
	            }break;

	            case LTC6804_RDCVC:{cmd[0] = 0x00;cmd[1] = 0x08;
	            }break;

	            case LTC6804_RDCVD:{cmd[0] = 0x00;cmd[1] = 0x0A;
	            }break;

	            case LTC6804_RDAUXA:{cmd[0] = 0x00;cmd[1] = 0x0C;
	            }break;

	            case LTC6804_RDAUXB:{cmd[0] = 0x00;cmd[1] = 0x0E;
	            }break;

	            case LTC6804_RDSTATA:{cmd[0] = 0x00;cmd[1] = 0x10;
	            }break;

	            case LTC6804_RDSTATB:{cmd[0] = 0x00;cmd[1] = 0x12;
	            }break;

	            default:break;
	        }

	        cmd_pec = LTC6804_Pec15_Calc(2, cmd);

	        cmd[2] = (uint8_t)(cmd_pec >> 8);
	        cmd[3] = (uint8_t)(cmd_pec);

	        LTC6804_WakeUp();

	        LTC6820_CS_Clear(GPIO_PTG7);

	        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	       		 	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	       		 HAL_SPI_Receive(hspi1, data, 8, 100);
	       		 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

	        return 0;
	    }

	    return -1;
	}
}
*/

	void ltc6804_adax(LTC6804_GPIOSelection_CH chg, SPI_HandleTypeDef *hspi1)
	{
	    uint8_t cmd[4];
	    uint16_t cmd_pec;

// chg 000 GPIO 1-5, 2nd Ref
//	    001 GPIO 1
//		010 GPIO 2
//		011 GPIO 3
//		100 GPIO 4
//		101 GPIO 5
	            cmd[0] = (uint8_t)0x05;  //0000 0101


	          //  cmd[1] = bit + 0x60 + chg;
	        	cmd[1] = (uint8_t)0x60 +chg; //0110 0000
	            cmd_pec = pec15(2, cmd,crcTable);

	            cmd[2] = (uint8_t)(cmd_pec >> 8);
	            cmd[3] = (uint8_t)(cmd_pec);

	            wakeup_idle(hspi1);

	            //LTC6820_CS_Clear(GPIO_PTG7);

	            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	            	HAL_SPI_Transmit(hspi1, cmd, 4,100);
	            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);


	        }

void ltc6804_rdaux(uint8_t ic_n, uint16_t aux_codes[][6],SPI_HandleTypeDef *hspi1){


	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8];

	cmd[0] = (uint8_t)0x00 + 8*ic_n;  //check
	wakeup_idle(hspi1);
	// ---- GPIO 1,2,3
	cmd[1]=0x0C;
	cmd[0]=0x00;
	cmd_pec = pec15(2, cmd, crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
		HAL_SPI_Receive(hspi1, data, 8, 100);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

		if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

			aux_codes[ic_n*9][0] = convert_voltage(data);
			//aux_codes[aux_codes*9+1][0] = convert_voltage(&data[2]);
			aux_codes[ic_n*9+2][0] = convert_voltage(&data[4]);
			aux_codes[ic_n*9][1] = 0;
			aux_codes[ic_n*9+1][1] = 0;
			aux_codes[ic_n*9+2][1] = 0;

			}
			else{

				aux_codes[ic_n*9][1]++;
				aux_codes[ic_n*9+1][1]++;
				aux_codes[ic_n*9+2][1]++;

			}

		// ---- GPIO 4, 5
			cmd[1] = 0x0e;
			cmd_pec = pec15(2, cmd,crcTable);
			cmd[2] = (uint8_t)(cmd_pec >> 8);
			cmd[3] = (uint8_t)(cmd_pec);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
			HAL_SPI_Transmit(hspi1, cmd, 4, 100);
			HAL_SPI_Receive(hspi1, data, 8, 100);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

			if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

				aux_codes[ic_n*9+3][0] = convert_voltage(data);
				aux_codes[ic_n*9+4][0] = convert_voltage(&data[2]);
				aux_codes[ic_n*9+3][1] = 0;
				aux_codes[ic_n*9+4][1] = 0;


			}
			else{

				aux_codes[ic_n*9+3][1]++;
				aux_codes[ic_n*9+4][1]++;

			}

}
//
////for using GPIO 4-5 as I2C(SDA, SCL)
///*****************************************************//**
// \brief Write the LTC6804 configuration register
// This command will write the configuration registers of the LTC6804-1s
// connected in a daisy chain stack. The configuration is written in descending
// order so the last device's configuration is written first.
// @param[in] uint8_t total_ic; The number of ICs being written to.
// @param[in] uint8_t config[][6] is a two dimensional array of the configuration data that will be written, the array should contain the 6 bytes for each
// IC in the daisy chain. The lowest IC in the daisy chain should be the first 6 byte block in the array. The array should
// have the following format:
// |  config[0][0]| config[0][1] |  config[0][2]|  config[0][3]|  config[0][4]|  config[0][5]| config[1][0] |  config[1][1]|  config[1][2]|  .....    |
// |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
// |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |
// The function will calculate the needed PEC codes for the write data
// and then transmit data to the ICs on a daisy chain.
//Command Code:
//-------------
//|               |             CMD[0]                              |                            CMD[1]                             |
//|---------------|---------------------------------------------------------------|---------------------------------------------------------------|
//|CMD[0:1]     |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
//|---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
//|WRCFG:         |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   1   |
//********************************************************/
//void LTC6804_wrcomm(uint8_t total_ic, //The number of ICs being written to
//                   uint8_t config[][6] //A two dimensional array of the configuration data that will be written
//									,SPI_HandleTypeDef *hspi1
//                  )
//{
//  const uint8_t BYTES_IN_REG = 6;
//  const uint8_t CMD_LEN = 4+(8*total_ic);
//  uint8_t *cmd;
//  uint16_t cfg_pec;
//  uint8_t cmd_index; //command counter
//
//  cmd = (uint8_t *)malloc(CMD_LEN*sizeof(uint8_t));
//
//  //1
//  cmd[0] = 0x07;
//  cmd[1] = 0x21;
//  cmd[2] = 0x24;
//  cmd[3] = 0xB2;
//
//  //2
//  cmd_index = 4;
//  for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--)       // executes for each LTC6804 in daisy chain, this loops starts with
//  {
//    // the last IC on the stack. The first configuration written is
//    // received by the last IC in the daisy chain
//
//    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) // executes for each of the 6 bytes in the CFGR register
//    {
//      // current_byte is the byte counter
//
//      cmd[cmd_index] = config[current_ic-1][current_byte];            //adding the config data to the array to be sent
//      cmd_index = cmd_index + 1;
//    }
//    //3
//    cfg_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &config[current_ic-1][0]);   // calculating the PEC for each ICs configuration register data
//    cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
//    cmd[cmd_index + 1] = (uint8_t)cfg_pec;
//    cmd_index = cmd_index + 2;
//  }
//
//  //4
//  wakeup_idle(hspi1);                                //This will guarantee that the LTC6804 isoSPI port is awake.This command can be removed.
//  //5
//
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
////		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
////		HAL_SPI_Receive(hspi1, data, 8, 100);
//
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
//
//  free(cmd);
//}
///*
//  WRCFG Sequence:
//  1. Load cmd array with the write configuration command and PEC
//  2. Load the cmd with LTC6804 configuration data
//  3. Calculate the pec for the LTC6804 configuration data being transmitted
//  4. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
//  5. Write configuration data to the LTC6804 daisy chain
//*/
//
///*****************************************************//**
// \brief Write the LTC6804 configuration register
// This command will write the configuration registers of the LTC6804-1s
// connected in a daisy chain stack. The configuration is written in descending
// order so the last device's configuration is written first.
// @param[in] uint8_t total_ic; The number of ICs being written to.
// @param[in] uint8_t config[][6] is a two dimensional array of the configuration data that will be written, the array should contain the 6 bytes for each
// IC in the daisy chain. The lowest IC in the daisy chain should be the first 6 byte block in the array. The array should
// have the following format:
// |  config[0][0]| config[0][1] |  config[0][2]|  config[0][3]|  config[0][4]|  config[0][5]| config[1][0] |  config[1][1]|  config[1][2]|  .....    |
// |--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|--------------|-----------|
// |IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC2 CFGR0     |IC2 CFGR1     | IC2 CFGR2    |  .....    |
// The function will calculate the needed PEC codes for the write data
// and then transmit data to the ICs on a daisy chain.
//Command Code:
//-------------
//|               |             CMD[0]                              |                            CMD[1]                             |
//|---------------|---------------------------------------------------------------|---------------------------------------------------------------|
//|CMD[0:1]     |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
//|---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
//|WRCFG:         |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   1   |
//********************************************************/
//void LTC6804_stcomm(uint8_t total_ic //The number of ICs being written to
//		,SPI_HandleTypeDef *hspi1
//                   )
//{
//  const uint8_t BYTES_IN_REG = 6;
//  const uint8_t CMD_LEN = 4+(8*total_ic);
//  uint8_t *cmd;
//  uint16_t cfg_pec;
//  uint8_t cmd_index; //command counter
//
//  cmd = (uint8_t *)malloc(CMD_LEN*sizeof(uint8_t));
//
//  //1
//  cmd[0] = 0x07;
//  cmd[1] = 0x23;
//  cmd[2] = 0xB9;
//  cmd[3] = 0xE4;
//
//  //2
//  cmd_index = 4;
//  for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--)       // executes for each LTC6804 in daisy chain, this loops starts with
//  {
//    // the last IC on the stack. The first configuration written is
//    // received by the last IC in the daisy chain
//
//    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) // executes for each of the 6 bytes in the CFGR register
//    {
//      // current_byte is the byte counter
//
//      cmd[cmd_index] = 0x00;            //adding the config data to the array to be sent
//      cmd_index = cmd_index + 1;
//    }
//    //3
//    cfg_pec = 0x0000;   // calculating the PEC for each ICs configuration register data
//    cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
//    cmd[cmd_index + 1] = (uint8_t)cfg_pec;
//    cmd_index = cmd_index + 2;
//  }
//
//  //4
//  wakeup_idle(hspi1);                            //This will guarantee that the LTC6804 isoSPI port is awake.This command can be removed.
//  //5
//
//  spi_write_array(CMD_LEN, cmd);
//
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
////		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
////		HAL_SPI_Receive(hspi1, data, 8, 100);
//
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
//
//
//  free(cmd);
//}
///*
//  WRCFG Sequence:
//  1. Load cmd array with the write configuration command and PEC
//  2. Load the cmd with LTC6804 configuration data
//  3. Calculate the pec for the LTC6804 configuration data being transmitted
//  4. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
//  5. Write configuration data to the LTC6804 daisy chain
//*/
//
///*!******************************************************
// \brief Reads configuration registers of a LTC6804 daisy chain
//@param[in] uint8_t total_ic: number of ICs in the daisy chain
//@param[out] uint8_t r_config[][8] is a two dimensional array that the function stores the read configuration data. The configuration data for each IC
//is stored in blocks of 8 bytes with the configuration data of the lowest IC on the stack in the first 8 bytes
//block of the array, the second IC in the second 8 byte etc. Below is an table illustrating the array organization:
//|r_config[0][0]|r_config[0][1]|r_config[0][2]|r_config[0][3]|r_config[0][4]|r_config[0][5]|r_config[0][6]  |r_config[0][7] |r_config[1][0]|r_config[1][1]|  .....    |
//|--------------|--------------|--------------|--------------|--------------|--------------|----------------|---------------|--------------|--------------|-----------|
//|IC1 CFGR0     |IC1 CFGR1     |IC1 CFGR2     |IC1 CFGR3     |IC1 CFGR4     |IC1 CFGR5     |IC1 PEC High    |IC1 PEC Low    |IC2 CFGR0     |IC2 CFGR1     |  .....    |
//@return int8_t, PEC Status.
//  0: Data read back has matching PEC
//  -1: Data read back has incorrect PEC
//Command Code:
//-------------
//|CMD[0:1]   |  15   |  14   |  13   |  12   |  11   |  10   |   9   |   8   |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
//|---------------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|-------|
//|RDCFG:         |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   0   |   1   |   0   |   0   |   1   |   0   |
//********************************************************/
//int8_t LTC6804_rdcomm(uint8_t total_ic, //Number of ICs in the system
//                     uint8_t r_config[][8] //A two dimensional array that the function stores the read configuration data.
//										,SPI_HandleTypeDef *hspi1
//                    )
//{
//  const uint8_t BYTES_IN_REG = 8;
//
//  uint8_t cmd[4];
//  uint8_t *rx_data;
//  int8_t pec_error = 0;
//  uint16_t data_pec;
//  uint16_t received_pec;
//
//  rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));
//
//  //1
//  cmd[0] = 0x07;
//  cmd[1] = 0x22;
//  cmd[2] = 0x32;
//  cmd[3] = 0xD6;
//
//  //2
//  wakeup_idle(hspi1); //This will guarantee that the LTC6804 isoSPI port is awake. This command can be removed.
//  //3
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
//  spi_write_read(cmd, 4, rx_data, (BYTES_IN_REG*total_ic));         //Read the configuration data of all ICs on the daisy chain into
//  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);                       //rx_data[] array
//
//  for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)       //executes for each LTC6804 in the daisy chain and packs the data
//  {
//    //into the r_config array as well as check the received Config data
//    //for any bit errors
//    //4.a
//    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
//    {
//      r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
//    }
//    //4.b
//    received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
//    data_pec = pec15_calc(6, &r_config[current_ic][0]);
//    if (received_pec != data_pec)
//    {
//      pec_error = -1;
//    }
//  }
//
//  free(rx_data);
//  //5
//  return(pec_error);
//}
///*
//  RDCFG Sequence:
//  1. Load cmd array with the write configuration command and PEC
//  2. wakeup isoSPI port, this step can be removed if isoSPI status is previously guaranteed
//  3. Send command and read back configuration data
//  4. For each LTC6804 in the daisy chain
//    a. load configuration data into r_config array
//    b. calculate PEC of received data and compare against calculated PEC
//  5. Return PEC Error
//*/
///*
