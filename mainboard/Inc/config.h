/**
 * @file		config.h
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef CONFIG_H
#define CONFIG_H
#include <inttypes.h>

#include "bal.h"

#define CONFIG_ADDRESS 0x00

#define CONFIG_VERSION 0x01

struct config {
	uint32_t version;
};
typedef struct config config_t;
const config_t config_default = {
	CONFIG_VERSION};

bool config_write(config_t *config);
bool config_load();
void config_get(config_t *config);

#endif