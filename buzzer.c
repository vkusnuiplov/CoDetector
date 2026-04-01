/**
  ******************************************************************************
  * @file    buzzer.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з бузером
  *****************************************************************************
  */

#include "buzzer.h"

  void buzzer_set_freq (buzzer_t *handle, uint16_t freq_hz) {
    uint32_t arr_value = 0;
         
    if(freq_hz == 0){
        buzzer_stop (handle);
        return;
    }

    arr_value = (1000000 / freq_hz) - 1;
    if (arr_value > 65535) arr_value = 65535;

    __HAL_TIM_SET_AUTORELOAD(handle->htim, arr_value);
    __HAL_TIM_SET_COMPARE (handle->htim, handle->channel, arr_value / 2);
    HAL_TIM_PWM_Start(handle->htim, handle->channel);
  }

  void buzzer_stop (buzzer_t *handle){
    HAL_TIM_PWM_Stop(handle->htim, handle->channel);
  }

  void buzzer_init (buzzer_t *handle, TIM_HandleTypeDef *htim, uint32_t channel) {
    handle->htim = htim;
    handle->channel = channel;
    handle->current_state = BUZZER_OFF;

    buzzer_stop(handle);
  }

  void buzzer_set_state (buzzer_t *handle, buzzer_state_e state) {
    if (handle->current_state == state)
    return;

    handle->current_state = state;
    handle->last_update_time = HAL_GetTick();

    switch(state){
        case BUZZER_ALARM:
            handle->current_freq = 1000;
            handle->freq_step = 50;
            buzzer_set_freq(handle, 1000);
            break;


        case BUZZER_ON:
            buzzer_set_freq(handle, 2000);
            break;

        default:
            buzzer_stop(handle);
            break;
    }
  }

  void buzzer_process (buzzer_t *handle, uint32_t current_time_ms) {
        if (handle->current_state != BUZZER_ALARM)
        return;

        if(current_time - handle->last_update_time >= 20){
            handle->last_update_time = current_time;
            handle->current_freq += handle->freq_step;

            if (handle->current_freq >= 3000) {
                handle->current_freq = 3000;
                handle->freq_step = -50;
            }

            if (handle->current_freq <= 1000) {
                handle->current_freq = 1000;
                handle->freq_step = 50;
            }

            buzzer_set_freq (handle, handle->current_freq);
        }
  }




