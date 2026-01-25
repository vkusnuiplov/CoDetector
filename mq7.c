/**
  ******************************************************************************
  * @file    mq7.c
  * @author  vkusnuiplov
  * @brief   Драйвер для роботи з датчиком CO MQ-7.
  * * Файл містить кінцевий автомат станів для керування датчиком MQ-7,
  * а також функції ініціалізації та обчислення результатів вимірювань.
  *
  ******************************************************************************
  */



#include "mq7.h"
#include <math.h>   // Для функції powf()
#include <stddef.h> // Для NULL


// --- ПРИВАТНІ ФУНКЦІЇ (Internal Helpers) ---
// static означає: "бачу тільки в цьому файлі"

static void _set_heating_high(mq7_t *handle) {
    // Вмикаємо повний нагрів (High Logic Level)
    HAL_GPIO_WritePin(handle->heater_port, handle->heater_pin, GPIO_PIN_SET);
}

static void _set_heating_low(mq7_t *handle) {
    // Вмикаємо слабкий нагрів (Low Logic Level)
    HAL_GPIO_WritePin(handle->heater_port, handle->heater_pin, GPIO_PIN_RESET);
}

static void _calculate_ppm(mq7_t *handle) {
    // variables for calculation
    uint32_t safe_adc = 0;     
    float adc_value_voltage_v = 0.0f;
    float sensor_resistance = 0.0f;
    float ratio = 0.0f;


    // 1. Захист від ділення на нуль

    if (handle->raw_adc_value == 0) {
        safe_adc = 1;
    } else {
        safe_adc = handle->raw_adc_value;
    }


    // 2. Розрахунок поточної напруги на АЦП
    // Voltage = ADC * (Vref / Max_ADC)
    adc_value_voltage_v = (float)safe_adc * handle->adc_step_v;

    // 3. Розрахунок опору сенсора 
        
    sensor_resistance = handle->rl_value * ((handle->sensor_vcc_v - adc_value_voltage_v) / adc_value_voltage_v);

    // 4. Розрахунок співвідношення Rs/R0
    ratio = sensor_resistance / handle->ro_value;

    // 5. Розрахунок PPM за формулою кривої
    // PPM = A * (ratio ^ B)
    handle->current_ppm = handle->curve_a * powf(ratio, handle->curve_b);
}

// --- ПУБЛІЧНІ ФУНКЦІЇ (API) ---

void mq7_init(mq7_t *handle) {
    if (handle == NULL) return; // Захист від дурня

    if (handle->adc_max_val != 0.0f) {
        handle->adc_step_v = handle->adc_vref / handle->adc_max_val; // Вольти на крок АЦП
    } 
    else {
        handle->adc_step_v = 0.0f; // Захист від ділення на нуль
    }
    // захист та дефолтні налаштування часових циклів
    if (handle->cycle_duration_high_ms == 0){
        handle->cycle_duration_high_ms = 60000; // 60 секунд дефолтне значення 
    }
    if (handle->cycle_duration_low_ms == 0){
        handle->cycle_duration_low_ms = 90000; // 90 секунд дефолтне значення
    }

    // Скидання змінних
    handle->current_ppm = 0.0f;
    handle->raw_adc_value = 0;
    
    // Старт з фази високого нагріву
    handle->state = MQ7_STATE_HEATING_HIGH_5V;
    handle->timer_start_ms = HAL_GetTick(); // Фіксуємо час старту
   
    _set_heating_high(handle);
}

void mq7_process(mq7_t *handle, uint32_t current_time_ms) {
    if (handle == NULL) return;

    // Розрахунок часу, що пройшов з останньої зміни стану
    // Працює коректно навіть при переповненні uint32_t (через ~49 днів)
    uint32_t elapsed = current_time_ms - handle->timer_start_ms;

    switch (handle->state) {
        
        // ФАЗА 1: Нагрів 5В (Очистка)
        case MQ7_STATE_HEATING_HIGH_5V:
            if (elapsed >= handle->cycle_duration_high_ms) {
                // Час переходити до вимірювання
                handle->state = MQ7_STATE_HEATING_LOW_1_4V;
                handle->timer_start_ms = current_time_ms; // Скидаємо таймер
                _set_heating_low(handle);
            }
            break;

        // ФАЗА 2: Нагрів 1.4В (Вимірювання)
        case MQ7_STATE_HEATING_LOW_1_4V:
            if (elapsed >= handle->cycle_duration_low_ms) {
                // Цикл завершено. Можна міряти!
                _calculate_ppm(handle);
                
                // Перезапуск циклу (знову на 5В)
                handle->state = MQ7_STATE_HEATING_HIGH_5V;
                handle->timer_start_ms = current_time_ms;
                _set_heating_high(handle);
            }
            break;
            
        default:
            // Якщо стан збився - скидаємо в Init
            mq7_init(handle);
            break;
    }
}

float mq7_get_ppm(mq7_t *handle) {
    if (handle == NULL) return -1.0f;
    return handle->current_ppm;
}