#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "system_msg_deal.h"
#include "prompt_player.h"
#include "voice_module_uart_protocol.h"
#include "i2c_protocol_module.h"
#include "ci_nvdata_manage.h"
#include "sn_log.h"
#include "snr991x_gpio.h"
#include "snr991x_uart.h"
#include "snr991x_dpmu.h"
#include "status_share.h"
#include "snr991x_timer.h"
#include "snr991x_pwm.h"
#include "snr991x_it.h"

///tag-insert-code-pos-1
void uart1_init(void);

/**
 * @brief 用户初始化
 *
 */
void userapp_initial(void)
{
    #if CPU_RATE_PRINT
    init_timer3_getresource();
    #endif

    #if MSG_COM_USE_UART_EN
    #if (UART_PROTOCOL_VER == 1)
    uart_communicate_init();
    #elif (UART_PROTOCOL_VER == 2)
    vmup_communicate_init();
    #elif (UART_PROTOCOL_VER == 255)
    UARTInterruptConfig((UART_TypeDef *)UART_PROTOCOL_NUMBER, UART_PROTOCOL_BAUDRATE);
    #endif
    #endif

    #if MSG_USE_I2C_EN
    i2c_communicate_init();
    #endif

    
    snr_gpio_init();
    snr_timer_init();
    snr_key_init();
    snr_pulse_init();
    snr_pwm_init();

    /* 用户初始化函数 */
    xTaskCreate(UserMain,"UserMain1",240,NULL,4,NULL);

    //初始化UART1(中断模式)
    pad_config_for_uart(UART1);
    UARTInterruptConfig(UART1,UART_BaudRate9600);
    uart1_init();

}

void uart1_init()
{
    dpmu_set_io_reuse(PA2,FORTH_FUNCTION);
    dpmu_set_io_reuse(PA3,FORTH_FUNCTION);
    dpmu_set_io_reuse(PA3,FORTH_FUNCTION);
    dpmu_set_io_pull(PA3,DPMU_IO_PULL_UP);  //RX需开启上拉

}


/**
 * @brief 处理按键消息（目前未实现该demo）
 *
 * @param key_msg 按键消息
 */
void userapp_deal_key_msg(sys_msg_key_data_t  *key_msg)
{
    (void)(key_msg);
}


/**
 * @brief 按语义ID响应asr消息处理
 *
 * @param asr_msg
 * @param cmd_handle
 * @param semantic_id
 * @return uint32_t
 */
uint32_t deal_asr_msg_by_semantic_id(sys_msg_asr_data_t *asr_msg, cmd_handle_t cmd_handle, uint32_t semantic_id)
{
    uint32_t ret = 1;
    if(0)
    //if (PRODUCT_GENERAL == get_product_id_from_semantic_id(semantic_id))
    {
        uint8_t vol;
        int select_index = -1;
        switch(get_function_id_from_semantic_id(semantic_id))
        {
        case VOLUME_UP:        //增大音量
            vol = vol_set(vol_get() + 1);
            select_index = (vol == VOLUME_MAX) ? 1:0;
            break;
        case VOLUME_DOWN:      //减小音量
            vol = vol_set(vol_get() - 1);
            select_index = (vol == VOLUME_MIN) ? 1:0;
            break;
        case MAXIMUM_VOLUME:   //最大音量
            vol_set(VOLUME_MAX);
            break;
        case MEDIUM_VOLUME:  //中等音量
            vol_set(VOLUME_MID);
            break;
        case MINIMUM_VOLUME:   //最小音量
            vol_set(VOLUME_MIN);
            break;
        case TURN_ON_VOICE_BROADCAST:    //开启语音播报
            prompt_player_enable(ENABLE);
            break;
        case TURN_OFF_VOICE_BROADCAST:    //关闭语音播报
            prompt_player_enable(DISABLE);
            break;
        default:
            ret = 0;
            break;
        }
        if (ret)
        {
            #if PLAY_OTHER_CMD_EN
            pause_voice_in();
            prompt_play_by_cmd_handle(cmd_handle, select_index, default_play_done_callback,true);
            #endif
        }
    }
    else
    {
        ret = 0;
    }
    return ret;
}


/**
 * @brief 按命令词id响应asr消息处理
 *
 * @param asr_msg
 * @param cmd_handle
 * @param cmd_id
 * @return uint32_t
 */
uint32_t deal_asr_msg_by_cmd_id(sys_msg_asr_data_t *asr_msg, cmd_handle_t cmd_handle, uint16_t cmd_id)
{
    uint32_t ret = 1;
    int select_index = -1;
    switch(cmd_id)
    {
        ///tag-asr-msg-deal-by-cmd-id-start
        case 2://“打开空调”
        {
            break;
        }
        case 3://“关闭空调”
        {
            break;
        }
        case 14://“除湿模式”
        {
            break;
        }
        case 15://"关闭除湿"
        {
            break;
        }
        case 22://"关闭睡眠模式"
        {
            break;
        }
        ///tag-asr-msg-deal-by-cmd-id-end
        default:
            ret = 0;
            break;
    }

    if (ret && select_index >= -1)
    {
        #if PLAY_OTHER_CMD_EN
        pause_voice_in();
        prompt_play_by_cmd_handle(cmd_handle, select_index, default_play_done_callback,true);
        #endif
    }

    return ret;
}

/**
 * @brief 用户自定义消息处理
 *
 * @param msg
 * @return uint32_t
 */
uint32_t deal_userdef_msg(sys_msg_t *msg)
{
    uint32_t ret = 1;
    switch(msg->msg_type)
    {
    /* 按键消息 */
    case SYS_MSG_TYPE_KEY:
    {
        sys_msg_key_data_t *key_rev_data;
        key_rev_data = &msg->msg_data.key_data;
        userapp_deal_key_msg(key_rev_data);
        break;
    }
    #if MSG_COM_USE_UART_EN
    /* CI串口协议消息 */
    case SYS_MSG_TYPE_COM:
    {
		#if ((UART_PROTOCOL_VER == 1) || (UART_PROTOCOL_VER == 2))
    	sys_msg_com_data_t *com_rev_data;
        com_rev_data = &msg->msg_data.com_data;
        userapp_deal_com_msg(com_rev_data);
        #endif
        break;
    }
    #endif
    #if MSG_USE_I2C_EN
    case SYS_MSG_TYPE_I2C:
    {
        sys_msg_i2c_data_t *i2c_rev_data;
        i2c_rev_data = &msg->msg_data.i2c_data;
        userapp_deal_i2c_msg(i2c_rev_data);
        break;
    }
    #endif
    default:
        break;
    }
    return ret;
}

