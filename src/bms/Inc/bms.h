
#include "fsm.h"

typedef enum {
	BMS_INIT,
	BMS_IDLE,
	BMS_PRECHARGE,
	BMS_ON,
	BMS_CHARGE,
	BMS_HALT,
	BMS_NUM_STATES
} bms_state_t;

extern state_func_t *const state_table[BMS_NUM_STATES][BMS_NUM_STATES];
extern const char *bms_state_names[BMS_NUM_STATES];