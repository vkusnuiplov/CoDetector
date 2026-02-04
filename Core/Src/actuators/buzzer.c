/**
  ******************************************************************************
  * @file    buzzer.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з бузером
  *****************************************************************************
  */

#include "buzzer.h"

  static const buzzer_setting_t buzz_setting_table [] = {
      [BUZZER_OFF]          = {0, 0, 0, 0},
      [BUZZER_ON]           = {BUZZER_ON_FREQ,    BUZZER_ON_FREQ,     0,    0},
      [BUZZER_BEEP]         = {BUZZER_ON_FREQ,    BUZZER_ON_FREQ,     0,    0},
      [BUZZER_WARN_ALARM]   = {WARN_ALARM_MIN_FREQ,   WARN_ALARM_MAX_FREQ,   WARN_ALARM_STEP,   WARN_ALARM_SPEED_MS},
      [BUZZER_DANGER_ALARM] = {DANGER_ALARM_MIN_FREQ, DANGER_ALARM_MAX_FREQ, DANGER_ALARM_STEP, DANGER_ALARM_SPEED_MS},

  };
//-------------------------------------------------------------------------
  static void _hw_buzzer_stop (buzzer_t *handle){
    __HAL_TIM_SET_COMPARE(handle->htim, handle->channel, 0);
  }

//-------------------------------------------------------------------------
  static void _hw_buzzer_set_freq (buzzer_t *handle, uint16_t freq_hz) {
    uint32_t arr_value = 0;

    if(freq_hz == 0){
        _hw_buzzer_stop (handle);
        return;
    }

    arr_value = (BUZZER_TIMER_FREQ / freq_hz) - 1;
    if (arr_value > BUZZER_ARR_MAX) arr_value = BUZZER_ARR_MAX;

    __HAL_TIM_SET_AUTORELOAD(handle->htim, arr_value);
    __HAL_TIM_SET_COMPARE (handle->htim, handle->channel, arr_value / PWM_DUTY);

    HAL_TIM_PWM_Start(handle->htim, handle->channel);
  }

//-------------------------------------------------------------------------
  void buzzer_init (buzzer_t *handle, TIM_HandleTypeDef *htim, uint32_t channel) {
    handle->htim = htim;
    handle->channel = channel;
    handle->state = BUZZER_OFF;

    handle->current_setting = &buzz_setting_table[BUZZER_OFF];

    handle->current_freq = 0;
    handle->current_delta = 0;
    handle->last_update_time = HAL_GetTick();
    handle->beep_start_time = 0;

    HAL_TIM_PWM_Start(handle->htim, handle->channel);
    _hw_buzzer_stop(handle);
  }
//-------------------------------------------------------------------------
void buzzer_beep (buzzer_t *handle) {
    if(handle->state == BUZZER_WARN_ALARM ||
         handle->state == BUZZER_DANGER_ALARM) {
          return;
    }
    handle->beep_start_time = HAL_GetTick();
    handle->beep_duration_ms = BUZZER_BEEP_TIME;
    buzzer_set_state(handle, BUZZER_BEEP);
  }
//-------------------------------------------------------------------------
void buzzer_set_state(buzzer_t *handle, buzzer_state_e state) {

    if (handle->state == state) return;
    handle->state = state;

    handle->current_setting = &buzz_setting_table[state];

    handle->current_freq = handle->current_setting->min_freq;
    handle->current_delta = (int16_t)handle->current_setting->step_size;
    handle->last_update_time = HAL_GetTick();

    if (state == BUZZER_OFF) {
      _hw_buzzer_stop(handle);
    }
    else {
      _hw_buzzer_set_freq(handle, handle->current_freq);
    }
}
//-------------------------------------------------------------------------
  void buzzer_process(buzzer_t *handle, uint32_t current_time_ms) {
    if(handle->state == BUZZER_BEEP) {
        if (current_time_ms - handle->beep_start_time >= handle->beep_duration_ms){
            buzzer_set_state(handle,BUZZER_OFF);
        }
        return;
    }

    if (handle->current_setting->step_size == 0) return;

    if (current_time_ms - handle->last_update_time >= handle->current_setting->period_ms) {

        handle->last_update_time = current_time_ms;

        int32_t next_freq = (int16_t)handle->current_freq + handle->current_delta;

        uint16_t max_f = handle->current_setting->max_freq;
        uint16_t min_f = handle->current_setting->min_freq;

        if (next_freq >= max_f) {
            next_freq = max_f;
            handle->current_delta = -((int16_t)handle->current_setting->step_size);
        }

        else if (next_freq <= min_f) {
            next_freq = min_f;
            handle->current_delta = ((int16_t)handle->current_setting->step_size);
        }
        handle->current_freq = (uint16_t)next_freq;
        _hw_buzzer_set_freq(handle, handle->current_freq);
    }
  }

