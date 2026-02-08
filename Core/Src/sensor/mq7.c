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
//-------------------------------------------------------------------------
static const mq7_cycle_step_t mq7_cycle [] = {
    [MQ7_STATE_INIT_CLEANING] = {SENSOR_INITIAL_CLEANING_TIME,  1,  MQ7_STATE_HEATING_HIGH},

    [MQ7_STATE_HEATING_HIGH] = {SENSOR_HEATING_HIGH_TIME,   1,  MQ7_STATE_HEATING_LOW},

    [MQ7_STATE_HEATING_LOW] = {SENSOR_HEATING_LOW_TIME, 0, MQ7_STATE_MEASURE},

    [MQ7_STATE_MEASURE] = {SENSOR_MEASURE_TIME, 0, MQ7_STATE_HEATING_HIGH },
};
//-------------------------------------------------------------------------
static void _hw_set_heater(mq7_t *handle, uint8_t heater_on) {
    if(handle->io.set_heater) {
        handle->io.set_heater(heater_on);
    }
}
//-------------------------------------------------------------------------
static void _hw_calculate_ppm(mq7_t *handle){
    float adc_step_voltage = 0.0f;
    float adc_value_voltage = 0.0f;
    float sensor_resistanse = 0.0f;
    float ratio = 0.0f;

    if(handle->io.get_adc_data) {
        handle->raw_adc_value = handle->io.get_adc_data();
    }

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
//-------------------------------------------------------------------------
void mq7_sensor_init (mq7_t *handle, mq7_io_t io) {
    if (handle == NULL) return;

    handle->io = io;

    handle->current_ppm = 0.0f;
    handle->raw_adc_value = 0;

    handle->state = MQ7_STATE_INIT_CLEANING;
    handle->timer_start_ms = 0;


    _hw_set_heater(handle, mq7_cycle[MQ7_STATE_INIT_CLEANING].heater_on);

}
//-------------------------------------------------------------------------
void mq7_process (mq7_t *handle, uint32_t current_time_ms) {
    if(handle == NULL) return;
    if (handle->timer_start_ms == 0) handle->timer_start_ms = current_time_ms;

    if (handle->state == MQ7_STATE_MEASURE) {
    	_hw_calculate_ppm(handle);
    }

    if (handle->io.get_adc_data) {
    	handle->raw_adc_value = handle->io.get_adc_data();
    }

    const mq7_cycle_step_t *step = &mq7_cycle[handle->state];

    if (current_time_ms - handle->timer_start_ms >= step->duration_ms) {

        handle->state = step->next_state;
        handle->timer_start_ms = current_time_ms;

        _hw_set_heater(handle, mq7_cycle[handle->state].heater_on);
    }

}
