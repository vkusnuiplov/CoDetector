/**
  ******************************************************************************
  * @file    bsp.c
  * @author  vkusnuiplov
  * @brief   Board support package file for connected board with drivers
  *****************************************************************************
  */

#include "bsp.h"
#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;
extern DMA_HandleTypeDef hdma_adc1;

volatile uint32_t bsp_adc_buffer[1];

//-------------------------------------------------------------------------
void BSP_Init(void) {
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_ADCEx_Calibration_Start(&hadc1);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)bsp_adc_buffer, 1);
}
//-------------------------------------------------------------------------
uint32_t BSP_GetTick(void) {
    return HAL_GetTick();
}
//-------------------------------------------------------------------------
void BSP_LED_Green_Set(uint8_t state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
//-------------------------------------------------------------------------
void BSP_LED_Red_Set(uint8_t state) {
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//-------------------------------------------------------------------------
void BSP_Buzzer_SetFreq (uint16_t freq_hz) {
    uint32_t arr_value = 0;

    if(freq_hz == 0){
        BSP_Buzzer_Stop ();
        return;
    }

    arr_value = (BSP_BUZZER_TIMER_FREQ / freq_hz) - 1;
    if (arr_value > BSP_BUZZER_ARR_MAX) arr_value = BSP_BUZZER_ARR_MAX;

    __HAL_TIM_SET_AUTORELOAD(&htim1, arr_value);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, arr_value / BSP_PWM_DUTY);


  }
//-------------------------------------------------------------------------
void BSP_Buzzer_Stop (void) {
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
}
//-------------------------------------------------------------------------
void BSP_MQ7_Heater_Set (uint8_t state) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
//-------------------------------------------------------------------------
uint32_t BSP_MQ7_Get_ADC_value (void) {
    return bsp_adc_buffer[0];
}
