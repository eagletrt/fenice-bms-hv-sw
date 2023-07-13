/**
 * @file    pack.h
 * @brief   This file contains the functions to manage the battery pack state
 *
 * @date    Apr 11, 2019
 * @author  Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#ifndef PACK_H
#define PACK_H

#include "error/error.h"
#include "main.h"

#include <inttypes.h>

#define TS_ON_VALUE  GPIO_PIN_SET
#define TS_OFF_VALUE GPIO_PIN_RESET

#define AIRN_OFF_VALUE GPIO_PIN_SET
#define AIRN_ON_VALUE  GPIO_PIN_RESET

#define AIRP_OFF_VALUE GPIO_PIN_SET
#define AIRP_ON_VALUE  GPIO_PIN_RESET

#define PRECHARGE_OFF_VALUE GPIO_PIN_SET
#define PRECHARGE_ON_VALUE  GPIO_PIN_RESET

#define BMS_FAULT_OFF_VALUE GPIO_PIN_SET
#define BMS_FAULT_ON_VALUE  GPIO_PIN_RESET

/**
 * @brief Get the AIR- status
 * 
 * @return GPIO_PinState The AIR- status
 */
GPIO_PinState pack_get_airn_off();
/**
 * @brief Set the AIR- status
 * 
 * @param value The value to set the AIR- status to
 */
void pack_set_airn_off(GPIO_PinState value);
/**
 * @brief Get the AIR+ statue
 * 
 * @return GPIO_PinState The AIR+ status
 */
GPIO_PinState pack_get_airp_off();
/**
 * @brief Set the AIR+ status
 * 
 * @param value The value to set the AIR+ status to
 */
void pack_set_airp_off(GPIO_PinState value);
/**
 * @brief Get the precharge status
 * 
 * @return GPIO_PinState The precharge status
 */
GPIO_PinState pack_get_precharge();
/**
 * @brief Set the precharge status
 * 
 * @param value The value to set the precharge status to
 */
void pack_set_precharge(GPIO_PinState value);
/**
 * @brief Get the fault status
 * 
 * @return GPIO_PinState The fault status
 */
GPIO_PinState pack_get_fault();
/**
 * @brief Set the fault status
 * 
 * @param value The value to set the fault status to
 */
void pack_set_fault(GPIO_PinState value);

/**
 * @brief Set the pack to the default off state
 * 
 * @param prech_delay The delay to wait to set the precharge off
 */
void pack_set_default_off(uint16_t prech_delay);

#endif // PACK_H
