#include "smoke.h"
#include "adc.h"
#include "math.h"
#include "oled.h"
#include "stdio.h"

//连接P5端口

#define R0 24329  // MQ2在纯净空气下的电阻值

smoke_t smoke = {
    .init = Smoke_Init,
    .get_val = Smoke_Get_Val,
    .test = Smoke_Test,
};

static ADC_Config adc1_config = {
    .rcu_gpio = RCU_GPIOA,
    .gpio_port = GPIOA,
    .gpio_pin = GPIO_PIN_1,
    .rcu_adc = RCU_ADC0,
    .adc_periph = ADC0,
    .adc_channel = ADC_CHANNEL_1,
    .sample_time = ADC_SAMPLETIME_15,
    .clock_div = ADC_ADCCK_PCLK2_DIV8
};

/* 初始化函数 */
void Smoke_Init(void)
{
	adc_init(&adc1_config);
}

/* 获取ADC采样值 */
static inline uint16_t get_adc_sample(void) {
    return adc_sample(&adc1_config);
}

/* 获取烟雾浓度值 */
int Smoke_Get_Val(void)
{
    float rs_ro;
    uint16_t adc_val = get_adc_sample();
    
    // 计算Rs/R0比值
    rs_ro = (50000 * 4095) / (3.3f * adc_val) - 20000/R0;
    
    // 计算并限制浓度值
    int concentration = (int)(609.34f * pow(rs_ro, -2.148f));
    return (concentration > 9999) ? 9999 : adc_val;
}

/* 测试显示函数 */
void Smoke_Test(void)
{
    char print_buf[20] = {0};
    int value = Smoke_Get_Val();
    
    sprintf(print_buf, "%4d", value);
    oled.show_int32_num(70, 4, get_adc_sample(), 4, 16);
    oled.show_str(64, 6, print_buf, 16);
}
