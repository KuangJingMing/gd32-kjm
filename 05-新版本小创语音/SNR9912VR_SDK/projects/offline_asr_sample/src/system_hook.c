#include <stdlib.h>

#include "command_info.h"
#include "voice_module_uart_protocol.h"
#include "i2c_protocol_module.h"
#include "snr991x_core_misc.h"
#include "snr991x_gpio.h"
#include "system_msg_deal.h"
#include "status_share.h"
#include "snr991x_timer.h"
#include "snr991x_pwm.h"
#include "user_config.h"


//-----用户循环函数
void UserMain(void* p)
{

    while(1)
    {
    	vTaskDelay(100);
        snr_play_compound_loop();
    }
}



/**
 * @brief 语音识别事件钩子，语音模块输出识别结果时会调用此函数
 * 
 */
__WEAK void sys_asr_result_hook(cmd_handle_t cmd_handle, uint8_t asr_score)
{
    sys_msg_com_data_t msg;
    uint32_t *sendid = (uint32_t *)msg.msg_data; 
    uint16_t cmd_id=0;
    
    int select_index = -1;
    
    cmd_id = cmd_info_get_command_id(cmd_handle);


    vmup_port_send_char(0xF4);   //串口发送
    vmup_port_send_char(0x06); 
    vmup_port_send_char(cmd_id); 
    vmup_port_send_char(0xFB); 


    switch(cmd_id )
    {
		  case 1:   //识别指令1
				
                 break;
		  case 2:   //识别指令2 

                 break; 
		  case 3:   //识别指令3
          
                    break; 
		  case 4: break; 
		  case 5: break; 
		  case 6: break; 
		  case 7: break; 
		  
		  default: break;
    }

}

/**
 * @brief 按键回调函数
 *
 */
#if _KEY0_MODULE==Enable || _KEY1_MODULE==Enable || _KEY2_MODULE==Enable || _KEY3_MODULE==Enable
void _key_deal_hook(uint8_t key_num,uint8_t key_event)
{
	if(key_num == _KEY0_GPIO)   //按键1
	{
		switch(key_event)
		{
			case KEY_SHORT_PRESS_RELEASE: 	//短按
				//用户代码

				break;
			case KEY_LONG_PRESS: 			//长按
				//用户代码


				break;
			default: break;
		}
	}else if(key_num == _KEY1_GPIO){
		switch(key_event)
		{

		}

	}
}

#endif


//======================================================
//  TIMER0 中断函数
//======================================================
#if _TIMER_TMR0_INT == Enable
void _timer0_isr_handle()
{

}
#endif
//======================================================
//  TIMER1 中断函数
//======================================================
#if _TIMER_TMR1_INT == Enable
void _timer1_isr_handle()
{

}
#endif
//======================================================
//  TIMER2 中断函数
//======================================================
#if _TIMER_TMR2_INT == Enable
void _timer2_isr_handle()
{

}
#endif


/**
 * @brief 系统启动事件钩子，系统启动时会调用此函数
 * 
 */
__WEAK void sys_power_on_hook(void)
{
}

/**
 * @brief 系统唤醒事件钩子，系统进入唤醒状态时会调用此函数
 * 
 */
__WEAK void sys_weakup_hook(void)
{
}

/**
 * @brief 系统退出唤醒事件钩子，系统退出唤醒状态时会调用此函数
 * 
 */
__WEAK void sys_sleep_hook(void)
{
	vmup_port_send_char(0xF4);   //串口发送
    vmup_port_send_char(0x06); 
    vmup_port_send_char(0xFF); 
    vmup_port_send_char(0xFB); 
}




