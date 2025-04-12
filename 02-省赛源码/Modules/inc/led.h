#ifndef __LED_H
#define __LED_H

#include "gd32f4xx.h"

#define LED_01_RTC RCU_GPIOE
#define LED_01_GPIO_Port GPIOE
#define LED_01_Pin GPIO_PIN_12

#define LED_02_RTC RCU_GPIOE
#define LED_02_GPIO_Port GPIOE
#define LED_02_Pin GPIO_PIN_13

#define LED_03_RTC RCU_GPIOE
#define LED_03_GPIO_Port GPIOE
#define LED_03_Pin GPIO_PIN_14

#define LED_04_RTC RCU_GPIOE
#define LED_04_GPIO_Port GPIOE
#define LED_04_Pin GPIO_PIN_15

// 定义LED状态
typedef enum {
    LED_STATE_1 = 0,
    LED_STATE_2,
    LED_STATE_3,
    LED_STATE_4
} LEDState;

static void LED_Init(void);
static void LED_Test(void);
static void Led_Set(uint8_t val);
static void Led_Set_All_Pin(uint8_t a, uint8_t b, uint8_t c, uint8_t d);

typedef struct led_i {
	uint8_t state;
  void (*init)(void);
  void (*test)(void);
	void (*set)(uint8_t);
	void (*set_all_pins)(uint8_t, uint8_t, uint8_t, uint8_t);
} led_i;

extern led_i led;

#endif
