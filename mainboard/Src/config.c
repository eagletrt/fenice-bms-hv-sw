/**
 * @file		config.c
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "config.h"

#include "fenice_config.h"
#include "main.h"
#include "spi.h"

#include <string.h>
m95256_t eeprom;

bool config_init(config_t *config, uint16_t address, void *default_data, size_t size) {
    config->address = address;
    config->size    = size;
    m95256_init(&eeprom, &spi_eeprom, CS_EEPROM_GPIO_Port, CS_EEPROM_Pin);

    config_read(config);
    if (((uint8_t *)config->data)[0] != ((uint8_t *)default_data)[0]) {
        // Data in memory is gibberish
        memcpy(config->data, default_data, size);
        return false;
    }
    return true;
}

bool config_read(config_t *config) {
    return m95256_ReadBuffer(eeprom, (uint8_t *)&config->data, config->address, config->size) == EEPROM_STATUS_COMPLETE;
}

bool config_write(config_t *config) {
    return m95256_WriteBuffer(eeprom, (uint8_t *)config->data, config->address, config->size) == EEPROM_STATUS_COMPLETE;
}

void *config_get(config_t *config) {
    return config->data;
}

void config_set(config_t *config, void *data) {
    memcpy(config->data, data, config->size);
}