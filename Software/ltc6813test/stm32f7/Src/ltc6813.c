/*
 * ltc6813.c
 *
 *  Created on: 27 nov 2018
 *      Author: Utente
 */
#include "ltc6813.h"

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

uint16_t pec15(uint8_t len,uint8_t data[],uint16_t crcTable[] ){

    uint16_t remainder,address;
	remainder = 16;					// PEC seed
	for (int i = 0; i < len; i++){

			address = ((remainder >> 7) ^ data[i]) & 0xff;//calculate PEC table address
			remainder = (remainder << 8 ) ^ crcTable[address];

	}
    return (remainder*2);//The CRC15 has a 0 in the LSB so the final value must be multiplied by 2

}

void ltc6813_adcv(uint8_t DCP, SPI_HandleTypeDef *hspi1){
		uint8_t cmd[4];
		uint16_t cmd_pec;
		cmd[0] = (uint8_t)0x03;
		cmd[1] = (uint8_t)0x60 + DCP * 16;
		//cmd_pec = pec15(2, cmd,crcTable);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
	    cmd[3] = (uint8_t)(cmd_pec);

	    wakeup_idle(hspi1);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_SPI_Transmit(hspi1, cmd, 4,100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

}

void wakeup_idle(SPI_HandleTypeDef *hspi1){

	uint8_t data = 0xFF;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, &data, 1, 1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
}

void ltc6813_rdcv_voltages(uint8_t ic_n, uint16_t cell_voltages[18][2], SPI_HandleTypeDef *hspi1){

	uint8_t cmd[4];
	uint16_t cmd_pec;
	uint8_t data[8];
	cmd[0] = (uint8_t)0x80;
	wakeup_idle(hspi1);

	// ---- Celle 1, 2, 3 rdcv A
	cmd[1] = (uint8_t)0x04;
	//cmd[1] = (uint8_t)0x10;
	cmd_pec = pec15(2, cmd, crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_SPI_Receive(hspi1, data, 8, 100);  //data 48

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
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

	// ---- Celle 4, 5, 6
	cmd[1] = 0x06;      //rdcv B
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_SPI_Receive(hspi1, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9+3][0] = convert_voltage(data);
		cell_voltages[ic_n*9+4][0] = convert_voltage(&data[2]);
		cell_voltages[ic_n*9+5][0] = convert_voltage(&data[4]);

		cell_voltages[ic_n*9+3][1] = 0;
		cell_voltages[ic_n*9+4][1] = 0;


	}
	else{

		cell_voltages[ic_n*9+3][1]++;
		cell_voltages[ic_n*9+4][1]++;

	}

	// ---- Celle 7,8,9 rdcv C
	cmd[1] = 0x08;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_SPI_Receive(hspi1, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9+6][0] = convert_voltage(data);
		cell_voltages[ic_n*9+7][0] = convert_voltage(&data[2]);
		cell_voltages[ic_n*9+8][0] = convert_voltage(&data[4]);

		cell_voltages[ic_n*9+5][1] = 0;
		cell_voltages[ic_n*9+6][1] = 0;
		cell_voltages[ic_n*9+7][1] = 0;


	}
	else{

		cell_voltages[ic_n*9+5][1]++;
		cell_voltages[ic_n*9+6][1]++;
		cell_voltages[ic_n*9+7][1]++;

	}

	// ---- Celle 10,11,12 rdcv D
	cmd[1] = 0x0A;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_SPI_Receive(hspi1, data, 8, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		cell_voltages[ic_n*9+9][0] = convert_voltage(data);
		cell_voltages[ic_n*9+10][0] = convert_voltage(data[2]);
		cell_voltages[ic_n*9+11][0] = convert_voltage(data[4]);

		cell_voltages[ic_n*9+8][1] = 0;

	}
	else
		{
		cell_voltages[ic_n*9+8][1]++;
		}

	// ---- Celle 13,14,15 rdcv E
		cmd[1] = (uint8_t)0x09;
		//cmd[1] = (uint8_t)0x10;
		cmd_pec = pec15(2, cmd, crcTable);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
		HAL_SPI_Receive(hspi1, data, 8, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
		if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

			cell_voltages[ic_n*9+12][0] = convert_voltage(data);
			cell_voltages[ic_n*9+13][0] = convert_voltage(&data[2]);
			cell_voltages[ic_n*9+14][0] = convert_voltage(&data[4]);

			cell_voltages[ic_n*9+11][1] = 0;
			cell_voltages[ic_n*9+12][1] = 0;
			cell_voltages[ic_n*9+13][1] = 0;

		}
		else{

			cell_voltages[ic_n*9+11][1]++;
			cell_voltages[ic_n*9+12][1]++;
			cell_voltages[ic_n*9+13][1]++;

		}


		// ---- Celle 16,17,18 rdcv F
				cmd[1] = (uint8_t)0x0C;
				//cmd[1] = (uint8_t)0x10;
				cmd_pec = pec15(2, cmd, crcTable);
				cmd[2] = (uint8_t)(cmd_pec >> 8);
				cmd[3] = (uint8_t)(cmd_pec);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
				HAL_SPI_Transmit(hspi1, cmd, 4, 100);
				HAL_SPI_Receive(hspi1, data, 8, 100);
				HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
				if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

					cell_voltages[ic_n*9+15][0] = convert_voltage(data);
					cell_voltages[ic_n*9+16][0] = convert_voltage(&data[2]);
					cell_voltages[ic_n*9+17][0] = convert_voltage(&data[4]);

					cell_voltages[ic_n*9+14][1] = 0;
					cell_voltages[ic_n*9+15][1] = 0;
					cell_voltages[ic_n*9+16][1] = 0;

				}
				else{

					cell_voltages[ic_n*9+14][1]++;
					cell_voltages[ic_n*9+15][1]++;
					cell_voltages[ic_n*9+16][1]++;

				}



}

//gpio conversion
void ltc6813_adax(uint8_t chg, SPI_HandleTypeDef *hspi1)
{
    uint8_t cmd[4];
    uint16_t cmd_pec;

// chg 000 GPIO 1-5, 2nd Ref
//	    001 GPIO 1
//		010 GPIO 2   read mux
//		011 GPIO 3
//		100 GPIO 4
//		101 GPIO 5

    		cmd[0] = (uint8_t)0x05;  //0000 0101
            //  cmd[1] = bit + 0x60 + chg; 01100000
            cmd[1]= 0x60;
            cmd_pec = pec15(2, cmd,crcTable);

            cmd[2] = (uint8_t)(cmd_pec >> 8);
            cmd[3] = (uint8_t)(cmd_pec);

            wakeup_idle(hspi1);


            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
            	HAL_SPI_Transmit(hspi1, cmd, 4,100);
            	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);


        }

void ltc6813_rdaux(uint8_t ic_n, uint16_t gpio_voltages[9][2],SPI_HandleTypeDef *hspi1){


uint8_t cmd[4];
uint16_t cmd_pec;
uint8_t data[8];

cmd[0] = (uint8_t)0x00 + 8*ic_n;  //check
wakeup_idle(hspi1);
// ---- GPIO 1,2,3 rdaux A
cmd[1]=0x0C;
//cmd[0]=0x00;
cmd_pec = pec15(2, cmd, crcTable);
cmd[2] = (uint8_t)(cmd_pec >> 8);
cmd[3] = (uint8_t)(cmd_pec);

HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_SPI_Receive(hspi1, data, 8, 100);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

	if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

		gpio_voltages[ic_n*9][0] = convert_voltage(data);
		gpio_voltages[ic_n*9+1][0] = convert_voltage(&data[2]);
		gpio_voltages[ic_n*9+2][0] = convert_voltage(&data[4]);
		gpio_voltages[ic_n*9][1] = 0;
		gpio_voltages[ic_n*9+1][1] = 0;
		gpio_voltages[ic_n*9+2][1] = 0;

		}
		else{

			gpio_voltages[ic_n*9][1]++;
			gpio_voltages[ic_n*9+1][1]++;
			gpio_voltages[ic_n*9+2][1]++;

		}

	// ---- GPIO 4, 5, REF  rdaux B
		cmd[1] = 0x0E;
		cmd_pec = pec15(2, cmd,crcTable);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);

		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
		HAL_SPI_Receive(hspi1, data, 8, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

		if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

			gpio_voltages[ic_n*9+3][0] = convert_voltage(data);
			gpio_voltages[ic_n*9+4][0] = convert_voltage(&data[2]);
			gpio_voltages[ic_n*9+5][0] = convert_voltage(&data[4]);
			gpio_voltages[ic_n*9+3][1] = 0;
			gpio_voltages[ic_n*9+4][1] = 0;


		}
		else{

			gpio_voltages[ic_n*9+3][1]++;
			gpio_voltages[ic_n*9+4][1]++;

		}


		// ---- GPIO 6,7,8 rdaux C
			cmd[1] = 0x0C;
			cmd_pec = pec15(2, cmd,crcTable);
			cmd[2] = (uint8_t)(cmd_pec >> 8);
			cmd[3] = (uint8_t)(cmd_pec);

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
			HAL_SPI_Transmit(hspi1, cmd, 4, 100);
			HAL_SPI_Receive(hspi1, data, 8, 100);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

				gpio_voltages[ic_n*9+6][0] = convert_voltage(data);
				gpio_voltages[ic_n*9+7][0] = convert_voltage(&data[2]);
				gpio_voltages[ic_n*9+8][0] = convert_voltage(&data[4]);
				gpio_voltages[ic_n*9+3][1] = 0;
				gpio_voltages[ic_n*9+4][1] = 0;


			}
			else{

				gpio_voltages[ic_n*9+3][1]++;
				gpio_voltages[ic_n*9+4][1]++;

			}

			// ---- GPIO 9,/,/ rdaux D
						cmd[1] = 0x0F;
						cmd_pec = pec15(2, cmd,crcTable);
						cmd[2] = (uint8_t)(cmd_pec >> 8);
						cmd[3] = (uint8_t)(cmd_pec);

						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
						HAL_SPI_Transmit(hspi1, cmd, 4, 100);
						HAL_SPI_Receive(hspi1, data, 8, 100);
						HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

						if(pec15(6, data, crcTable) == (uint16_t) (data[6]*256 + data[7])){

							gpio_voltages[ic_n*9+9][0] = convert_voltage(data);
//							gpio_voltages[ic_n*9+7][0] = convert_voltage(&data[2]);
//							gpio_voltages[ic_n*9+8][0] = convert_voltage(&data[4]);
							gpio_voltages[ic_n*9+3][1] = 0;
							gpio_voltages[ic_n*9+4][1] = 0;


						}
						else{

							gpio_voltages[ic_n*9+3][1]++;
							gpio_voltages[ic_n*9+4][1]++;

						}
}

uint16_t convert_voltage(uint8_t v_data[]){

	return v_data[0] + (v_data[1] << 8);

}

void ltc6813_adstat( SPI_HandleTypeDef *hspi1){

	uint8_t cmd[4];
	uint16_t cmd_pec;
	cmd[0] = (uint8_t)0x05;
	cmd[1] = (uint8_t)0x68;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);

    wakeup_idle(hspi1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4,100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

}

void ltc6804_rdstat(SPI_HandleTypeDef *hspi1,uint16_t stat_voltages[18][2] ){
	uint8_t cmd[4];
		uint16_t cmd_pec;
		uint8_t data[8];
		cmd[0] = (uint8_t)0x00;
		wakeup_idle(hspi1);
		//wakeup_idle1(hspi1);

	//ITMP  rdstat A
		cmd[1] = (uint8_t)0x10;
		cmd_pec = pec15(2, cmd, crcTable);
		cmd[2] = (uint8_t)(cmd_pec >> 8);
		cmd[3] = (uint8_t)(cmd_pec);


		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
		HAL_SPI_Receive(hspi1, data, 8, 100);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);


		//cell_voltages[0*9][0] = convert_voltage(data);
		stat_voltages[0*9+1][0] = convert_voltage(&data[2]);
		//cell_voltages[0*9+2][0] = convert_voltage(&data[4]);
}

void ltc6813_clrcell(SPI_HandleTypeDef *hspi1){
	uint8_t cmd[4];
	uint16_t cmd_pec;
	cmd[0]=(uint8_t) 0x07;
	cmd[1]=(uint8_t) 0x11;
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	wakeup_idle(&hspi1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

}

void ltc6813_clraux(SPI_HandleTypeDef *hspi1){
	uint8_t cmd[4];
	uint16_t cmd_pec;
	cmd[0]=(uint8_t) 0x07;
	cmd[1]=(uint8_t) 0x12;
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);
	wakeup_idle(&hspi1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

}

//for using GPIO 4-5 as I2C(SDA, SCL)
//
void LTC6804s_I2CMUX(SPI_HandleTypeDef *hspi1,uint8_t ch){
		uint8_t total_ic=1;
		const uint8_t CMD_LEN = 4+8*total_ic;
		  const uint8_t BYTES_IN_REG = 6;
		uint8_t cmd[8];
		uint8_t config[6] ;
           // commadn ltc1380

		// COMM commands EEPROM
//		config[0] = 0x6A;
//		config[1] = 0x08;
//		config[2] = 0x00;
//		config[3] = 0x18;
//		config[4] = 0x0A;
//		config[5] = 0xA9;
		//COMM  LTC1380

		config[0] = 0x64;
		config[1] = 0xC8;
		config[2] = 0x00+ ch; //0x08-0x0F
		//config[3] = 0x 8+ch;
		config[3] = 0x80;   //data ???
		config[4] = 0x90;
 //u have to read value (V) from GPIOX

	  //uint16_t COMM_X[16];
		 uint8_t cmd_index; //command counter
		 uint16_t cfg_pec;
	  //1   wrcomm commands
	    cmd[0] = 0x07;
	    cmd[1] = 0x21;

	    cmd[2] = 0x24;
	    cmd[3] = 0xB2;


	    //2
	    cmd_index =4;


	    for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++) // executes for each of the 6 bytes in the CFGR register
	    {
	      // current_byte is the byte counter

	      cmd[cmd_index] = config[current_byte];            //adding the config data to the array to be sent
	      cmd_index = cmd_index + 1;
	    }
	    //3
	    cfg_pec = (uint16_t)pec15(BYTES_IN_REG, config,crcTable);   // calculating the PEC for each ICs configuration register data
	       cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
	       cmd[cmd_index + 1] = (uint8_t)cfg_pec;
	       cmd_index = cmd_index + 2;
	    wakeup_idle(hspi1);                                //This will guarantee that the LTC6804 isoSPI port is awake.This command can be removed.
	     //5

	   	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	   //		HAL_SPI_Transmit(hspi1, cmd, 4, 100);
	   //		HAL_SPI_Receive(hspi1, data, 8, 100);
	   	HAL_SPI_Transmit(hspi1, cmd, 8, 100);



	   		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

}

//// Starts cell voltage  and GPIO 1&2 conversion
//void ltc6813_adcvax(
//		  uint8_t DCP,SPI_HandleTypeDef *hspi1) //Discharge Permit)
//		  {
//		uint8_t cmd[4];
//		uint16_t cmd_pec;
//		cmd[0] = (uint8_t)0x05;  //0000 0101
//		cmd[1] = (uint8_t)0x6F + DCP * 16; //0110 1111
//		cmd_pec = pec15(2, cmd,crcTable);
//		cmd[2] = (uint8_t)(cmd_pec >> 8);
//	    cmd[3] = (uint8_t)(cmd_pec);
//
//	    wakeup_idle(hspi1);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
//		HAL_SPI_Transmit(hspi1, cmd, 4,100);
//		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
//}
////balancing
//
uint16_t balancing_update(uint16_t cell_voltages[108][2]) {

    // Find lowest cell
	 uint8_t i;
	 uint8_t TOT_IC =1;
	 uint8_t CELL_MARGIN =400;//mv
		    uint16_t lowest_v = 42250;
		    for (i=0;i<TOT_IC*9;i++){
+
		        if (cell_voltages[i][0] != -1 && cell_voltages[i][0] < lowest_v)
		            lowest_v = cell_voltages[i][0];
		    }
		    return i;
    // Determine cells for balancing
    for (i=0;i<9*TOT_IC;i++){
        set_balancing(i, cell_voltages[i][0] != -1 && cell_voltages[i][0] > lowest_v + CELL_MARGIN);
    }
}

void balancing_stop() {
    uint8_t i;
    uint8_t TOT_IC =1;
    for (i=0;i<TOT_IC*9;i++)
        set_balancing(i, 0);
}

void set_balancing(uint8_t cell, uint8_t state) {
  //1
    uint8_t ltc = cell % 9;   //1
    cell -= ltc * 9;//1-9

//    if (cell >= 0 && cell < 8) {
//        if (state)
//            ltc6804_cfg[ltc][4] |= 0b1 << cell;
//        else
//            ltc6804_cfg[ltc][4] &= ~(0b1 << cell);
//    } else if (cell > 7 && cell < 12) {
//        if (state)
//            ltc6804_cfg[ltc][5] |= 0b1 << (cell-8);
//        else
//            ltc6804_cfg[ltc][5] &= ~(0b1 << (cell-8));
//    }
}

void ltc6813_DischargeCell_Enable(SPI_HandleTypeDef *hspi1){

	uint8_t cmd[4];
	uint8_t cfng[8];
	uint16_t cmd_pec;
	cmd[0] = 0x00; //WRCFG
	cmd[1] = 0x01;
	cmd_pec = pec15(2, cmd,crcTable);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
    cmd[3] = (uint8_t)(cmd_pec);
	cfng[0] = 0x00;
	cfng[1] = 0x00;
	cfng[2] = 0x00;
	cfng[3] = 0x00;
	cfng[4] = 0x80;//DCC1


//		if (parity == 0){
//
//			cfng[4] = 0x4A;
//			cfng[5] = 0x01;
//
//		}
//		else{
//
//			cfng[4] = 0x95;
//			cfng[5] = 0x02;
//
//		}
//
//	}
//	else{
//
//		cfng[4] = 0x00;
//		cfng[5] = 0x00;
		//cfng[]

	cmd_pec = pec15(6, cfng, crcTable);
	cfng[6] = (uint8_t)(cmd_pec >> 8);
	cfng[7] = (uint8_t)(cmd_pec);

    wakeup_idle(hspi1);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi1, cmd, 4,10);
	HAL_SPI_Transmit(hspi1, cfng, 8,10);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);

}




