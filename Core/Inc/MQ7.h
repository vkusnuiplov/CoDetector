/**
  ******************************************************************************
  * @file    mq7.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи з датчиком CO MQ-7.
  * * Файл містить кінцевий автомат станів для керування датчиком MQ-7,
  * а також функції ініціалізації та обчислення результатів вимірювань.
  *
  ******************************************************************************
  */

#ifndef MQ7_H
#define MQ7_H

#include "main.h"
#include <stdint.h>

#define SENSOR_VCC_V             5.0f
#define SENSOR_R0_RESISTANSE     920.0f
#define SENSOR_RL_RESISTASE      2000.0f

#define SENSOR_INITIAL_CLEANING_TIME    180000U
#define SENSOR_HEATING_HIGH_TIME 60000U
#define SENSOR_HEATING_LOW_TIME  85000U
#define SENSOR_MEASURE_TIME      50000U

#define SENSOR_EMERGENCY_RAW_ADC_THRESHOLD 3900U


// ------- [MATH] Коефіцієнти кривої (PPM = a * ratio^b) -------
#define SENSOR_COEFF_A 100.0f
#define SENSOR_COEFF_B -1.43f

#define ADC_REFF_VCC 3.3f
#define ADC_MAX_VALUE 4095.f

#define ADC_ERROR_VALUE 10U


typedef enum {
    MQ7_STATE_INIT_CLEANING,
    MQ7_STATE_HEATING_HIGH,      // 60 sec
    MQ7_STATE_HEATING_LOW,       // 85sec
    MQ7_STATE_MEASURE            // 5 sec heating low
} mq7_state_e;

typedef struct {
    GPIO_TypeDef* heater_port;
    uint16_t      heater_pin;
    ADC_HandleTypeDef* hadc;

    volatile uint32_t raw_adc_value;
    float             current_ppm;
    mq7_state_e       state;
    uint32_t          timer_start_ms;
} mq7_t;

void mq7_sensor_init (mq7_t *handle, GPIO_TypeDef* port, uint16_t pin, ADC_HandleTypeDef* hadc);
void mq7_process (mq7_t *handle, uint32_t current_time_ms);
uint8_t mq7_danger_adc_check (mq7_t *handle);

#endif
