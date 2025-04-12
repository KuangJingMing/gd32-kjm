/**
 * @file snr991x_gpio.c
 * @brief  GPIO驱动文件
 * @version 0.1
 * @date 2019-05-07
 *
 * @copyright Copyright (c) 2019    
 *
 */
#include <stdlib.h>
#include "snr991x_gpio.h"
#include "snr991x_dpmu.h"
#include "user_config.h"
#include "sn_log.h"
#include "system_hook.h"

static gpio_irq_callback_list_t g_gpio_callback_list_root[6] =
{
    {NULL,NULL},{NULL,NULL},{NULL,NULL},{NULL,NULL},{NULL,NULL},{NULL,NULL}
};


/**
 * @brief GPIO控制器寄存器定义
 */
typedef struct
{
	volatile uint32_t gpio_data[256];    /*!< 0x000-0x3FC 数据寄存器 */
	volatile uint32_t gpio_dir;          /*!< 0x400-0x404 输入输出控制寄存器 */
	volatile uint32_t gpio_is;           /*!< 0x404-0x408 中断源（边沿/电平）寄存器 */
	volatile uint32_t gpio_ibe;          /*!< 0x408-0x40C 中断源双边沿触发寄存器 */
	volatile uint32_t gpio_iev;          /*!< 0x40C-0x410 中断事件寄存器 */
	volatile uint32_t gpio_ie;           /*!< 0x410-0x414 中断屏蔽寄存器 */
	volatile  uint32_t gpio_ris;          /*!< 0x414-0x418 中断原始状态寄存器 */
	volatile  uint32_t gpio_mis;          /*!< 0x418-0x41C 中断屏蔽状态寄存器 */
	volatile uint32_t gpio_ic;           /*!< 0x41C-0x420 中断清除寄存器 */
	volatile uint32_t gpio_afsel;        /*!< 0x420-0x424 模式控制寄存器 */
}gpio_register_t;

/*--------------------以下API可同时操作一个或多个pin脚------------------------*/

/**
 * @brief 管脚配置为输出模式
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 */
void gpio_set_output_mode(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_dir |= pins;
}

/**
 * @brief 管脚配置为输入模式
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 */
void gpio_set_input_mode(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_dir &= ~pins;
}

/**
 * @brief 获取管脚输入输出状态
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 *
 * @return uint8_t :pin(0-7)对应status的bit(0-7) 输出对应bit为1 输入对应bit为0
 */
uint8_t gpio_get_direction_status(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    uint8_t status;
    status = gpio->gpio_dir & pins;
    return status;
}

/**
 * @brief 屏蔽管脚中断
 *
 * @param base :PA PB PC 
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 */
void gpio_irq_mask(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_ie &= ~pins;
}

/**
 * @brief 取消屏蔽管脚中断
 *
 * @param base :PA PB PC
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 */
void gpio_irq_unmask(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_ie |= pins;
}

/**
 * @brief 设置管脚的中断触发方式
 *
 * @param base :PA PB PC
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 * @param trigger :high_level_trigger、low_level_trigger、up_edges_trigger、
                    down_edges_trigger、both_edges_trigger
 */
void gpio_irq_trigger_config(gpio_base_t base,gpio_pin_t pins,
                            gpio_trigger_t trigger)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    switch(trigger)
    {
        case high_level_trigger:
        {
            gpio->gpio_iev |= pins;
            gpio->gpio_ibe &= ~(pins);
            gpio->gpio_is |= pins;
            break;
        }
        case low_level_trigger:
        {
            gpio->gpio_iev &= ~(pins);
            gpio->gpio_ibe &= ~(pins);
            gpio->gpio_is |= pins;
            break;
        }
        case up_edges_trigger:
        {
            gpio->gpio_iev |= pins;
            gpio->gpio_ibe &= ~(pins);
            gpio->gpio_is &= ~(pins);
            break;
        }
        case down_edges_trigger:
        {
            gpio->gpio_iev &= ~(pins);
            gpio->gpio_ibe &= ~(pins);
            gpio->gpio_is &= ~(pins);
            break;
        }
        case both_edges_trigger:
        {
            gpio->gpio_ibe |= pins;
            gpio->gpio_is &= ~(pins);
            break;
        }
    }
    gpio_irq_unmask(base,pins);
}

/**
 * @brief 管脚输出高电平
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 */
void gpio_set_output_high_level(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_data[pins] = 0xFF;
}

/**
 * @brief 管脚输出低电平
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 */
void gpio_set_output_low_level(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_data[pins] = 0x0;
}

/**
 * @brief 获取管脚输入电平
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7,pin_all;中的一个或多个
 *
 * @return uint8_t :pin(0-7)对应value的bit(0-7) 高电平对应bit为1 低电平对应bit为0
 */
uint8_t gpio_get_input_level(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    uint8_t value;
    value = gpio->gpio_data[pins];
    return value;
}

/*----------------------以下API一次只操作一个pin脚----------------------------*/

/**
 * @brief 获取某一个管脚输入输出状态
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7;中的一个
 *
 * @return uint8_t :输出:1 输入:0
 */
uint8_t gpio_get_direction_status_single(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    uint8_t status;
    status = gpio->gpio_dir & pins;
    return (status?1:0);
}

/**
 * @brief 获取某个管脚中断屏蔽前状态
 *
 * @param base :PA PB PC
 * @param pins :pin_0,pin_1...pin_7;中的一个
 *
 * @return uint8_t :有中断为1 无中断为0
 */
uint8_t gpio_get_irq_raw_status_single(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    uint8_t status;
    status = gpio->gpio_ris & pins;
    return (status?1:0);
}

/**
 * @brief 获取某个管脚中断屏蔽后状态
 *
 * @param base :PA PB PC
 * @param pins :pin_0,pin_1...pin_7;中的一个
 *
 * @return uint8_t :没有屏蔽为1 屏蔽为0
 */
uint8_t gpio_get_irq_mask_status_single(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    uint8_t status;
    status = gpio->gpio_mis & pins;
    return (status?1:0);
}

/**
 * @brief 清除某个管脚的中断
 *
 * @param base :PA PB PC
 * @param pins :pin_0,pin_1...pin_7;中的一个
 */
void gpio_clear_irq_single(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_ic |= pins;
}

/**
 * @brief 某个管脚输出电平
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7;中的一个
 * @param level :1:输出高电平   0:输出低电平
 */
void gpio_set_output_level_single(gpio_base_t base,gpio_pin_t pins,uint8_t level)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    gpio->gpio_data[pins] = (level?0xFF:0x0);
}

/**
 * @brief 获取某个管脚输入电平
 *
 * @param base :PA PB PC PD
 * @param pins :pin_0,pin_1...pin_7;中的一个
 *
 * @return uint8_t :高电平:1 低电平:0
 */
uint8_t gpio_get_input_level_single(gpio_base_t base,gpio_pin_t pins)
{
    gpio_register_t* gpio = (gpio_register_t*)base;
    uint8_t value;
    value = gpio->gpio_data[pins] & pins;
    return (value?1:0);
}


/**
 * @brief gpio中断回调函数链表注册节点
 *
 * @param gpio_irq_callback_list 链表根节点
 * @param gpio_irq_callback_node 注册节点
 */
static void gpio_callback_add_node_in_list(gpio_irq_callback_list_t* gpio_irq_callback_list, gpio_irq_callback_list_t* gpio_irq_callback_node)
{
    gpio_irq_callback_list_t *list  = gpio_irq_callback_list;
    if(list->gpio_irq_callback == NULL)
    {
        *list = *gpio_irq_callback_node;
        return;
    }
    for( ; list->next != NULL ;list = list->next);
    list->next = gpio_irq_callback_node;
}


/**
 * @brief 搜索链表调用中断回调函数
 *
 * @param gpio_irq_callback_list 链表根节点
 */
static void call_irq_callback(gpio_irq_callback_list_t* gpio_irq_callback_list)
{
    gpio_irq_callback_list_t *list  = gpio_irq_callback_list;

    do
    {
        if(list->gpio_irq_callback != NULL)
        {
            list->gpio_irq_callback();
        }
        list = list->next;
    }while(list != NULL);
}


/**
 * @brief 注册中断回掉函数
 *
 * @param base :PA,PB,PC
 * @param gpio_irq_callback_node 中断回调函数节点
 */
void registe_gpio_callback(gpio_base_t base, gpio_irq_callback_list_t *gpio_irq_callback_node)
{
    switch (base)
    {
        case PC:
        {
            gpio_callback_add_node_in_list(&g_gpio_callback_list_root[2], gpio_irq_callback_node);
            break;
        }
        case PB:
        {
            gpio_callback_add_node_in_list(&g_gpio_callback_list_root[1], gpio_irq_callback_node);
            break;
        }
        case PA:
        {
            gpio_callback_add_node_in_list(&g_gpio_callback_list_root[0], gpio_irq_callback_node);
            break;
        }
    }
}

void snr_gpio_init()
{
#if _GPIOA_2_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOA_2_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOA_2_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOA_2_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_2);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_2,1);            //输出高电平
#elif _GPIOA_2_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_2);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_2,0);            //输出高电平
#endif

#if _GPIOA_3_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOA_3_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOA_3_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOA_3_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_3);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_3,1);            //输出高电平
#elif _GPIOA_3_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_3);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_3,0);            //输出高电平
#endif

#if _GPIOA_4_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOA_4_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOA_4_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOA_4_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_4);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_4,1);            //输出高电平
#elif _GPIOA_4_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_4);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_4,0);            //输出高电平
#endif

#if _GPIOA_5_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOA_5_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOA_5_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOA_5_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_5);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_5,1);            //输出高电平
#elif _GPIOA_5_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_5);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_5,0);            //输出高电平
#endif

#if _GPIOA_6_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOA_6_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOA_6_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOA_6_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_6);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_6,1);            //输出高电平
#elif _GPIOA_6_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PA,pin_6);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_6,0);            //输出高电平
#endif

#if _GPIOB_5_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOB_5_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOB_5_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOB_5_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PB,pin_5);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PB,pin_5,1);            //输出高电平
#elif _GPIOB_5_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PB,pin_5);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PB,pin_5,0);            //输出高电平
#endif

#if _GPIOB_6_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOB_6_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOB_6_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOB_6_MODE == GPIO_OUTPUT_HIGH
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PB,pin_6);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PA,pin_6,1);            //输出高电平
#elif _GPIOB_6_MODE == GPIO_OUTPUT_LOW
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_OUTPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
    gpio_set_output_mode(PB,pin_6);                      //GPIO的pin脚配置成输出模式
    gpio_set_output_level_single(PB,pin_6,0);            //输出高电平
#endif

#if _GPIOC_4_MODE == GPIO_INPUT_PULLHIGH
    scu_set_device_gate((unsigned int)PC,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PC4,SECOND_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PC4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PC4,DPMU_IO_PULL_UP);                 //设置关闭上下拉
#elif _GPIOC_4_MODE == GPIO_INPUT_PULLDOWN
    scu_set_device_gate((unsigned int)PC,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PC4,SECOND_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PC4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PC4,DPMU_IO_PULL_DOWN);            //设置关闭上下拉
#elif _GPIOC_4_MODE == GPIO_INPUT_FLOAT
    scu_set_device_gate((unsigned int)PC,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PC4,SECOND_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PC4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输入输出模式
    dpmu_set_io_pull(PC4,DPMU_IO_PULL_DISABLE);            //设置关闭上下拉
#elif _GPIOC_4_MODE == GPIO_OUTPUT_HIGH
    dpmu_set_io_direction(PC4,DPMU_IO_DIRECTION_OUTPUT);
    dpmu_set_io_pull(PC4,DPMU_IO_PULL_DISABLE);
    dpmu_set_io_reuse(PC4,SECOND_FUNCTION);
    gpio_set_output_mode(PC,pin_4);
    gpio_set_output_level_single(PC,pin_4,1);            //输出高电平
#elif _GPIOC_4_MODE == GPIO_OUTPUT_LOW
    dpmu_set_io_direction(PC4,DPMU_IO_DIRECTION_OUTPUT);
    dpmu_set_io_pull(PC4,DPMU_IO_PULL_DISABLE);
    dpmu_set_io_reuse(PC4,SECOND_FUNCTION);
    gpio_set_output_mode(PC,pin_4);
    gpio_set_output_level_single(PC,pin_4,0);            //输出高电平
#endif
}

void gpio_output(uint8_t io_Num,uint8_t level)
{
    switch (io_Num)
    {
    case 0:  gpio_set_output_level_single(PA,pin_2,level); break;
    case 1:  gpio_set_output_level_single(PA,pin_3,level); break;
    case 2:  gpio_set_output_level_single(PA,pin_4,level); break;
    case 3:  gpio_set_output_level_single(PA,pin_5,level); break;
    case 4:  gpio_set_output_level_single(PA,pin_6,level); break;
    case 5:  gpio_set_output_level_single(PB,pin_5,level); break;
    case 6:  gpio_set_output_level_single(PB,pin_6,level); break;
    case 7:  gpio_set_output_level_single(PC,pin_4,level); break;
    default:
        break;
    }
}

uint8_t gpio_get(uint8_t io_Num)
{
    uint8_t get_level=0;
    switch (io_Num)
    {
    case 0: get_level=gpio_get_input_level_single(PA,pin_2); break;
    case 1: get_level=gpio_get_input_level_single(PA,pin_3); break;
    case 2: get_level=gpio_get_input_level_single(PA,pin_4); break;
    case 3: get_level=gpio_get_input_level_single(PA,pin_5); break;
    case 4: get_level=gpio_get_input_level_single(PA,pin_6); break;
    case 5: get_level=gpio_get_input_level_single(PB,pin_5); break;
    case 6: get_level=gpio_get_input_level_single(PB,pin_6); break;
    case 7: get_level=gpio_get_input_level_single(PC,pin_4); break;
    default:
        break;
    }
    return get_level;
}

/*------------------------以下API为中断处理函数-------------------------------*/

void GPIO_IRQ_Default_proc(gpio_base_t base, int gpio_port_index)
{
	call_irq_callback(&g_gpio_callback_list_root[gpio_port_index]);
	for (int i = 0; i < 8; i++)
	{
		if(gpio_get_irq_mask_status_single(base,(0x1<<i)))
		{
			gpio_clear_irq_single(base,(0x1<<i));
			//sn_loginfo(LOG_GPIO_DRIVER,"GPIO%d Pin%d IRQ\n", gpio_port_index, i);
		}
	}
}

/**
 * @brief PA中断处理函数
 *
 */
void PA_IRQHandler(void)
{
	GPIO_IRQ_Default_proc(PA, 0);
}

/**
 * @brief PB中断处理函数
 *
 */
void PB_IRQHandler(void)
{
	GPIO_IRQ_Default_proc(PB, 1);
}

/**
 * @brief PC中断处理函数
 *
 */
void AON_PC_IRQHandler(void)
{
	GPIO_IRQ_Default_proc(PC, 2);
}

#if _KEY0_MODULE == Enable
uint8_t key0_step=0;
uint16_t key0_debounce=0;
uint16_t key0_long_press_debounce=0;
#endif
#if _KEY1_MODULE == Enable
uint8_t key1_step=0;
uint16_t key1_debounce=0;
uint16_t key1_long_press_debounce=0;
#endif
#if _KEY2_MODULE == Enable
uint8_t key2_step=0;
uint16_t key2_debounce=0;
uint16_t key2_long_press_debounce=0;
#endif
#if _KEY3_MODULE == Enable
uint8_t key3_step=0;
uint16_t key3_debounce=0;
uint16_t key3_long_press_debounce=0;
#endif

void snr_key_init()
{
#if _KEY0_MODULE==Enable
#if _KEY0_GPIO==GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_UP);                 //设置关闭上下拉
	#define KEY0_PORT PA
	#define KEY0_NUM  pin_2
#elif _KEY0_GPIO==GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY0_PORT PA
	#define KEY0_NUM  pin_3
#elif _KEY0_GPIO==GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY0_PORT PA
	#define KEY0_NUM  pin_4
#elif _KEY0_GPIO==GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY0_PORT PA
	#define KEY0_NUM  pin_5
#elif _KEY0_GPIO==GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY0_PORT PA
	#define KEY0_NUM  pin_6
#elif _KEY0_GPIO==GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY0_PORT PB
	#define KEY0_NUM  pin_5
#elif _KEY0_GPIO==GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY0_PORT PB
	#define KEY0_NUM  pin_6
#endif
#endif
#if _KEY1_MODULE==Enable
#if _KEY1_GPIO==GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PA
	#define KEY1_NUM  pin_2
#elif _KEY1_GPIO==GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PA
	#define KEY1_NUM  pin_3
#elif _KEY1_GPIO==GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PA
	#define KEY1_NUM  pin_4
#elif _KEY1_GPIO==GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PA
	#define KEY1_NUM  pin_5
#elif _KEY1_GPIO==GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PA
	#define KEY1_NUM  pin_6
#elif _KEY1_GPIO==GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PB
	#define KEY1_NUM  pin_5
#elif _KEY1_GPIO==GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY1_PORT PB
	#define KEY1_NUM  pin_6
#endif
#endif
#if _KEY2_MODULE==Enable
#if _KEY2_GPIO==GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PA
	#define KEY2_NUM  pin_2
#elif _KEY2_GPIO==GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PA
	#define KEY2_NUM  pin_3
#elif _KEY2_GPIO==GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PA
	#define KEY2_NUM  pin_4
#elif _KEY2_GPIO==GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PA
	#define KEY2_NUM  pin_5
#elif _KEY2_GPIO==GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PA
	#define KEY2_NUM  pin_6
#elif _KEY2_GPIO==GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PB
	#define KEY2_NUM  pin_5
#elif _KEY2_GPIO==GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY2_PORT PB
	#define KEY2_NUM  pin_6
#endif
#endif
#if _KEY3_MODULE==Enable
#if _KEY3_GPIO==GPIOA_2
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA2,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA2,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA2,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PA
	#define KEY3_NUM  pin_2
#elif _KEY3_GPIO==GPIOA_3
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA3,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA3,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PA
	#define KEY3_NUM  pin_3
#elif _KEY3_GPIO==GPIOA_4
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA4,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA4,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA4,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PA
	#define KEY3_NUM  pin_4
#elif _KEY3_GPIO==GPIOA_5
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PA
	#define KEY3_NUM  pin_5
#elif _KEY3_GPIO==GPIOA_6
    scu_set_device_gate((unsigned int)PA,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PA6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PA6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PA6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PA
	#define KEY3_NUM  pin_6
#elif _KEY3_GPIO==GPIOB_5
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB5,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB5,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB5,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PB
	#define KEY3_NUM  pin_5
#elif _KEY3_GPIO==GPIOB_6
    scu_set_device_gate((unsigned int)PB,ENABLE);        //开启时钟
    dpmu_set_io_reuse(PB6,FIRST_FUNCTION);               //设置引脚功能复用为GPIO
    dpmu_set_io_direction(PB6,DPMU_IO_DIRECTION_INPUT); //设置引脚功能为输出模式
    dpmu_set_io_pull(PB6,DPMU_IO_PULL_UP);                 //设置关闭上下拉
    #define KEY3_PORT PB
	#define KEY3_NUM  pin_6
#endif
#endif
}


///============按键功能
void snr_key_loop()
{
#if _KEY0_MODULE == Enable
	switch(key0_step){
				case 0:
					if(_KEY0_EFFECT == gpio_get_input_level_single(KEY0_PORT,KEY0_NUM)){
							key0_debounce++;
							if(key0_debounce >= _KEY0_DEBOUNCE){  //按键按下
									key0_debounce=0;
									key0_step++;
                                    _key_deal_hook(_KEY0_GPIO,KEY_SHORT_PRESS);
							}
					}else{
							key0_debounce=0;
					}
					break;
				case 1:
					if(!_KEY0_EFFECT == gpio_get_input_level_single(KEY0_PORT,KEY0_NUM)){
							key0_debounce++;
							if(key0_debounce >= _KEY0_DEBOUNCE){   //按键松开
									key0_step=0;
                                    _key_deal_hook(_KEY0_GPIO,KEY_SHORT_PRESS_RELEASE);
							}
					}else{
							key0_debounce=0;
                            key0_long_press_debounce++;
                            if(key0_long_press_debounce >= _KEY0_LONGPRESS_TIMER)   //发生长按
                            {
                                key0_long_press_debounce=0;
                                key0_step=3;
                                _key_deal_hook(_KEY0_GPIO,KEY_LONG_PRESS);
                            }
					}
					break;
                case 3:
					if(!_KEY0_EFFECT == gpio_get_input_level_single(KEY0_PORT,KEY0_NUM)){  //等待长按松开
							key0_debounce++;
							if(key0_debounce >= _KEY0_DEBOUNCE){   //按键松开
									key0_step=0;
                                    _key_deal_hook(_KEY0_GPIO,KEY_LONG_PRESS_RELEASE);
							}
					}else{
							key0_debounce=0;
					}
                    break;
			}
#endif

#if _KEY1_MODULE == Enable
	switch(key1_step){
				case 0:
					if(_KEY1_EFFECT == gpio_get_input_level_single(KEY1_PORT,KEY1_NUM)){
							key1_debounce++;
							if(key1_debounce >= _KEY1_DEBOUNCE){  //按键按下
									key1_debounce=0;
									key1_step++;
                                    _key_deal_hook(_KEY1_GPIO,KEY_SHORT_PRESS);
							}
					}else{
							key1_debounce=0;
					}
					break;
				case 1:
					if(!_KEY1_EFFECT == gpio_get_input_level_single(KEY1_PORT,KEY1_NUM)){
							key1_debounce++;
							if(key1_debounce >= _KEY1_DEBOUNCE){   //按键松开
									key1_step=0;
                                    _key_deal_hook(_KEY1_GPIO,KEY_SHORT_PRESS_RELEASE);
							}
					}else{
							key1_debounce=0;
                            key1_long_press_debounce++;
                            if(key1_long_press_debounce >= _KEY1_LONGPRESS_TIMER)   //发生长按
                            {
                                key1_long_press_debounce=0;
                                key1_step=3;
                                _key_deal_hook(_KEY1_GPIO,KEY_LONG_PRESS);
                            }
					}
					break;
                case 3:
					if(!_KEY1_EFFECT == gpio_get_input_level_single(KEY1_PORT,KEY1_NUM)){  //等待长按松开
							key1_debounce++;
							if(key1_debounce >= _KEY1_DEBOUNCE){   //按键松开
									key1_step=0;
                                    _key_deal_hook(_KEY1_GPIO,KEY_LONG_PRESS_RELEASE);
							}
					}else{
							key1_debounce=0;
					}
                    break;
			}
#endif

#if _KEY2_MODULE == Enable
	switch(key2_step){
				case 0:
					if(_KEY2_EFFECT == gpio_get_input_level_single(KEY2_PORT,KEY2_NUM)){
							key2_debounce++;
							if(key2_debounce >= _KEY2_DEBOUNCE){  //按键按下
									key2_debounce=0;
									key2_step++;
                                    _key_deal_hook(_KEY2_GPIO,KEY_SHORT_PRESS);
							}
					}else{
							key2_debounce=0;
					}
					break;
				case 1:
					if(!_KEY2_EFFECT == gpio_get_input_level_single(KEY2_PORT,KEY2_NUM)){
							key2_debounce++;
							if(key2_debounce >= _KEY2_DEBOUNCE){   //按键松开
									key2_step=0;
                                    _key_deal_hook(_KEY2_GPIO,KEY_SHORT_PRESS_RELEASE);
							}
					}else{
							key2_debounce=0;
                            key2_long_press_debounce++;
                            if(key2_long_press_debounce >= _KEY2_LONGPRESS_TIMER)   //发生长按
                            {
                                key2_long_press_debounce=0;
                                key2_step=3;
                                _key_deal_hook(_KEY2_GPIO,KEY_LONG_PRESS);
                            }
					}
					break;
                case 3:
					if(!_KEY2_EFFECT == gpio_get_input_level_single(KEY2_PORT,KEY2_NUM)){  //等待长按松开
							key2_debounce++;
							if(key2_debounce >= _KEY2_DEBOUNCE){   //按键松开
									key2_step=0;
                                    _key_deal_hook(_KEY2_GPIO,KEY_LONG_PRESS_RELEASE);
							}
					}else{
							key2_debounce=0;
					}
                    break;
			}
#endif

#if _KEY3_MODULE == Enable
	switch(key3_step){
				case 0:
					if(_KEY3_EFFECT == gpio_get_input_level_single(KEY3_PORT,KEY3_NUM)){
							key3_debounce++;
							if(key3_debounce >= _KEY3_DEBOUNCE){  //按键按下
									key3_debounce=0;
									key3_step++;
                                    _key_deal_hook(_KEY3_GPIO,KEY_SHORT_PRESS);
							}
					}else{
							key3_debounce=0;
					}
					break;
				case 1:
					if(!_KEY3_EFFECT == gpio_get_input_level_single(KEY3_PORT,KEY3_NUM)){
							key3_debounce++;
							if(key3_debounce >= _KEY3_DEBOUNCE){   //按键松开
									key3_step=0;
                                    _key_deal_hook(_KEY3_GPIO,KEY_SHORT_PRESS_RELEASE);
							}
					}else{
							key3_debounce=0;
                            key3_long_press_debounce++;
                            if(key3_long_press_debounce >= _KEY3_LONGPRESS_TIMER)   //发生长按
                            {
                                key3_long_press_debounce=0;
                                key3_step=3;
                                _key_deal_hook(_KEY3_GPIO,KEY_LONG_PRESS);
                            }
					}
					break;
                case 3:
					if(!_KEY3_EFFECT == gpio_get_input_level_single(KEY3_PORT,KEY3_NUM)){  //等待长按松开
							key3_debounce++;
							if(key3_debounce >= _KEY3_DEBOUNCE){   //按键松开
									key3_step=0;
                                    _key_deal_hook(_KEY3_GPIO,KEY_LONG_PRESS_RELEASE);
							}
					}else{
							key3_debounce=0;
					}
                    break;
			}
#endif
}
