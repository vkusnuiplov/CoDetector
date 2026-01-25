/**
  ******************************************************************************
  * @file    buzzer.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи з бузером
  ******************************************************************************
  */


  #ifndef BUZZER_H
  #define BUZZER_H

  #include "main.h"

  typedef enum {
        BUZZER_OFF,
        BUZZER_ON,
        BUZZER_ALARM
  } buzzer_state_e;

  typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    buzzer_state_e current_state;
    uint32_t last_update_time;
    uint16_t current_freq;
    int16_t freq_step;

  } buzzer_t;

void buzzer_set_freq (buzzer_t *handle, uint16_t freq_hz);
void buzzer_stop (handle->htim, handle->channel);
void buzzer_init (buzzer_t *handle, TIM_HandleTypeDef *htim, uint32_t channel);
void buzzer_set_state (buzzer_t *handle, buzzer_state_e state);
void buzzer_process (buzzer_t *handle, uint32_t current_time_ms);


#endif

