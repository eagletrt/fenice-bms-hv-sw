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

#include <stdlib.h>
#include <string.h>

struct config {
    uint8_t version;
    uint16_t address;
    bool dirty;
    size_t size;
    void *data;
};
static m95256_t eeprom = NULL;

bool config_init(config_t *config, uint16_t address, void *default_data, size_t size) {
    *config            = (config_t)malloc(sizeof(struct config));
    (*config)->address = address;
    (*config)->size    = size;
    (*config)->dirty   = false;

    if (eeprom == NULL) {
        m95256_init(&eeprom, &spi_eeprom, CS_EEPROM_GPIO_Port, CS_EEPROM_Pin);
    }

    config_read(*config);
    if (((uint8_t *)(*config)->data)[0] != ((uint8_t *)default_data)[0]) {
        // Data in memory is gibberish
        memcpy((*config)->data, default_data, size);
        (*config)->dirty = true;
        return false;
    }
    return true;
}

bool config_read(config_t config) {
    return m95256_ReadBuffer(eeprom, (uint8_t *)&config->data, config->address, config->size) == EEPROM_STATUS_COMPLETE;
}

bool config_write(config_t config) {
    if (config->dirty) {
        if (m95256_WriteBuffer(eeprom, (uint8_t *)config->data, config->address, config->size) ==
            EEPROM_STATUS_COMPLETE) {
            config->dirty = false;
            return true;
        }
    }
    return true;
}

void *config_get(config_t config) {
    return config->data;
}

void config_set(config_t config, void *data) {
    memcpy(config->data, data, config->size);
    config->dirty = true;
}