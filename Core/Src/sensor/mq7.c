/**
  ******************************************************************************
  * @file    mq7.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з датчиком CO MQ-7
  * * Файл містить кінцевий автомат станів для керування датчиком MQ-7,
  * а також функції ініціалізації та обчислення результатів вимірювань
  *
  ******************************************************************************
	@section MQ-7 Hardware & Sensing Principle

	СХЕМА ПІДКЛЮЧЕННЯ:
  		Датчик увімкнений як плече резистивного подільника напруги:
  		VCC->Sensor_resistance (RS)->Load_resistance (RL)->GND
  		Точка між Sensor_resistance та Load_resistance підключеня до входу АЦП
  		Для подальших розрахунків необхідно визначити R0 опір датчика

	ПРИНЦИП ВИМІРЮВАННЯ:
		При зростанні концентрації CO опір чутливого елемента (RS) падає.
		Це призводить до зростання напруги на навантажувальному резисторі (RL).
		АЦП фіксує це зростання та перетворює у цифрове значення (Raw ADC).

  ******************************************************************************
	@section MQ-7 Operational Logic

  	Робота датчика базується на циклічному нагріві чутливого елемента.

  	1. Фаза очищення
  		Тривалість: 60 секунд
  		State: MQ7_STATE_INIT_CLEANING, MQ7_STATE_HEATING_HIGH

  		На нагрівальний елемент подано напругу 5 вольт з метою очищення чутливого елементу
  		шляхом випалювання молекул газу, що накопичилися.
  		Дані з АЦП під час цієї фази не валідні, ігноруються

  	2. Фаза накопичення CO
  		Тривалість: 85 секунд
  		State: MQ7_STATE_HEATING_LOW

  		На нагрівальний елемент подано напругу 1.4 вольта, температура
  		чутливого елементу в цей момент знижується.
  		При низькій температурі CO вступає в реакцію, змінюючи опір шару SnO2
  		яким вкритий чутливий елемент.
  		Дані з АЦП під час цієї фази можна брати до уваги для детектування
  		критичного рівня СО.

  	3. Фаза вимірювання та розрахунків
  	    Тривалість: останні 5 секунд циклу
  	    State: MQ7_STATE_MEASURE

  		Під час завершення всього циклу роботи датчика, в останні 3-5 секунд проводяться виміри
  		опору чутливого елемента MQ7_STATE_MEASURE
  		Дані з АЦП під час цієї фази валідні, їх приймає функція обрахунку PPM.
  		Функція передає результати розрахунків в основну логіку

	@section PPM calculation logic

		ADC_voltage = (ADC_value * ADC_reff_voltage) / ADC_word_size

		RS = RL * ((Sensor_VCC - ADC_voltage) / ADC_voltage)

		PPM = 100 * ((RS / R0)^(-1.43))

  ******************************************************************************
  */

#include "mq7.h"
#include <math.h>
#include <stddef.h>

//-------------------------------------------------------------------------
static const mq7_cycle_step_t mq7_cycle [] = {
    [MQ7_STATE_INIT_CLEANING] = {SENSOR_INITIAL_CLEANING_TIME,  HEATER_ON,  MQ7_STATE_HEATING_HIGH},

    [MQ7_STATE_HEATING_HIGH]  = {SENSOR_HEATING_HIGH_TIME,      HEATER_ON,  MQ7_STATE_HEATING_LOW},

    [MQ7_STATE_HEATING_LOW]   = {SENSOR_HEATING_LOW_TIME,       HEATER_OFF, MQ7_STATE_MEASURE},

    [MQ7_STATE_MEASURE]       = {SENSOR_MEASURE_TIME,           HEATER_OFF, MQ7_STATE_HEATING_HIGH },
};
//-------------------------------------------------------------------------
static void _hw_set_heater(mq7_t *handle, uint8_t heater_on) {
    if(handle->io.set_heater) {
        handle->io.set_heater(heater_on);
    }
}
//-------------------------------------------------------------------------
static void _hw_calculate_ppm(mq7_t *handle) {
    float adc_step_voltage = 0.0f;
    float adc_value_voltage = 0.0f;
    float sensor_resistanse = 0.0f;
    float ratio = 0.0f;

    if(handle->io.get_adc_data) {
        handle->raw_adc_value = handle->io.get_adc_data();
    }

    if(handle->raw_adc_value == 0) {
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

    _hw_set_heater (handle, mq7_cycle[MQ7_STATE_INIT_CLEANING].heater_on);

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
//-------------------------------------------------------------------------
