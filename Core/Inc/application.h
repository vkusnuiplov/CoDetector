/**
  ******************************************************************************
  * @file    application.h
  * @author  vkusnuiplov
  * @brief   Реалізація бізнес логіки
  ******************************************************************************
  */

#ifndef APPLICATION_H
#define APPLICATION_H

#include "main.h"
#include "mq7.h"
#include "led.h"
#include "buzzer.h"

#define DEFAULT_LEVEL_PPM 20.0f
#define WARNING_LEVEL_PPM 40.0f
#define ALARM_LEVEL_PPM 60.0f
#define HYSTERESIS_PPM  5.0f

#define APP_WARMUP_TIME_MS      SENSOR_INITIAL_CLEANING_TIME


typedef enum {
    APP_STATE_WARMUP,
    APP_STATE_DEFAULT,
    APP_STATE_WARNING,
    APP_STATE_ALARM,
    APP_STATE_CRITICAL,
    APP_STATE_ERROR
} app_state_t;

typedef struct {
    mq7_t* sensor;
    led_t* led_green;
    led_t* led_red;
    buzzer_t* buzzer;

    app_state_t state;

} application_t;

void app_init(application_t *app, mq7_t *sensor, led_t *green, led_t* red, buzzer_t *buzzer);
void app_process(application_t *app, uint32_t now);


#endif
