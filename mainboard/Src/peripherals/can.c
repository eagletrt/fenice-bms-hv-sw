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
	flatcc_builder_t builder;
	flatcc_builder_t *B = &builder;
	flatcc_builder_init(B);

	switch (id) {
		case ID_HV_VOLTAGE:

			HV_VOLTAGE_create(B, 42000, 41900, 38000, 37500);

			break;
		case ID_HV_CURRENT:
			HV_CURRENT_create(B, 2000, 69);	 // nice
			break;

		default:
			return HAL_ERROR;
	}
	size_t size;
	uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);
	uint8_t buffer[8];
	memcpy(buffer, buf, 8);

	tx_header.Identifier = id;
	tx_header.DataLength = size << 16;	// Only valid for classic can frames

	HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, buf);
	if (status != HAL_OK) {
		error_set(ERROR_CAN, 0, HAL_GetTick());
	} else {
		error_unset(ERROR_CAN, 0);
	}

	free(buf);
	flatcc_builder_reset(B);
	flatcc_builder_clear(B);
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

		if (rx_header.Identifier == ID_SET_TS_STATUS) {
			uint8_t status = SET_TS_STATUS_ts_status_set((void *)rx_data);

			switch (status) {
				case Ts_Status_Set_OFF:
					//tsoff
					break;
				case Ts_Status_Set_ON:
					//tson
					break;
			}
		}
	}
}