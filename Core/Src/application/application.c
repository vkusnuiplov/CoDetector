/**
  ******************************************************************************
  * @file    application.c
  * @author  vkusnuiplov
  * @brief   Реалізація бізнес логіки
  ******************************************************************************

 	@section Application Architecture

	Архітектура базується на циклічному опитуванні (Polling) та подієвій моделі:

	1. Initialization (app_init)
		Ініціалізація BSP
		Зв'язування абстрактних драйверів з функціями BSP.
		Встановлення початкового стану APP_STATE_WARMUP.

	2. Processing Loop (app_process)
		Оновлення таймерів драйверів
		Перевірка умов переходу FSM

  ******************************************************************************
	@section FSM States & Indication Logic

	Поведінка пристрою визначається таблицею індикації

	1. APP_STATE_WARMUP
		Indication: Green SLOW Blink | Red OFF | Silent
		Старт системи. Очікування виходу сенсора на робочий режим (MEASURE)

	2. APP_STATE_DEFAULT
		Indication: Green HEARTBEAT | Red OFF | Silent
		Рівень CO в межах норми < DEFAULT_LEVEL_PPM

	3. APP_STATE_WARNING
		Indication: Green OFF | Red FAST Blink | Silent
		Рівень CO підвищений > DEFAULT_LEVEL_PPM, але не критичний

	4. APP_STATE_ALARM
		Indication: Green OFF | Red FAST Blink | Siren Sound
		Критичний рівень CO > WARNING_LEVEL_PPM або аварійний стрибок поза фазою вимірювання

	5. APP_STATE_ERROR
		Indication: Green OFF | Red ON | Silent
		Виявлено обрив сенсора або некоректні дані АЦП < Min Threshold

  ******************************************************************************
	@section Safety & Hysteresis Logic

	Алгоритм запобігання хибним спрацьовуванням та мерехтіння станів:

		Emergency Check: Під час фази Low Heating (1.4V) перевіряється сире
	  	значення АЦП. Якщо воно перевищує критичний поріг -> миттєва тривога
	    ALARM, ігноруючи розрахунок PPM

	    PPM Measurement: Під час фази Measure аналізується розрахований PPM.
	    Переходи між станами Default <-> Warning <-> Alarm мають гістерезис
	    у вигляді власного діапазону станів, для стабільності роботи

		Реалізовано механізм швидкого скидання тривоги Alarm -> Default,
	  	якщо концентрація газу різко впала, минаючи проміжні стани

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

    if (app->sensor->raw_adc_value < ADC_ERROR_VALUE) {
        app->state = APP_STATE_ERROR;
    }

    else if (app->state == APP_STATE_ERROR) {
        app->state = APP_STATE_WARMUP;
    }

    else if (app->sensor->state == MQ7_STATE_HEATING_LOW) {
        if (app->sensor->raw_adc_value > SENSOR_EMERGENCY_RAW_ADC_THRESHOLD) {
            if (app->state != APP_STATE_ALARM) {
                app->state = APP_STATE_ALARM;
            }
        }
    }

    else if (app->sensor->state == MQ7_STATE_MEASURE) {
        float current_ppm = app->sensor->current_ppm;

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
//-------------------------------------------------------------------------
