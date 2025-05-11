#ifndef __STATUS_SHARE_H__
#define __STATUS_SHARE_H__

#include "sn_log.h"

typedef unsigned long status_t;

#define STATUS_SHARE_MODULE
#define USE_WAIT_FUNC (0)

#define INVALID_STATUS (((status_t)0) - 1) //无效状态值

#define MAX_WAITABLE_STATUS_NUM 24 //可等待状态的最大数量

#define LOG_STATUS_SHARE SN_LOG_WARN

typedef enum
{
    EM_STATUS_ID_START = 0, //普通状态起始ID，本模块内部使用，请不要修改。
    SN_SS_VAD_STATE,        //VAD状态：1、VAD start；2、没有VAD start
    SN_SS_ASR_SYS_STATE,    //ASR系统状态：1、ASR启动；2、ASR未启动（后续可能会增加ASR状态）
    SN_SS_WAKING_UP_STATE,  //当前是否处于唤醒状态：1、处于唤醒状态；2、处于非唤醒状态
    SN_SS_CMD_STATE,        //当前命令词是唤醒词还是命令词：1、唤醒词；2、命令词
    SN_SS_CMD_SCORE,                //当前命令词的分数
    SN_SS_CMD_SCORE_CHA0,           //双通道命令词状态下，通道0分数
    SN_SS_CMD_SCORE_CHA1,           //双通道命令词状态下，通道1分数
    SN_SS_WAKING_UP_STATE_FOR_SSP,  //当前是否处于唤醒状态：1、处于唤醒状态；2、处于非唤醒状态
    SN_SS_CMD_STATE_FOR_SSP,        //当前命令词是唤醒词还是命令词：1、唤醒词；2、命令词
    SN_SS_PLAY_STATE,               //播报状态：1、未播报；2、播报（可能会增加，本地播报或者网络播报，播放什么格式等的状态）
    SN_SS_FLASH_HOST_STATE,
    SN_SS_FLASH_BNPU_STATE,
    SN_SS_MIC_VOICE_STATUE, //MIC输入的语音的状态：0、未启动；1、正常状态；2、mute状态
    SN_SS_ALC_STATE,        //ALC开启关闭状态：0、关闭；1、开启
    SN_SS_CWSL_OUTPUT_FLAG,  //CWSL是否有识别结果输出：0、无识别结果；1、已有识别结果输出
    SN_SS_INTERCEPT_ASR_OUT, //是否拦截ASR输出：0、不拦截；1、拦截

    //带同步等待状态功能的状态ID
    EM_STATUS_WAITABLE_START, //本模块内部使用，请不要修改
    EM_STATUS_WAITABLE_1,

    //下面的值用于统计状态ID数量，本模块内部使用，请不要修改。
    EM_STATUS_NUM,
} status_id_t;

typedef enum
{
    SN_SS_VAD_IDLE = 0, //VAD状态检测为非人声
    SN_SS_VAD_START,
    SN_SS_VAD_ON, //VAD状态检测为人声
    SN_SS_VAD_END
} ci_ss_vad_state_t;

typedef enum
{
    SN_SS_ALC_OFF = 0, //ALC处于关闭状态
    SN_SS_ALC_UP,      //ALC处于开启状态
} ci_ss_alc_state_t;
typedef enum
{
    SN_SS_ASR_SYS_POWER_OFF = 0, //ASR还未启动
    SN_SS_ASR_SYS_STARTED_UP,    //ASR启动了
} ci_ss_asr_sys_state_t;

typedef enum
{
    SN_SS_NO_WAKEUP = 0, //当前系统处于非唤醒状态
    SN_SS_WAKEUPED,      //当前系统处于唤醒阶段
} ci_ss_wakeup_state_t;

typedef enum
{
    SN_SS_CMD_IS_NULL = 0,
    SN_SS_CMD_IS_WAKEUP, //当前的命令词是唤醒词
    SN_SS_CMD_IS_NORMAL,     //当前的命令词是非唤醒词
} ci_ss_cmd_state_t;

typedef enum
{
    SN_SS_PLAY_STATE_IDLE = 0, //当前未播报
    SN_SS_PLAY_STATE_PLAYING,  //当前处于播报状态
} ci_ss_play_state;

typedef enum
{
    SN_SS_FLASH_POWER_OFF = 0,
    SN_SS_FLASH_IDLE,
    SN_SS_FLASH_READ,
    SN_SS_FLASH_WRITE,
    SN_SS_FLASH_ERASE,
    SN_SS_FLASH_READ_UNIQUE_ID,
} ci_ss_flash_state;

typedef enum
{
    SN_SS_MIC_VOICE_NOT_START = 0,
    SN_SS_MIC_VOICE_NORMAL,
    SN_SS_MIC_VOICE_MUTE,
} ci_ss_mic_voice_state;

/**
 * @brief 信息共享模块初始化.
 */
void ciss_init(void);

/**
 * @brief 设置状态信息，如果状态有变化且是可等待状态，发送状态等待事件标志位.
 * 
 * @param id 状态信息标识，指定要设置的状态.
 * @param value 要设置的状态值.
 */
void ciss_set(status_id_t id, status_t value);

/**
 * @brief 读取状态信息.
 * 
 * @param id 状态信息标识，指定要读取的状态.
 * @return 要读取的状态值.
 */
status_t ciss_get(status_id_t id);

#if 0
/*/**
 * @brief 
 * 
 */
 * 
 * @param id 状态信息标识，指定要等待的状态.
 * @param value 要等待的状态值.
 * @param xTicksToWait 等待的超时时间.
 */
int ciss_wait(status_id_t id, status_t value, TickType_t xTicksToWait);
#endif

#endif
