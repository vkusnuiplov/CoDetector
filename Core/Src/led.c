/**
  ******************************************************************************
  * @file    led.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи зі світлодіодом.
  *****************************************************************************
  */

#include "led.h"
// Вмикає лед з урахуванням який потенціал повинен бути на піні
static void _hw_write(led_t *handle, uint8_t want_turn_on) {
    GPIO_PinState level_to_write;

    if (want_turn_on != 0) {
        level_to_write = handle->pin_active_state;
    }
    else {
        if (handle->pin_active_state == GPIO_PIN_SET) {
            level_to_write = GPIO_PIN_RESET;
        }
        else {
            level_to_write = GPIO_PIN_SET;
        }
    }
    HAL_GPIO_WritePin(handle->port, handle->pin, level_to_write);
    handle->is_on = want_turn_on;
}

// Інвертує поточний стан
static void _hw_toggle(led_t *handle) {
    _hw_write(handle, !handle->is_on);
}

void led_init(led_t *handle, GPIO_TypeDef* port, uint16_t pin, led_polarity_e polarity) {
    if (handle == NULL) return;

    handle->port = port;
    handle->pin  = pin;

    if (polarity == LED_ACTIVE_HIGH) {
    	handle->pin_active_state = GPIO_PIN_SET;
    }
    else {
    	handle->pin_active_state = GPIO_PIN_RESET;
    }

    handle->mode = LED_OFF;
    handle->is_on = 0;
    handle->timer_on_ms = LED_TIME_SLOW_ON_MS;
    handle->timer_off_ms = LED_TIME_SLOW_OFF_MS;

    handle->last_toggle_time = HAL_GetTick();

    _hw_write(handle, 0);
}

void led_on(led_t *handle) {
    handle->mode = LED_ON;
    _hw_write(handle, 1);
}

void led_off(led_t *handle) {
    handle->mode = LED_OFF;
    _hw_write(handle, 0);
}

void led_toggle(led_t *handle) {
    _hw_toggle(handle);
    if (handle->is_on) {
        handle->mode = LED_ON;
    } else {
        handle->mode = LED_OFF;
    }
}

void led_set_mode(led_t *handle, led_mode_e mode) {

	if (handle->mode == mode) return;
    handle->mode = mode;
    handle->last_toggle_time = HAL_GetTick();

    switch (mode) {
        case LED_OFF:
            _hw_write(handle, 0);
            break;

        case LED_ON:
            _hw_write(handle, 1);
            break;

        case LED_BLINK_FAST:
            handle->timer_on_ms = LED_TIME_FAST_ON_MS;
            handle->timer_off_ms = LED_TIME_FAST_OFF_MS;
            _hw_write(handle, 1);
            break;

        case LED_BLINK_MEDIUM:
            handle->timer_on_ms = LED_TIME_MED_ON_MS;
            handle->timer_off_ms = LED_TIME_MED_OFF_MS;
            _hw_write(handle, 1);
            break;

        case LED_BLINK_SLOW:
            handle->timer_on_ms = LED_TIME_SLOW_ON_MS;
            handle->timer_off_ms = LED_TIME_SLOW_OFF_MS;
            _hw_write(handle, 1);
            break;

        case LED_BLINK_HEARTBEAT:
            handle->timer_on_ms = LED_TIME_DEFAULT_ON_MS;
            handle->timer_off_ms = LED_TIME_DEFAULT_OFF_MS;
            _hw_write(handle, 1);
            break;
    }
}

void led_process(led_t *handle, uint32_t current_time_ms) {
    if (handle->mode < LED_BLINK_FAST) return;

    uint32_t time_diff = current_time_ms - handle->last_toggle_time;
    uint32_t wait_time;

    if(handle->is_on){
        wait_time = handle->timer_on_ms;
    }
    else {
        wait_time = handle->timer_off_ms;
    }

    if(time_diff >= wait_time) {
        _hw_toggle(handle);
        handle->last_toggle_time = current_time_ms;
    }
}


