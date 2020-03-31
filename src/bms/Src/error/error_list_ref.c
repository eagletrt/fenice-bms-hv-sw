/**
 * @file			error_list_ref.h
 * @brief			This file contains variables that reference node_t in error_list: error list reference/s
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

#include "error_list_ref.h"

#include <stdio.h>

/*  
    This file can contain a variable if and only if:
        1) A peripheral/device can generate data that could have errors
        1.1) This peripheral/device data must be defiend in data.c
        2) The peripheral/device data has an error_type_t descriptor
        2.1) NB two ore error_type_t descriptors can be associated to the same error_list_ref
    then:
    the name of the peripheral/device error list reference is defined as follows:
    node_t* error_list_ref_<insert the relative data name in data.c>;
*/

node_t *error_list_ref_voltages[PACK_CELL_COUNT];

node_t *error_list_ref_temperatures[PACK_TEMP_COUNT];

node_t *error_list_ref_total_voltage;
node_t *error_list_ref_max_voltage;
node_t *error_list_ref_min_voltage;

node_t *error_list_ref_avg_temperature;
node_t *error_list_ref_max_temperature;
node_t *error_list_ref_min_temperature;

node_t *error_list_ref_current;

node_t *error_list_ref_ltc[LTC6813_COUNT];

node_t *error_list_ref_can;

node_t *error_adc_init;
node_t *error_adc_timeout;

node_t **const error_list_ref_array[ERROR_NUM_ERRORS] = {NULL, error_list_ref_ltc, error_list_ref_voltages, error_list_ref_voltages, error_list_ref_temperatures, &error_list_ref_current,
														 &error_list_ref_can, [ERROR_ADC_INIT] = &error_adc_init, [ERROR_ADC_TIMEOUT] = &error_adc_timeout};