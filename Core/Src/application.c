/**
  ******************************************************************************
  * @file    application.c
  * @author  vkusnuiplov
  * @brief   Реалізація бізнес логіки
  ******************************************************************************
  */

#include "application.h"

static void _set_indication_warmup(application_t *app) {
    led_set_mode(app->led_green, LED_BLINK_SLOW);
    led_set_mode(app->led_red, LED_OFF);
    buzzer_set_state(app->buzzer, BUZZER_OFF);
}

static void _set_indication_default(application_t *app) {
    led_set_mode(app->led_green, LED_BLINK_HEARTBEAT);
    led_set_mode(app->led_red, LED_OFF);
    buzzer_set_state(app->buzzer, BUZZER_OFF);
}

static void _set_indication_warning(application_t *app) {
    led_set_mode(app->led_green, LED_BLINK_MEDIUM);
    led_set_mode(app->led_red, LED_OFF);
    buzzer_set_state(app->buzzer, BUZZER_OFF);
}

static void _set_indication_alarm(application_t *app) {
    led_set_mode(app->led_green, LED_OFF);
    led_set_mode(app->led_red, LED_BLINK_FAST);
    buzzer_set_state(app->buzzer, BUZZER_OFF);
}

static void _set_indication_critical(application_t *app) {
    led_set_mode(app->led_green, LED_OFF);
    led_set_mode(app->led_red, LED_BLINK_FAST);
    buzzer_set_state(app->buzzer, BUZZER_DANGER_ALARM);
}

static void _set_indication_error(application_t *app) {
    led_set_mode(app->led_green, LED_OFF);
    led_set_mode(app->led_red, LED_ON);
    buzzer_set_state(app->buzzer, BUZZER_OFF);
}

static void _app_check_levels(application_t *app) {
    if(app->sensor->raw_adc_value < ADC_ERROR_VALUE){
        if(app->state != APP_STATE_ERROR) {
            app->state = APP_STATE_ERROR;
            _set_indication_error(app);
        }
        return;
    }

    if(app->state == APP_STATE_ERROR) {
       app->state = APP_STATE_WARMUP;
       _set_indication_warmup(app);
       return;
    }

    if(app->sensor->state == MQ7_STATE_HEATING_HIGH ||
       app->sensor->state == MQ7_STATE_INIT_CLEANING) {
        return;
    }

    if(app->sensor->state == MQ7_STATE_HEATING_LOW) {
        if (app->sensor->raw_adc_value > SENSOR_EMERGENCY_RAW_ADC_THRESHOLD) {
            if (app->state != APP_STATE_CRITICAL){
                app->state = APP_STATE_CRITICAL;
                _set_indication_critical(app);
            }
        }
        return;
    }


    float current_ppm = app->sensor->current_ppm;
    app_state_t old_state = app->state;

    switch (app->state) {
        case APP_STATE_DEFAULT:
            if(current_ppm > DEFAULT_LEVEL_PPM) {
                app->state = APP_STATE_WARNING;
            }
        break;

        case APP_STATE_WARNING:
            if(current_ppm > WARNING_LEVEL_PPM) {
                app->state = APP_STATE_ALARM;
            }
            else if(current_ppm < DEFAULT_LEVEL_PPM) {
                app->state = APP_STATE_DEFAULT;
            }
        break;

        case APP_STATE_ALARM:
            if(current_ppm > ALARM_LEVEL_PPM) {
                app->state = APP_STATE_CRITICAL;
            }
            else if(current_ppm < WARNING_LEVEL_PPM) {
                app->state = APP_STATE_WARNING;
            }
        break;

        case APP_STATE_CRITICAL:
            if(current_ppm < ALARM_LEVEL_PPM){
                app->state = APP_STATE_ALARM;
            }
        break;

        default: break;
    }

    if(app->state != old_state){
        switch (app->state) {
            case APP_STATE_DEFAULT:     _set_indication_default(app);  break;
            case APP_STATE_WARNING:     _set_indication_warning(app);  break;
            case APP_STATE_ALARM:       _set_indication_alarm(app);    break;
            case APP_STATE_CRITICAL:    _set_indication_critical(app); break;
            default: break;
        }
    }
}

void app_init(application_t *app, mq7_t *sensor, led_t *green, led_t* red, buzzer_t *buzzer) {
    app->sensor = sensor;
    app->led_green = green;
    app->led_red = red;
    app->buzzer = buzzer;

    app->state = APP_STATE_WARMUP;
    _set_indication_warmup(app);

    buzzer_beep(app->buzzer);
}

void app_process(application_t *app, uint32_t now) {
    mq7_process(app->sensor, now);
    led_process(app->led_green, now);
    led_process(app->led_red, now);
    buzzer_process(app->buzzer, now);
    switch(app->state) {
        case APP_STATE_WARMUP:
            if(app->sensor->state == MQ7_STATE_MEASURE) {
                app->state = APP_STATE_DEFAULT;
                _set_indication_default(app);
                buzzer_beep(app->buzzer);
            }

        case APP_STATE_DEFAULT:
        case APP_STATE_WARNING:
        case APP_STATE_ALARM:
        case APP_STATE_CRITICAL:
        case APP_STATE_ERROR:

        _app_check_levels(app);

        break;
    }

}

