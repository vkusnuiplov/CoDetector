/**
  ******************************************************************************
  * @file    led.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи зі світлодіодом.
  ******************************************************************************
  */

#ifndef LED_H
#define LED_H

#include "main.h"


#define LED_TIME_FAST_ON_MS         200U
#define LED_TIME_FAST_OFF_MS        200U

#define LED_TIME_MED_ON_MS          500U
#define LED_TIME_MED_OFF_MS         500U

#define LED_TIME_SLOW_ON_MS         1000U
#define LED_TIME_SLOW_OFF_MS        1000U

#define LED_TIME_DEFAULT_ON_MS      300U
#define LED_TIME_DEFAULT_OFF_MS     4700U

typedef enum {
    LED_OFF,
    LED_ON,
    LED_BLINK_FAST,
    LED_BLINK_MEDIUM,
    LED_BLINK_SLOW,
    LED_BLINK_HEARTBEAT
} led_mode_e;

typedef enum {
	LED_ACTIVE_LOW,
	LED_ACTIVE_HIGH
} led_polarity_e;

typedef struct {

    GPIO_TypeDef* port;
    uint16_t      pin;

    GPIO_PinState pin_active_state;

    led_mode_e    mode;
    led_polarity_e polarity;
    uint8_t       is_on;

    uint32_t      timer_on_ms;
    uint32_t      timer_off_ms;

    //uint32_t      blink_period_ms;
    uint32_t      last_toggle_time;
} led_t;

void led_init(led_t *handle, GPIO_TypeDef* port, uint16_t pin, led_polarity_e polarity);
void led_on(led_t *handle);
void led_off(led_t *handle);
void led_toggle(led_t *handle);
void led_set_mode(led_t *handle, led_mode_e mode);
void led_process(led_t *handle, uint32_t current_time_ms);

#endif




