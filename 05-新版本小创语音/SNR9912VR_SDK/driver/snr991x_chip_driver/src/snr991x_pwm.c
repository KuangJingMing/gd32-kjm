/**
 * @file snr991x_pwm.c
 * @brief  PWM驱动文件
 * @version 0.1
 * @date 2019-04-03
 *
 * @copyright Copyright (c) 2019    
 *
 */
#include "snr991x_pwm.h"
#include "snr991x_dpmu.h"
#include "user_config.h"
#include "sn_assert.h"
#include "sn_log.h"
#include "platform_config.h"

#define PWM_EXIT_CLK  (get_osc_clk()/8)

typedef struct
{
	/*配置寄存器*/
	volatile unsigned int pwm_cfg;		/*offect:0x00;功能：分频系数、计数模式、时钟源、中断极性、中断信号宽度*/
	/*配置寄存器1*/
	volatile unsigned int pwm_cfg1;		/*offect:0x04;功能：重新计数、暂停计数、清除中断、当前计数值位置*/
	/*事件寄存器*/
	volatile unsigned int pwm_ew;		/*offect:0x08;功能：事件计数*/
	/*周期寄存器*/
	volatile unsigned int pwm_sc;		/*offect:0x0c;功能：计数周期值*/
	/*计数值寄存器*/
	volatile unsigned int pwm_cc;		/*offect:0x10;功能：当前计数值*/
	/*pwm周期寄存器*/
	volatile unsigned int pwm_spwmc;	/*offect:0x14;功能：pwm周期值*/
	/*配置寄存器0*/
	volatile unsigned int pwm_cfg0;		/*offect:0x18;功能：计数器时钟、timer_reset_in信号使能、timer_ru信号使能*/
    /*毛刺处理寄存器*/
    volatile unsigned int pwm_restart_md; /*offect:0x1c;功能：配置RES后是立即生效还是等待正在输出的PWM波完成以后才生效*/
}pwm_register_t;

/**
 * @brief pwm初始化
 *
 * @param base 外设基地址
 * @param init 初始化结构体
 */
void pwm_init(pwm_base_t base,pwm_init_t init)
{
    /* 功能实现 */
    pwm_register_t* pwm_p = (pwm_register_t*)base;
    /*配置pwm*/
	unsigned int temp = 0;
    temp &= ~(0x1 << 0);/*分频系数 不分频*/
    temp |= (0x1 << 2);/*计数模式 auto*/
    if(init.clk_sel == 0)
    {
    	temp &= ~(0x1 << 5);/*时钟源（0 pclk，1 exit）*/
    }
    else
    {
    	temp |= (0x1 << 5);/*时钟源（0 pclk，1 exit）*/
    }
	pwm_p->pwm_cfg = temp;
    pwm_p->pwm_cfg1 |= (0x1 << 3);/*当前计数值（1当前位置计数值，0当前位置前的计数值）*/
    if(init.clk_sel == 0)
    {
    	pwm_p->pwm_sc = get_apb_clk() / (init.freq);/*计数周期*/
    }
    else
    {
    	pwm_p->pwm_sc = get_src_clk() / (init.freq);/*计数周期*/
    }
	pwm_p->pwm_spwmc = pwm_p->pwm_sc * (init.duty) / (init.duty_max);/*pwm周期*/
}

/**
 * @brief 启动pwm
 *
 * @param base 外设基地址
 */
void pwm_start(pwm_base_t base)
{
    /* 功能实现 */
    pwm_register_t* pwm_p = (pwm_register_t*)base;
    /*计数值复位*/
	pwm_p->pwm_cfg1 |= (0x1 << 0);/*重新计数（1重新计数，0无影响）*/
    /*开始正常计数*/
    pwm_p->pwm_cfg1 &= ~(0x1 << 1);/*暂停计数（1暂停计数，0正常计数）*/
}

/**
 * @brief 暂停pwm
 *
 * @param base 外设基地址
 */
void pwm_stop(pwm_base_t base)
{
    /* 功能实现 */
    pwm_register_t* pwm_p = (pwm_register_t*)base;
    /*暂停计数*/
	pwm_p->pwm_cfg1 |= (0x1 << 1);/*暂停计数（1暂停计数，0正常计数）*/
}

/**
 * @brief 设置pwm占空比
 *
 * @param base 外设基地址
 * @param duty 占空比
 * @param duty_max 最大占空比
 */
void pwm_set_duty(pwm_base_t base,unsigned int duty,unsigned int duty_max)
{
    /* 功能实现 */
    pwm_register_t* pwm_p = (pwm_register_t*)base;
    pwm_p->pwm_spwmc = pwm_p->pwm_sc * duty / duty_max;

    pwm_p->pwm_cfg1 |= (0x1 << 0);/*重新计数（1重新计数，0无影响）*/
}

/**
 * @brief 设置pwm restart_md 重新计数生效模式
 *
 * @param base 外设基地址
 * @param cmd 0，重新计数立即生效；1，等待正在进行的PWM波完整输出后重新计数才生效
 */
void pwm_set_restart_md(pwm_base_t base, uint8_t cmd)
{
    /* 功能实现 */
    pwm_register_t* pwm_p = (pwm_register_t*)base;

    if(cmd)
    {
        pwm_p->pwm_restart_md |= (0x1 << 0); /*重新计数（1：等待正在输出的PWM波完成以后才生效）*/
    }
    else
    {
        pwm_p->pwm_restart_md &= ~(0x1 << 0); /*重新计数（0：立即生效）*/
    }
}

void snr_pwm_init()
{
#if _PWM0_MODULE == Enable
scu_set_device_gate(HAL_PWM0_BASE,ENABLE);
dpmu_set_io_reuse(PA2,FIFTH_FUNCTION);                //设置引脚功能复用
dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
dpmu_set_io_pull(PA2,DPMU_IO_PULL_DOWN);          //设置关闭上下拉
pwm_init_t init;
init.clk_sel = 0;
init.freq = _PWM0_FREQ;
init.duty = _PWM0_DUTY;
init.duty_max = _PWM0_DUTY_MAX;
pwm_init(PWM0,init);
#endif
#if _PWM1_MODULE == Enable
scu_set_device_gate(HAL_PWM1_BASE,ENABLE);
dpmu_set_io_reuse(PA3,FIFTH_FUNCTION);                //设置引脚功能复用
dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
dpmu_set_io_pull(PA3,DPMU_IO_PULL_DOWN);          //设置关闭上下拉
pwm_init_t init1;
init1.clk_sel = 0;
init1.freq = _PWM1_FREQ;
init1.duty = _PWM1_DUTY;
init1.duty_max = _PWM1_DUTY_MAX;
pwm_init(PWM1,init1);
#endif
#if _PWM2_MODULE == Enable
scu_set_device_gate(HAL_PWM2_BASE,ENABLE);
dpmu_set_io_reuse(PA4,FIFTH_FUNCTION);                //设置引脚功能复用
dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
dpmu_set_io_pull(PA4,DPMU_IO_PULL_DOWN);          //设置关闭上下拉
pwm_init_t init2;
init2.clk_sel = 0;
init2.freq = _PWM2_FREQ;
init2.duty = _PWM2_DUTY;
init2.duty_max = _PWM2_DUTY_MAX;
pwm_init(PWM2,init2);
#endif
#if _PWM3_MODULE == Enable
scu_set_device_gate(HAL_PWM3_BASE,ENABLE);
dpmu_set_io_reuse(PA5,FIFTH_FUNCTION);                //设置引脚功能复用
dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输出模式
dpmu_set_io_pull(PA5,DPMU_IO_PULL_DOWN);          //设置关闭上下拉
pwm_init_t init3;
init3.clk_sel = 0;
init3.freq = _PWM3_FREQ;
init3.duty = _PWM3_DUTY;
init3.duty_max = _PWM3_DUTY_MAX;
pwm_init(PWM3,init3);
pwm_stop(PWM3);
#endif
}
