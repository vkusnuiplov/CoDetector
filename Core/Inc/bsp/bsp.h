/**
  ******************************************************************************
  * @file    bsp.h
  * @author  vkusnuiplov
  * @brief   Board Support Package (BSP) header.
  * @details Defines hardware mappings, abstraction macros, and the public API
  * for peripheral control (LEDs, Buzzer, ADC, Timers).
  ******************************************************************************
  */

#ifndef BSP_H
#define BSP_H

#include "main.h"
#include <stdint.h>

/* --- Peripheral Handles --- */
#define BSP_ADC_1_HANDLE        hadc1
#define BSP_TIM_BUZZER_HANDLE   htim1
#define BSP_DMA_ADC_HANDLE      hdma_adc1

/* --- GPIO Pin Mappings --- */
#define BSP_LED_GREEN_PORT      GPIOB
#define BSP_LED_GREEN_PIN       GPIO_PIN_3

#define BSP_LED_RED_PORT        GPIOB
#define BSP_LED_RED_PIN         GPIO_PIN_5

#define BSP_MQ7_HEATER_PORT     GPIOA
#define BSP_MQ7_HEATER_PIN      GPIO_PIN_6

/* --- Hardware Configuration --- */
#define BSP_ADC_BUFFER_SIZE     32U
#define BSP_TIM_BUZZER_CHANNEL  TIM_CHANNEL_1
#define BSP_TIM_BUZZER_FREQ     1000000U
#define BSP_TIM_BUZZER_ARR_MAX  65535U
#define BSP_PWM_DUTY            2U

/* --- Public API --- */

/**
 * @brief Initializes BSP hardware peripherals (PWM, ADC, DMA)
 */
void BSP_Init(void);

/**
 * @brief Gets current system tick
 * @retval uint32_t System time in milliseconds
 */
uint32_t BSP_GetTick(void);

/**
 * @brief Controls the green LED
 * @param state 1 for ON, 0 for OFF
 */
void BSP_LED_Green_Set(uint8_t state);

/**
 * @brief Controls the red LED
 * @param state 1 for ON, 0 for OFF
 */
void BSP_LED_Red_Set(uint8_t state);

/**
 * @brief Sets buzzer frequency
 * @param freq_hz Frequency in Hz. 0 stops the buzzer
 */
void BSP_Buzzer_SetFreq (uint16_t freq_hz);

/**
 * @brief Stops the buzzer output
 */
void BSP_Buzzer_Stop (void);

/**
 * @brief Controls the MQ-7 heater element
 * @param state 1 for ON, 0 for OFF
 */
void BSP_MQ7_Heater_Set (uint8_t state);

/**
 * @brief Retrieves the averaged ADC reading for the sensor
 * @retval uint32_t Averaged ADC value from the DMA buffer
 */
uint32_t BSP_MQ7_Get_ADC_value (void);

#endif


