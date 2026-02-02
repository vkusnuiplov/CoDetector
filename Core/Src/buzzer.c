/**
  ******************************************************************************
  * @file    buzzer.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з бузером
  *****************************************************************************
  */

#include "buzzer.h"


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
    //_hw_buzzer_stop(handle);
  }
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
// public

  void buzzer_init (buzzer_t *handle, TIM_HandleTypeDef *htim, uint32_t channel) {
    handle->htim = htim;
    handle->channel = channel;
    handle->state = BUZZER_OFF;

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

    handle->state = BUZZER_BEEP;
    _hw_buzzer_set_freq(handle, BUZZER_ON_FREQ);
  }

void buzzer_set_state(buzzer_t *handle, buzzer_state_e state) {

    if (handle->state == state) return;
    handle->state = state;
    switch (handle->state) {
      case BUZZER_OFF:
        _hw_buzzer_stop(handle);
        break;

      case BUZZER_ON:
        _hw_buzzer_set_freq (handle, BUZZER_ON_FREQ);
        break;

      case BUZZER_BEEP:
        _hw_buzzer_set_freq (handle, BUZZER_ON_FREQ);
        break;

      case BUZZER_WARN_ALARM:
        handle->state_min_freq = WARN_ALARM_MIN_FREQ;
        handle->state_max_freq = WARN_ALARM_MAX_FREQ;
        handle->state_freq_step_size = WARN_ALARM_STEP;
        handle->state_period_ms = WARN_ALARM_SPEED_MS;

        handle->current_freq = WARN_ALARM_MIN_FREQ;
        handle->current_delta = (int16_t)handle->state_freq_step_size;
        handle->last_update_time = HAL_GetTick();

        _hw_buzzer_set_freq(handle, handle->current_freq);
        break;

      case BUZZER_DANGER_ALARM:
        handle->state_min_freq = DANGER_ALARM_MIN_FREQ;
        handle->state_max_freq = DANGER_ALARM_MAX_FREQ;
        handle->state_freq_step_size = DANGER_ALARM_STEP;
        handle->state_period_ms = DANGER_ALARM_SPEED_MS;

        handle->current_freq = DANGER_ALARM_MIN_FREQ;
        handle->current_delta = (int16_t)handle->state_freq_step_size;
        handle->last_update_time = HAL_GetTick();

        _hw_buzzer_set_freq(handle, handle->current_freq);
        break;
    }
  }

  void buzzer_process(buzzer_t *handle, uint32_t current_time_ms) {
    if(handle->state == BUZZER_BEEP) {
      if(current_time_ms - handle->beep_start_time >= handle->beep_duration_ms){
        buzzer_set_state(handle,BUZZER_OFF);
      }
      return;
    }


    if(handle->state != BUZZER_WARN_ALARM &&
         handle->state != BUZZER_DANGER_ALARM) {
          return;
         }

      if (current_time_ms - handle->last_update_time >= handle->state_period_ms) {
        handle->last_update_time = current_time_ms;

        int32_t next_freq = (int32_t)handle->current_freq + handle->current_delta;

      if(next_freq >= handle->state_max_freq){
        next_freq = handle->state_max_freq;
        handle->current_delta = -((int16_t)handle->state_freq_step_size);
      }

      else if(next_freq <= handle->state_min_freq){
        next_freq = handle->state_min_freq;
        handle->current_delta = (int16_t)handle->state_freq_step_size;
      }
      handle->current_freq = (uint16_t)next_freq;
      _hw_buzzer_set_freq (handle, handle->current_freq);
      }

  }



