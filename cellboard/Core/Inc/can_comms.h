/**
 * @file		can_comms.h
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef CAN_COMMS_H
#define CAN_COMMS_H

#include <inttypes.h>

/** @brief Init CAN with filters and masks */
void can_init_with_filter();
/**
 * @brief Send data with the specified ID to the mainboard via CAN
 * 
 * @param id The ID of the CAN message
 */
void can_send(uint16_t id);

#endif  // CAN_COMMS_H
