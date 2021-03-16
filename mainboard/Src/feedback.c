/**
 * @file		feedback.c
 * @brief		Feedback parsing utilities
 *
 * @date		Mar 16, 2021
 * @author  	Matteo Bonora [matteo.bonora@studenti.unitn.it]
 */

#include "feedback.h"

#include <stm32g4xx_hal.h>

#include "main.h"

feedback_t feedback;

void feedback_read(feedback_t fb_mask) {
	//initialize the feedback value to 0 on the mask bits;
	feedback &= (~fb_mask);
	for (uint8_t i = 0; i < FEEDBACK_N; ++i) {
		if ((1U << i) & fb_mask) {
			HAL_GPIO_WritePin(MUX_A0_GPIO_Port, MUX_A0_Pin, (i & 0b00000001));
			HAL_GPIO_WritePin(MUX_A1_GPIO_Port, MUX_A1_Pin, (i & 0b00000010));
			HAL_GPIO_WritePin(MUX_A2_GPIO_Port, MUX_A2_Pin, (i & 0b00000100));
			HAL_GPIO_WritePin(MUX_A3_GPIO_Port, MUX_A3_Pin, (i & 0b00001000));

			feedback |= (HAL_GPIO_ReadPin(ANALOG_DATA_GPIO_Port, ANALOG_DATA_Pin) << i);
		}
	}
}

bool feedback_check(feedback_t fb_check_mask, feedback_t fb_value, error_id error_id) {
	//remove not used bit with the mask and find the ones that differ with the xor
	uint16_t difference = (fb_check_mask & feedback) ^ fb_value;

	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
		if (fb_check_mask & (1U << i)) {
			error_toggle_check(difference & (1 << i), error_id, i);
		}
	}

	return feedback == fb_value;
}

// //this ckeck is performed during ON state and from PRECHARGE TO ON
// bool pack_feedback_check_on() {
// 	uint16_t difference = pd_feedback ^ FEEDBACK_ON;

// 	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
// 		error_toggle_check(difference & (1 << i), ERROR_FEEDBACK_HARD, i);
// 	}

// 	return pd_feedback == FEEDBACK_ON;
// }

// bool pack_feedback_check_charge() {
// 	uint16_t difference = pd_feedback ^ FEEDBACK_CHARGE;

// 	for (uint8_t i = 0; i < FEEDBACK_N; i++) {
// 		error_toggle_check(difference & (1 << i), ERROR_FEEDBACK_HARD, i);
// 	}

// 	return pd_feedback == FEEDBACK_CHARGE;
// }