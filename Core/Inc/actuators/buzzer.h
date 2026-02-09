/**
  ******************************************************************************
  * @file    buzzer.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи з бузером
  ******************************************************************************
  */
  #ifndef BUZZER_H
  #define BUZZER_H

  //#include "main.h"
  #include <stdint.h>

  #define BUZZER_OFF_FREQ         0U
  #define BUZZER_OFF_STEP         0U
  #define BUZZER_OFF_SPEED        0U

  #define BUZZER_ON_FREQ          2000U
  #define BUZZER_ON_STEP          0U
  #define BUZZER_ON_SPEED         0U

  #define BUZER_BEEP_STEP         0U
  #define BUZZER_BEEP_SPEED       0U
  #define BUZZER_BEEP_TIME        200U

  #define WARN_ALARM_MIN_FREQ     500U
  #define WARN_ALARM_MAX_FREQ     1000U
  #define WARN_ALARM_STEP         20U
  #define WARN_ALARM_SPEED_MS     50U

  #define DANGER_ALARM_MIN_FREQ   1000U
  #define DANGER_ALARM_MAX_FREQ   2000U
  #define DANGER_ALARM_STEP       100U
  #define DANGER_ALARM_SPEED_MS   10U



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

  typedef void (*bsp_buzzer_set_freq_fn)(uint16_t freq_hz);
  typedef void (*bsp_buzzer_stop_fn)(void);

  typedef struct {
    bsp_buzzer_set_freq_fn  set_freq;
    bsp_buzzer_stop_fn      stop;
  } buzzer_io_t;

  typedef struct {

    buzzer_io_t io;

    buzzer_state_e state;

    const buzzer_setting_t *current_setting;

    uint16_t current_freq;
    int16_t current_delta;
    uint32_t last_update_time;

    uint32_t beep_start_time;
    uint32_t beep_duration_ms;

  } buzzer_t;

void buzzer_init (buzzer_t *handle, buzzer_io_t hw_buzz_bsp);
void buzzer_set_state (buzzer_t *handle, buzzer_state_e state);
void buzzer_process (buzzer_t *handle, uint32_t current_time_ms);
void buzzer_beep (buzzer_t *handle);

#endif

