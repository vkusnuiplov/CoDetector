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

  #define BUZZER_TIMER_FREQ       1000000U
  #define BUZZER_ARR_MAX          65535U
  #define PWM_DUTY                2U

  #define BUZZER_ON_FREQ          2000U

  #define WARN_ALARM_MIN_FREQ     500U
  #define WARN_ALARM_MAX_FREQ     1000U
  #define WARN_ALARM_STEP         20U
  #define WARN_ALARM_SPEED_MS     50U

  #define DANGER_ALARM_MIN_FREQ   1000U
  #define DANGER_ALARM_MAX_FREQ   2000U
  #define DANGER_ALARM_STEP       100U
  #define DANGER_ALARM_SPEED_MS   10U

  #define BUZZER_BEEP_TIME        200U


  typedef enum {
    BUZZER_OFF,
    BUZZER_ON,
    BUZZER_BEEP,
    BUZZER_WARN_ALARM,
    BUZZER_DANGER_ALARM
  } buzzer_state_e;

   typedef struct {
    uint16_t min_freq;
    uint16_t max_freq;
    uint16_t step_size;
    uint16_t period_ms;
  } buzzer_setting_t;

  typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    buzzer_state_e state;

    const buzzer_setting_t *current_setting;

    uint16_t current_freq;
    int16_t current_delta;
    uint32_t last_update_time;

    uint32_t beep_start_time;
    uint32_t beep_duration_ms;

  } buzzer_t;

void buzzer_init (buzzer_t *handle, TIM_HandleTypeDef *htim, uint32_t channel);
void buzzer_set_state (buzzer_t *handle, buzzer_state_e state);
void buzzer_process (buzzer_t *handle, uint32_t current_time_ms);
void buzzer_beep (buzzer_t *handle);

#endif

