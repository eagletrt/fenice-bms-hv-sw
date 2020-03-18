/**
 * @file			error_list_ref.h
 * @brief			This file contains variables that reference er_node_t in error_list: error list reference/s
 * @deatails	This file is a "database" of all variables concerning errors.
* 						<b>ERRORS ARE NOT CONTAINED IN THESE VARIABLES</b>
 * 						These variables reference positions in the error_list DatStruct.
 * 						Their main purpose is to give access in O(1) complexity to errors they are
 * 						representing. The way these error_list_ref_X variables are called is by the use of
 * 						the error_list_reference array, indexed by the error type (error_type_t)
 * 						and an offset (for error types that can have multiple instances).
 * 
 * @date			March 12, 2019
 *
 * @author		Matteo Bonora [matteo.bonora@studenti.unitn.it]
 * @author		Simone Ruffini [simone.ruffini@studenti.unitn.it]
 */

#ifndef ERROR_LIST_REF_H
#define ERROR_LIST_REF_H

#include <stdio.h>

#include "../../../fenice_config.h"
#include "error/error_def.h"
#include "error/list.h"

/*  
    This file can contain a variable if and only if:
        1) A peripheral/device can generate data that could have errors
        1.1) This peripheral/device data must be defiend in data.c
        2) The peripheral/device data has an error_type_t descriptor
        2.1) NB two ore error_type_t descriptors can be associated to the same error_list_ref
    then:
    the name of the peripheral/device error list reference is defined as follows:
    er_node_t* error_list_ref_<insert the relative data name in data.c>;
*/

er_node_t *error_list_ref_voltages[PACK_CELL_COUNT];

er_node_t *error_list_ref_temperatures[PACK_TEMP_COUNT];

er_node_t *error_list_ref_total_voltage;
er_node_t *error_list_ref_max_voltage;
er_node_t *error_list_ref_min_voltage;

er_node_t *error_list_ref_avg_temperature;
er_node_t *error_list_ref_max_temperature;
er_node_t *error_list_ref_min_temperature;

er_node_t *error_list_ref_current;

er_node_t *error_list_ref_ltc[LTC6813_COUNT];

er_node_t *error_list_ref_can;

er_node_t *error_adc_init;
er_node_t *error_adc_timeout;

/**
 * @brief	this array contains the references to error_list_ref_XXX variables
 * @details	this array is indexed by using error_type_t enum, an element is contained in this array if:
 *          is defined prevoiusly in this file as an er_node_t variable
 *          The position that this variable will take in this array dependes on the descriptor 
 *          error_type_t value associated to it, example:
 *          error_list_ref_voltages is associated to both ERROR_CELL_UNDER_VOLTAGE and ERROR_CELL_OVER_VOLTAGE,
 *          so error_list_ref_voltages must be placed in position valueof(ERROR_CELL_UNDER_VOLTAGE) 
 *          and valueof(ERROR_CELL_OVER_VOLTAGE)
 * 
 */
er_node_t **error_list_ref_array[ERROR_NUM_ERRORS] = {NULL, error_list_ref_ltc, error_list_ref_voltages, error_list_ref_voltages, error_list_ref_temperatures, &error_list_ref_current,
													  &error_list_ref_can, [ERROR_ADC_INIT] = &error_adc_init, [ERROR_ADC_TIMEOUT] = &error_adc_timeout};
#endif