/**
 * @file snr991x_init.c
 * @brief C环境启动预初始
 * @version 1.0.0
 * @date 2019-11-21
 *
 * @copyright Copyright (c) 2019    
 *
 */
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "snr991x_core_eclic.h"
#include "snr991x_core_misc.h"
#include "snr991x_core_timer.h"
#include "snr991x_scu.h"
#include "snr991x_uart.h"
#include "snr991x_spiflash.h"
#include "platform_config.h"
#include "sdk_default_config.h"
#include "snr991x_dpmu.h"

//static void SystemInit(void);

void _init()
{
    #ifndef NO_RC_CLOCK_CONFIG
    dpmu_pll_12d_config(MAIN_FREQUENCY);
    extern void board_clk_source_set(void);
    board_clk_source_set();
    #endif

    /* ECLIC init */
    eclic_init(ECLIC_NUM_INTERRUPTS);
    eclic_mode_enable();

    disable_mcycle_minstret();

    //SystemInit();

    //init_clk_div();
    init_irq_pri();

    /* 设置中断优先级分组 */
    eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO0);

    /* 开启全局中断 */
    eclic_global_interrupt_enable();

    enable_mcycle_minstret();

#ifdef STDOUT_INTERFACE
#if (STDOUT_INTERFACE == HAL_UART0_BASE) || (STDOUT_INTERFACE == HAL_UART1_BASE) || (STDOUT_INTERFACE == HAL_UART2_BASE)
    UARTPollingConfig((UART_TypeDef*)STDOUT_INTERFACE, UART_BaudRate921600);
#endif
#endif
}

void _fini()
{
}

void SystemInit(void)
{ 
    /*scu关闭时钟*/
    scu_unlock_clk_config();
    scu_unlock_reset_config();
    scu_unlock_system_config();
    SCU->AHB_CLKGATE_CFG &= ~(0x1F << 1);   //DSU不关
    SCU->APB0_CLKGATE_CFG &= ~(0x7FF << 0);   //CODEC不关
    SCU->APB1_CLKGATE_CFG &= ~(0x71B << 0);   //IIS0、IIS1、UART0不关
    scu_lock_clk_config();
    scu_lock_reset_config();
    scu_lock_system_config();

    dpmu_unlock_cfg_config();
    DPMU->AON_CLKGATE_CFG &= ~(0x1f << 0);    /*dpmu关闭时钟*/
    DPMU->PAD_FILTER_CFG_ADDR |= (0x1 << 10);  /*设置复位脚防抖动功能*/
    dpmu_lock_cfg_config();
}

