/**
 * @file		can.c
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can.h"

#include "pack.h"

static flatcc_builder_t builder, *B;

void can_init() {
	flatcc_builder_init(B);
	B = &builder;
}

HAL_StatusTypeDef can_send(uint16_t id) {
	FDCAN_TxHeaderTypeDef tx_header;

	switch (id) {
		case HV_VOLTAGE:

			//HV_VOLTAGE_create(B, ...data... );

			break;
		case HV_CURRENT:
			break;

		default:
			return HAL_ERROR;
	}

	tx_header.Identifier = id;

	size_t size;
	uint8_t *buf = flatcc_builder_finalize_buffer(B, &size);

	HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, buf);

	flatcc_builder_reset(B);
	return status;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
		// New message
		uint8_t rx_data[8] = {'\0'};
		FDCAN_RxHeaderTypeDef rx_header;

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
			error_set(ERROR_CAN, 0, HAL_GetTick());
			return;
		}
		error_unset(ERROR_CAN, 0);

		if (rx_header.Identifier == SET_TS_STATUS) {
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