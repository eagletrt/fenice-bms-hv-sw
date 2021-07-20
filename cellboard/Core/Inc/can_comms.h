/**
 * @file		can_comms.h
 * @brief		CAN communication stuff
 *
 * @date		Jul 16, 2021
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef _CAN_COMMS_

#include "../../bms/c/bms.h"
#include "../bms/ids.h"

void can_send(uint16_t id);

#endif
