/*
 * task_manager.h
 *
 * created: 4/1/2025
 *  author: 
 */

#ifndef _TASK_MANAGER_H
#define _TASK_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "stdbool.h"

#define MAX_TASK_ACTION_SIZE 5

typedef void (*func_)(void *);

typedef struct {
    void (*task_func)(void *arg);  // 任务处理函数
    void (*release_func)(void);    // 释放函数（可为空）
    const char *task_name;         // 任务名称
    uint8_t priority;              // 优先级
    TaskHandle_t *handler;         // 指向对应任务句柄
		uint8_t task_start_cmd;        // 任务启动命令
		uint8_t task_stop_cmd; 				 // 任务终止命令
		func_ actions[MAX_TASK_ACTION_SIZE];
		uint8_t actions_cmds[MAX_TASK_ACTION_SIZE];
} TaskInfo;

bool start_task(TaskFunction_t taskFunction, const char *taskName, UBaseType_t priority, TaskHandle_t *taskHandle);
void stop_task(TaskHandle_t *taskHandle, const char *taskName, void (*releaseResources)(void));


#ifdef __cplusplus
}
#endif

#endif // _TASK_MANAGER_H
