/**
 * @file sn_system_info.h
 * @brief 获取堆栈、任务等信息接口
 * @version 1.0
 * @date 2019-02-21
 * 
 * @copyright Copyright (c) 2019   
 * 
 */
#ifndef _SN_SYSTEM_INFO_H_
#define _SN_SYSTEM_INFO_H_

#include "sn_debug_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

#if CONFIG_DEBUG_EN
#define PRINT_MEM_STATUS()     get_mem_status()
#define PRINT_FMEM_STATUS()    get_fmem_status()
#define PRINT_TASK_STATUS()    get_task_status()
#else
#define PRINT_MEM_STATUS()     do{}while(0)
#define PRINT_FMEM_STATUS()    do{}while(0)
#define PRINT_TASK_STATUS()    do{}while(0)
#endif

/**
 * @ingroup assist
 * @defgroup system_info 系统状态查询
 * @brief 系统task、heap状态查询，用于调试使用
 * @{
 */
extern void get_mem_status(void);
extern void get_fmem_status(void);
extern void get_task_status(void);
//extern void get_task_runtime(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* _SN_SYSTEM_INFO_H_ */
