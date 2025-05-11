/**
 * @file snr991x_timer.c
 * @brief  Timer驱动文件
 * @version 0.1
 * @date 2019-04-03
 *
 * @copyright Copyright (c) 2019    
 *
 */
#include "snr991x_core_eclic.h"
#include "snr991x_timer.h"
#include "sn_assert.h"
#include "snr991x_gpio.h"
#include "user_config.h"
#include "snr991x_dpmu.h"
#include "sn_log.h"

typedef struct
{
	/*配置寄存器*/
	volatile unsigned int timer_cfg;	/*offect:0x00;功能：分频系数、计数模式、时钟源、中断极性、中断信号宽度*/
	/*配置寄存器1*/
	volatile unsigned int timer_cfg1;	/*offect:0x04;功能：重新计数、暂停计数、清除中断、当前计数值位置*/
	/*事件寄存器*/
	volatile unsigned int timer_ew;		/*offect:0x08;功能：事件计数*/
	/*周期寄存器*/
	volatile unsigned int timer_sc;		/*offect:0x0c;功能：计数周期值*/
	/*计数值寄存器*/
	volatile unsigned int timer_cc;		/*offect:0x10;功能：当前计数值*/
	/*pwm周期寄存器*/
	volatile unsigned int timer_spwmc;	/*offect:0x14;功能：pwm周期值*/
	/*配置寄存器0*/
	volatile unsigned int timer_cfg0;	/*offect:0x18;功能：计数器时钟、timer_reset_in信号使能、timer_ru信号使能*/
}timer_register_t;

/**
 * @brief 定时器初始化
 *
 * @param base 外设基地址
 * @param init 初始化结构体
 */
void timer_init(timer_base_t base,timer_init_t init)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
    if(init.width != timer_iqr_width_f)
    {
        switch(base)
        {
            case TIMER0:
            {
                eclic_set_posedge_trig(TIMER0_IRQn);
                break;
            }
            case TIMER1:
            {
                eclic_set_posedge_trig(TIMER1_IRQn);
                break;
            }
            case TIMER2:
            {
                eclic_set_posedge_trig(TIMER2_IRQn);
                break;
            }
            case TIMER3:
            {
                eclic_set_posedge_trig(TIMER3_IRQn);
                break;
            }
            case AON_TIMER0:
            {
                eclic_set_posedge_trig(AON_TIM_INT0_IRQn);
                break;
            }
            case AON_TIMER1:
            {
                eclic_set_posedge_trig(AON_TIM_INT1_IRQn);
                break;
            }
        }
    }
    else
    {
        switch(base)
        {
            case TIMER0:
            {
            	eclic_set_level_trig(TIMER0_IRQn);
                break;
            }
            case TIMER1:
            {
            	eclic_set_level_trig(TIMER1_IRQn);
                break;
            }
            case TIMER2:
            {
            	eclic_set_level_trig(TIMER2_IRQn);
                break;
            }
            case TIMER3:
            {
            	eclic_set_level_trig(TIMER3_IRQn);
                break;
            }
            case AON_TIMER0:
            {
            	eclic_set_level_trig(AON_TIM_INT0_IRQn);
                break;
            }
            case AON_TIMER1:
            {
            	eclic_set_level_trig(AON_TIM_INT1_IRQn);
                break;
            }
        }
    }
    /*配置定时器*/
	unsigned int temp = 0;
    temp |= init.div;
    temp |= (init.mode << 2);
    temp &= ~(0x1 << 5);/*时钟源（0 pclk，1 exit）*/
    temp &= ~(0x1 << 6);/*中断极性(0高有效，1低有效)*/
    temp &= ~(0x3 << 7);
    temp |= (init.width << 7);/*中断信号宽度*/
	timer_p->timer_cfg = temp;
	temp = 0;
    temp |= (0x1 << 2);/*清除中断（1清除中断，0无影响）*/
    temp |= (0x1 << 3);/*当前计数值（1当前位置计数值，0当前位置前的计数值）*/
	timer_p->timer_cfg1 = temp;
    timer_p->timer_ew = 0x1;/*事件计数(0 无影响，1 计数器减一)*/
    timer_p->timer_sc = init.count;/*计数周期值*/
}

/**
 * @brief 设置计数模式
 *
 * @param base TIMER0/TIMER1/TIMER2
 * @param mode timer_mode_single/timer_mode_auto/timer_mode_free/timer_mode_event
 */
void timer_set_mode(timer_base_t base,timer_count_mode_t mode)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
    /*选择定时器模式*/
    timer_p->timer_cfg &= ~(7 << 2);
    timer_p->timer_cfg |= (mode << 2);
}

/**
 * @brief 启动定时器
 *
 * @param base TIMER0/TIMER1/TIMER2
 */
void timer_start(timer_base_t base)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
    /*计数值复位*/
	timer_p->timer_cfg1 |= (0x1 << 0);/*重新计数控制（1重新计数，0无影响）*/
    /*开始正常计数*/
    timer_p->timer_cfg1 &= ~(0x1 << 1);/*暂停计数控制（1暂停计数，0正常计数）*/
    /*清除中断*/
    timer_p->timer_cfg1 |= 1 << 2;
}

/**
 * @brief 暂停定时器
 *
 * @param base TIMER0/TIMER1/TIMER2
 */
void timer_stop(timer_base_t base)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
    timer_p->timer_cfg1 |= (0x1 << 1);/*暂停计数（1暂停计数，0正常计数）*/
    /*清除中断*/
    timer_p->timer_cfg1 |= 1 << 2;
}

/**
 * @brief 事件计数(必须处于事件计数模式下)
 *
 * @param base TIMER0/TIMER1/TIMER2
 */
void timer_event_start(timer_base_t base)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
    timer_p->timer_ew = 0x1;/*事件计数(0 无影响，1 计数器减一),设置事件标志。*/
}

/**
 * @brief 设置计时周期
 *
 * @param base TIMER0/TIMER1/TIMER2
 * @param count 计数周期
 */
void timer_set_count(timer_base_t base,unsigned int count)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
	timer_p->timer_sc = count;/*设置计数周期*/
}

/**
 * @brief 获取当前计数值
 *
 * @param base TIMER0/TIMER1/TIMER2
 * @param count 当前计数值
 */
void timer_get_count(timer_base_t base,unsigned int* count)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
	*count = timer_p->timer_cc;/*获取当前计数值*/
}

/**
 * @brief 设置级联时钟计数模式
 *
 * @param base TIMER0/TIMER1/TIMER2
 * @param count 计数值
 */
void timer_cascade_set(timer_base_t base,unsigned int count)
{
    /* 功能实现 */
    timer_register_t* timer_p = (timer_register_t*)base;
	timer_p->timer_cfg0 |= (0x1 << 0);/*获取当前计数值*/
    timer_p->timer_sc = count;/*设置计数周期*/
}


/**
 * @brief 清除中断
 *
 * @param base TIMER0/TIMER1/TIMER2
 */
void timer_clear_irq(timer_base_t base)
{
    timer_register_t* timer_p = (timer_register_t*)base;
	/*清除中断*/
    timer_p->timer_cfg1 |= 1 << 2;
}

void snr_timer_init()
{
#if _TIMER_TMR0_INT == Enable
    eclic_irq_enable(TIMER0_IRQn);
    scu_set_device_gate(HAL_TIMER0_BASE,ENABLE);
    timer_init_t init;
    init.mode = timer_count_mode_auto;
    init.div = timer_clk_div_0;
    init.width = timer_iqr_width_f;
    init.count = (get_apb_clk() / 1000000)*(_TIMER_TMR0_DUR_US+_TIMER_TMR0_DUR_MS*1000+_TIMER_TMR0_DUR_S*1000000);
    timer_init(TIMER0,init);
    timer_start(TIMER0);
#endif
#if _TIMER_TMR1_INT == Enable
    eclic_irq_enable(TIMER1_IRQn);
    scu_set_device_gate(HAL_TIMER1_BASE,ENABLE);
    timer_init_t init1;
    init1.mode = timer_count_mode_auto;
    init1.div = timer_clk_div_0;
    init1.width = timer_iqr_width_f;
    init1.count = (get_apb_clk() / 1000000)*(_TIMER_TMR1_DUR_US+_TIMER_TMR1_DUR_MS*1000+_TIMER_TMR1_DUR_S*1000000);
    timer_init(TIMER1,init1);
    timer_start(TIMER1);
#endif
#if _TIMER_TMR2_INT == Enable
    eclic_irq_enable(TIMER2_IRQn);
    scu_set_device_gate(HAL_TIMER2_BASE,ENABLE);
    timer_init_t init2;
    init2.mode = timer_count_mode_auto;
    init2.div = timer_clk_div_0;
    init2.width = timer_iqr_width_f;
    init2.count = (get_apb_clk() / 1000000)*(_TIMER_TMR2_DUR_US+_TIMER_TMR2_DUR_MS*1000+_TIMER_TMR2_DUR_S*1000000);
    timer_init(TIMER2,init2);
    timer_start(TIMER2);
#endif
    eclic_irq_enable(TIMER3_IRQn);
    scu_set_device_gate(HAL_TIMER3_BASE,ENABLE);
    timer_init_t init3;
    init3.mode = timer_count_mode_auto;
    init3.div = timer_clk_div_0;
    init3.width = timer_iqr_width_f;
    init3.count = (get_apb_clk() / 1000) * 1;
    timer_init(TIMER3,init3);
    timer_start(TIMER3);
}

#if _PULSE0_MODULE == Enable
uint8_t pulse0_step=0;
uint16_t pulse0_timer=0;
uint16_t pulse0_counter=0;
#endif
#if _PULSE1_MODULE == Enable
uint8_t pulse1_step=0;
uint16_t pulse1_timer=0;
uint16_t pulse1_counter=0;
#endif
#if _PULSE2_MODULE == Enable
uint8_t pulse2_step=0;
uint16_t pulse2_timer=0;
uint16_t pulse2_counter=0;
#endif
#if _PULSE3_MODULE == Enable
uint8_t pulse3_step=0;
uint16_t pulse3_timer=0;
uint16_t pulse3_counter=0;
#endif

void snr_pulse_init()
{
#if _PULSE0_MODULE == Enable
#if _PULSE0_ON_GPIO == GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PA
	#define PULSE0_NUM  pin_2
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#if _PULSE0_ON_GPIO == GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PA
	#define PULSE0_NUM  pin_3
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#if _PULSE0_ON_GPIO == GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PA
	#define PULSE0_NUM  pin_4
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#if _PULSE0_ON_GPIO == GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PA
	#define PULSE0_NUM  pin_5
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#if _PULSE0_ON_GPIO == GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PA
	#define PULSE0_NUM  pin_6
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#if _PULSE0_ON_GPIO == GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PB
	#define PULSE0_NUM  pin_5
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#if _PULSE0_ON_GPIO == GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE0_PORT PB
	#define PULSE0_NUM  pin_6
    gpio_set_output_mode(PULSE0_PORT,PULSE0_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE0_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,0);
    else if(_PULSE0_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,1);
#endif
#endif

#if _PULSE1_MODULE == Enable
#if _PULSE1_ON_GPIO == GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PA
	#define PULSE1_NUM  pin_2
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#if _PULSE1_ON_GPIO == GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PA
	#define PULSE1_NUM  pin_3
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#if _PULSE1_ON_GPIO == GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PA
	#define PULSE1_NUM  pin_4
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#if _PULSE1_ON_GPIO == GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PA
	#define PULSE1_NUM  pin_5
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#if _PULSE1_ON_GPIO == GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PA
	#define PULSE1_NUM  pin_6
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#if _PULSE1_ON_GPIO == GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PB
	#define PULSE1_NUM  pin_5
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#if _PULSE1_ON_GPIO == GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE1_PORT PB
	#define PULSE1_NUM  pin_6
    gpio_set_output_mode(PULSE1_PORT,PULSE1_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE1_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,0);
    else if(_PULSE1_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,1);
#endif
#endif

#if _PULSE2_MODULE == Enable
#if _PULSE2_ON_GPIO == GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PA
	#define PULSE2_NUM  pin_2
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#if _PULSE2_ON_GPIO == GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PA
	#define PULSE2_NUM  pin_3
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#if _PULSE2_ON_GPIO == GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PA
	#define PULSE2_NUM  pin_4
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#if _PULSE2_ON_GPIO == GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PA
	#define PULSE2_NUM  pin_5
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#if _PULSE2_ON_GPIO == GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启PA时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PA
	#define PULSE2_NUM  pin_6
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#if _PULSE2_ON_GPIO == GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PB
	#define PULSE2_NUM  pin_5
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#if _PULSE2_ON_GPIO == GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启PB时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DISABLE);          //设置关闭上下拉
    #define PULSE2_PORT PB
	#define PULSE2_NUM  pin_6
    gpio_set_output_mode(PULSE2_PORT,PULSE2_NUM);                      //GPIO的pin脚配置成输出模式
    if(_PULSE2_DEFAULT_LEVEL == 0)      gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,0);
    else if(_PULSE2_DEFAULT_LEVEL == 1) gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,1);
#endif
#endif
}

void pulse_start(uint8_t pulse_num)
{
    switch (pulse_num)
    {
    case 0:
#if _PULSE0_MODULE == Enable
    	pulse0_step=1;
#endif
    	break;
    case 1:
#if _PULSE1_MODULE == Enable
    	pulse1_step=1;
#endif
    	break;
    case 2:
#if _PULSE2_MODULE == Enable
    	pulse2_step=1;
#endif
    	break;
    case 3:
    	break;
    default:
        break;
    }
}

void snr_pulse_loop()
{
#if _PULSE0_MODULE == Enable
    switch (pulse0_step)
    {
    case 0:

        break;
    case 1:
        gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,!_PULSE0_DEFAULT_LEVEL);
        pulse0_step++;
        pulse0_counter=0;
        break;
    case 2:
        pulse0_counter++;
        if(pulse0_counter >= _PULSE0_WIDE)
        {
            pulse0_counter=0;
            gpio_set_output_level_single(PULSE0_PORT,PULSE0_NUM,_PULSE0_DEFAULT_LEVEL);
            pulse0_timer++;
            if(pulse0_timer >= _PULSE0_TIME)    //判定输出次数是否足够
            {
				pulse0_timer=0;
                pulse0_step=0;
            }else{
                pulse0_step=3;
            }
        }
        break;
    case 3:
        pulse0_counter++;
        if(pulse0_counter >= _PULSE0_WIDE)
        {
            pulse0_counter=0;
			pulse0_step=1;
        }
        break;
    default:
        break;
    }
#endif
#if _PULSE1_MODULE == Enable
    switch (pulse1_step)
    {
    case 0:

        break;
    case 1:
        gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,!_PULSE1_DEFAULT_LEVEL);
        pulse1_step++;
        pulse1_counter=0;
        break;
    case 2:
        pulse1_counter++;
        if(pulse1_counter >= _PULSE1_WIDE)
        {
            pulse1_counter=0;
            gpio_set_output_level_single(PULSE1_PORT,PULSE1_NUM,_PULSE1_DEFAULT_LEVEL);
            pulse1_timer++;
            if(pulse1_timer >= _PULSE1_TIME)    //判定输出次数是否足够
            {
				pulse1_timer=0;
                pulse1_step=0;
            }else{
                pulse1_step=3;
            }
        }
        break;
    case 3:
        pulse1_counter++;
        if(pulse1_counter >= _PULSE1_WIDE)
        {
            pulse1_counter=0;
			pulse1_step=1;
        }
        break;
    default:
        break;
    }
#endif
#if _PULSE2_MODULE == Enable
    switch (pulse2_step)
    {
    case 0:

        break;
    case 1:
        gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,!_PULSE2_DEFAULT_LEVEL);
        pulse2_step++;
        pulse2_counter=0;
        break;
    case 2:
        pulse2_counter++;
        if(pulse2_counter >= _PULSE2_WIDE)
        {
            pulse2_counter=0;
            gpio_set_output_level_single(PULSE2_PORT,PULSE2_NUM,_PULSE2_DEFAULT_LEVEL);
            pulse2_timer++;
            if(pulse2_timer >= _PULSE2_TIME)    //判定输出次数是否足够
            {
				pulse2_timer=0;
                pulse2_step=0;
            }else{
                pulse2_step=3;
            }
        }
        break;
    case 3:
        pulse2_counter++;
        if(pulse2_counter >= _PULSE2_WIDE)
        {
            pulse2_counter=0;
			pulse2_step=1;
        }
        break;
    default:
        break;
    }
#endif
}

