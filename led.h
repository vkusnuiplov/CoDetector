/**
  ******************************************************************************
  * @file    led.h
  * @author  vkusnuiplov
  * @brief   Заголовочний файл для роботи зі світлодіодом.
  ******************************************************************************
  */

#ifndef LED_H
#define LED_H

#include "main.h"   

typedef enum {
    LED_OFF,    
    LED_ON,     
    LED_BLINK   
} led_mode_e;

typedef struct {
    
    GPIO_TypeDef* port;
    uint16_t      pin;              
        
    GPIO_PinState pin_active_state;  
    
    led_mode_e    mode;            
    uint8_t       is_on;            
       
    uint32_t      blink_period_ms;   
    uint32_t      last_toggle_time;  
} led_t;

void led_init(led_t *handle, GPIO_TypeDef* port, uint16_t pin, uint8_t is_active_low);

void led_on(led_t *handle);
void led_off(led_t *handle);
void led_toggle(led_t *handle); 
void led_blink(led_t *handle, uint32_t period_ms);
void led_process(led_t *handle, uint32_t current_time_ms);

#endif
