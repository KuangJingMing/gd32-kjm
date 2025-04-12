/**
 ******************************************************************************
 * @文件    snr991x_it.c
 * @版本    V1.0.0
 * @日期    2022-2-11
 * @概要    .
 ******************************************************************************
 * @注意
 *
 *  
 *
 ******************************************************************************
 */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "snr991x_dma.h"
#include "snr991x_it.h"
#include "snr991x_iic.h"
#include "snr991x_TimerWdt.h"
#include "snr991x_iwdg.h"
#include "sn_log.h"
#include "snr991x_iisdma.h"
#include "snr991x_uart.h"
#include "system_msg_deal.h"
#include "snr991x_timer.h"
#include "system_hook.h"

char fa_start_rec_flag = 0;
char fa_vol_set=5;
char fa_vol_change=0;

void fa_uart1_rx(char dat)
{

	if(fa_start_rec_flag==1)
	{
		fa_start_rec_flag=0;

        fa_vol_set = dat;

        fa_vol_change=1;
	}

    //fa_vol_set=1;

    if (dat == 0x3C)            //头码OK，开始接收
    {
      fa_start_rec_flag = 1;
    }
}

__attribute__((weak)) void MAIL_BOX_IRQHandler(void){}
__attribute__((weak)) void IWDG_IRQHandler(void){}
__attribute__((weak)) void TWDG_IRQHandler(void){}
__attribute__((weak)) void SCU_IRQHandler(void){}
__attribute__((weak)) void NPU_IRQHandler(void){}
__attribute__((weak)) void ADC_IRQHandler(void){}
__attribute__((weak)) void DMA_IRQHandler(void){
	dma_irq_handler();
}
__attribute__((weak)) void TIMER0_IRQHandler(void){
    timer_clear_irq(TIMER0);
#if _TIMER_TMR0_INT == Enable
    _timer0_isr_handle();
#endif
}
__attribute__((weak)) void TIMER1_IRQHandler(void){
    timer_clear_irq(TIMER1);
#if _TIMER_TMR1_INT == Enable
    _timer1_isr_handle();
#endif
}
__attribute__((weak)) void TIMER2_IRQHandler(void){
    timer_clear_irq(TIMER2);
#if _TIMER_TMR2_INT == Enable
    _timer2_isr_handle();
#endif
}
__attribute__((weak)) void TIMER3_IRQHandler(void){
    timer_clear_irq(TIMER3);
    snr_pulse_loop();
    snr_key_loop();
}
__attribute__((weak)) void IIC0_IRQHandler(void){}//有点大
__attribute__((weak)) void PA_IRQHandler(void){}//有点大
__attribute__((weak)) void PB_IRQHandler(void){}//有点大
__attribute__((weak)) void UART0_IRQHandler(void){}//有点大
__attribute__((weak)) void UART1_IRQHandler(void){
    /*发送数据*/
    if (UART1->UARTMIS & (1UL << UART_TXInt))
    {
        ;
    }
    /*接受数据*/
    if (UART1->UARTMIS & (1UL << UART_RXInt))
    {
        //here FIFO DATA must be read out
        //UART_RXDATA(UART1);
        fa_uart1_rx(UART_RXDATA(UART1));
        UART_IntClear(UART1,UART_RXInt);
    }
    UART_IntClear(UART1,UART_AllInt);

}
__attribute__((weak)) void UART2_IRQHandler(void){}
__attribute__((weak)) void IIS0_IRQHandler(void){}
__attribute__((weak)) void IIS1_IRQHandler(void){}
__attribute__((weak)) void IIS2_IRQHandler(void){}
__attribute__((weak)) void IIS_DMA_IRQHandler(void){}//有点大
__attribute__((weak)) void ALC_IRQHandler(void){}
__attribute__((weak)) void PDM_IRQHandler(void){}
__attribute__((weak)) void DTR_IRQHandler(void){}
__attribute__((weak)) void V11_OK_IRQHandler(void){}
__attribute__((weak)) void VDT_IRQHandler(void){}
__attribute__((weak)) void EXT0_IRQHandler(void){}
__attribute__((weak)) void EXT1_IRQHandler(void){}
__attribute__((weak)) void AON_TIM_INT0_IRQHandler(void){}
__attribute__((weak)) void AON_TIM_INT1_IRQHandler(void){}
__attribute__((weak)) void AON_EFUSE_IRQHandler(void){}
__attribute__((weak)) void AON_PC_IRQHandler(void){}

/***************** (C) COPYRIGHT    *****END OF FILE****/
