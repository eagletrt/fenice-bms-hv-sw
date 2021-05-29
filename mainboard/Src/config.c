/**
 * @file		config.c
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "config.h"

#include <string.h>

#include "m95256.h"

m95256_t memory;
config_t config = config_default;

bool config_write(config_t *config) {
	return m95256_WriteBuffer(memory, (uint8_t *)config, CONFIG_ADDRESS, sizeof(*config)) == EEPROM_STATUS_COMPLETE;
}

bool config_load() {
	m95256_ReadBuffer(memory, (uint8_t *)&config, CONFIG_ADDRESS, sizeof(config));
	return config.version == CONFIG_VERSION;
}

void config_get(config_t *config) {
	memcpy(&config, config, sizeof(config));
}