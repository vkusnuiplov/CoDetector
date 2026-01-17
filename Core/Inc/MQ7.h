/*
 * MQ7.h
 *
 *  Created on: Jan 14, 2026
 *      Author: kosti
 */

#ifndef MQ7_H
#define MQ7_H

#include "main.h"

// Функція таймера (викликати в HAL_TIM_PeriodElapsedCallback)
void MQ7_Timer(void);

// Функція обробки даних (викликати в HAL_ADC_ConvCpltCallback)
void MQ7_ppm_calculation (uint32_t raw_adc_value);


void MQ7_alarm_state (void);


#endif
