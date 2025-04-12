/**
  ******************************************************************************
  * @file    voice_in_manage_inner.c
  * @version V1.0.0
  * @date    2021.11.02
  * @brief  在HOST端接收
  ******************************************************************************
  */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "codec_manage_inner_port.h"
#include "codec_manager.h"
#include "audio_in_manage.h"

#include "sdk_default_config.h"
#include "snr991x_system_ept.h"
#include "audio_in_manage_inner.h"
#include "snr991x_nuclear_com.h"
#include "status_share.h"
#include "system_msg_deal.h"
#include "snr991x_audio_pre_rslt_out.h"
#include "romlib_runtime.h"
#include "alg_preprocess.h"
#include "snr991x_nuclear_com.h"

/**************************************************************************
                    typedef
****************************************************************************/

/**************************************************************************
                    global
****************************************************************************/

/**************************************************************************
                    function
****************************************************************************/

static void send_audio_data_to_asr(uint32_t buf, uint32_t size)
{
    extern int get_asrtop_asrfrmshift(void);
    int frm_shif = get_asrtop_asrfrmshift();
    int frms = size / (frm_shif * sizeof(short));
    extern int send_runvad_msg(unsigned int voice_ptr, int voice_frm_nums, int irq_flag);
    send_runvad_msg(buf, frms, 0);
}

static void audio_in_iis_callback(void)
{
    uint32_t iis3_reg[7] = {0};
    for (int i = 0; i < 7; i++)
    {
        iis3_reg[i] = *(volatile uint32_t *)((uint32_t)IIS2 + 4 * i);
    }
    //scu_set_device_reset((uint32_t)IIS2);
    //scu_set_device_reset_release((uint32_t)IIS2);
    for (int i = 0; i < 7; i++)
    {
        *(volatile uint32_t *)((uint32_t)IIS2 + 4 * i) = iis3_reg[i];
    }
    mprintf("IIS watchdog callback\n");
}

static void audio_in_ept_cal_serve_rpmsg(char *para, uint32_t size)
{
    nuclear_com_t str;
    // MASK_ROM_LIB_FUNC->newlibcfunc.memset_p((void*)&str,0,sizeof(str));
    str.src_ept_num = 0;
    str.dst_ept_num = audio_in_serve_ept_num;
    str.data_p = (void *)para;
    str.data_len = size;
    nuclear_com_send(&str, 0xfffff);
}

void send_voice_info_to_audio_in_manage(audio_in_get_voice_t *str_p)
{
    uint32_t para[6];
    para[0] = send_voice_info_to_audio_in_manage_ept_num;
    para[1] = str_p->cha_num;
    para[2] = str_p->channel_flag;
    para[3] = str_p->addr1;
    para[4] = str_p->addr2;
    para[5] = str_p->size;
    audio_in_ept_cal_serve_rpmsg((char *)para, sizeof(para));
}


void set_ssp_registe(audio_capture_t* audio_capture, ci_ssp_st* ci_ssp, int module_num)
{
	uint32_t para[4];
    para[0] = set_ssp_registe_ept_num;
    para[1] = (uint32_t)audio_capture;
    para[2] = (uint32_t)ci_ssp;
    para[3] = (uint32_t)module_num;
    audio_in_ept_cal_serve_rpmsg((char *)para, sizeof(para));
}


void audio_in_manage_inner_task(void *p)
{
    //打印中间结果
    //ctr_asr_detail_result(1);

    // extern void set_nn_to_dtw_init_info(bool cinn_is_using_to_dtw,uint16_t max_num_to_dtw);
    // set_nn_to_dtw_init_info(true,20);

    while (SN_SS_ASR_SYS_POWER_OFF == ciss_get(SN_SS_ASR_SYS_STATE))
    {
        vTaskDelay(100);
        mprintf("wait asr start done\n");
    }

    char asr_ver_buf[100] = {0};

    extern int get_asr_sys_verinfo(char *version_buf);
    get_asr_sys_verinfo(asr_ver_buf);

    sn_loginfo(LOG_USER, "asr_ver:[%s]\n", asr_ver_buf);

    cm_start_codec(HOST_MIC_RECORD_CODEC_ID, CODEC_INPUT);
    ciss_set(SN_SS_MIC_VOICE_STATUE, SN_SS_MIC_VOICE_NORMAL);

    sys_msg_t send_msg;
    send_msg.msg_type = SYS_MSG_TYPE_AUDIO_IN_STARTED;
    send_msg_to_sys_task(&send_msg, NULL);

    #if 1//(USE_IIS1_OUT_PRE_RSLT_AUDIO)
    audio_pre_rslt_out_play_card_init();
    #endif
    //语音前端算法处理模块初始化
    
    
    for (;;)
    {
        uint32_t data_addr = 0, data_size;
        cm_read_codec(HOST_MIC_RECORD_CODEC_ID, &data_addr, &data_size);
        if (!data_addr)
        {
            mprintf("iisdma int too slow\n");
            continue;
        }

        audio_in_get_voice_t voice_info_str;
        voice_info_str.cha_num = 1;
        voice_info_str.channel_flag = 1;
        voice_info_str.addr1 = data_addr;
        voice_info_str.addr2 = 0;
        voice_info_str.size = data_size;

        
        send_voice_info_to_audio_in_manage(&voice_info_str);
    }
}
