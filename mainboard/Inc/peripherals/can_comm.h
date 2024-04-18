/**
 * @file can_comm.h
 * @brief CAN bus serialization middleware
 *
 * @date Mar 1, 2021
 * @author Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author Antonio Gelain [antonio.gelain@studenti.unitn.it]
 */

#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "bms_network.h"
#include "primary_network.h"
#include "can.h"

#define CAN_1MBIT_PRE 3
#define CAN_1MBIT_BS1 CAN_BS1_12TQ
#define CAN_1MBIT_BS2 CAN_BS2_2TQ

#define CAN_125KBIT_PRE 20
#define CAN_125KBIT_BS1 CAN_BS1_15TQ
#define CAN_125KBIT_BS2 CAN_BS2_2TQ

typedef enum { CAN_BITRATE_1MBIT, CAN_BITRATE_125KBIT } CAN_Bitrate;

#define CAN_SLAVE_START_FILTER_BANK 14

extern bool is_handcart_connected;
extern primary_hv_debug_signals_converted_t conv_debug;

/**
 * @brief Check if the message received via CAN are forwarded
 * 
 * @return true If the messages are forwarded
 * @return false Otherwise
 */
bool can_is_forwarding();

/**
  * @brief Check if the lv bms has done the discharge
  *
  * @return true If the lv bms has done the discharge
  * @return false Otherwise
  */
bool can_is_lv_discharged();

/** @brief Initialize external CAN peripheral */
void can_car_init();
/** @brief Initialize internal CAN peripheral */
void can_bms_init();

/**
 * @brief Send data via a CAN peripheral
 * 
 * @param hcan The CAN handler structure
 * @param buffer The data to be sent
 * @param header The CAN header structure
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef can_send(CAN_HandleTypeDef * hcan, uint8_t * buffer, CAN_TxHeaderTypeDef * header);
/**
 * @brief Send data via the external CAN peripheral
 * 
 * @param id The ID of the message to send
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef can_car_send(uint16_t id);
/**
 * @brief Send data via the internal CAN peripheral
 * 
 * @param id The ID of the message to send
 * @return HAL_StatusTypeDef The result of the operation
 */
HAL_StatusTypeDef can_bms_send(uint16_t id);

/** @brief Check if the internal CAN peripheral is working */
void can_cellboards_check();

/**
 * @brief Change the CAN bitrate
 * 
 * @param hcan The CAN handler structure
 * @param bitrate The new bitrate
 */
void CAN_change_bitrate(CAN_HandleTypeDef * hcan, CAN_Bitrate bitrate);

#endif // CAN_COMM_H
