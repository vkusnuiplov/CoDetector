/**
  ******************************************************************************
  * @file    mq7.h
  * @author  vkusnuiplov
  * @brief   Header file for the MQ-7 CO sensor driver
  * @details Contains FSM definitions, configuration macros, and the public API
  * for initializing and reading the MQ-7 sensor
  ******************************************************************************
  */

#ifndef MQ7_H
#define MQ7_H

#include <stdint.h>

/* --- Hardware & ADC Configurations --- */
#define SENSOR_VCC_MV 5000U
#define ADC_REFF_VCC_MV 3300U
#define ADC_MAX_VALUE                       4095U
#define ADC_ERROR_VALUE                     10U

/* --- Sensor Characteristics --- */
#define SENSOR_R0_RESISTANSE                920U
#define SENSOR_RL_RESISTASE                 2000U
#define SENSOR_EMERGENCY_RAW_ADC_THRESHOLD  3900U

/* --- FSM Timings (in milliseconds) --- */
#define SENSOR_INITIAL_CLEANING_TIME        180000U
#define SENSOR_HEATING_HIGH_TIME            60000U
#define SENSOR_HEATING_LOW_TIME             85000U
#define SENSOR_MEASURE_TIME                 5000U
#define SENSOR_PPM_CALC_INTERVAL            250U

/* --- Heater States --- */
#define HEATER_ON                           1U
#define HEATER_OFF                          0U

/* --- Look-Up Table for PPM Interpolation --- */
#define SENSOR_LUT_SIZE 10U
#define SENSOR_LUT_X_VALUES {1000, 1200, 1400, 1700, 2000, 2400, 2900, 3500, 4500, 6000}
#define SENSOR_LUT_Y_VALUES {100,  77,   61,   47,   37,   29,   22,   17,   12,   8}

/**
 * @brief MQ-7 finite state machine phases
 */
typedef enum {
    MQ7_STATE_INIT_CLEANING,
    MQ7_STATE_HEATING_HIGH,      // 60 sec
    MQ7_STATE_HEATING_LOW,       // 85sec
    MQ7_STATE_MEASURE            // 5 sec heating low
} mq7_state_e;

/**
 * @brief Driver execution status codes
 */
typedef enum {
    MQ7_OK,
    MQ7_ERR_NULL_PTR,
    MQ7_ERR_INVALID_STATE,
    MQ7_ERR_IO
} mq7_status_e;

/**
 * @brief Step configuration for the sensor cycle
 */
typedef struct {
    uint32_t duration_ms;
    uint8_t heater_on;
    mq7_state_e next_state;
} mq7_cycle_step_t;

/* IO function pointers */
typedef void (*mq7_set_heater_fn)(uint8_t state);
typedef uint32_t (*mq7_get_adc_fn)(void);

/**
 * @brief Hardware IO mapping structure
 */
typedef struct {
    mq7_set_heater_fn   set_heater;
    mq7_get_adc_fn      get_adc_data;
} mq7_io_t;

/**
 * @brief MQ-7 sensor instance structure (Handle)
 */
typedef struct {
    mq7_io_t io;
    volatile uint32_t raw_adc_value;
    uint16_t          current_ppm;
    mq7_state_e       state;
    uint32_t          timer_start_ms;
    uint32_t          last_ppm_calc_time_ms;
} mq7_t;

/* --- Public API --- */

/**
 * @brief  Initializes the MQ-7 sensor instance and starts the cleaning phase
 * @param  handle Pointer to the MQ-7 sensor instance
 * @param  io Structure containing hardware-specific IO callbacks
 */
void mq7_sensor_init (mq7_t *handle, mq7_io_t io);

/**
 * @brief  Processes the MQ-7 sensor FSM and handles periodic PPM calculations
 * @param  handle Pointer to the MQ-7 sensor instance
 * @param  current_time_ms Current system time in milliseconds
 * @retval mq7_status_e Status of the execution
 */
mq7_status_e mq7_process (mq7_t *handle, uint32_t current_time_ms);

#endif
