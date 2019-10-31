#ifndef FSM_H
#define FSM_H

#include "error.h"
#include "pack.h"

typedef struct {
	PACK_T pack;

	ERROR_STATUS_T can_error;

	ERROR_T error;
	uint8_t error_index;

	bool balancing;

} state_global_data_t;

typedef enum {
	BMS_INIT,
	BMS_IDLE,
	BMS_PRECHARGE,
	BMS_ON,
	BMS_CHARGE,
	BMS_HALT,
	BMS_NUM_STATES
} BMS_STATE_T;

extern const char *bms_state_names[BMS_NUM_STATES];

typedef BMS_STATE_T state_func_t(state_global_data_t *data);
typedef void transition_func_t(state_global_data_t *data);

#endif