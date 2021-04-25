/**
 * @file		can.c
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can.h"

#include "fsm_bms.h"
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
	uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];
	if (id == ID_HV_VOLTAGE) {
		serialize_Primary_HV_VOLTAGE(42069, 42069, 4000, 3900, buffer, CAN_MAX_PAYLOAD_LENGTH);
	} else if (id == ID_HV_CURRENT) {
		serialize_Primary_HV_CURRENT(690, 120, buffer, CAN_MAX_PAYLOAD_LENGTH);
	} else if (id == ID_TS_STATUS) {
		serialize_Primary_TS_STATUS(Primary_Ts_Status_ON, buffer, CAN_MAX_PAYLOAD_LENGTH);
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

		if (rx_header.Identifier == ID_SET_TS_STATUS) {
			Primary_TS_STATUS ts_status;
			deserialize_Primary_TS_STATUS(rx_data, len, &ts_status);

			switch (ts_status.ts_status) {
				case Primary_Ts_Status_Set_OFF:
					fsm_bms_ts_off_handler();
					break;
				case Primary_Ts_Status_Set_ON:
					fsm_bms_ts_on_handler();
					break;

				default:
					break;
			}
		}
	}
}