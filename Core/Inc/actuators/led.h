/**
  ******************************************************************************
  * @file    led.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи зі світлодіодом
  ******************************************************************************
  */

#ifndef LED_H
#define LED_H

#include <stdint.h>

#define LED_TIME_ON_PATTERN_ON_MS   0U
#define LED_TIME_ON_PATTERN_OFF_MS  0U

#define LED_TIME_OFF_PATTERN_ON_MS  0U
#define LED_TIME_OFF_PATTERN_OFF_MS 0U

#define LED_TIME_FAST_ON_MS         200U
#define LED_TIME_FAST_OFF_MS        200U

#define LED_TIME_MED_ON_MS          500U
#define LED_TIME_MED_OFF_MS         500U

#define LED_TIME_SLOW_ON_MS         1000U
#define LED_TIME_SLOW_OFF_MS        1000U

#define LED_TIME_DEFAULT_ON_MS      300U
#define LED_TIME_DEFAULT_OFF_MS     4700U

#define LED_ACTIVE_POT_HIGH         1U
#define LED_ACTIVE_POT_LOW          0U

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
    uint16_t on_ms;
    uint16_t off_ms;
} blink_timing_t;

typedef void (*bsp_led_control)(uint8_t state);

typedef struct {
    bsp_led_control hw_control;
    led_mode_e    mode;
    led_polarity_e polarity;
    uint8_t       is_on;
    uint32_t      timer_on_ms;
    uint32_t      timer_off_ms;
    uint32_t      last_toggle_time;
} led_t;

void led_init(led_t *handle, bsp_led_control control_fn, led_polarity_e polarity);
void led_on(led_t *handle);
void led_off(led_t *handle);
void led_set_mode(led_t *handle, led_mode_e mode);
void led_process(led_t *handle, uint32_t current_time_ms);

#endif




