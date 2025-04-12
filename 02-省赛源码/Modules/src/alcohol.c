#include "adc.h"
#include "alcohol.h"
#include "oled.h"
#include "math.h"
#include "stdio.h"

//连接P5端口

#define R0 24329  // MQ2在纯净空气下的电阻值

static int cvalue = 0;

alcohol_t alcohol = {
    .init = Alcohol_Init,
    .test = Alcohol_Test,
    .get_val = Alcohol_Get_Value,
};

// 定义一个 ADC 配置
static ADC_Config adc0_config = {
    .rcu_gpio = RCU_GPIOA,
    .gpio_port = GPIOA,
    .gpio_pin = GPIO_PIN_0,
    .rcu_adc = RCU_ADC0,
    .adc_periph = ADC0,
    .adc_channel = ADC_CHANNEL_0,
    .sample_time = ADC_SAMPLETIME_15,
    .clock_div = ADC_ADCCK_PCLK2_DIV8
};


/* 初始化函数 */
void Alcohol_Init(void)
{
	adc_init(&adc0_config);
}

/* 获取酒精浓度值 */
int Alcohol_Get_Value(void)
{
    float adc_val = adc_sample(&adc0_config) + cvalue;
    float rs_ro = (50000 * 4095) / (3.3f * adc_val) - 20000/R0;
    float concentration = 609.34f * pow(rs_ro, -2.148f);
    
    concentration = (adc_val * 6.6f / 4096);  // 合并电压计算
    return (concentration > 0.6f) ? (int)((concentration - 0.6f) * 200.0f) : 0;
}

/* 测试显示函数 */
void Alcohol_Test(void)
{
    static char print_buf[20] = {0};
    int value = Alcohol_Get_Value();
    float adc_val = adc_sample(&adc0_config);
    
    snprintf(print_buf, sizeof(print_buf),"%4d", value);
    oled.show_int32_num(70, 4, (uint32_t)adc_val, 4, 16);
    oled.show_str(164, 6, print_buf, 16);
}
