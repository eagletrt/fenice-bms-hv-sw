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
#include "soc.h"

#define CONFIG_ADDRESS 0x00

#define CONFIG_VERSION 0x01

struct config {
	uint32_t version;

	uint32_t total_coulomb;
	uint32_t total_joule;
	uint32_t partial_coulomb;
	uint32_t partial_joule;
};
typedef struct config config_t;

extern const config_t config_default;

bool config_write(config_t *config);
bool config_load();
void config_get(config_t *config);

#endif