/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.h
  * @brief   This file contains all the function prototypes for
  *          the tim.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __TIM_H__
#define __TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim9;

/* USER CODE BEGIN Private defines */

/**
 * @brief     Check whether the timer is placed in either APB1 or APB2 bus
 * 
 * @param     __HANDLE__ TIM Handle
 * @return    True if the timer is on APB1 bus false if it is on APB2 
 */
#define _M_GET_TIM_APB_PLACEMENT(__HANDLE__) (((__HANDLE__)->Instance < (TIM_TypeDef *)APB2PERIPH_BASE) ? 1U : 0U)

#define TIM_GET_FREQ(TIM)             (uint32_t)(TIM_GetInternalClkFreq((TIM)) / ((TIM)->Instance->PSC + 1))

#define TIM_MS_TO_TICKS(TIM, MS)      (uint32_t)(((uint64_t)TIM_GET_FREQ((TIM)) * (MS)) / 1000)

#define TIM_TICKS_TO_MS(TIM, TICKS)   (uint32_t)(((uint64_t)(TICKS) * 1000) / TIM_GET_FREQ((TIM)))

/* USER CODE END Private defines */

void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);
void MX_TIM5_Init(void);
void MX_TIM9_Init(void);

/* USER CODE BEGIN Prototypes */

uint16_t TIM_computePulse(TIM_HandleTypeDef *htim, uint32_t chFrequency);
uint32_t TIM_GetInternalClkFreq(TIM_HandleTypeDef *htim);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H__ */

