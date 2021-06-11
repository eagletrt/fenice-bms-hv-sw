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
#include "bal.h"
#include "m95256.h"

#include <inttypes.h>

struct config {
    uint8_t version;
    uint16_t address;
    size_t size;
    void *data;
};

typedef struct config config_t;

bool config_init(config_t *config, uint16_t address, void *default_data, size_t size);
bool config_write(config_t *config);
bool config_read(config_t *config);

void *config_get(config_t *config);
void config_set(config_t *config, void *data);

#endif