/**
 * @file		can.c
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can.h"

HAL_StatusTypeDef can_send(uint16_t id) {
	FDCAN_TxHeaderTypeDef tx_header;

	flatcc_builder_t builder, B *;
	B = &builder;

	flatcc_builder_init(B);
	switch (id) {
		case HV_VOLTAGE:

			// HV_VOLTAGE_create(B,...parameters...);

			break;
		case HV_CURRENT:
			break;

		default:
			return HAL_ERROR;
	}

	tx_header.Identifier = id;

	size_t size;
	uint8_t buf * = flatcc_builder_finalize_buffer(B, &size);

	HAL_StatusTypeDef status = HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &tx_header, buf);

	flatcc_builder_free(buf);
	flatcc_builder_clean(B);
	return status;
}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs) {
	if (RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) {
		// New message

		if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &rx_header, rx_data) != HAL_OK) {
			errroroororor
		}
	}
}