/**
 * @file		config.h
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <inttypes.h>

#include "bal.h"
#include "m95256.h"
#include "soc.h"

#define CONFIG_ADDRESS 0x00

#define CONFIG_VERSION 0x01

struct config {
	uint32_t version;

	double total_joule;
	double charge_joule;
};
typedef struct config config_t;

extern const config_t config_default;

bool config_write(m95256_t memory);
bool config_load(m95256_t memory);
void config_get(config_t *config);
void config_set(config_t *config);

#endif