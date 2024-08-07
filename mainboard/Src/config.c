/**
 * @file		config.c
 * @brief		This file contains the configuration handler
 *
 * @date		May 28, 2021
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "config.h"

#include "error_simple.h"
#include "mainboard_config.h"
#include "spi.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static m95256_t eeprom = NULL;

bool config_init(config_t *config, uint16_t address, uint32_t version, void *default_data, size_t size) {
    assert(size + CONFIG_VERSION_SIZE <= EEPROM_BUFFER_SIZE);

    config->address = address;
    config->version = version;
    config->size    = size;
    config->dirty   = false;

    if (eeprom == NULL) {
        m95256_init(&eeprom, &SPI_EEPROM, EEPROM_CS_GPIO_Port, EEPROM_CS_Pin);
    }

    if (config_read(config)) {
        return true;
    } else {
        // Data in eeprom is gibberish
        config_set(config, default_data);
    }
    return false;
}

bool config_read(config_t *config) {
    uint8_t buffer[EEPROM_BUFFER_SIZE] = {0};

    if (m95256_ReadBuffer(eeprom, buffer, config->address, config->size + CONFIG_VERSION_SIZE) ==
        EEPROM_STATUS_COMPLETE) {
        error_simple_reset(ERROR_GROUP_ERROR_EEPROM_COMM, 0);

        // Check if EEPROM's version matches config's
        if (*((CONFIG_VERSION_TYPE *)buffer) == config->version) {
            memcpy(config->data, buffer + CONFIG_VERSION_SIZE, config->size);
            config->dirty = false;

            return true;
        }
    } else {
        error_simple_set(ERROR_GROUP_ERROR_EEPROM_COMM, 0);
    }
    return false;
}

bool config_write(config_t *config) {
    if (config->dirty) {
        uint8_t buffer[EEPROM_BUFFER_SIZE] = {0};
        memcpy(buffer, &(config->version), CONFIG_VERSION_SIZE);           // Copy version
        memcpy(buffer + CONFIG_VERSION_SIZE, config->data, config->size);  // Copy data after version

        if (m95256_WriteBuffer(eeprom, buffer, config->address, config->size + CONFIG_VERSION_SIZE) ==
            EEPROM_STATUS_COMPLETE) {
            error_simple_reset(ERROR_GROUP_ERROR_EEPROM_COMM, 0);

            // Read just-written data and compare for errors
            uint8_t testbuf[EEPROM_BUFFER_SIZE] = {0};
            if (m95256_ReadBuffer(eeprom, testbuf, config->address, config->size + CONFIG_VERSION_SIZE) ==
                EEPROM_STATUS_COMPLETE) {
                if (memcmp(buffer, testbuf, CONFIG_VERSION_SIZE) == 0) {
                    error_simple_reset(ERROR_GROUP_ERROR_EEPROM_WRITE, 0);
                    config->dirty = false;
                    return true;
                }

                error_simple_set(ERROR_GROUP_ERROR_EEPROM_WRITE, 0);
                return false;
            }

        } else {
            error_simple_set(ERROR_GROUP_ERROR_EEPROM_COMM, 0);
            return false;
        }
    }
    return true;
}

void *config_get(config_t *config) {
    return config->data;
}

void config_set(config_t *config, void *data) {
    memcpy(config->data, data, config->size);  // Copy data
    config->dirty = true;
}
