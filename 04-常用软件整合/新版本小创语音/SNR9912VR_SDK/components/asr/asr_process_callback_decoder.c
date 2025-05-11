/**
  ******************************************************************************
  * @文件    asr_process_callbak.c
  * @版本    V1.0.1
  * @日期    2019-3-15
  * @概要  asr 回调函数，VAD相关
  ******************************************************************************
  * @注意
  *
  *  
  *
  ******************************************************************************
  */
#include <stdlib.h>
#include <stdio.h>

#include "sdk_default_config.h"
#include "platform_config.h"
#include "sn_log_config.h"
#include "asr_process_callback_decoder.h"
#include "system_msg_deal.h"
#include "status_share.h"
// #include "voiceprint_port.h"

#include "sn_log.h"


int send_asr_prediction_msg(uint32_t cmd_handle)
{
    return 0;
}

int send_asr_pre_cancel_msg(void)
{
    return 0;
}

int send_asr_pre_confirm_msg(uint32_t cmd_handle)
{
    return 0;
}


extern int confir_asr_prediction_msg(int cmd_handle);

// #pragma GCC optimize("O0")
int asr_result_callback(callback_asr_result_type_t *asr)
{
    mprintf("send result:%s %d\n", asr->cmd_word, asr->confidence);
    #if USE_CWSL
    // 如果已有CWSL输出，就忽略此次ASR输出
    if(ciss_get(SN_SS_CWSL_OUTPUT_FLAG) == 1)
    {
        mprintf("hit cwls output\n");
        return 0;
    }
    #endif

#if (!DEBUG_ASR_NOT_PLAY)
    if (INVALID_HANDLE != asr->cmd_handle)
    {
        #if USE_AEC_MODULE
        if (!(cmd_info_is_wakeup_word(asr->cmd_handle)))
        {
            if (ciss_get(SN_SS_INTERCEPT_ASR_OUT))
            {
                return 0;
            }
        }
        #endif

        confir_asr_prediction_msg(asr->cmd_handle);
        sys_msg_t send_msg;
        send_msg.msg_type = SYS_MSG_TYPE_ASR;
        send_msg.msg_data.asr_data.asr_status = MSG_ASR_STATUS_GOOD_RESULT;
        send_msg.msg_data.asr_data.asr_cmd_handle = asr->cmd_handle;
        mprintf("asr->cmd_handle is %x\n",(asr->cmd_handle));
        send_msg.msg_data.asr_data.asr_score = asr->confidence;
        send_msg.msg_data.asr_data.asr_pcm_base_addr = asr->asrvoice_ptr;
        send_msg.msg_data.asr_data.asr_frames = asr->vocie_valid_frame_len;
        send_msg_to_sys_task(&send_msg, NULL);
    }
    else
    {
        mprintf("asr->cmd_handle is INVALID_HANDLE\n");
    }
#endif
    return 0;
}


int asr_lite_result_callback(char * words,short cfd)
{
    mprintf("out2 :%s %d\n",words,cfd);
    /*add code for app*/
    
    return 0;
}


/***************** (C) COPYRIGHT    *****END OF FILE****/
