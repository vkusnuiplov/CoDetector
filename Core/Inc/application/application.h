/**
  ******************************************************************************
  * @file    application.h
  * @author  vkusnuiplov
  * @brief   Header file for the application business logic
  * @details Defines application states, gas concentration thresholds, and
  * the public API for the main system task
  ******************************************************************************
  */
#ifndef APPLICATION_H
#define APPLICATION_H

#include "mq7.h"
#include "led.h"
#include "buzzer.h"
#include <stdbool.h>

/* --- Gas Concentration Thresholds --- */
#define DEFAULT_LEVEL_PPM   30U
#define WARNING_LEVEL_PPM   60U

/* --- Timing Configurations --- */
#define APP_WARMUP_TIME_MS  SENSOR_INITIAL_CLEANING_TIME

/**
 * @brief High-level system states
 */
typedef enum {
    APP_STATE_WARMUP,
    APP_STATE_DEFAULT,
    APP_STATE_WARNING,
    APP_STATE_ALARM,
    APP_STATE_ERROR
} app_state_t;

/**
 * @brief Application instance structure (Handle)
 * Aggregates all peripheral drivers and system state
 */
typedef struct {
    mq7_t* sensor;
    led_t* led_green;
    led_t* led_red;
    buzzer_t* buzzer;

    app_state_t state;
    bool warmup_beep_done;

} application_t;

/**
 * @brief Configuration mapping for actuators
 */
typedef struct {
    led_mode_e      green_mode;
    led_mode_e      red_mode;
    buzzer_state_e  buzzer_state;
} indication_cfg_t;

/* --- Public API --- */

/**
 * @brief  Initializes the application, links hardware drivers, and sets initial state
 * @param  app Pointer to the application instance
 */
void app_init(application_t *app);

/**
 * @brief  Main application task. Processes sensor logic and updates actuators
 * @param  app Pointer to the application instance
 * @param  now Current system time in milliseconds
 */
void app_process(application_t *app, uint32_t now);

#endif
