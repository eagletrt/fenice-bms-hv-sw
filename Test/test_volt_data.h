#ifndef TEST_VOLT_DATA_H
#define TEST_VOLT_DATA_H

#include <inttypes.h>

uint16_t volt_max(uint16_t volts[]);
uint16_t volt_min(uint16_t volts[]);
void volt_gen_random(uint16_t volts[], uint16_t max_delta, uint16_t center);
#endif