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

#include "error/error_list_ref.h"

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

llist_node error_list_ref_voltages[PACK_CELL_COUNT] = {NULL};

llist_node error_list_ref_temperatures[PACK_TEMP_COUNT] = {NULL};

llist_node error_list_ref_current[1] = {NULL};
llist_node error_list_ref_ltc[LTC6813_COUNT] = {NULL};
llist_node error_list_ref_can[1] = {NULL};
llist_node error_adc_init[1] = {NULL};
llist_node error_adc_timeout[1] = {NULL};
llist_node error_feedback_hard[1] = {NULL};
llist_node error_feedback_soft[1] = {NULL};

llist_node *const error_list_ref_array[ERROR_NUM_ERRORS] = {
	[ERROR_LTC_PEC_ERROR] = error_list_ref_ltc,
	[ERROR_CELL_UNDER_VOLTAGE] = error_list_ref_voltages,
	[ERROR_CELL_OVER_VOLTAGE] = error_list_ref_voltages,
	[ERROR_CELL_OVER_TEMPERATURE] = error_list_ref_temperatures,
	[ERROR_OVER_CURRENT] = error_list_ref_current,
	[ERROR_CAN] = error_list_ref_can,
	[ERROR_ADC_INIT] = error_adc_init,
	[ERROR_ADC_TIMEOUT] = error_adc_timeout,
	[ERROR_FEEDBACK_HARD] = error_feedback_hard,
	[ERROR_FEEDBACK_SOFT] = error_feedback_soft,
};

/**
 * @brief Returns the cell memory position of error_list_ref_array
 * 
 * @details error_list_ref_array is an array of llist_node arrays (not a matrix). After first dereference
 * 			by @param id we get the memory addres of the relative error_<id> error variable. 
 * 			With that memory addres we can offset by @param offset (in case the error_<id> is an array) 
 * 			and get a memory addres: &error_<id>[offset].
 * @param 	id 			The error id
 * @param	offset		The &error_<id>[offset]
 * 
 * @returns a 
 */
llist_node *error_list_ref_array_element(uint16_t id, uint16_t offset) {
	llist_node *tmp = *(error_list_ref_array + id);
	return tmp + offset;
}