/**
  ******************************************************************************
  * @file    application.c
  * @author  vkusnuiplov
  * @brief   Реалізація бізнес логіки
  ******************************************************************************
  */

#include "application.h"
//-------------------------------------------------------------------------
static const indication_cfg_t indication_table[] = {
    [APP_STATE_WARMUP] =   {LED_BLINK_SLOW,      LED_OFF,        BUZZER_OFF},
    [APP_STATE_DEFAULT] =  {LED_BLINK_HEARTBEAT, LED_OFF,        BUZZER_OFF},
    [APP_STATE_WARNING] =  {LED_BLINK_MEDIUM,    LED_OFF,        BUZZER_OFF},
    [APP_STATE_ALARM] =    {LED_OFF,             LED_BLINK_FAST, BUZZER_OFF},
    [APP_STATE_CRITICAL] = {LED_OFF,            LED_BLINK_FAST, BUZZER_DANGER_ALARM},
    [APP_STATE_ERROR] =    {LED_OFF,             LED_ON,         BUZZER_OFF},

    };
//-------------------------------------------------------------------------
static void _app_update_indication(application_t *app) {
    const indication_cfg_t *cfg = &indication_table [app->state];

    led_set_mode(app->led_green, cfg->green_mode);
    led_set_mode(app->led_red,   cfg->red_mode);
    buzzer_set_state(app->buzzer, cfg->buzzer_state);
}
//-------------------------------------------------------------------------
static void _app_check_levels(application_t *app) {
    app_state_t old_state = app->state;

    if (app->sensor->raw_adc_value < ADC_ERROR_VALUE) {
        app->state = APP_STATE_ERROR;
    }

    else if (app->state == APP_STATE_ERROR) {
        app->state = APP_STATE_WARMUP;
    }

    else if (app->sensor->state == MQ7_STATE_HEATING_HIGH ||
             app->sensor->state == MQ7_STATE_INIT_CLEANING) {
                // do nothing
             }

    else if (app->sensor->state == MQ7_STATE_HEATING_LOW) {
        if (app->sensor->raw_adc_value > SENSOR_EMERGENCY_RAW_ADC_THRESHOLD) {
            if (app->state != APP_STATE_CRITICAL) {
                app->state = APP_STATE_CRITICAL;
            }
        }
    }

    else if (app->sensor->state == MQ7_STATE_MEASURE) {
        float current_ppm = app->sensor->current_ppm;

        switch (app->state) {
            case APP_STATE_DEFAULT:
                if (current_ppm > DEFAULT_LEVEL_PPM) app->state = APP_STATE_WARNING;
                break;

            case APP_STATE_WARNING:
                if (current_ppm > WARNING_LEVEL_PPM) app->state = APP_STATE_ALARM;
                else if (current_ppm < DEFAULT_LEVEL_PPM) app->state = APP_STATE_DEFAULT;
                break;

            case APP_STATE_ALARM:
                if (current_ppm > ALARM_LEVEL_PPM) app->state = APP_STATE_CRITICAL;
                else if (current_ppm < WARNING_LEVEL_PPM) app->state = APP_STATE_WARNING;
                break;

            case APP_STATE_CRITICAL:
                if (current_ppm < ALARM_LEVEL_PPM) app->state = APP_STATE_ALARM;
                break;

            default: break;
        }
    }
    if (app->state != old_state) {
        _app_update_indication(app);
    }
}

//-------------------------------------------------------------------------
void app_init(application_t *app, mq7_t *sensor, led_t *green, led_t* red, buzzer_t *buzzer) {
    app->sensor = sensor;
    app->led_green = green;
    app->led_red = red;
    app->buzzer = buzzer;

    app->state = APP_STATE_WARMUP;
    _app_update_indication(app);
    buzzer_beep(app->buzzer);
}
//-------------------------------------------------------------------------
void app_process(application_t *app, uint32_t now) {
    mq7_process(app->sensor, now);
    led_process(app->led_green, now);
    led_process(app->led_red, now);
    buzzer_process(app->buzzer, now);

    switch(app->state) {
        case APP_STATE_WARMUP:
            if(app->sensor->state == MQ7_STATE_MEASURE) {
                app->state = APP_STATE_DEFAULT;
                _app_update_indication(app);
                buzzer_beep(app->buzzer);
            }
            break;

        default:
            _app_check_levels(app);
            break;
    }
}

