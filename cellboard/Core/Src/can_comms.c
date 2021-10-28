/**
 * @file		can_comms.c
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "can_comms.h"

#include "bal_fsm.h"
#include "can.h"
#include "cellboard_config.h"
#include "error.h"
#include "main.h"
#include "temp.h"
#include "volt.h"

#include <math.h>

void can_send(uint16_t id) {
    CAN_TxHeaderTypeDef tx_header;
    uint8_t buffer[CAN_MAX_PAYLOAD_LENGTH];

    tx_header.ExtId = 0;
    tx_header.StdId = id;
    tx_header.IDE   = CAN_ID_STD;
    tx_header.RTR   = CAN_RTR_DATA;

    if (id == ID_BOARD_STATUS) {
        bms_balancing_status state = BAL_OFF;
        switch (fsm_get_state(bal.fsm)) {
            case BAL_OFF:
                state = bms_balancing_status_OFF;
                break;
            case BAL_COMPUTE:
                state = bms_balancing_status_COMPUTE;
                break;
            case BAL_DISCHARGE:
                state = bms_balancing_status_DISCHARGE;
                break;
        }

        tx_header.DLC = serialize_bms_BOARD_STATUS(buffer, cellboard_index, (uint8_t *)&errors, state);
    } else if (id == ID_TEMP_STATS) {
        tx_header.DLC = serialize_bms_TEMP_STATS(
            buffer,
            cellboard_index,
            temp_serialize(temp_get_average()),
            temp_serialize(temp_get_max()),
            temp_serialize(temp_get_min()));
    } else if (id == ID_VOLTAGES_0) {
        tx_header.DLC = serialize_bms_VOLTAGES_0(buffer, cellboard_index, voltages[0], voltages[1], voltages[2]);
    } else if (id == ID_VOLTAGES_1) {
        tx_header.DLC = serialize_bms_VOLTAGES_1(buffer, cellboard_index, voltages[3], voltages[4], voltages[5]);
    } else if (id == ID_VOLTAGES_2) {
        tx_header.DLC = serialize_bms_VOLTAGES_2(buffer, cellboard_index, voltages[6], voltages[7], voltages[8]);
    } else if (id == ID_VOLTAGES_3) {
        tx_header.DLC = serialize_bms_VOLTAGES_3(buffer, cellboard_index, voltages[9], voltages[10], voltages[11]);
    } else if (id == ID_VOLTAGES_4) {
        tx_header.DLC = serialize_bms_VOLTAGES_4(buffer, cellboard_index, voltages[12], voltages[13], voltages[14]);
    } else if (id == ID_VOLTAGES_5) {
        tx_header.DLC = serialize_bms_VOLTAGES_5(buffer, cellboard_index, voltages[15], voltages[16], voltages[17]);
    }

    uint32_t mailbox = 0;

    if(HAL_CAN_IsTxMessagePending(&BMS_CAN, CAN_TX_MAILBOX0))
        mailbox = CAN_TX_MAILBOX0;
    else if(HAL_CAN_IsTxMessagePending(&BMS_CAN, CAN_TX_MAILBOX1))
        mailbox = CAN_TX_MAILBOX1;
    else
        mailbox = CAN_TX_MAILBOX2;

    if (HAL_CAN_AddTxMessage(&BMS_CAN, &tx_header, buffer, &mailbox) == HAL_OK) {
        ERROR_UNSET(ERROR_CAN);
    } else {
        ERROR_SET(ERROR_CAN);
    }
}
