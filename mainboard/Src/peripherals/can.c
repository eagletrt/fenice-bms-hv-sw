/**
 * @file		can.c
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can.h"

#include "pack.h"

FDCAN_TxHeaderTypeDef tx_header;

void can_init() {
	tx_header.BitRateSwitch = FDCAN_BRS_OFF;
	tx_header.FDFormat = FDCAN_CLASSIC_CAN;
	tx_header.IdType = FDCAN_STANDARD_ID;
	tx_header.MessageMarker = 0;
	tx_header.TxEventFifoControl = FDCAN_STORE_TX_EVENTS;
	tx_header.TxFrameType = FDCAN_DATA_FRAME;
	tx_header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
}

HAL_StatusTypeDef can_send(uint16_t id) {
	uint8_t buffer[8];
	if (id == ID_HV_VOLTAGE) {
		Primary_HV_VOLTAGE voltage = {.bus_voltage = 42069, .pack_voltage = 42069, .max_cell_voltage = 4000, .min_cell_voltage = 3900};
		serialize_Primary_HV_VOLTAGE(&voltage, buffer, 8);
	} else if (id == ID_HV_CURRENT) {
		Primary_HV_CURRENT current = {.current = 690, .power = 120};
		serialize_Primary_HV_CURRENT(&current, buffer, 8);
	} else {
		return HAL_ERROR;
	}

	tx_header.Identifier = id;
	tx_header.DataLength = sizeof(Primary_HV_VOLTAGE) << 16;  // Only valid for classic can frames

	HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, buffer);
	if (status != HAL_OK) {
		error_set(ERROR_CAN, 0, HAL_GetTick());
	} else {
		error_unset(ERROR_CAN, 0);
	}

	return status;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	if (hfdcan->Instance == FDCAN1 && RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
		// New message
		uint8_t rx_data[8] = {'\0'};
		FDCAN_RxHeaderTypeDef rx_header;

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
			error_set(ERROR_CAN, 1, HAL_GetTick());
			return;
		}
		error_unset(ERROR_CAN, 1);

		size_t len = rx_header.DataLength >> 16;

		if (rx_header.Identifier == ID_HV_VOLTAGE) {
			Primary_HV_VOLTAGE voltage;
			deserialize_Primary_HV_VOLTAGE(rx_data, len, &voltage);

		} else if (rx_header.Identifier == ID_HV_CURRENT) {
			Primary_HV_CURRENT current;
			deserialize_Primary_HV_CURRENT(rx_data, len, &current);

		} else if (rx_header.Identifier == ID_SET_TS_STATUS) {
			Primary_TS_STATUS ts_status;
			deserialize_Primary_TS_STATUS(rx_data, len, &ts_status);

			switch (ts_status.ts_status) {
				case Primary_Ts_Status_Set_OFF:
					//tsoff
					break;
				case Primary_Ts_Status_Set_ON:
					//tson
					break;
			}
		}
	}
}