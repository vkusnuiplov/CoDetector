/**
  ******************************************************************************
  * @file    mq7.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи з датчиком CO MQ-7
  * * Файл містить кінцевий автомат станів для керування датчиком MQ-7,
  * а також функції ініціалізації та обчислення результатів вимірювань
  *
  ******************************************************************************
  */

#ifndef MQ7_H
#define MQ7_H

#include <stdint.h>


#define SENSOR_VCC_MV 5000U
#define ADC_REFF_VCC_MV 3300U
#define ADC_MAX_VALUE                       4095U

#define ADC_ERROR_VALUE                     10U

#define SENSOR_R0_RESISTANSE                920U
#define SENSOR_RL_RESISTASE                 2000U

#define SENSOR_INITIAL_CLEANING_TIME        180000U
#define SENSOR_HEATING_HIGH_TIME            60000U
#define SENSOR_HEATING_LOW_TIME             85000U
#define SENSOR_MEASURE_TIME                 5000U
#define SENSOR_PPM_CALC_INTERVAL            250U

#define HEATER_ON                           1U
#define HEATER_OFF                          0U

#define SENSOR_EMERGENCY_RAW_ADC_THRESHOLD  3900U

#define SENSOR_LUT_SIZE 10U
#define SENSOR_LUT_X_VALUES {1000, 1200, 1400, 1700, 2000, 2400, 2900, 3500, 4500, 6000}
#define SENSOR_LUT_Y_VALUES {100,  77,   61,   47,   37,   29,   22,   17,   12,   8}

typedef enum {
    MQ7_STATE_INIT_CLEANING,
    MQ7_STATE_HEATING_HIGH,      // 60 sec
    MQ7_STATE_HEATING_LOW,       // 85sec
    MQ7_STATE_MEASURE            // 5 sec heating low
} mq7_state_e;

typedef enum {
    MQ7_OK,
    MQ7_ERR_NULL_PTR,
    MQ7_ERR_INVALID_STATE,
    MQ7_ERR_IO
} mq7_status_e;

typedef struct {
    uint32_t duration_ms;
    uint8_t heater_on;
    mq7_state_e next_state;
} mq7_cycle_step_t;

typedef void (*mq7_set_heater_fn)(uint8_t state);
typedef uint32_t (*mq7_get_adc_fn)(void);

typedef struct {
    mq7_set_heater_fn   set_heater;
    mq7_get_adc_fn      get_adc_data;
} mq7_io_t;

typedef struct {
    mq7_io_t io;
    volatile uint32_t raw_adc_value;
    uint16_t          current_ppm;
    mq7_state_e       state;
    uint32_t          timer_start_ms;
    uint32_t          last_ppm_calc_time_ms;
} mq7_t;

void mq7_sensor_init (mq7_t *handle, mq7_io_t io);
mq7_status_e mq7_process (mq7_t *handle, uint32_t current_time_ms);

#endif
