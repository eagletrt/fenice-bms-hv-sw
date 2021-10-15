/**
 * @file		config.c
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "config.h"

#include "error.h"
#include "mainboard_config.h"
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
    *config         = (config_t)malloc(sizeof(struct config));
    (*config)->data = malloc(size);

    (*config)->address = address;
    (*config)->size    = size;
    (*config)->dirty   = false;

    if (eeprom == NULL) {
        m95256_init(&eeprom, &SPI_EEPROM, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin);
    }

    if (config_read(*config)) {
        error_reset(ERROR_EEPROM_COMM, 0);
        if (((uint8_t *)(*config)->data)[0] == ((uint8_t *)default_data)[0]) {
            return true;
        }
        // Data in eeprom is gibberish
        memcpy((*config)->data, default_data, size);
        (*config)->dirty = true;

        return false;
    }
    error_set(ERROR_EEPROM_COMM, 0, HAL_GetTick());
    return false;
}

void config_deinit(config_t config) {
    free(config->data);
    free(config);
}

bool config_read(config_t config) {
    uint8_t buffer[64] = {0};

    if (m95256_ReadBuffer(eeprom, buffer, config->address, config->size) == EEPROM_STATUS_COMPLETE) {
        error_reset(ERROR_EEPROM_COMM, 0);
        memcpy(config->data, buffer, config->size);
        config->dirty = false;
        return true;
    }

    error_set(ERROR_EEPROM_COMM, 0, HAL_GetTick());
    return false;
}

bool config_write(config_t config) {
    if (config->dirty) {
        if (m95256_WriteBuffer(eeprom, (uint8_t *)config->data, config->address, config->size) ==
            EEPROM_STATUS_COMPLETE) {
            // Read memory to check write success
            uint8_t testbuf[EEPROM_BUFFER_SIZE] = {0};
            if (m95256_ReadBuffer(eeprom, testbuf, config->address, config->size) == EEPROM_STATUS_COMPLETE) {
                if (memcmp(config->data, testbuf, config->size) != 0) {
                    error_set(ERROR_EEPROM_WRITE, 0, HAL_GetTick());
                } else {
                    error_reset(ERROR_EEPROM_WRITE, 0);
                }
            }

            config->dirty = false;
            return true;
        } else {
            error_set(ERROR_EEPROM_COMM, 0, HAL_GetTick());
            return false;
        }
    }
    error_reset(ERROR_EEPROM_COMM, 0);
    return true;
}

void *config_get(config_t config) {
    return config->data;
}

void config_set(config_t config, void *data) {
    memcpy(config->data, data, config->size);
    config->dirty = true;
}