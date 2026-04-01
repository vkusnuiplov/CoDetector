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

void led_init(led_t *handle, GPIO_TypeDef* port, uint16_t pin, uint8_t active_potential_led) {
    if (handle == NULL) return;

    handle->port = port;
    handle->pin  = pin;
    
    if (active_potential_led) {
        handle->pin_active_state = GPIO_PIN_RESET;
    } else {
        handle->pin_active_state = GPIO_PIN_SET;
    }

    handle->mode = LED_OFF;
    handle->is_on = 0;
    handle->blink_period_ms = 1000; 
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

void led_blink(led_t *handle, uint32_t period_ms) {
    if (handle->mode == LED_BLINK && handle->blink_period_ms == period_ms) {
        return;
    }

    handle->mode = LED_BLINK;
    handle->blink_period_ms = period_ms;
    handle->last_toggle_time = HAL_GetTick();

    _hw_write(handle, 1);
}

void led_process(led_t *handle, uint32_t current_time_ms) {
    if (handle->mode != LED_BLINK) return; 

    if (current_time_ms - handle->last_toggle_time >= handle->blink_period_ms) {
        _hw_toggle(handle);
        handle->last_toggle_time = current_time_ms;
    }
}