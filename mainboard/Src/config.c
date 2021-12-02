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

#define VERSION_TYPE uint32_t
#define VERSION_SIZE sizeof(VERSION_TYPE)
struct config {
    VERSION_TYPE version;
    uint16_t address;
    bool dirty;
    size_t size;
    void *data;
};
static m95256_t eeprom = NULL;

bool config_init(config_t *config, uint16_t address, uint32_t version, void *default_data, size_t size) {
    assert(size + VERSION_SIZE <= EEPROM_BUFFER_SIZE);

    *config         = (config_t)malloc(sizeof(struct config));
    (*config)->data = malloc(size);

    (*config)->address = address;
    (*config)->version = version;
    (*config)->size    = size;
    (*config)->dirty   = false;

    if (eeprom == NULL) {
        m95256_init(&eeprom, &SPI_EEPROM, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin);
    }

    if (config_read(*config)) {
        return true;
    } else {
        // Data in eeprom is gibberish
        config_set(*config, default_data);
    }
    return false;
}

void config_deinit(config_t config) {
    free(config->data);
    free(config);
}

bool config_read(config_t config) {
    uint8_t buffer[EEPROM_BUFFER_SIZE] = {0};

    if (m95256_ReadBuffer(eeprom, buffer, config->address, config->size + VERSION_SIZE) == EEPROM_STATUS_COMPLETE) {
        error_reset(ERROR_EEPROM_COMM, 0);

        // Check if EEPROM's version matches config's
        if (*((VERSION_TYPE *)buffer) == config->version) {
            memcpy(config->data, buffer + VERSION_SIZE, config->size);
            config->dirty = false;

            return true;
        }
    } else {
        error_set(ERROR_EEPROM_COMM, 0, HAL_GetTick());
    }
    return false;
}

bool config_write(config_t config) {
    if (config->dirty) {
        uint8_t buffer[EEPROM_BUFFER_SIZE] = {0};
        memcpy(buffer, &(config->version), VERSION_SIZE);                   // Copy version
        memcpy(buffer + VERSION_SIZE, config->data, config->size);          // Copy data after version

        if (m95256_WriteBuffer(eeprom, buffer, config->address, config->size + VERSION_SIZE) ==
            EEPROM_STATUS_COMPLETE) {
            error_reset(ERROR_EEPROM_COMM, 0);

            // Read just-written data and compare for errors
            uint8_t testbuf[EEPROM_BUFFER_SIZE] = {0};
            if (m95256_ReadBuffer(eeprom, testbuf, config->address, config->size + VERSION_SIZE) == EEPROM_STATUS_COMPLETE) {
                if (memcmp(buffer, testbuf, VERSION_SIZE) == 0 ) {
                    error_reset(ERROR_EEPROM_WRITE, 0);
                    config->dirty = false;
                    return true;
                }

                error_set(ERROR_EEPROM_WRITE, 0, HAL_GetTick());
                return false;
            }

        } else {
            error_set(ERROR_EEPROM_COMM, 0, HAL_GetTick());
            return false;
        }
    }
    return true;
}

void *config_get(config_t config) {
    return config->data;
}

void config_set(config_t config, void *data) {
    memcpy(config->data, data, config->size);   // Copy data
    config->dirty = true;
}