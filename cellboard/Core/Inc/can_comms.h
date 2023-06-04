/**
 * @file		can_comms.h
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef _CAN_COMMS_

#include "bms/network.h"

void can_send(uint16_t id);
void can_init_with_filter();

#endif
