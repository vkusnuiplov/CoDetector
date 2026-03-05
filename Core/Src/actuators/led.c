/**
  ******************************************************************************
  * @file    led.c
  * @author  vkusnuiplov
  * @brief   Driver for the LED actuator
  * @details Manages LED states, polarity abstraction (active high/low),
  * and non-blocking blink patterns using delta-time logic
  *****************************************************************************
  */

#include "led.h"
#include <stdint.h>
#include <stddef.h>

/**
 * @brief Configuration table for different LED blinking patterns
 * Defines the logical ON and OFF duration in milliseconds for each state
 */
static const blink_timing_t blink_table[] = {
    [LED_OFF]             = {LED_TIME_OFF_PATTERN_ON_MS,    LED_TIME_OFF_PATTERN_OFF_MS},
    [LED_ON]              = {LED_TIME_ON_PATTERN_ON_MS,     LED_TIME_ON_PATTERN_OFF_MS},
    [LED_BLINK_SLOW]      = {LED_TIME_SLOW_ON_MS,           LED_TIME_SLOW_OFF_MS},
    [LED_BLINK_MEDIUM]    = {LED_TIME_MED_ON_MS,            LED_TIME_MED_OFF_MS},
    [LED_BLINK_FAST]      = {LED_TIME_FAST_ON_MS,           LED_TIME_FAST_OFF_MS},
    [LED_BLINK_HEARTBEAT] = {LED_TIME_DEFAULT_ON_MS,        LED_TIME_DEFAULT_OFF_MS},
};

//-------------------------------------------------------------------------
/**
 * @brief  Writes the desired logical state to the physical LED pin
 * @note   Automatically handles hardware polarity (Active High vs Active Low)
 * @param  handle Pointer to the LED instance
 * @param  want_turn_on Logical state (1 to turn ON, 0 to turn OFF)
 */
static void _hw_write(led_t *handle, uint8_t want_turn_on) {
    uint8_t pin_state;

    if (want_turn_on) {
        pin_state = (handle->polarity == LED_ACTIVE_HIGH) ? LED_ACTIVE_POT_HIGH : LED_ACTIVE_POT_LOW;
    }
    else {
        pin_state = (handle->polarity == LED_ACTIVE_HIGH) ? LED_ACTIVE_POT_LOW : LED_ACTIVE_POT_HIGH;
    }

    if (handle->hw_control != NULL) {
        handle->hw_control(pin_state);
    }

    handle->is_on = want_turn_on;
}

//-------------------------------------------------------------------------
/**
 * @brief  Toggles the current logical state of the LED
 * @param  handle Pointer to the LED instance
 */
static void _hw_toggle(led_t *handle) {
    _hw_write(handle, !handle->is_on);
}

//-------------------------------------------------------------------------
/**
 * @brief  Initializes the LED instance and links hardware IO
 * @param  handle Pointer to the LED instance
 * @param  control_fn Hardware IO callback for controlling the physical pin
 * @param  polarity Hardware wiring polarity (Active High or Active Low)
 */
void led_init(led_t *handle, bsp_led_control control_fn, led_polarity_e polarity) {
    if (handle == NULL) return;

    handle->hw_control = control_fn;
    handle->polarity  = polarity;

    handle->mode = LED_OFF;
    handle->is_on = 0;
    handle->timer_on_ms = 0;
    handle->timer_off_ms = 0;

    _hw_write(handle, LED_ACTIVE_POT_LOW);
}

//-------------------------------------------------------------------------
/**
 * @brief  Turns the LED ON continuously
 * @param  handle Pointer to the LED instance
 */
void led_on(led_t *handle) {
    led_set_mode(handle, LED_ON);
}

//-------------------------------------------------------------------------
/**
 * @brief  Turns the LED OFF continuously
 * @param  handle Pointer to the LED instance
 */
void led_off(led_t *handle) {
    led_set_mode(handle, LED_OFF);
}

//-------------------------------------------------------------------------
/**
 * @brief  Sets the operational mode (blink pattern) of the LED
 * @param  handle Pointer to the LED instance
 * @param  mode Desired LED mode (e.g., LED_BLINK_FAST, LED_ON)
 */
void led_set_mode(led_t *handle, led_mode_e mode) {
	if (handle->mode == mode) return;
    handle->mode = mode;
    handle->last_toggle_time = 0;

    if (mode == LED_OFF) {
        _hw_write(handle, LED_ACTIVE_POT_LOW);
        return;
    }

    if (mode == LED_ON) {
        _hw_write(handle, LED_ACTIVE_POT_HIGH);
        return;
    }

    const blink_timing_t *cfg = &blink_table[mode];
    handle->timer_on_ms = cfg->on_ms;
    handle->timer_off_ms = cfg->off_ms;
    _hw_write(handle, LED_ACTIVE_POT_HIGH);
}

//-------------------------------------------------------------------------
/**
 * @brief  Processes the non-blocking blink logic based on current mode timings
 * @note   Must be called continuously in the main application loop
 * @param  handle Pointer to the LED instance
 * @param  current_time_ms Current system time in milliseconds
 */
void led_process(led_t *handle, uint32_t current_time_ms) {
    if (handle->mode == LED_OFF || handle->mode == LED_ON) return;

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
//-------------------------------------------------------------------------


