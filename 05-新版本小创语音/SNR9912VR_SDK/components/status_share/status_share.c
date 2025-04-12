/**
 * @file status_share.c
 * @brief 示例程序
 * @version 1.0.0
 * @date 2021-12-16
 *
 * @copyright Copyright (c) 2021    
 *
 */

#include <stdio.h>
#include "FreeRTOS.h"
#include "semphr.h" 
#include "event_groups.h"
#include "status_share.h"
#include "snr991x_system_ept.h"
#include "snr991x_nuclear_com.h"


typedef struct
{
    EventGroupHandle_t event_group_handle[MAX_WAITABLE_STATUS_NUM/24];
    SemaphoreHandle_t semaphore;
}status_module_info_t;


#ifdef STATUS_SHARE_MODULE
volatile static status_t status_list[EM_STATUS_NUM] = {0};
#endif


#if USE_WAIT_FUNC
static status_module_info_t status_module_info;

status_t waitting_status[EM_STATUS_NUM - EM_STATUS_WAITABLE_START] = { INVALID_STATUS};
#endif



/**
 * @brief 信息共享模块初始化.
 */
void ciss_init(void)
{
    #if USE_WAIT_FUNC
    status_module_info.semaphore = xSemaphoreCreateMutex();
    for (int i=0;i < MAX_WAITABLE_STATUS_NUM/24;i++)
    {
        status_module_info.event_group_handle[i] = xEventGroupCreate();
    }
    #endif
    void other_func_rpmsg_init(void);
    other_func_rpmsg_init();
    sn_loginfo(LOG_STATUS_SHARE, "status_list addr = %08x\n",(uint32_t)status_list);
}

/**
 * @brief 读取状态信息.
 * 
 * @param id 状态信息标识，指定要读取的状态.
 * @return 要读取的状态值.
 */
status_t ciss_get(status_id_t id)
{
    if (id < EM_STATUS_NUM)
    {
        return status_list[id];
    }
    else
    {
        return INVALID_STATUS;
    }
}

/**
 * @brief 设置状态信息，如果状态有变化且是可等待状态，发送状态等待事件标志位.
 * 
 * @param id 状态信息标识，指定要设置的状态.
 * @param value 要设置的状态值.
 */
void ciss_set(status_id_t id, status_t value)
{
    if (id < EM_STATUS_NUM)
    {
        // xSemaphoreTake(status_module_info.semaphore, portMAX_DELAY);
        if (status_list[id] != value)
        {
            status_list[id] = value;
            #if USE_WAIT_FUNC
            if (id >= EM_STATUS_WAITABLE_START)
            {
                int event_group_index = (id - EM_STATUS_WAITABLE_START - 1);
                if (waitting_status[event_group_index] == value)
                {
                    xEventGroupSetBits(
                        status_module_info.event_group_handle[event_group_index/24],
                        1<<(event_group_index%24));
                }
            }
            #endif
        }
        // xSemaphoreGive(status_module_info.semaphore);
    }
}

/**
 * @brief 带阻塞功能，等待状态改变为指定的状态值才返回.
 * 
 * @param id 状态信息标识，指定要等待的状态.
 * @param value 要等待的状态值.
 * @param xTicksToWait 等待的超时时间.
 */
#if 0
int ciss_wait(status_id_t id, status_t value, TickType_t xTicksToWait)
{
    #if USE_WAIT_FUNC
    int event_group_index = (id - EM_STATUS_WAITABLE_START - 1);
    while (status_list[id] != value)
    {        
        xSemaphoreTake(status_module_info.semaphore, portMAX_DELAY);
        waitting_status[event_group_index] = value;
        xSemaphoreGive(status_module_info.semaphore);
        if (pdTRUE != xEventGroupWaitBits(
                status_module_info.event_group_handle[event_group_index/24],
                1<<(event_group_index%24),
                pdTRUE,
                pdTRUE,
                xTicksToWait))
        {
            waitting_status[event_group_index] = INVALID_STATUS;
            return 0;
        }
    }
    waitting_status[event_group_index] = INVALID_STATUS;
    return 1;
    #else
    return 0;
    #endif
}
#endif


#if 0
void TaskWaitStatus(void *p_arg)
{
    mprintf("TaskWaitStatus start\n");
    while(1)
    {
        if (ciss_wait(EM_STATUS_WAITABLE_1, 32, portMAX_DELAY))
        {
            mprintf("wait EM_STATUS_WAITABLE_1 successful\n");
            ciss_set(EM_STATUS_WAITABLE_1, 0);
        }
        else
        {
            mprintf("wait EM_STATUS_WAITABLE_1 failed\n");
        }
        //vTaskDelay(1);
    }
}

int status_share_test(void)
{
    mprintf("status module test start\n");
    ciss_init();
    ciss_set(EM_STATUS_2, 555);
    mprintf("set EM_STATUS_2 555\n");
    status_t v = ciss_get(EM_STATUS_2);
    mprintf("get EM_STATUS_2 %d\n", v);
    xTaskCreate(TaskWaitStatus, "TaskWaitStatus", 64, NULL, 4, NULL );
}
#endif


#define OTHER_FUNC_SERVE_MAX_PARA_NUM    (8)
uint32_t other_func_msg_buf[OTHER_FUNC_SERVE_MAX_PARA_NUM] = {0};

int32_t other_func_serve_cb(void *payload, uint32_t payload_len, uint32_t src, void *priv)
{
    // if(payload_len > sizeof(other_func_msg_buf))
    // {
    //     mprintf("payload_len = %d\n",payload_len);
    //     SN_ASSERT(0,"\n");
    // }
    // MASK_ROM_LIB_FUNC->newlibcfunc.memcpy_p((void*)other_func_msg_buf,(void*)payload,payload_len);
    uint32_t* data = (uint32_t*)payload;

    #if THIS_MODULE_PRINT_OPEN
    mprintf("V I %d\n",data[0]);
    #endif
    switch(data[0])
    {
        case ciss_init_ept_num:
        {
            uint32_t* addr_p = (uint32_t*)data[1];
            *addr_p = (uint32_t)status_list;
            break;
        }
        default:
            break;
    }
    return 0;
}


void other_func_rpmsg_init(void)
{
    nuclear_com_registe_serve(other_func_serve_cb,other_func_inner_serve_ept_num);
}


