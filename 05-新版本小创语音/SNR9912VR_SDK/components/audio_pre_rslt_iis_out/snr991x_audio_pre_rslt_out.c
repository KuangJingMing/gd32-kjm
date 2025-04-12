#include "codec_manager.h"
#include "snr991x_audio_pre_rslt_out.h"
#include <string.h>
#include "snr991x_codec.h"
#include "snr991x_dpmu.h"
#include "board.h"
#include "snr991x_gpio.h"
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sdk_default_config.h"


#define BUFFER_NUM (4)

typedef struct
{
    audio_pre_rslt_out_init_t init_str;
    //写了多少次数据
    uint32_t write_data_cnt;
    //已经发送了多少次数据，写数据频次是固定的，我们不能改变，只能改变发送数据的频度，
    //如果发送太快，会在mute模式再发送上一个buf，
    //如果发送太慢，会扔掉一个buf不发送
    uint32_t send_data_cnt;
    
    int32_t write_send_sub_slave;//在slave模式下，write和send的差值
    //是否使用硬件tx merge功能
    bool hardware_tx_merge;
}audio_pre_init_tmp_t;


static audio_pre_init_tmp_t sg_init_tmp_str;


#define PI (3.1416926f)
void sine_wave_generate(int16_t* sine_wave, uint32_t sample_rate,uint32_t wave_fre,uint32_t point_num)
{
    int16_t num_of_one_period = sample_rate/wave_fre;//每个周期的采样点数
    for(int i=0;i<point_num;i++)
    {
        sine_wave[i] = (int16_t)(32767.0f * sinf( (2*PI*i) / num_of_one_period ));
    }
}


void audio_pre_rslt_out_play_card_init(void)
{
    #if USE_IIS1_OUT_PRE_RSLT_AUDIO
    uint16_t block_size = AUDIO_CAP_POINT_NUM_PER_FRM * 2 * sizeof(int16_t);
    
    sg_init_tmp_str.init_str.block_size = block_size;
    sg_init_tmp_str.write_data_cnt = 0;
    sg_init_tmp_str.send_data_cnt = 0;

    audio_pre_rslt_out_codec_init();
    #endif
}


/**
 * @brief 写数据到发送端
 * 
 * @param rslt 处理结果的起始指针
 * @param origin 原始数据的起始指针
 *  此函数不可用于中断
 */
void audio_pre_rslt_write_data(int16_t* left,int16_t* right)
{
    #if USE_IIS1_OUT_PRE_RSLT_AUDIO
    uint32_t block_size = sg_init_tmp_str.init_str.block_size;

    uint32_t write_pcm_addr = 0;
    cm_get_pcm_buffer(PLAY_PRE_AUDIO_CODEC_ID,&write_pcm_addr,0);    //TODO HSL
    int16_t* pcm_data_p = (int16_t*)write_pcm_addr;
    if(0 == write_pcm_addr)
    {
        return;
    }

    int num = block_size/sizeof(int16_t)/2;

    for(int i=0;i<num;i++)
    {
        pcm_data_p[2*i] = left[i];
        pcm_data_p[2*i + 1] = right[i];
    }
    
    cm_write_codec(PLAY_PRE_AUDIO_CODEC_ID, (void*)write_pcm_addr,0);

    sg_init_tmp_str.send_data_cnt++;

    sg_init_tmp_str.write_data_cnt++;

    #endif
}


/**
 * @brief 语音前处理输出停止
 * 
 */
void audio_pre_rslt_stop(void)
{
    #if USE_IIS1_OUT_PRE_RSLT_AUDIO
    cm_stop_codec(PLAY_PRE_AUDIO_CODEC_ID, CODEC_OUTPUT);
    #endif
}


/**
 * @brief 语音前处理输出开始
 * 
 */
void audio_pre_rslt_start(void)
{
    #if USE_IIS1_OUT_PRE_RSLT_AUDIO
    cm_start_codec(PLAY_PRE_AUDIO_CODEC_ID, CODEC_OUTPUT);
    #endif
}

/********** (C) COPYRIGHT    *****END OF FILE****/
