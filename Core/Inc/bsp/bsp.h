/**
  ******************************************************************************
  * @file    bsp.h
  * @author  vkusnuiplov
  * @brief   Board support package hreader file for connected board with drivers
  *****************************************************************************
  */

#ifndef BSP_H
#define BSP_H

#include "main.h"
#include <stdint.h>

#define BSP_BUZZER_TIMER_FREQ       1000000U
#define BSP_BUZZER_ARR_MAX          65535U
#define BSP_PWM_DUTY                2U

void BSP_Init(void);
uint32_t BSP_GetTick(void);

void BSP_LED_Green_Set(uint8_t state);
void BSP_LED_Red_Set(uint8_t state);

void BSP_Buzzer_SetFreq (uint16_t freq_hz);
void BSP_Buzzer_Stop (void);

void BSP_MQ7_Heater_Set (uint8_t state);
uint32_t BSP_MQ7_Get_ADC_value (void);




#endif
