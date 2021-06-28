/**
 * @file		can.c
 * @brief		CAN bus serialization middleware
 *
 * @date		Mar 1, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can.h"

#include "bms_fsm.h"
#include "cli_bms.h"
#include "pack.h"

FDCAN_TxHeaderTypeDef tx_header;

void can_init() {
    tx_header.BitRateSwitch       = FDCAN_BRS_OFF;
    tx_header.FDFormat            = FDCAN_CLASSIC_CAN;
    tx_header.IdType              = FDCAN_STANDARD_ID;
    tx_header.MessageMarker       = 0;
    tx_header.TxEventFifoControl  = FDCAN_STORE_TX_EVENTS;
    tx_header.TxFrameType         = FDCAN_DATA_FRAME;
    tx_header.ErrorStateIndicator = FDCAN_ESI_PASSIVE;
}

HAL_StatusTypeDef can_send(uint16_t id) {
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];
    if (id == ID_HV_VOLTAGE) {
        serialize_Primary_HV_VOLTAGE(
            pack_get_int_voltage(),
            pack_get_bus_voltage(),
            pack_get_max_voltage(),
            pack_get_min_voltage(),
            buffer,
            CAN_MAX_PAYLOAD_LENGTH);
    } else if (id == ID_HV_CURRENT) {
        serialize_Primary_HV_CURRENT(pack_get_current(), pack_get_power(), buffer, CAN_MAX_PAYLOAD_LENGTH);
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
        cli_bms_debug("CAN: Error sending message", 27);

    } else {
        error_unset(ERROR_CAN, 0);
        //cli_bms_debug("CAN: Sent message", 18);
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
            cli_bms_debug("CAN: Error receiving message", 29);
            return;
        }
        error_unset(ERROR_CAN, 1);

        size_t len = rx_header.DataLength >> 16;

        if (rx_header.Identifier == ID_SET_TS_STATUS) {
            Primary_SET_TS_STATUS ts_status;
            deserialize_Primary_SET_TS_STATUS(rx_data, len, &ts_status);

            switch (ts_status.ts_status_set) {
                case Primary_Ts_Status_Set_OFF:
                    fsm_catch_event(bms.fsm, BMS_EV_TS_OFF);
                    break;
                case Primary_Ts_Status_Set_ON:
                    fsm_catch_event(bms.fsm, BMS_EV_TS_ON);
                    break;
            }
        }
        if (rx_header.Identifier == ID_SET_CHG_STATUS) {
            Primary_SET_CHG_STATUS chg_status;
            deserialize_Primary_SET_CHG_STATUS(rx_data, len, &chg_status);

            switch (chg_status.status) {
                case Primary_Status_CHG_OFF:
                    break;
                case Primary_Status_CHG_TC:
                    break;
                case Primary_Status_CHG_CC:
                    break;
                case Primary_Status_CHG_CV:
                    break;
            }
        }
    }
}