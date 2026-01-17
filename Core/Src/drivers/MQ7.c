/*
 * MQ7.c
 *
 *  Created on: Jan 14, 2026
 *      Author: kosti
 *
 *
 *
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);	Керування нагрівачем SET -> 5V // RESET -> 1.4V
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET); Зелений світлодіод
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET); Червоний світлодіод
		HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

		HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
		HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
 */


#include "MQ7.h"
#include "math.h"
#include "main.h"


// --- НАЛАШТУВАННЯ ---
#define RESISTANCE_0_VALUE	920		// Опір датчика на чистому повітрі (калібрований)
#define LOAD_RESISTANCE     2000	// Опір навантаження (2 кОм)
#define ALARM_TRESHOLD 		2500	// Поріг "сирого" АЦП для миттєвої тривоги
#define VCC_MV 				5000	// Напруга живлення датчика

#define LEVEL_WARN			20
#define LEVEL_HIGH			50
#define LEVEL_CRIT			75



// --- ЗМІННІ МОДУЛЯ ---
static uint32_t mq7_ticks = 0;   	// Лічильник часу (1 тік = 100 мс)
static uint32_t current_ppm = 0; 	// Останнє значення PPM
static uint32_t alarm_flag = 0;   	// Прапорець аварії

// Доступ до АЦП з main.c
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim1;

// --- ЛОГІКА ТАЙМЕРА (100 мс) ---
	void MQ7_Timer(void) {
		mq7_ticks++;

		if (mq7_ticks % 10 == 0)
			HAL_ADC_Start_IT(&hadc1);

		if (mq7_ticks < 600)
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET); // 5v on heater

		else if (mq7_ticks < 1500)
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET); // 1.4v on heater

		else
			mq7_ticks = 0;
	}


/*
 * -----Розрахунок ppm за отриманими з АЦП значеннями --------------------------------------------------------------------------------
 */
	void MQ7_ppm_calculation (uint32_t raw_adc_value) {

		uint32_t adc_voltage_mv = 0;
		uint32_t sensor_resistance = 0;
		float rs_r0_ratio = 0.0;
		float ppm_value = 0.0;


		if (raw_adc_value == 0) return;

		if (raw_adc_value > ALARM_TRESHOLD) alarm_flag = 1;

		else alarm_flag = 0;


		if (mq7_ticks >= 1450 && mq7_ticks < 1500)
		{
			adc_voltage_mv = (raw_adc_value * 3300) >> 12;
			if (adc_voltage_mv == 0) return;

			sensor_resistance = (LOAD_RESISTANCE * (VCC_MV - adc_voltage_mv)) / adc_voltage_mv;

			rs_r0_ratio = (float) sensor_resistance / (float) RESISTANCE_0_VALUE;

			ppm_value = 100.0f * powf(rs_r0_ratio, -1.43f);

			current_ppm = (uint32_t) ppm_value;
		}
	}
//-----------------------------------------------------------------------------------------------------------------------------

	void MQ7_alarm_state (void) {

	uint32_t one_second_cycle = mq7_ticks % 20;


		if (alarm_flag == 1 || current_ppm > LEVEL_CRIT)
		{
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

			if (one_second_cycle < 10)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
				HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);
				HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
			}
		}

		else if (current_ppm > LEVEL_HIGH)
		{
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET);
		}

		else if (current_ppm > LEVEL_WARN)
		{
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
						HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

			if (one_second_cycle < 10)
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
			}
			else
			{
				HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_RESET);
			}

		}

		else
		{
			HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

			HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, GPIO_PIN_SET);
		}
	}




