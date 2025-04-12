#include "motor.h"
#include "gd32f4xx_it.h"

//连接P16端口 PWM
//连接P14端口 编码器

// 编码器计数，volatile 确保中断中修改的值能被主循环读取
volatile int32_t encoder_count = 0;

// 电机对象实例
motor_t motor = {
    .init = motor_init,
    .set_speed = set_motor_speed,
    .get_rpm = get_motor_speed,
};

void motor_init(void) {
    motor_gpio_init();
    motor_timer_init();
    encoder_init();
}

void motor_gpio_init(void) {
    // 启用 GPIO 时钟
    rcu_periph_clock_enable(MOTOR_RCU);

    // 初始化电机控制引脚 (IN1, IN2) 作为备用功能 (AF) 推挽输出
    gpio_mode_set(MOTOR_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE, MOTOR_IN1_PIN | MOTOR_IN2_PIN);
    gpio_output_options_set(MOTOR_GPIO_PORT, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, MOTOR_IN1_PIN | MOTOR_IN2_PIN);
    gpio_af_set(MOTOR_GPIO_PORT, GPIO_AF_2, MOTOR_IN1_PIN | MOTOR_IN2_PIN); 

    // 初始化编码器引脚 (H1, H2) 作为输入
    gpio_mode_set(ENCODER_GPIO_PORT, GPIO_MODE_INPUT, GPIO_PUPD_NONE, ENCODER_H1_PIN | ENCODER_H2_PIN);
}

void motor_timer_init(void) {
    // 启用 TIMER1 时钟
    rcu_periph_clock_enable(MOTOR_TIMER_RCU);

    // 配置 TIMER1 基本参数
    timer_parameter_struct timer_initpara;
    timer_struct_para_init(&timer_initpara);
    timer_initpara.prescaler         = 16799;
    timer_initpara.alignedmode       = TIMER_COUNTER_EDGE; // 边沿对齐模式
    timer_initpara.counterdirection  = TIMER_COUNTER_UP; // 向上计数
    timer_initpara.period            = 999; // PWM 周期，决定 PWM 频率
    timer_initpara.clockdivision     = TIMER_CKDIV_DIV1; // 时钟分频
    timer_init(MOTOR_TIMER, &timer_initpara);

    // 配置 TIMER1 输出比较通道参数 (CH2 和 CH3)
    timer_oc_parameter_struct timer_ocinitpara;
    timer_channel_output_struct_para_init(&timer_ocinitpara);
    timer_ocinitpara.outputstate  = TIMER_CCX_ENABLE;       // 使能通道主输出
    timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;      // 禁用通道互补输出 (如果 CH2N/CH3N 存在)
    timer_ocinitpara.ocpolarity   = TIMER_OC_POLARITY_HIGH;  // 输出极性为高
    timer_ocinitpara.ocnpolarity  = TIMER_OCN_POLARITY_HIGH; // 互补输出极性 (禁用时无效)
    timer_ocinitpara.ocidlestate  = TIMER_OC_IDLE_STATE_LOW; // 空闲状态输出低
    timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW; // 互补空闲状态 (禁用时无效)

    timer_channel_output_config(MOTOR_TIMER, MOTOR_TIMER_CH1, &timer_ocinitpara); // 配置 CH2
    timer_channel_output_config(MOTOR_TIMER, MOTOR_TIMER_CH2, &timer_ocinitpara); // 配置 CH3

    // 配置 CH2 和 CH3 为 PWM 模式 0 (PWM Mode 1 极性相反，根据驱动器和需求选择)
    timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH1, 0); // 初始占空比 0%
    timer_channel_output_mode_config(MOTOR_TIMER, MOTOR_TIMER_CH1, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(MOTOR_TIMER, MOTOR_TIMER_CH1, TIMER_OC_SHADOW_DISABLE); // 禁用影子寄存器 (简单应用可以禁用)

    timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH2, 0); // 初始占空比 0%
    timer_channel_output_mode_config(MOTOR_TIMER, MOTOR_TIMER_CH2, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(MOTOR_TIMER, MOTOR_TIMER_CH2, TIMER_OC_SHADOW_DISABLE); // 禁用影子寄存器

    // 使能 Timer 的自动重载影子寄存器和 Timer 本身
    timer_auto_reload_shadow_enable(MOTOR_TIMER);
    timer_enable(MOTOR_TIMER);
}

void encoder_init(void) {
    // 启用 GPIO 时钟 (GPIOA 在 motor_gpio_init 中已启用，这里可以省略，但重复启用也没问题)
    rcu_periph_clock_enable(ENCODER_RCU);

    // 编码器引脚已在 motor_gpio_init 中初始化为输入模式

    // 配置外部中断 EXTI2 和 EXTI3，触发方式为双边沿 (上升沿和下降沿都触发)
    exti_init(EXTI_2, EXTI_INTERRUPT, EXTI_TRIG_BOTH); // H_OUT1 (PA2)
    exti_init(EXTI_3, EXTI_INTERRUPT, EXTI_TRIG_BOTH); // H_OUT2 (PA3)

    // 使能 EXTI2 和 EXTI3 的 NVIC 中断，并设置优先级 (这里优先级设置为 1,1，可根据需要调整)
    nvic_irq_enable(EXTI2_IRQn, 1, 1);
    nvic_irq_enable(EXTI3_IRQn, 1, 1);
}

void set_motor_speed(int16_t speed) {
    if (speed > 0) {
        // 正转：CH2 输出 PWM，CH3 输出 0
        timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH1, speed); // CH2
        timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH2, 0);     // CH3
    } else if (speed < 0) {
        // 反转：CH3 输出 PWM，CH2 输出 0
        timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH1, 0);     // CH2
        timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH2, -speed); // CH3 (注意速度取负号)
    } else {
        // 停止：CH2 和 CH3 都输出 0
        timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH1, 0);     // CH2
        timer_channel_output_pulse_value_config(MOTOR_TIMER, MOTOR_TIMER_CH2, 0);     // CH3
    }
}

float get_motor_speed(void) {
    static int32_t last_count = 0;
    int32_t current_count = encoder_count;
    int32_t delta_count = current_count - last_count;
    last_count = current_count;

    // 计算转速 (RPM - Revolutions Per Minute)
    // 假设每秒读取速度一次，实际应用中可能需要更精确的定时来计算时间间隔
    float speed_rpm = (float)delta_count * 60.0f / ENCODER_PPR; // RPM = (脉冲差值 * 60) / PPR
    return speed_rpm;
}
