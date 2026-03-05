/**
  ******************************************************************************
  * @file    bsp.c
  * @author  vkusnuiplov
  * @brief   Board Support Package (BSP) file for hardware abstraction
  * @details This module acts as a hardware abstraction layer wrapper serving
  * two main purposes:
  * 1. Binding logical drivers to physical MCU resources (Pins, Timers, ADC)
  * 2. Fully isolating business logic from STM32 HAL specific implementations
  * * Logic Flow: Application -> Generic BSP API -> Hardware-Specific HAL -> Physical Pins
  * Any schematic changes require modifications only in this file
  *****************************************************************************
  */

#include "bsp.h"
#include "main.h"

extern ADC_HandleTypeDef BSP_ADC_1_HANDLE;
extern TIM_HandleTypeDef BSP_TIM_BUZZER_HANDLE;
extern DMA_HandleTypeDef BSP_DMA_ADC_HANDLE;

/** @brief DMA buffer for ADC oversampling */
volatile uint32_t bsp_adc_buffer[BSP_ADC_BUFFER_SIZE];

//-------------------------------------------------------------------------
/**
 * @brief  Initializes BSP hardware peripherals (PWM, ADC, DMA)
 */
void BSP_Init(void) {
    HAL_TIM_PWM_Start(&BSP_TIM_BUZZER_HANDLE, BSP_TIM_BUZZER_CHANNEL);
    HAL_ADCEx_Calibration_Start(&BSP_ADC_1_HANDLE);
    HAL_ADC_Start_DMA(&BSP_ADC_1_HANDLE, (uint32_t*)bsp_adc_buffer, BSP_ADC_BUFFER_SIZE);
}

//-------------------------------------------------------------------------
/**
 * @brief  Provides the current system tick value
 * @retval uint32_t Current system time in milliseconds
 */
uint32_t BSP_GetTick(void) {
    return HAL_GetTick();
}

//-------------------------------------------------------------------------
/**
 * @brief  Sets the state of the Green LED
 * @param  state 1 to turn ON, 0 to turn OFF
 */
void BSP_LED_Green_Set(uint8_t state) {
    HAL_GPIO_WritePin(BSP_LED_GREEN_PORT, BSP_LED_GREEN_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//-------------------------------------------------------------------------
/**
 * @brief  Sets the state of the Red LED
 * @param  state 1 to turn ON, 0 to turn OFF
 */
void BSP_LED_Red_Set(uint8_t state) {
    HAL_GPIO_WritePin(BSP_LED_RED_PORT, BSP_LED_RED_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//-------------------------------------------------------------------------
/**
 * @brief  Sets the buzzer PWM frequency
 * @param  freq_hz Desired frequency in Hertz. Passing 0 stops the buzzer
 */
void BSP_Buzzer_SetFreq (uint16_t freq_hz) {
    uint32_t arr_value = 0;

    if(freq_hz == 0) {
        BSP_Buzzer_Stop ();
        return;
    }

    arr_value = (BSP_TIM_BUZZER_FREQ / freq_hz) - 1;
    if (arr_value > BSP_TIM_BUZZER_ARR_MAX) arr_value = BSP_TIM_BUZZER_ARR_MAX;

    __HAL_TIM_SET_AUTORELOAD(&BSP_TIM_BUZZER_HANDLE, arr_value);
    __HAL_TIM_SET_COMPARE(&BSP_TIM_BUZZER_HANDLE, BSP_TIM_BUZZER_CHANNEL, arr_value / BSP_PWM_DUTY);

  }

//-------------------------------------------------------------------------
/**
 * @brief  Stops the buzzer by setting the PWM duty cycle to 0
 */
void BSP_Buzzer_Stop (void) {
    __HAL_TIM_SET_COMPARE(&BSP_TIM_BUZZER_HANDLE, BSP_TIM_BUZZER_CHANNEL, 0);
}

//-------------------------------------------------------------------------
/**
 * @brief  Sets the state of the MQ-7 sensor heater
 * @param  state 1 to turn ON, 0 to turn OFF
 */
void BSP_MQ7_Heater_Set (uint8_t state) {
    HAL_GPIO_WritePin(BSP_MQ7_HEATER_PORT, BSP_MQ7_HEATER_PIN, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

//-------------------------------------------------------------------------
/**
 * @brief  Retrieves the averaged ADC value from the DMA buffer
 * @note   Performs hardware oversampling to reduce noise
 * @retval uint32_t Averaged ADC reading
 */
uint32_t BSP_MQ7_Get_ADC_value (void) {
    uint32_t adc_sum = 0;
    for (uint16_t i = 0; i < BSP_ADC_BUFFER_SIZE; i++){
        adc_sum += bsp_adc_buffer[i];
    }
    return adc_sum / BSP_ADC_BUFFER_SIZE;
}
//-------------------------------------------------------------------------
