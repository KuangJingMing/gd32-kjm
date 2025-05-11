/**
 * @file system_msg_deal.c
 * @brief  系统消息处理任务
 * @version V1.0.0
 * @date 2019.01.22
 *
 * @copyright Copyright (c) 2019    
 *
 */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "system_msg_deal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "sn_log.h"
#include "audio_play_api.h"


#include "sdk_default_config.h"
#include "snr991x_iisdma.h"
#include "snr991x_iis.h"
#include "snr991x_lowpower.h"
#include "snr991x_core_misc.h"
// #include "snr991x_audio_capture.h"
#include "voice_module_uart_protocol.h"
#include "prompt_player.h"
#include "product_semantic.h"
#include "ci_nvdata_manage.h"
#include "asr_api.h"
#include "user_msg_deal.h"
#include "system_hook.h"
#include "status_share.h"

/* 系统消息处理任务状态 */
typedef enum
{
    USERSTATE_WAIT_MSG = 0,    /* 等待消息 */
    SYS_STATE_WAKUP_TIMEOUT,   /* 消息超时 */
}user_task_state_t;

/* 系统状态结构 */
struct sys_manage_type
{
    uint8_t user_msg_state;            /* 系统消息处理任务状态                    */
    sys_wakeup_state_t wakeup_state;   /* 系统状态         0:非唤醒状态 1:唤醒状态 */
    sys_asr_state_t asr_state;         /* asr状态          0:空闲       1:忙碌   */
    uint8_t volset;                     /* 音量设置 */
    uint8_t mute_voice_count;          /* 用于保证语音识别开关配对                 */
}sys_manage_data;

/* 唤醒互斥锁 */
SemaphoreHandle_t WakeupMutex = NULL;
/* 语音输入mute保护互斥锁 */
SemaphoreHandle_t muteMutex = NULL;
/* 退出唤醒定时器 */
xTimerHandle exit_wakeup_timer = NULL;

/* 系统消息队列 */
static QueueHandle_t sys_msg_queue = NULL;
/* 用于忽略退出唤醒的标志，因为存在收到asr识别结果的同时收到退出唤醒的定时器事件，此时不应退出唤醒 */
static int8_t ignore_exit_wakeup = 0;

/*用于忽略语音识别消息 */
static int8_t ignore_asr_msg = 0;


/**
 * @brief check now playing state, AEC or DENOISE maybe used this function
 *
 * @return true now is playing or ready to playing
 * @return false now not playing or ready to stop play
 */
bool check_current_playing(void)
{
    if(AUDIO_PLAY_STATE_IDLE != get_audio_play_state())
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * @brief pause voice in, used for stop ASR when playing, if config use AEC,
 *        this function just empty.
 *
 */
void pause_voice_in(void)
{
    //xSemaphoreTake(muteMutex, portMAX_DELAY);
    //if (sys_manage_data.mute_voice_count == 0)
    //{
    //    #if PAUSE_VOICE_IN_WITH_PLAYING
    //    mute_voice_in_flag = true;
    //        audio_cap_mute(AUDIO_CAP_INNER_CODEC,ENABLE);/*mute: but audio continue in*/
	//		asrtop_asr_system_pause();
    //    #endif
    //
    //}
    //sys_manage_data.mute_voice_count++;
    //xSemaphoreGive(muteMutex);
}


/**
 * @brief resume voice in, so can recover ASR, call this after play done,
 *
 */
void  resume_voice_in(void)
{
    //xSemaphoreTake(muteMutex, portMAX_DELAY);
    //
    //if (sys_manage_data.mute_voice_count > 0)
    //{
    //    sys_manage_data.mute_voice_count--;
    //}
    //if (sys_manage_data.mute_voice_count == 0)
    //{
    //    #if PAUSE_VOICE_IN_WITH_PLAYING
    //        audio_cap_mute(AUDIO_CAP_INNER_CODEC,DISABLE);
	//		asrtop_asr_system_continue();
    //        mute_voice_in_flag = false;
    //    #endif
    //
    //}
    //xSemaphoreGive(muteMutex);
}


/**
 * @brief 命令词识别结果播报完成回调函数
 *
 * @param cmd_handle 命令信息句柄
 */
void default_play_done_callback(cmd_handle_t cmd_handle)
{
    resume_voice_in();
}


/**
 * @brief Get the wakeup state object
 *
 * @return sys_wakeup_state_t
 */
sys_wakeup_state_t get_wakeup_state(void)
{
    return sys_manage_data.wakeup_state;
}


/**
 * @brief Get the asr state
 *
 * @return sys_asr_state_t asr状态
 */
sys_asr_state_t get_asr_state(void)
{
    return sys_manage_data.asr_state;
}


/**
 * @brief 设置状态为唤醒
 *
 * @param exit_wakup_ms 下次退出唤醒时间，单位ms
 */
void set_state_enter_wakeup(uint32_t exit_wakup_ms)
{
    sys_manage_data.wakeup_state = SYS_STATE_WAKEUP;/*update wakeup state*/
    ciss_set(SN_SS_WAKING_UP_STATE,SN_SS_WAKEUPED);
    ciss_set(SN_SS_WAKING_UP_STATE_FOR_SSP,SN_SS_WAKEUPED);
    xTimerStop(exit_wakeup_timer,0);
    xTimerChangePeriod(exit_wakeup_timer,pdMS_TO_TICKS(exit_wakup_ms),0);/*or used a new timer*/
    xTimerStart(exit_wakeup_timer,0);
}


/**
 * @brief 更新唤醒超时时间，保持唤醒状态
 *
 */
void update_awake_time(void)
{
    if (sys_manage_data.wakeup_state == SYS_STATE_WAKEUP)
    {
        set_state_enter_wakeup(EXIT_WAKEUP_TIME);
    }
}


/**
 * @brief 设置状态为退出唤醒
 *
 */
void set_state_exit_wakeup(void)
{
    xTimerStop(exit_wakeup_timer,0);
    sys_manage_data.wakeup_state = SYS_STATE_UNWAKEUP;
    ciss_set(SN_SS_WAKING_UP_STATE,SN_SS_NO_WAKEUP);
    ciss_set(SN_SS_WAKING_UP_STATE_FOR_SSP,SN_SS_NO_WAKEUP);
    ciss_set(SN_SS_CMD_STATE,SN_SS_CMD_IS_NULL);
    ciss_set(SN_SS_CMD_STATE_FOR_SSP,SN_SS_CMD_IS_NULL);
}


#if USE_LOWPOWER_OSC_FREQUENCY
/* 退出降频模式进入osc时钟模式定时器 */
xTimerHandle exit_down_freq_mode_timer = NULL;

/**
 * @brief 退出降频模式进入osc时钟模式定时器
 *
 * @param xTimer 定时器句柄
 */
void exit_down_freq_mode_cb(TimerHandle_t xTimer)
{
    xTimerStop(exit_down_freq_mode_timer,0);

    xSemaphoreTake(WakeupMutex, portMAX_DELAY);

    /* TODO: 检查逻辑确认这样判断的正确性？？？？ */
    if((SYS_STATE_UNWAKEUP == get_wakeup_state())&&(POWER_MODE_NORMAL == get_curr_power_mode()))
    {
        if(( 0 == asrtop_sys_isbusy())&&(AUDIO_PLAY_STATE_IDLE == get_audio_play_state()))
        {
            audio_cap_stop(AUDIO_CAP_NUM_LP);
            audio_pre_rslt_stop();
            pause_voice_in();
            asrtop_asr_system_pause();
            #if (USE_DENOISE_MODULE)
            power_mode_switch(POWER_MODE_DOWN_FREQUENCY);
            #else
            power_mode_switch(POWER_MODE_OSC_FREQUENCY);
            #endif
            asrtop_asr_system_continue();
            resume_voice_in();
            audio_cap_start(AUDIO_CAP_NUM_LP);
            audio_pre_rslt_start();
        }
        else
        {
            xTimerStart(exit_down_freq_mode_timer,0);
        }
    }
    xSemaphoreGive(WakeupMutex);
}
#endif

#if USE_LOWPOWER_OSC_FREQUENCY
/**
 * @brief vad start中断回调函数，这个函数在vad start中断中调用，
 *          此时需要将频率提升降频模式以上方可保证识别流程正常进行
 */
void vad_start_irq_cb(void)
{
    /* 如果系统处于晶振频率模式，在vad start时切换到正常模式运行 */
    if(POWER_MODE_OSC_FREQUENCY == get_curr_power_mode())
    {
        audio_cap_stop(AUDIO_CAP_NUM_LP);
        audio_pre_rslt_stop();
        power_mode_switch(POWER_MODE_NORMAL);
        audio_cap_start(AUDIO_CAP_NUM_LP);
        audio_pre_rslt_start();
        xTimerStartFromISR(exit_down_freq_mode_timer,0);
    }
}
#endif



/**
 * @brief 切换唤醒模型，这个函数是sys msg任务调用，其他任务需要切换模型需要发送切换模型消息
 *          通过sys msg任务调用
 */
void change_asr_wakeup_word(void)
{
    xSemaphoreTake(WakeupMutex, portMAX_DELAY);

    /*set wakeup state*/
    set_state_exit_wakeup();

    sys_sleep_hook();
    
    xSemaphoreGive(WakeupMutex);

    #if ADAPTIVE_THRESHOLD
    dynmic_confidence_en_cfg(0);
    #endif
    #if USE_SEPARATE_WAKEUP_EN
    cmd_info_change_cur_model_group(1);
    ignore_asr_msg++;

    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
    send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_ENABLE_PROCESS_ASR;
    send_msg_to_sys_task(&send_msg, NULL);
    #endif

    #if (ASR_SKIP_FRAME_CONFIG == 1)
    //if (get_cur_lm_states() < CLOSE_SKIP_MAX_MODEL_SIZE)
    {
        asr_dynamic_skip_close();
    }
    #endif

    #if USE_LOWPOWER_OSC_FREQUENCY
    // 晶振时钟模式，可以产生VAD START，不能识别需要在VAD START中断内切换到正常模式才可以识别
    audio_cap_stop(AUDIO_CAP_NUM_LP);
    audio_pre_rslt_stop();
    pause_voice_in();
    asrtop_asr_system_pause();

    #if (USE_DENOISE_MODULE)
    power_mode_switch(POWER_MODE_DOWN_FREQUENCY);
    #else
    power_mode_switch(POWER_MODE_OSC_FREQUENCY);
    #endif

    asrtop_asr_system_continue();
    resume_voice_in();
    audio_cap_start(AUDIO_CAP_NUM_LP);
    audio_pre_rslt_start();
    #elif USE_LOWPOWER_DOWN_FREQUENCY
    // 降频模式，可以进行正常识别，在进入唤醒模式后切换回正常模式即可
    audio_cap_stop(AUDIO_CAP_NUM_LP);
    audio_pre_rslt_stop();
    pause_voice_in();
    asrtop_asr_system_pause();
    power_mode_switch(POWER_MODE_DOWN_FREQUENCY);
    asrtop_asr_system_continue();
    resume_voice_in();
    audio_cap_start(AUDIO_CAP_NUM_LP);
    audio_pre_rslt_start();
    #endif
}


/**
 * @brief 切换正常模型，这个函数是sys msg任务调用，其他任务需要切换模型需要发送切换模型消息
 *          通过sys msg任务调用
 *
 */
void change_asr_normal_word(void)
{
    #if (ASR_SKIP_FRAME_CONFIG == 1)
    asr_dynamic_skip_open();
    #endif
    
    #if USE_SEPARATE_WAKEUP_EN
    cmd_info_change_cur_model_group(0);
    ignore_asr_msg++;

    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
    send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_ENABLE_PROCESS_ASR;
    send_msg_to_sys_task(&send_msg, NULL);
    #endif

    #if ADAPTIVE_THRESHOLD
    dynmic_confidence_en_cfg(1);
    dynmic_confidence_config(-20, 10, 1);
    #endif
}


/**
 * @brief 进入唤醒模式播放完毕回调函数
 *          相比正常的播放完毕回调函数，除了要解除mute之外需要发送切换模型的消息以切换到正常模型
 *
 * @param cmd_handle
 */
void play_enter_wakeup_done_cb(cmd_handle_t cmd_handle)
{
    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
    send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_POST_CHANGE_ASR_NORMAL_WORD;
    send_msg_to_sys_task(&send_msg, NULL);

    resume_voice_in();
}


/**
 * @brief 退出唤醒模式播放完毕回调函数
 *          相比正常的播放完毕回调函数，除了要解除mute之外需要发送切换模型的消息以切换到唤醒模型，并设置系统状态
 *
 * @param cmd_handle
 */
void play_exit_wakeup_done_cb(cmd_handle_t cmd_handle)
{
    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
    send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_POST_CHANGE_ASR_WAKEUP_WORD;
    send_msg_to_sys_task(&send_msg, NULL);

    resume_voice_in();

    mprintf("inactivate\n");
}


/**
 * @brief when exit wakup state, need deal some common thing, such as
 *        power mode set, change asr word, wakup state flag set, play wakeup prompt.
 *        asr_busy_check used for immediately exit or wait for current ASR deal done.
 *
 * @param asr_busy_check : 0:no need check asr busy, 1:need check asr busy
 */
void exit_wakeup_deal(uint32_t asr_busy_check)
{
    xSemaphoreTake(WakeupMutex, portMAX_DELAY);

    /*already in unwakeup state, so do nothing*/
    if(SYS_STATE_UNWAKEUP == get_wakeup_state())
    {
        xSemaphoreGive(WakeupMutex);
        return;
    }

    /*now asr busy,so need wait not busy, then call this function again*/
    if((1 == asr_busy_check)&&(SYS_STATE_ASR_BUSY == get_asr_state()))
    {
        sys_manage_data.user_msg_state = SYS_STATE_WAKUP_TIMEOUT;
        xSemaphoreGive(WakeupMutex);
        return;
    }
    #if PLAY_EXIT_WAKEUP_EN
    /*play exit wakeup voice*/
    pause_voice_in();
    prompt_play_by_cmd_string("<inactivate>", -1, play_exit_wakeup_done_cb,false);
    #else
    pause_voice_in();
    play_exit_wakeup_done_cb(0);
    #endif

    xSemaphoreGive(WakeupMutex);
}


/**
 * @brief when enter wakup state, need deal some common thing, such as
 *        power mode set, change asr word, wakup state flag set, play wakeup prompt
 *
 * @param exit_wakup_ms : wakeup state keep times,
 */
void enter_wakeup_deal(uint32_t exit_wakup_ms, cmd_handle_t cmd_handle )
{
    xSemaphoreTake(WakeupMutex, portMAX_DELAY);

    /*if last state is unwakeup, need change asr word*/
    if(SYS_STATE_UNWAKEUP == get_wakeup_state())
    {
        #if (USE_LOWPOWER_OSC_FREQUENCY||USE_LOWPOWER_DOWN_FREQUENCY)
        audio_cap_stop(AUDIO_CAP_NUM_LP);
        audio_pre_rslt_stop();
        pause_voice_in();
        asrtop_asr_system_pause();
        power_mode_switch(POWER_MODE_NORMAL);
        asrtop_asr_system_continue();
        resume_voice_in();
        audio_cap_start(AUDIO_CAP_NUM_LP);
        audio_pre_rslt_start();
        #endif
    }

    //if(cmd_handle != INVALID_HANDLE)
    {
        pause_voice_in();
        /*if last state is unwakeup, need change asr word*/
        if(SYS_STATE_UNWAKEUP == get_wakeup_state())
        {
            #if PLAY_ENTER_WAKEUP_EN
            prompt_play_by_cmd_handle(cmd_handle, -1, play_enter_wakeup_done_cb,true);
            #else
            play_enter_wakeup_done_cb(cmd_handle);
            #endif
        }
        else
        {
            #if PLAY_ENTER_WAKEUP_EN
            prompt_play_by_cmd_handle(cmd_handle, -1, default_play_done_callback,true);
            #else
            default_play_done_callback(cmd_handle);
            #endif
        }
    }

    /*set wakeup state,and update timer*/
    set_state_enter_wakeup(exit_wakup_ms);
	sys_weakup_hook();

    xSemaphoreGive(WakeupMutex);
}


/**
 * @brief : exit wakup timer callback function, this sample code will wait asr idle when exit wakeup,
 *          if you want immediately eixt wakeup, use exit_wakeup_deal(0)
 *
 * @param xTimer : timer handle
 */
void exit_wakeup_timer_callback(TimerHandle_t xTimer)
{
    (void)xTimer;
    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
    send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_EXIT_WAKEUP;
    send_msg_to_sys_task(&send_msg, NULL);
}


/**
 * @brief 音量设置函数
 *
 * @param vol 音量值
 */
uint8_t vol_set(char vol)
{
    if(vol <= VOLUME_MAX && vol >= VOLUME_MIN && sys_manage_data.volset != vol)
    {
        sys_manage_data.volset = vol;
        audio_play_set_vol_gain(67*vol/VOLUME_MAX + 27);
        cinv_item_write(NVDATA_ID_VOLUME, sizeof(sys_manage_data.volset), &sys_manage_data.volset);
    }
    return sys_manage_data.volset;
}

uint8_t vol_get(void)
{
    return sys_manage_data.volset;
}

uint8_t vol;
int select_index = -1;

void vol_up()
{
    vol = vol_set(vol_get() + 1);
    select_index = (vol == VOLUME_MAX) ? 1:0;
}

void vol_down()
{
    vol = vol_set(vol_get() - 1);
    select_index = (vol == VOLUME_MIN) ? 1:0;
}

void vol_max()
{
    vol_set(VOLUME_MAX);
}

void vol_mid()
{
    vol_set(VOLUME_MID);
}

void vol_min()
{
    vol_set(VOLUME_MIN);
}

void vol_mute()
{
    prompt_player_enable(DISABLE);
}

void vol_unmute()
{
    prompt_player_enable(ENABLE);
}

/**
 * @brief 系统消息任务资源初始化
 *
 */
void sys_msg_task_initial(void)
{
    sys_msg_queue = xQueueCreate(16, sizeof(sys_msg_t));
    if(!sys_msg_queue)
    {
        mprintf("not enough memory:%d,%s\n",__LINE__,__FUNCTION__);
    }

    WakeupMutex = xSemaphoreCreateMutex();
    if(!WakeupMutex)
    {
        mprintf("not enough memory:%d,%s\n",__LINE__,__FUNCTION__);
    }

    muteMutex = xSemaphoreCreateMutex();
    if(!muteMutex)
    {
        mprintf("not enough memory:%d,%s\n",__LINE__,__FUNCTION__);
    }
}


/**
 * @brief A simple code for other component send system message to this module, just wrap freertos queue function
 *
 * @param flag_from_isr : 0 call this function not from isr, other call this from isr
 * @param send_msg : system message
 * @param xHigherPriorityTaskWoken : if call this not from isr set NULL
 * @return BaseType_t
 */
BaseType_t send_msg_to_sys_task(sys_msg_t *send_msg,BaseType_t *xHigherPriorityTaskWoken)
{
    if(0 != check_curr_trap())
    {
        return xQueueSendFromISR(sys_msg_queue,send_msg,xHigherPriorityTaskWoken);
    }
    else
    {
        return xQueueSend(sys_msg_queue,send_msg,0);
    }
}


/**
 * @brief 处理一些切换模型请求
 *
 * @param cmd_info_msg
 */
void sys_deal_cmd_info_msg(sys_msg_cmd_info_data_t *cmd_info_msg)
{
    /**
     * 这个函数处理退出唤醒模式和切换模型的请求，这些请求的发起方往往是其他线程，切模型和播放如果是异步的
     * 将导致播放获取的固件信息丢失而产生播放错误，所以播放请求和切模型的流程全部在sys_msg线程中执行将不会
     * 出现这个问题。
     * 关于理解ignore_exit_wakeup时，请切记播放是一个异步的请求，在播放到播放完毕期间是可能产生新的系统状态
     * 更新请求，并切换模型，低功耗的流程。
     */
    if(MSG_CMD_INFO_STATUS_POST_CHANGE_ASR_WAKEUP_WORD == cmd_info_msg->cmd_info_status)
    {
        /**
         * 切换到唤醒网络请求，这个请求是在播放完退出唤醒提示音后发送的消息，这里要判断ignore_exit_wakeup变量
         * 是由于在退出唤醒播放的过程中可能出现再次唤醒，这时候再发送切换模型的消息应该将其丢弃，系统状态已经
         * 被新的识别消息重置。切换完毕模型之后将进入低功耗策略，也就是切模型本身是工作于非低功耗下。
         */
        if(ignore_exit_wakeup == 0)
        {
            /*change asr wakeup word*/
            change_asr_wakeup_word();
        }
    }
    else if(MSG_CMD_INFO_STATUS_POST_CHANGE_ASR_NORMAL_WORD == cmd_info_msg->cmd_info_status)
    {
        /* 切换到正常的命令词网络，这个请求是在播放完唤醒词播报后发出的请求，此时系统已经进入到工作模式（非低功耗），
           这样时切换模型是合理的 */
        /*change asr normal word*/
        change_asr_normal_word();
    }
    else if(MSG_CMD_INFO_STATUS_EXIT_WAKEUP == cmd_info_msg->cmd_info_status)
    {
        /**
         * 由软定时器发起的请求，这里需要判断ignore_exit_wakeup变量，因为存在极端情况识别结果消息和该消息同时进入队列
         * 如果识别消息已经处理，则此时系统状态已经更新，这里便不可继续退出唤醒的动作，直接丢弃消息即可，否则可能覆盖了
         * 唤醒状态而未完全执行完毕所有的唤醒动作，产生错误。而如果此消息先处理，则会被识别消息刷新状态，不会产生问题
         */
        if(ignore_exit_wakeup == 0)
        {
            exit_wakeup_deal(1);
        }
    }
    else if(MSG_CMD_INFO_STATUS_ENABLE_EXIT_WAKEUP == cmd_info_msg->cmd_info_status)
    {
        /**
         * 由处理识别消息的位置发送的此消息，收到此消息后便可以继续接受正常的退出唤醒的消息了，因为系统状态更新的全流程
         * 执行完毕了此时再接受退出唤醒消息刷新系统状态之后完整进行退出唤醒流程是允许的。
         */
        if(ignore_exit_wakeup > 0)
        {
            ignore_exit_wakeup--;
        }
        SN_ASSERT(ignore_exit_wakeup>=0,"ignore_exit_wakeup err\n");
    }
    else if (MSG_CMD_INFO_STATUS_ENABLE_PROCESS_ASR == cmd_info_msg->cmd_info_status)
    {
        if (ignore_asr_msg > 0)
        {
            ignore_asr_msg--;
        }
    }
}


/**
 * @brief 处理asr发送的消息
 *
 * @param asr_msg
 */
void sys_deal_asr_msg(sys_msg_asr_data_t *asr_msg)
{
    cmd_handle_t cmd_handle;

    if(MSG_ASR_STATUS_GOOD_RESULT == asr_msg->asr_status && (ignore_asr_msg == 0))
    {
        int send_cancel_ignore_msg = 0;
		cmd_handle = (cmd_handle_t)asr_msg->asr_cmd_handle;
        if (SYS_STATE_WAKEUP == get_wakeup_state())
        {
            /* 这里已经唤醒了，所以目前已经在队列里的退出唤醒消息无效了，开启这个忽略退出唤醒消息的标志 */
            ignore_exit_wakeup++;
            send_cancel_ignore_msg = 1;
        }

        ciss_set(SN_SS_CMD_SCORE,asr_msg->asr_score);
        if(cmd_info_is_wakeup_word(cmd_handle)) /*wakeup word*/
        {
            /*updata wakeup state*/
            ciss_set(SN_SS_CMD_STATE,SN_SS_CMD_IS_WAKEUP);
            ciss_set(SN_SS_CMD_STATE_FOR_SSP,SN_SS_CMD_IS_WAKEUP);
            enter_wakeup_deal(EXIT_WAKEUP_TIME,cmd_handle);    /*updata wakeup state*/
        }
        else if (SYS_STATE_WAKEUP == get_wakeup_state())
        {
            ciss_set(SN_SS_CMD_STATE,SN_SS_CMD_IS_NORMAL);
            ciss_set(SN_SS_CMD_STATE_FOR_SSP,SN_SS_CMD_IS_NORMAL);
            set_state_enter_wakeup(EXIT_WAKEUP_TIME);

            uint32_t semantic_id = cmd_info_get_semantic_id(cmd_handle);
            uint16_t cmd_id = cmd_info_get_command_id(cmd_handle);
            sn_loginfo(LOG_USER,"asr cmd_id:%d,semantic_id:%08x\n", cmd_id, semantic_id);
            //先按用户ID处理，如果用户没有处理，再按语义ID处理
            if (0 == deal_asr_msg_by_cmd_id(asr_msg, cmd_handle, cmd_id))
            {
                if (0 == deal_asr_msg_by_semantic_id(asr_msg, cmd_handle, semantic_id))
                {                    
                    #if PLAY_OTHER_CMD_EN
                    pause_voice_in();
                    prompt_play_by_cmd_handle(cmd_handle, -1, default_play_done_callback,true);
                    #endif
                }
            }
        }

        if (send_cancel_ignore_msg)
        {
            /* 发送一个取消忽略退出唤醒的消息到队列尾部，处理该消息后就允许退出唤醒了 */
            sys_msg_t send_msg;
            send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
            send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_ENABLE_EXIT_WAKEUP;
            send_msg_to_sys_task(&send_msg, NULL);
        }
        sys_asr_result_hook(cmd_handle, asr_msg->asr_score);
    }
    else if((MSG_ASR_STATUS_NO_RESULT == asr_msg->asr_status)||
        (MSG_ASR_STATUS_VAD_END == asr_msg->asr_status))
    {
        sys_manage_data.asr_state = SYS_STATE_ASR_IDLE;/*idle*/
    }
    else if(MSG_ASR_STATUS_VAD_START == asr_msg->asr_status)
    {
        sys_manage_data.asr_state = SYS_STATE_ASR_BUSY;/*busy*/
    }
    else
    {

    }
}


/**
 * @brief system message deal function and user main UI flow. system message include ASR, player, KEY, COM
 *
 * @param p_arg
 */
void UserTaskManageProcess(void *p_arg)
{
    sys_msg_t rev_msg;
    BaseType_t err = pdPASS;

    /* 上电初始化状态 */
    sys_manage_data.wakeup_state = SYS_STATE_UNWAKEUP;
    ciss_set(SN_SS_WAKING_UP_STATE,SN_SS_NO_WAKEUP);
    ciss_set(SN_SS_WAKING_UP_STATE_FOR_SSP,SN_SS_NO_WAKEUP);
    sys_manage_data.user_msg_state = USERSTATE_WAIT_MSG;
    sys_manage_data.mute_voice_count = 0;

    /* 退出唤醒timer和led pwm控制timer 初始化 */
    exit_wakeup_timer = xTimerCreate("exit_wakeup", pdMS_TO_TICKS(EXIT_WAKEUP_TIME),
        pdFALSE, (void *)0, exit_wakeup_timer_callback);

    if(NULL == exit_wakeup_timer)
    {
        sn_logerr(LOG_USER,"user task create timer error\n");
    }

    #if USE_LOWPOWER_OSC_FREQUENCY
    /* osc时钟低功耗模式使用的timer，用于vad start但未唤醒的超时timer */
    exit_down_freq_mode_timer = xTimerCreate("exit_down_freq_mode", pdMS_TO_TICKS(15000),
        pdFALSE, (void *)0, exit_down_freq_mode_cb);
    #endif

    while(1)
    {
        /* 阻塞接收系统消息 */
        err = xQueueReceive(sys_msg_queue, &rev_msg, portMAX_DELAY);

        if(pdPASS == err)
        {
            /* 根据消息来源来处理对应消息，用户可以自己创建属于自己的系统消息类型 */
            switch (rev_msg.msg_type)
            {
                /* 来自ASR的消息，主要通知识别结果，asr状态 */
                case SYS_MSG_TYPE_ASR:
                {
                    sys_msg_asr_data_t *asr_rev_data;
                    asr_rev_data = &(rev_msg.msg_data.asr_data);
                    sys_deal_asr_msg(asr_rev_data);
                    break;
                }
                /* 系统控制消息，主要通知处理识别模型切换以及低功耗模式切换的请求 */
                case SYS_MSG_TYPE_CMD_INFO:
                {
                    sys_msg_cmd_info_data_t *cmd_info_rev_data;
                    cmd_info_rev_data = &(rev_msg.msg_data.cmd_info_data);
                    sys_deal_cmd_info_msg(cmd_info_rev_data);
                    break;
                }
                /* 音频采集任务消息，目前处理音频采集开启完成，在这里播放欢迎词 */
                case SYS_MSG_TYPE_AUDIO_IN_STARTED:
                {
                    uint8_t volume;
                    uint16_t real_len;

                    /* 从nvdata里读取播放音量 */
                    if(CINV_OPER_SUCCESS != cinv_item_read(NVDATA_ID_VOLUME, sizeof(volume), &volume, &real_len))
                    {
                        /* nvdata内无播放音量则配置为初始默认音量并写入nv */
                        volume = VOLUME_DEFAULT;
                        cinv_item_init(NVDATA_ID_VOLUME, sizeof(volume), &volume);
                    }
                    /* 音量设置 */
                    vol_set(volume);

                    #if (EXCEPTION_RST_SKIP_BOOT_PROMPT)
                    if (RETURN_OK != scu_get_system_reset_state())
                    {
                        /* 本次开机属于异常复位，不需要播放欢迎词直接进入切换非唤醒模式流程 */
                        sys_msg_t send_msg;
                        send_msg.msg_type = SYS_MSG_TYPE_CMD_INFO;
                        send_msg.msg_data.cmd_info_data.cmd_info_status = MSG_CMD_INFO_STATUS_POST_CHANGE_ASR_WAKEUP_WORD;
                        send_msg_to_sys_task(&send_msg, NULL);
                    }
                    else
                    #endif
                    {
                        pause_voice_in();
                        
                        #if PLAY_WELCOME_EN
                        vTaskDelay(pdMS_TO_TICKS(300));     // 等待功放开启

                        /* mute语音输入并播放欢迎词，播放完毕后调用play_exit_wakeup_done_cb回调以开启语音输入并进入切换非唤醒模式流程 */
                        prompt_play_by_cmd_string("<welcome>", -1, play_exit_wakeup_done_cb,true);
                        #else
                        play_exit_wakeup_done_cb(INVALID_HANDLE);
                        #endif
                    }
					userapp_initial();

					/* 通过串口协议发送模块上电通知 */
					sys_power_on_hook();
                    break;
                }
                default:
                    deal_userdef_msg(&rev_msg);
                    break;
            }

            switch(sys_manage_data.user_msg_state)
            {
                case SYS_STATE_WAKUP_TIMEOUT:
                {
                    if(SYS_STATE_ASR_IDLE == get_asr_state())
                    {
                        exit_wakeup_deal(0);
                        sys_manage_data.user_msg_state = USERSTATE_WAIT_MSG;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            // TODO:
            /*timeout for feed watchdog*/
        }
    }
}



