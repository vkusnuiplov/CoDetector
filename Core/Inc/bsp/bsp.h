/**
  ******************************************************************************
  * @file    bsp.h
  * @author  vkusnuiplov
  * @brief   Board support package header для зв'язки плати та драйверів
  *****************************************************************************
  */

#ifndef BSP_H
#define BSP_H

#include "main.h"
#include <stdint.h>

#define BSP_ADC_1_HANDLE        hadc1
#define BSP_TIM_BUZZER_HANDLE   htim1
#define BSP_DMA_ADC_HANDLE      hdma_adc1

#define BSP_LED_GREEN_PORT      GPIOB
#define BSP_LED_GREEN_PIN       GPIO_PIN_3

#define BSP_LED_RED_PORT        GPIOB
#define BSP_LED_RED_PIN         GPIO_PIN_5

#define BSP_MQ7_HEATER_PORT     GPIOA
#define BSP_MQ7_HEATER_PIN      GPIO_PIN_6

#define BSP_ADC_BUFFER_SIZE     1U
#define BSP_TIM_BUZZER_CHANNEL  TIM_CHANNEL_1
#define BSP_TIM_BUZZER_FREQ     1000000U
#define BSP_TIM_BUZZER_ARR_MAX  65535U
#define BSP_PWM_DUTY            2U

void BSP_Init(void);
uint32_t BSP_GetTick(void);

void BSP_LED_Green_Set(uint8_t state);
void BSP_LED_Red_Set(uint8_t state);

void BSP_Buzzer_SetFreq (uint16_t freq_hz);
void BSP_Buzzer_Stop (void);

void BSP_MQ7_Heater_Set (uint8_t state);
uint32_t BSP_MQ7_Get_ADC_value (void);

#endif


