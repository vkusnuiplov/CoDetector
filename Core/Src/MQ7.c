/**
  ******************************************************************************
  * @file    mq7.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з датчиком CO MQ-7.
  * * Файл містить кінцевий автомат станів для керування датчиком MQ-7,
  * а також функції ініціалізації та обчислення результатів вимірювань.
  *
  ******************************************************************************
  */

#include "mq7.h"
#include <math.h>
#include <stddef.h>

static void _hw_set_heating_high(mq7_t *handle) {
    HAL_GPIO_WritePin(handle->heater_port, handle->heater_pin, GPIO_PIN_SET);
}

static void _hw_set_heating_low(mq7_t *handle) {
    HAL_GPIO_WritePin(handle->heater_port, handle->heater_pin, GPIO_PIN_RESET);
}

static void _hw_calculate_ppm(mq7_t *handle){
    uint32_t safe_adc = 0;
    float adc_step_voltage = 0.0f;
    float adc_value_voltage = 0.0f;
    float sensor_resistanse = 0.0f;
    float ratio = 0.0f;

    if(handle->raw_adc_value == 0){
        handle->current_ppm = 0.0f;
        return;
    }

    adc_step_voltage = (ADC_REFF_VCC / ADC_MAX_VALUE);

    adc_value_voltage = (float)handle->raw_adc_value * adc_step_voltage;

    sensor_resistanse = SENSOR_RL_RESISTASE * ((SENSOR_VCC_V - adc_value_voltage) / adc_value_voltage);

    ratio = sensor_resistanse / SENSOR_R0_RESISTANSE;

    handle->current_ppm = SENSOR_COEFF_A * powf(ratio, SENSOR_COEFF_B);

}

void mq7_sensor_init (mq7_t *handle, GPIO_TypeDef* port, uint16_t pin, ADC_HandleTypeDef* hadc) {
    if (handle == NULL) return;

    handle->heater_port = port;
    handle->heater_pin = pin;
    handle->hadc = hadc;

    handle->current_ppm = 0.0f;
    handle->raw_adc_value = 0;

    handle->state = MQ7_STATE_INIT_CLEANING;
    handle->timer_start_ms = HAL_GetTick();
    _hw_set_heating_high(handle);

    HAL_ADCEx_Calibration_Start(handle->hadc);

    HAL_ADC_Start_DMA(handle->hadc, (uint32_t*)&handle->raw_adc_value, 1);
}

void mq7_process (mq7_t *handle, uint32_t current_time_ms) {
    if(handle == NULL) return;

    uint32_t elapsed_time;

    elapsed_time = current_time_ms - handle->timer_start_ms;

    switch (handle->state) {

            case MQ7_STATE_INIT_CLEANING:
                if (elapsed_time >= SENSOR_INITIAL_CLEANING_TIME) {
                    handle->state = MQ7_STATE_HEATING_HIGH;
                    handle->timer_start_ms = current_time_ms;
                    _hw_set_heating_high (handle);
                }
                break;

            case MQ7_STATE_HEATING_HIGH:
                if (elapsed_time >= SENSOR_HEATING_HIGH_TIME) {
                    handle->state = MQ7_STATE_HEATING_LOW;
                    handle->timer_start_ms = current_time_ms;
                    _hw_set_heating_low (handle);
                }
                break;

            case MQ7_STATE_HEATING_LOW:
                if(elapsed_time >= SENSOR_HEATING_LOW_TIME) {

                    handle->state = MQ7_STATE_MEASURE;
                    handle->timer_start_ms = current_time_ms;

                }
                break;

            case MQ7_STATE_MEASURE:
                if (elapsed_time >= SENSOR_MEASURE_TIME) {
                    _hw_calculate_ppm(handle);
                    handle->state = MQ7_STATE_HEATING_HIGH;
                    handle->timer_start_ms = current_time_ms;
                    _hw_set_heating_high (handle);
                }
                break;
    }
}


