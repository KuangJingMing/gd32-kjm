#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>
#include "gd32f4xx.h"

// ADC 配置结构体
typedef struct {
    rcu_periph_enum rcu_gpio;          // GPIO 时钟 (如 RCU_GPIOA)
	  rcu_periph_enum rcu_adc;           // ADC 时钟 (如 RCU_ADC0)
    uint32_t gpio_port;         // GPIO 端口 (如 GPIOA)
    uint32_t gpio_pin;          // GPIO 引脚 (如 GPIO_PIN_1)
    uint32_t adc_periph;        // ADC 外设 (如 ADC0)
    uint8_t  adc_channel;       // ADC 通道 (如 ADC_CHANNEL_1)
    uint32_t sample_time;       // 采样时间 (如 ADC_SAMPLETIME_15)
    uint32_t clock_div;         // ADC 时钟分频 (如 ADC_ADCCK_PCLK2_DIV8)
} ADC_Config;

// 函数声明
void adc_init(const ADC_Config *config);
uint16_t adc_sample(const ADC_Config *config);

#endif
