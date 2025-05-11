/**/
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "snr991x_gpio.h"
#include "system_hook.h"

#define Enable 1
#define Disable 0

#ifndef VOLUME_DEFAULT
#define VOLUME_DEFAULT                  5       //默认音量0-7
#endif

//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * GPIO 模块 * 
//  GPIO_OUTPUT_HIGH  GPIO_OUTPUT_LOW  GPIO_INPUT_PULLHIGH  GPIO_INPUT_PULLDOWN  GPIO_INPUT_FLOAT  GPIO_PIN_OFF
//====================================================================================
#define _GPIOA_2_MODE              GPIO_PIN_OFF    // Set PA2
#define _GPIOA_3_MODE              GPIO_PIN_OFF    // Set PA3
#define _GPIOA_4_MODE              GPIO_PIN_OFF    // Set PA4
#define _GPIOA_5_MODE              GPIO_PIN_OFF    // Set PA5
#define _GPIOA_6_MODE              GPIO_PIN_OFF    // Set PA6
#define _GPIOB_5_MODE              GPIO_PIN_OFF    // Set PB5
#define _GPIOB_6_MODE              GPIO_PIN_OFF    // Set PB6
#define _GPIOC_4_MODE              GPIO_PIN_OFF    // demo板上默认接到控制功放使能  
//  * 可调用函数 *
// gpio_output(GPIOA_5,1);   --说明：IO输出设置        参数1：GPIO口号   参数2：输出电平高（1）/低（0）    返回值：无
// gpio_get(GPIOA_5);      --说明：IO口状态获取     参数1：GPIO口号    返回值：高电平（1）/低电平（0）
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * 脉冲输出模块 *
//====================================================================================
#define _PULSE0_MODULE     			 Disable   //脉冲输出使能
#define _PULSE0_ON_GPIO              GPIOA_5  //输出口设置
#define _PULSE0_DEFAULT_LEVEL        0         //初始电平
#define _PULSE0_WIDE                 500        //脉冲宽度（单位：毫秒）
#define _PULSE0_TIME                 100        //脉冲输出次数

#define _PULSE1_MODULE     			 Disable   //脉冲输出使能
#define _PULSE1_ON_GPIO              GPIOA_5  //输出口设置
#define _PULSE1_DEFAULT_LEVEL        0         //初始电平
#define _PULSE1_WIDE                 10        //脉冲宽度（单位：毫秒）
#define _PULSE1_TIME                 1         //脉冲输出次数

#define _PULSE2_MODULE     			 Disable   //脉冲输出使能
#define _PULSE2_ON_GPIO              GPIOA_5  //输出口设置
#define _PULSE2_DEFAULT_LEVEL        0         //初始电平
#define _PULSE2_WIDE                 10        //脉冲宽度（单位：毫秒）
#define _PULSE2_TIME                 1         //脉冲输出次数
//  * 可调用函数 *
// pulse_start(pulse0);   --说明：脉冲输出执行      参数1：脉冲序号口号     返回值：无
//////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * TIMER Module *
//====================================================================================
#define _TIMER_TMR0_INT             Disable             // 使能定时器0并开启中断
#define _TIMER_TMR0_DUR_S           0                   // 定时长度（单位秒）
#define _TIMER_TMR0_DUR_MS          0                   // 定时长度（单位毫秒）
#define _TIMER_TMR0_DUR_US          0                   // 定时长度（单位微妙）

#define _TIMER_TMR1_INT             Disable             // 使能定时器1并开启中断
#define _TIMER_TMR1_DUR_S           0                   // 定时长度（单位秒）
#define _TIMER_TMR1_DUR_MS          0                   // 定时长度（单位毫秒）
#define _TIMER_TMR1_DUR_US          0                   // 定时长度（单位微妙）

#define _TIMER_TMR2_INT             Disable             // 使能定时器2并开启中断
#define _TIMER_TMR2_DUR_S           0                   // 定时长度（单位秒）
#define _TIMER_TMR2_DUR_MS          0                   // 定时长度（单位毫秒）
#define _TIMER_TMR2_DUR_US          0                   // 定时长度（单位微妙）
//  * 可调用函数 *
// timer_start(TIMER0);   			--说明：定时器开启      参数1：定时器序号     			返回值：无
// timer_stop(TIMER0);    			--说明：定时器停止      参数1：定时器序号    			返回值：无
//
//  * 中断函数    *
// _timer0_isr_handle()              --说明：在system_hook.c执行
// _timer1_isr_handle()   			 --说明：在system_hook.c执行
// _timer2_isr_handle()				 --说明：在system_hook.c执行


//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * PWM 模块 *
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////
#define _PWM0_MODULE  				Disable     //使能   PWM0 默认为PA2
#define _PWM0_FREQ                  1000       //频率
#define _PWM0_DUTY					10	 	   //占空比值
#define _PWM0_DUTY_MAX				100	 	   //占空比最大值
#define _PWM1_MODULE  				Disable    //使能   PWM1 默认为PA3
#define _PWM1_FREQ                  1000       //频率
#define _PWM1_DUTY					10	 	   //占空比值
#define _PWM1_DUTY_MAX				100	 	   //占空比最大值 
#define _PWM2_MODULE  				Disable    //使能   PWM2 默认为PA4
#define _PWM2_FREQ                  1000       //频率
#define _PWM2_DUTY					10	 	   //占空比值
#define _PWM2_DUTY_MAX				100	 	   //占空比最大值
#define _PWM3_MODULE  				Disable    //使能   PWM3 默认为PA5
#define _PWM3_FREQ                  1000       //频率
#define _PWM3_DUTY					50	 	   //占空比值
#define _PWM3_DUTY_MAX				100	 	   //占空比最大值
//  * 可调用函数 *
//
// pwm_start(PWM0);   			--说明：PWM输出执行      参数1：PWM序号     									返回值：无
// pwm_stop(PWM0);    			--说明：PWM停止输出      参数1：PWM序号     									返回值：无
// pwm_set_duty(PWM0,50,100);  	--说明：PWM频率设置      参数1：PWM序号    参数2：占空比    参数3：最大占空比值	返回值：无
//
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * UART Module *
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////
#define CONFIG_SN_LOG_UART HAL_UART0_BASE         //系统LOG打印串口
#define MSG_COM_USE_UART_EN 1
#define UART_PROTOCOL_NUMBER (HAL_UART1_BASE)     //通讯串口
#define UART_PROTOCOL_BAUDRATE (UART_BaudRate9600)
#define UART_PROTOCOL_VER 2 
//  * 可调用函数 *
//
// vmup_port_send_char(p);   			--说明：串口发送一个byte      参数1：发送值     									返回值：无
// vmup_port_send_string(uint8_t *p);   --说明：串口发送字符串        参数1：字符串首地址     									返回值：无
//
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * KEY Module *
//
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////
#define _KEY0_MODULE             Disable
#define _KEY0_GPIO 			     GPIOA_4     //IO口序号
#define _KEY0_EFFECT             0           //按键有效电平（0：低电平 1：高电平）
#define _KEY0_DEBOUNCE			 10 		 //按键消抖时间（毫秒）
#define _KEY0_LONGPRESS_TIMER    1500		 //长按时间（毫秒）

#define _KEY1_MODULE             Disable
#define _KEY1_GPIO 			     GPIOA_5     //IO口序号
#define _KEY1_EFFECT             0           //按键有效电平（0：低电平 1：高电平）
#define _KEY1_DEBOUNCE			 10 		 //按键消抖时间（毫秒）
#define _KEY1_LONGPRESS_TIMER    1500		 //长按时间（毫秒）

#define _KEY2_MODULE             Disable
#define _KEY2_GPIO 			     GPIOA_5     //IO口序号
#define _KEY2_EFFECT             0           //按键有效电平（0：低电平 1：高电平）
#define _KEY2_DEBOUNCE			 10 		 //按键消抖时间（毫秒）
#define _KEY2_LONGPRESS_TIMER    1500		 //长按时间（毫秒）

#define _KEY3_MODULE             Disable
#define _KEY3_GPIO 			     GPIOA_5     //IO口序号
#define _KEY3_EFFECT             0           //按键有效电平（0：低电平 1：高电平）
#define _KEY3_DEBOUNCE			 10 		 //按键消抖时间（毫秒）
#define _KEY3_LONGPRESS_TIMER    1500		 //长按时间（毫秒）
//
//  /* 可调用函数 */
//
//  void _key_deal_hook(uint8_t key_num,uint8_t key_event);    --说明：按键点击处理函数，在system_hook.c中使用
//
//////////////////////////////////////////////////////////////////////////////////////

#define AUDIO_PLAYER_ENABLE 1 //用于屏蔽播放器任务相关代码      0：屏蔽，1：开启
#if AUDIO_PLAYER_ENABLE
#define USE_PROMPT_DECODER 1          //播放器是否支持prompt解码器
#define USE_MP3_DECODER 1             //为1时加入mp3解码器
#define AUDIO_PLAY_SUPPT_MP3_PROMPT 1 //播放器默认开启mp3播报音
#define USE_MS_WAV_DECODER 0          //播放器是否支持ms wav解码器
#define AUDIO_PLAY_BLOCK_CONT 4 //播放器底层缓冲区个数
#endif
//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * 音频播放 *
//
//  void play_voice(1,true);  --说明：音频播放      参数1：音频序号    参数2：打断播放 
//
//  uint8_t voice_id[5] = {1,2,3,4,5};  //填写需要播放的语音序号
//  user_play_compound(voice_id);   	  --说明：音频组合播放        	   参数1：组合语音序号地址      	     		返回值：无
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * 音量控制指令 *
//  vol_up();          --说明：音量增大
//  vol_down();        --说明：音量减小
//  vol_max();         --说明：最大音量
//  vol_mid();         --说明：中等音量
//  vol_min();         --说明：最小音量
//  vol_mute();        --说明：静音
//  vol_unmute();      --说明：取消静音
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////

#define USE_ALC_AUTO_SWITCH_MODULE 1          //使用动态alc模块:1开启，0关闭
#define USE_DENOISE_MODULE         0         //使用降噪模块:1开启，0关闭
#define USE_DOA_MODULE             0        //使用声源定位模块：1开启，0关闭
#define USE_DEREVERB_MODULE        0       //使用降混响模块：1开启，0关闭
#define USE_BEAMFORMING_MODULE     0       //使用双麦语音增强模块:1开启，0关闭
#define USE_AEC_MODULE             0      //使用回声消除模块:1开启，0关闭
#if USE_AEC_MODULE
#define PAUSE_VOICE_IN_WITH_PLAYING  0//开启aec时关闭
#define IF_JUST_CLOSE_HPOUT_WHILE_NO_PLAY   1
#endif

// 语音识别退出唤醒时间
#ifndef EXIT_WAKEUP_TIME
#define EXIT_WAKEUP_TIME                15000   //default exit wakeup time,unit ms
#endif

//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * 语音识别功能设置 *
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////
//====================================================================================
//  * 语音识别控制 *
//  asr_wakeup();       --说明：语音识别唤醒
//  asr_sleep();        --说明：语音识别超时
//====================================================================================
//////////////////////////////////////////////////////////////////////////////////////


#if USE_BEAMFORMING_MODULE || USE_AEC_MODULE || USE_DOA_MODULE ||USE_DEREVERB_MODULE
#define HOST_CODEC_CHA_NUM  2
#endif


#define CONFIG_SYSTEMVIEW_EN 0 //不使能systemview

#endif /* _USER_CONFIG_H_ */
