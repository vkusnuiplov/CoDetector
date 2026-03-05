/**
  ******************************************************************************
  * @file    buzzer.c
  * @author  vkusnuiplov
  * @brief   Driver for the buzzer actuator
  * @details Implements non-blocking frequency sweeps for alarms and
  * manages single beep generation
  ******************************************************************************
  */

#include "buzzer.h"
#include <stdint.h>

/**
 * @brief Configuration table for different buzzer states
 * Defines minimum/maximum frequencies, step sizes, and update intervals
 */
  static const buzzer_setting_t buzz_setting_table [] = {
      [BUZZER_OFF]          = {BUZZER_OFF_FREQ,         BUZZER_OFF_FREQ,        BUZZER_OFF_STEP,    BUZZER_OFF_SPEED},
      [BUZZER_ON]           = {BUZZER_ON_FREQ,          BUZZER_ON_FREQ,         BUZZER_ON_STEP,     BUZZER_ON_SPEED},
      [BUZZER_BEEP]         = {BUZZER_ON_FREQ,          BUZZER_ON_FREQ,         BUZER_BEEP_STEP,    BUZZER_BEEP_SPEED},
      [BUZZER_WARN_ALARM]   = {WARN_ALARM_MIN_FREQ,     WARN_ALARM_MAX_FREQ,    WARN_ALARM_STEP,    WARN_ALARM_SPEED_MS},
      [BUZZER_DANGER_ALARM] = {DANGER_ALARM_MIN_FREQ,   DANGER_ALARM_MAX_FREQ,  DANGER_ALARM_STEP,  DANGER_ALARM_SPEED_MS},

  };

//-------------------------------------------------------------------------
/**
* @brief  Stops the physical buzzer output
* @param  handle Pointer to the buzzer instance
*/
  static void _hw_buzzer_stop (buzzer_t *handle) {
    if (handle->io.stop) handle->io.stop();
  }

//-------------------------------------------------------------------------
/**
* @brief  Sets the physical buzzer frequency
* @param  handle Pointer to the buzzer instance
* @param  freq_hz Desired frequency in Hz
*/
  static void _hw_buzzer_set_freq (buzzer_t *handle, uint16_t freq_hz) {

    if (handle->io.set_freq) handle->io.set_freq(freq_hz);
  }

//-------------------------------------------------------------------------
/**
* @brief  Initializes the buzzer instance and its hardware IO callbacks
* @param  handle Pointer to the buzzer instance
* @param  hw_io Structure containing hardware IO function pointers
*/
  void buzzer_init (buzzer_t *handle, buzzer_io_t hw_io) {
    handle->io = hw_io;

    handle->state = BUZZER_OFF;

    handle->current_setting = &buzz_setting_table[BUZZER_OFF];

    handle->current_freq = 0;
    handle->current_delta = 0;
    handle->last_update_time = 0;
    handle->beep_start_time = 0;

    _hw_buzzer_stop(handle);
  }

//-------------------------------------------------------------------------
/**
* @brief  Triggers a single short beep
* @note   This action is ignored if the buzzer is currently in an active alarm state
* @param  handle Pointer to the buzzer instance
*/
void buzzer_beep (buzzer_t *handle) {
    if(handle->state == BUZZER_WARN_ALARM ||
         handle->state == BUZZER_DANGER_ALARM) {
          return;
    }
    handle->beep_start_time = 0;
    handle->beep_duration_ms = BUZZER_BEEP_TIME;
    buzzer_set_state(handle, BUZZER_BEEP);
  }

//-------------------------------------------------------------------------
/**
 * @brief  Changes the active state of the buzzer
 * @param  handle Pointer to the buzzer instance
 * @param  state The new state to apply (e.g., BUZZER_OFF, BUZZER_WARN_ALARM)
 */
void buzzer_set_state(buzzer_t *handle, buzzer_state_e state) {

    if (handle->state == state) return;
    handle->state = state;

    handle->current_setting = &buzz_setting_table[state];

    handle->current_freq = handle->current_setting->min_freq;
    handle->current_delta = (int16_t)handle->current_setting->step_size;
    handle->last_update_time = 0;

    if (state == BUZZER_OFF) {
      _hw_buzzer_stop(handle);
    }
    else {
      _hw_buzzer_set_freq(handle, handle->current_freq);
    }
}

//-------------------------------------------------------------------------
/**
 * @brief  Processes the buzzer logic (frequency sweeps, beep timeouts)
 * @note   Must be called continuously in the main application loop
 * @param  handle Pointer to the buzzer instance
 * @param  current_time_ms Current system time in milliseconds
 */
  void buzzer_process(buzzer_t *handle, uint32_t current_time_ms) {
    if (handle->state == BUZZER_BEEP && handle->beep_start_time == 0) {
      handle->beep_start_time = current_time_ms;
    }

    if(handle->state == BUZZER_BEEP) {
        if (current_time_ms - handle->beep_start_time >= handle->beep_duration_ms){
            buzzer_set_state(handle,BUZZER_OFF);
        }
        return;
    }

    if (handle->current_setting->step_size == 0) return;

    if (current_time_ms - handle->last_update_time >= handle->current_setting->period_ms) {

        handle->last_update_time = current_time_ms;

        int32_t next_freq = (int16_t)handle->current_freq + handle->current_delta;

        uint16_t max_f = handle->current_setting->max_freq;
        uint16_t min_f = handle->current_setting->min_freq;

        if (next_freq >= max_f) {
            next_freq = max_f;
            handle->current_delta = -((int16_t)handle->current_setting->step_size);
        }

        else if (next_freq <= min_f) {
            next_freq = min_f;
            handle->current_delta = ((int16_t)handle->current_setting->step_size);
        }
        handle->current_freq = (uint16_t)next_freq;
        _hw_buzzer_set_freq(handle, handle->current_freq);
    }
  }
  //-------------------------------------------------------------------------

