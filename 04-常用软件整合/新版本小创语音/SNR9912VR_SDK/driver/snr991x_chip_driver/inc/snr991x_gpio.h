/**
 * @file snr991x_gpio.h
 * @brief  GPIO驱动文件
 * @version 0.1
 * @date 2019-05-07
 *
 * @copyright Copyright (c) 2019    
 *
 */

#ifndef _SNR991X_GPIO_H
#define _SNR991X_GPIO_H

#include "snr991x_system.h"
#include "snr991x_scu.h"

/**
 * @ingroup snr991x_chip_driver
 * @defgroup snr991x_gpio 
 * @brief SNR991X芯片GPIO驱动
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif



/* GPIO口模式  */
#define GPIO_INPUT_PULLHIGH         0
#define GPIO_INPUT_PULLDOWN         1
#define GPIO_INPUT_FLOAT            2
#define GPIO_OUTPUT_HIGH			3
#define GPIO_OUTPUT_LOW		        4
#define GPIO_PIN_OFF                0xFF

/* GPIO口序号  */
#define	GPIOA_2 0
#define	GPIOA_3 1
#define	GPIOA_4 2
#define	GPIOA_5 3
#define	GPIOA_6 4
#define	GPIOB_5 5
#define	GPIOB_6 6
#define	GPIOC_4 7

#define KEY_SHORT_PRESS 1
#define KEY_SHORT_PRESS_RELEASE 2
#define KEY_LONG_PRESS 3
#define KEY_LONG_PRESS_RELEASE 4

/**
 * @brief GPIO控制器定义
 */
typedef enum
{
    PA = HAL_PA_BASE,/*!< PA控制器 */
    PB = HAL_PB_BASE,/*!< PB控制器 */
    PC = HAL_PC_BASE,/*!< PC控制器 */
    PD = HAL_PD_BASE,/*!< PD控制器 */
}gpio_base_t;

/**
 * @brief GPIO pin脚定义
 */
typedef enum
{
    pin_0 = (0x1 << 0),/*!< Pin0脚 */
    pin_1 = (0x1 << 1),/*!< Pin1脚 */
    pin_2 = (0x1 << 2),/*!< Pin2脚 */
    pin_3 = (0x1 << 3),/*!< Pin3脚 */
    pin_4 = (0x1 << 4),/*!< Pin4脚 */
    pin_5 = (0x1 << 5),/*!< Pin5脚 */
    pin_6 = (0x1 << 6),/*!< Pin6脚 */
    pin_7 = (0x1 << 7),/*!< Pin7脚 */
    pin_all = 0xFF,/*!< 所有Pin脚 */
}gpio_pin_t;

/**
 * @brief GPIO中断触发模式定义
 */
typedef enum
{
    high_level_trigger       = 1,/*!< 高电平触发中断 */
    low_level_trigger        = 2,/*!< 低电平触发中断 */
    up_edges_trigger         = 3,/*!< 上升沿触发中断 */
    down_edges_trigger       = 4,/*!< 下降沿触发中断 */
    both_edges_trigger       = 5,/*!< 双边沿触发中断 */
}gpio_trigger_t;

/**
 * @brief GPIO信息
 */
typedef struct
{
    gpio_base_t base;/*!< GPIO基地址 */
    gpio_pin_t pin;/*!< GPIO pin */
}gpio_info_t;

typedef void(*gpio_irq_callback_t)(void);

typedef struct gpio_irq_callback_list_s
{
    gpio_irq_callback_t gpio_irq_callback;
    struct gpio_irq_callback_list_s * next;
}gpio_irq_callback_list_t;


/*-------------------以下API可同时操作一个或多个pin脚-------------------------*/
void gpio_set_output_mode(gpio_base_t gpio,gpio_pin_t pins);
void gpio_set_input_mode(gpio_base_t gpio,gpio_pin_t pins);
uint8_t gpio_get_direction_status(gpio_base_t gpio,gpio_pin_t pins);
void gpio_irq_mask(gpio_base_t gpio,gpio_pin_t pins);
void gpio_irq_unmask(gpio_base_t gpio,gpio_pin_t pins);
void gpio_irq_trigger_config(gpio_base_t gpio,gpio_pin_t pins,gpio_trigger_t trigger);
void gpio_set_output_high_level(gpio_base_t gpio,gpio_pin_t pins);
void gpio_set_output_low_level(gpio_base_t gpio,gpio_pin_t pins);
uint8_t gpio_get_input_level(gpio_base_t gpio,gpio_pin_t pins);

/*----------------------以下API一次只操作一个pin脚----------------------------*/
uint8_t gpio_get_direction_status_single(gpio_base_t gpio,gpio_pin_t pins);
uint8_t gpio_get_irq_raw_status_single(gpio_base_t gpio,gpio_pin_t pins);
uint8_t gpio_get_irq_mask_status_single(gpio_base_t gpio,gpio_pin_t pins);
void gpio_clear_irq_single(gpio_base_t gpio,gpio_pin_t pins);
void gpio_set_output_level_single(gpio_base_t gpio,gpio_pin_t pins,uint8_t level);
uint8_t gpio_get_input_level_single(gpio_base_t gpio,gpio_pin_t pins);

void snr_gpio_init(void);
void snr_key_init(void);
void gpio_output(uint8_t io_Num,uint8_t level);
uint8_t gpio_get(uint8_t io_Num);
void snr_key_loop(void);

/*------------------------以下API为中断处理函数-------------------------------*/
void registe_gpio_callback(gpio_base_t base, gpio_irq_callback_list_t *gpio_irq_callback_node);
void PA_IRQHandler(void);
void PB_IRQHandler(void);
void AON_PC_IRQHandler(void);

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif
