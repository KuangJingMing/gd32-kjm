#include "infrared_distance.h"
#include "adc.h"
#include "math.h"

//连接P5端口

 infrared_distance_t infrared_distance = {
	.init = Infrared_Distance_Init,
	.get_distance = Infrared_Distance_Get_Value,
 };

 static ADC_Config adc2_config = {
    .rcu_gpio = RCU_GPIOA,
    .gpio_port = GPIOA,
    .gpio_pin = GPIO_PIN_0,
    .rcu_adc = RCU_ADC0,
    .adc_periph = ADC0,
    .adc_channel = ADC_CHANNEL_0,
    .sample_time = ADC_SAMPLETIME_15,
    .clock_div = ADC_ADCCK_PCLK2_DIV8
};
 
void Infrared_Distance_Init(void)
{
	adc_init(&adc2_config);
}

//红外测距
float Infrared_Distance_Get_Value(void)
{
    float voltage, distance;
    uint16_t adc_val = adc_sample(&adc2_config);

    // 计算输出电压值
    voltage = adc_val * (3.3f / 4095.0f);  // 假设ADC值范围是0-4095, 3.3V参考电压

    // 计算距离（假设有对应的公式，保持一致的计算方法）
    distance = 12.08f * pow(voltage, -1.058f);  // 适配你的红外传感器特性公式

    // 限制距离范围
    if (distance < 4.0f) distance = 4.0f;
    if (distance > 80.0f) distance = 80.0f;
	
    return distance * 2.53f;
}
