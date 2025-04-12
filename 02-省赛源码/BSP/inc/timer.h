#ifndef TIM_H__
#define TIM_H__

#include "gd32f4xx.h"

extern uint16_t TIME_MS;

extern volatile uint32_t delay_ms_const;

#define FAN_PWM_RTC RCU_GPIOA
#define FAN_PWM_GPIO_Port GPIOA
#define FAN_PWM_Pin GPIO_PIN_0
#define FAN_PWM_CHANNEL TIMER_CH_0

void pwm_gpio_config(void);
void TIMER1_init(uint16_t arr, uint16_t psc);
void TIMER4_init(uint16_t arr, uint16_t psc);
void TIMER3_init(uint16_t arr, uint16_t psc);


#endif
