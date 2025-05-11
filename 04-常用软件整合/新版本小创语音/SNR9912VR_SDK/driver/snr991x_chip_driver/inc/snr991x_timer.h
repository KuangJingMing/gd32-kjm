/**
 * @file snr991x_timer.h
 * @brief  Timer驱动文件
 * @version 0.1
 * @date 2019-04-03
 *
 * @copyright Copyright (c) 2019    
 *
 */

#ifndef _SNR991X_TIMER_H_
#define _SNR991X_TIMER_H_

#include "snr991x_system.h"

/**
 * @ingroup snr991x_chip_driver
 * @defgroup snr991x_timer 
 * @brief SNR991X芯片TIMER驱动
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif

#define pulse0 0
#define pulse1 1
#define pulse2 2
#define pulse3 3
#define pulse4 4
#define pulse5 5

/**
 * @brief TIMER控制器定义
 */
typedef enum
{
	TIMER0     = HAL_TIMER0_BASE,/*!< TIMER0控制器 */
	TIMER1     = HAL_TIMER1_BASE,/*!< TIMER1控制器 */
	TIMER2     = HAL_TIMER2_BASE,/*!< TIMER2控制器 */
	TIMER3     = HAL_TIMER3_BASE,/*!< TIMER3控制器 */
	AON_TIMER0 = HAL_PWM4_BASE,  /*!< AON TIMER0控制器 */
	AON_TIMER1 = HAL_PWM5_BASE,  /*!< AON TIMER1控制器 */
}timer_base_t;

/**
 * @brief 计数模式定义
 */
typedef enum
{
	timer_count_mode_single = 0,/*!< 单周期模式 */
	timer_count_mode_auto = 1,	/*!< 自动重新计数模式 */
	timer_count_mode_free = 2,	/*!< 自由计数模式 */
	timer_count_mode_event = 3,	/*!< 事件计数模式 */
}timer_count_mode_t;

/**
 * @brief 分频系数定义
 */
typedef enum
{
	timer_clk_div_0 = 0,	/*!< 不分频 */
	timer_clk_div_2 = 1,	/*!< 2分频 */
	timer_clk_div_4 = 2,	/*!< 4分频 */
	timer_clk_div_16 = 3,	/*!< 16分频 */
}timer_clock_div_t;

/**
 * @brief 中断信号宽度定义
 */
typedef enum
{
	timer_iqr_width_f = 0,/*!< 由 TIMER_CFG1[CT]清除 */
	timer_iqr_width_2 = 1,/*!< 2 个时钟周期 */
	timer_iqr_width_4 = 2,/*!< 4 个时钟周期 */
	timer_iqr_width_8 = 3,/*!< 8 个时钟周期 */
}timer_iqr_width_t;

/**
 * @brief timer配置结构体定义
 */
typedef struct
{
	timer_count_mode_t mode;/*!< 计数模式 */
	timer_clock_div_t div;/*!< 时钟分频系数 */
	timer_iqr_width_t width;/*!< 中断信号宽度 */
	unsigned int count;/*!< 计数值 */
}timer_init_t;

//函数接口声明
void timer_init(timer_base_t base,timer_init_t init);
void timer_set_mode(timer_base_t base,timer_count_mode_t mode);
void timer_start(timer_base_t base);
void timer_stop(timer_base_t base);
void timer_event_start(timer_base_t base);
void timer_set_count(timer_base_t base,unsigned int count);
void timer_get_count(timer_base_t base,unsigned int* count);
void timer_cascade_set(timer_base_t base,unsigned int count);
void timer_clear_irq(timer_base_t base);

void snr_timer_init(void);
void pulse_start(uint8_t pulse_num);
void snr_pulse_loop(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
