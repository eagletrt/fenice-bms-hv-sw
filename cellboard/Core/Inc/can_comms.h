/**
 * @file		can_comms.h
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef _CAN_COMMS_

#include "../Lib/can/lib/bms/c/ids.h"
#include "../Lib/can/lib/bms/c/network.h"

void can_send(uint16_t id);
void can_init_with_filter();

#endif
