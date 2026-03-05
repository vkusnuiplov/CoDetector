/**
  ******************************************************************************
  * @file    mq7.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з датчиком CO MQ-7
  * * Файл містить кінцевий автомат станів для керування датчиком MQ-7,
  * а також функції ініціалізації та обчислення результатів вимірювань
  *
  ******************************************************************************
  */

#include "mq7.h"
#include <stddef.h>

//-------------------------------------------------------------------------
static const mq7_cycle_step_t mq7_cycle [] = {
    [MQ7_STATE_INIT_CLEANING] = {SENSOR_INITIAL_CLEANING_TIME,  HEATER_ON,  MQ7_STATE_HEATING_HIGH},

    [MQ7_STATE_HEATING_HIGH]  = {SENSOR_HEATING_HIGH_TIME,      HEATER_ON,  MQ7_STATE_HEATING_LOW},

    [MQ7_STATE_HEATING_LOW]   = {SENSOR_HEATING_LOW_TIME,       HEATER_OFF, MQ7_STATE_MEASURE},

    [MQ7_STATE_MEASURE]       = {SENSOR_MEASURE_TIME,           HEATER_OFF, MQ7_STATE_HEATING_HIGH },
};
//-------------------------------------------------------------------------
static const uint32_t lut_x[SENSOR_LUT_SIZE] = SENSOR_LUT_X_VALUES;
static const uint32_t lut_y[SENSOR_LUT_SIZE] = SENSOR_LUT_Y_VALUES;
//-------------------------------------------------------------------------
static void _hw_set_heater(mq7_t *handle, uint8_t heater_on) {
    if(handle->io.set_heater) {
        handle->io.set_heater(heater_on);
    }
}
//-------------------------------------------------------------------------
static uint16_t _hw_interpolate_ppm (uint32_t ratio_scaled) {

    if (ratio_scaled <= lut_x [0]) {
        return (uint16_t) lut_y [0];
    }

    if (ratio_scaled >= lut_x[SENSOR_LUT_SIZE - 1]) {
        return (uint16_t) lut_y[SENSOR_LUT_SIZE - 1];
    }

    for (uint8_t i = 0; i < SENSOR_LUT_SIZE - 1; i++) {
        if (ratio_scaled >= lut_x[i] && ratio_scaled <= lut_x[i+1]) {

            uint32_t x1 = lut_x[i];
            uint32_t x2 = lut_x[i+1];
            uint32_t y1 = lut_y[i];
            uint32_t y2 = lut_y[i+1];

            uint32_t dx = x2 - x1;
            uint32_t dy = y1 - y2;
            uint32_t x_offset = ratio_scaled - x1;

            uint32_t ppm = y1 - ((x_offset * dy) / dx);

            return (uint16_t)ppm;
        }
    }
    return 0;
}
//-------------------------------------------------------------------------
static void _hw_calculate_ppm(mq7_t *handle) {
    uint32_t adc_value_mv = 0;
    uint32_t sensor_resistanse = 0;
    uint32_t ratio_scaled = 0;
    uint32_t voltage_diff_mv = 0;

    if (handle->raw_adc_value == 0) {
        handle->current_ppm = 0;
        return;
    }

    adc_value_mv = (handle->raw_adc_value * ADC_REFF_VCC_MV) / ADC_MAX_VALUE;
    if (adc_value_mv == 0) adc_value_mv = 1;

    voltage_diff_mv = SENSOR_VCC_MV - adc_value_mv;
    sensor_resistanse = (SENSOR_RL_RESISTASE * voltage_diff_mv) / adc_value_mv;

    ratio_scaled = (sensor_resistanse * 1000) / SENSOR_R0_RESISTANSE;

    handle->current_ppm = _hw_interpolate_ppm(ratio_scaled);

}
//-------------------------------------------------------------------------
void mq7_sensor_init (mq7_t *handle, mq7_io_t io) {
    if (handle == NULL) return;

    handle->io = io;

    handle->current_ppm = 0;
    handle->raw_adc_value = 0;

    handle->state = MQ7_STATE_INIT_CLEANING;
    handle->timer_start_ms = 0;
    handle->last_ppm_calc_time_ms = 0;

    _hw_set_heater (handle, mq7_cycle[MQ7_STATE_INIT_CLEANING].heater_on);

}
//-------------------------------------------------------------------------
mq7_status_e mq7_process (mq7_t *handle, uint32_t current_time_ms) {
    if(handle == NULL) {
        return MQ7_ERR_NULL_PTR;
    }
    if (handle->io.get_adc_data == NULL || handle->io.set_heater == NULL) {
        return MQ7_ERR_IO;
    }
    handle->raw_adc_value = handle->io.get_adc_data();

    if (handle->timer_start_ms == 0) handle->timer_start_ms = current_time_ms;

    if (handle->state == MQ7_STATE_MEASURE) {
        if (current_time_ms - handle->last_ppm_calc_time_ms >= SENSOR_PPM_CALC_INTERVAL){
            _hw_calculate_ppm(handle);
            handle->last_ppm_calc_time_ms = current_time_ms;
        }
    }

    if (handle->state > MQ7_STATE_MEASURE) {
        return MQ7_ERR_INVALID_STATE;
    }

    const mq7_cycle_step_t *step = &mq7_cycle[handle->state];

    if (current_time_ms - handle->timer_start_ms >= step->duration_ms) {

        handle->state = step->next_state;
        handle->timer_start_ms = current_time_ms;

        _hw_set_heater(handle, mq7_cycle[handle->state].heater_on);
    }
    return MQ7_OK;
}
//-------------------------------------------------------------------------
