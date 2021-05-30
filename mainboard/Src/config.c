/**
 * @file		config.c
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "config.h"

#include <stdbool.h>
#include <string.h>

const config_t config_default = {
	CONFIG_VERSION,
	0, 0};

bool dirty = false;
config_t config = config_default;

bool config_write(m95256_t memory) {
	if (!dirty) {
		return true;
	}
	if (m95256_WriteBuffer(memory, (uint8_t *)&config, CONFIG_ADDRESS, sizeof(config)) == EEPROM_STATUS_COMPLETE) {
		dirty = false;
		return true;
	}

	return false;
}

bool config_load(m95256_t memory) {
	config_t tmpconf;
	if (m95256_ReadBuffer(memory, (uint8_t *)&tmpconf, CONFIG_ADDRESS, sizeof(config)) != EEPROM_STATUS_COMPLETE) {
		return false;
	}
	if (config.version == CONFIG_VERSION) {
		memcpy(&config, &tmpconf, sizeof(config));
		dirty = true;
		return true;
	}
	return false;
}

void config_get(config_t *conf) {
	memcpy(conf, &config, sizeof(config));
}

void config_set(config_t *conf) {
	memcpy(&config, conf, sizeof(config));
	dirty = true;
}