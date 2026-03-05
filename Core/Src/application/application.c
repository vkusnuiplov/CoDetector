/**
  ******************************************************************************
  * @file    application.c
  * @author  vkusnuiplov
  * @brief   Реалізація бізнес логіки
  ******************************************************************************
  */

#include "application.h"
#include "bsp.h"

static led_t h_led_green;
static led_t h_led_red;
static mq7_t h_sensor;
static buzzer_t h_buzzer;

//-------------------------------------------------------------------------
static const indication_cfg_t indication_table[] = {
    [APP_STATE_WARMUP] =   {LED_BLINK_SLOW,      LED_OFF,        BUZZER_OFF},
    [APP_STATE_DEFAULT] =  {LED_BLINK_HEARTBEAT, LED_OFF,        BUZZER_OFF},
    [APP_STATE_WARNING] =  {LED_OFF,             LED_BLINK_FAST, BUZZER_OFF},
    [APP_STATE_ALARM] =    {LED_OFF,             LED_BLINK_FAST, BUZZER_DANGER_ALARM},
    [APP_STATE_ERROR] =    {LED_OFF,             LED_BLINK_SLOW, BUZZER_OFF},
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

    if (app->sensor->state == MQ7_STATE_HEATING_LOW) {
        if (app->sensor->raw_adc_value > SENSOR_EMERGENCY_RAW_ADC_THRESHOLD) {
            if (app->state != APP_STATE_ALARM) {
                app->state = APP_STATE_ALARM;
            }
        }
    }

    else if (app->sensor->state == MQ7_STATE_MEASURE) {
        uint16_t current_ppm = app->sensor->current_ppm;

        switch (app->state) {
            case APP_STATE_DEFAULT:
                if (current_ppm > WARNING_LEVEL_PPM) app->state = APP_STATE_ALARM;
                else if (current_ppm > DEFAULT_LEVEL_PPM) app->state = APP_STATE_WARNING;
                break;

            case APP_STATE_WARNING:
                if (current_ppm > WARNING_LEVEL_PPM) app->state = APP_STATE_ALARM;
                else if (current_ppm < DEFAULT_LEVEL_PPM) app->state = APP_STATE_DEFAULT;
                break;

            case APP_STATE_ALARM:
                if (current_ppm < DEFAULT_LEVEL_PPM) app->state = APP_STATE_DEFAULT;
                else if (current_ppm < WARNING_LEVEL_PPM) app->state = APP_STATE_WARNING;
                break;

            default: break;
        }
    }
    if (app->state != old_state) {
        _app_update_indication(app);
    }
}

//-------------------------------------------------------------------------
void app_init(application_t *app) {
    BSP_Init();
    mq7_io_t mq_io = {
        .set_heater = BSP_MQ7_Heater_Set,
        .get_adc_data = BSP_MQ7_Get_ADC_value
    };
    mq7_sensor_init (&h_sensor, mq_io);

    led_init (&h_led_green, BSP_LED_Green_Set, LED_ACTIVE_HIGH);
    led_init (&h_led_red, BSP_LED_Red_Set, LED_ACTIVE_HIGH);

    buzzer_io_t buzz_io = {
        .set_freq = BSP_Buzzer_SetFreq,
        .stop = BSP_Buzzer_Stop
    };
    buzzer_init (&h_buzzer, buzz_io);

    app->sensor = &h_sensor;
    app->led_green = &h_led_green;
    app->led_red = &h_led_red;
    app->buzzer = &h_buzzer;

    app->state = APP_STATE_WARMUP;
    app->warmup_beep_done = false;
    _app_update_indication(app);
    buzzer_beep(app->buzzer);
}
//-------------------------------------------------------------------------
void app_process(application_t *app, uint32_t now) {

    if (app == NULL) return;

    mq7_status_e sensor_status = mq7_process (app->sensor, now);
    led_process(app->led_green, now);
    led_process(app->led_red, now);
    buzzer_process(app->buzzer, now);

    bool is_hardware_error = (sensor_status != MQ7_OK) || (app->sensor->raw_adc_value < ADC_ERROR_VALUE);

    if (is_hardware_error) {
        if (app->state != APP_STATE_ERROR) {
            app->state = APP_STATE_ERROR;
            _app_update_indication(app);
        }
        return;
    }

    if (app->state == APP_STATE_ERROR && !is_hardware_error) {
        app->state = APP_STATE_DEFAULT;
        _app_update_indication(app);
        return;
    }

    switch(app->state) {
        case APP_STATE_WARMUP:
            if(app->sensor->state == MQ7_STATE_HEATING_HIGH && app->warmup_beep_done == false) {
                buzzer_beep(app->buzzer);
                app->warmup_beep_done = true;
        }

        if(app->sensor->state == MQ7_STATE_MEASURE) {
            app->state = APP_STATE_DEFAULT;
            _app_update_indication(app);
        }

        break;

        case APP_STATE_DEFAULT:
        case APP_STATE_WARNING:
        case APP_STATE_ALARM:
            _app_check_levels(app);
            break;

        default:
            break;
    }
}

//-------------------------------------------------------------------------
