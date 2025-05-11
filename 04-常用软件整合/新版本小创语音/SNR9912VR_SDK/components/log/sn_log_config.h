/**
 * @file sn_log_config.h
 * @brief sdk log 输出配置宏
 * @version 0.1
 * @date 2019-01-09
 *
 * @copyright Copyright (c) 2019   
 *
 */
#ifndef _SN_LOG_CONFIG_H_
#define _SN_LOG_CONFIG_H_

#include "snr991x_system.h"
#include "sdk_default_config.h"

/* log打印等级 参考Andriod Logcat */
#define SN_LOG_VERBOSE             (1 << 6)
#define SN_LOG_DEBUG               (1 << 5)
#define SN_LOG_INFO                (1 << 4)
#define SN_LOG_WARN                (1 << 3)
#define SN_LOG_ERROR               (1 << 2)
#define SN_LOG_ASSERT              (1 << 1)
#define SN_LOG_NONE                (1 << 0)

/* 打印模块开关 */
#if (CONFIG_SN_LOG_UART != 0)
#define CONFIG_SN_LOG_EN            1
#define CONFIG_SMT_PRINTF_EN        1
#else
#define CONFIG_SN_LOG_EN            0
#define CONFIG_SMT_PRINTF_EN        0
#endif

/*only print long log will save time, because uart fifo so big, 64byte*/
#define UART_LOG_UASED_INT_MODE     0
#define UART_LOG_BUFF_SIZE          512
/* 日志打印使用的串口 */
#define CONFIG_SN_LOG_UART_PORT     ((UART_TypeDef*)CONFIG_SN_LOG_UART)

/* 打印等级 */
#define LOG_ASSERT                  SN_LOG_ERROR
#define LOG_SYS_INFO                SN_LOG_DEBUG

#define LOG_PLAY_CARD               SN_LOG_INFO
#define LOG_SPI                     SN_LOG_INFO
#define LOG_TWDT                    SN_LOG_DEBUG
#define LOG_IWDT                    SN_LOG_DEBUG
#define LOG_SDIO                    SN_LOG_INFO
#define LOG_IIS                     SN_LOG_INFO
#define LOG_CACHE                   SN_LOG_INFO
#define LOG_PM                      SN_LOG_INFO
#define LOG_IIC_DRIVER              SN_LOG_INFO
#define LOG_QSPI_DRIVER             SN_LOG_INFO
#define LOG_GPIO_DRIVER             SN_LOG_INFO
#define LOG_NORMAL_SPI_DRIVER       SN_LOG_DEBUG
#define LOG_NORMAL_SPI_SD           SN_LOG_DEBUG

#define LOG_FLASH_CTL               SN_LOG_INFO
#define LOG_WIFI_EVENT              SN_LOG_DEBUG
#define LOG_DUERAPP                 SN_LOG_INFO
#define LOG_AUDIO_PLAY              SN_LOG_INFO
#define LOG_AUDIO_GET_DATA          SN_LOG_INFO
#define LOG_USER                    SN_LOG_DEBUG
#define LOG_SYS_MONITOR             SN_LOG_INFO
#define LOG_COM_UART                SN_LOG_DEBUG
#define LOG_NVDATA                  SN_LOG_INFO
#define LOG_CMD_INFO                SN_LOG_INFO
#define LOG_FAULT_INT               SN_LOG_VERBOSE
#define LOG_ASR_PCM_BUF             SN_LOG_ERROR
#define LOG_AUDIO_IN                SN_LOG_VERBOSE
#define LOG_ASR_DECODER             SN_LOG_INFO
#define LOG_MIDEA                   SN_LOG_DEBUG
#define LOG_IR                      SN_LOG_INFO
#define LOG_MEDIA                   SN_LOG_DEBUG
#define LOG_OTA                     SN_LOG_INFO
#define LOG_VOICE_UPLOAD            SN_LOG_DEBUG
#define LOG_VOICE_CAPTURE           SN_LOG_DEBUG
#define LOG_SSP_MODULE              SN_LOG_DEBUG
#define LOG_CWSL					SN_LOG_DEBUG
#endif
