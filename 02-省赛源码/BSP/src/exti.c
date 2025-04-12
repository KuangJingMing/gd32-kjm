#include "exti.h"

void my_EXTI_Init(void)
{
    /* enable the key clock */
    rcu_periph_clock_enable(Ultrasonic_OUT_RTC);
    rcu_periph_clock_enable(RCU_SYSCFG);

    /* configure key pin as input */
    gpio_mode_set(Ultrasonic_OUT_GPIO_Port, GPIO_MODE_INPUT, GPIO_PUPD_NONE, Ultrasonic_OUT_Pin);

    /* enable and set key EXTI interrupt to the lowest priority */
    nvic_irq_enable(EXTI_x_IRQn, 2U, 0U);

    /* connect key EXTI line to key GPIO pin */
    syscfg_exti_line_config(EXTI_SOURCE_GPIOx, EXTI_SOURCE_PINx);

    /* configure key EXTI line */
    exti_init(EXTI_x, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_x);
}

void EXTI_IRQHandler(void)
{
    if(RESET != exti_interrupt_flag_get(EXTI_x)){
       
			timer_disable(TIMER1);									//关使能定时器1
			real_time = status ;
			dis_temp=(float)real_time*1.7f-2;      	//计算距离	定时10us，S=Vt/2（减2是误差补尝）单位：mm
    }
    exti_interrupt_flag_clear(EXTI_x);
}
