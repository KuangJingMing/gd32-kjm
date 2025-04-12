/*!
    \file  systick.c
    \brief the systick configuration file
*/

/*
    Copyright (C) 2017 GigaDevice

    2014-12-26, V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
    2017-06-19, V3.1.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "systick.h"
#include "freertos.h"
#include "gd32f4xx.h"
#include "task.h"
#include "timer.h"



TaskDelay_t delay_tasks[MAX_DELAY_US_TASKS] = {0};
volatile uint32_t global_count = 0;
volatile uint8_t timer_is_running = 0;  // 定时器状态，0 表示停止，1 表示运行。

/*!
    \brief      configure systick
    \param[in]  none
    \param[out] none
    \retval     none
*/
void systick_config(void) {
  /* systick clock source is from HCLK/8 */
  systick_clksource_set(SYSTICK_CLKSOURCE_HCLK_DIV8);
}

/*!
    \brief      delay a time in microseconds in polling mode
    \param[in]  count: count in microseconds
    \param[out] none
    \retval     none
*/



// 微秒级延时函数
void delay_1us(uint32_t us) {
    // 如果延时大于 1000 微秒（1 毫秒）
    if (us >= 1000) {
        // 用 vTaskDelay 处理毫秒级部分延时（任务挂起，让出 CPU）
        vTaskDelay((us / 1000) / portTICK_PERIOD_MS); // us 转换为 ticks
        us = us % 1000;                               // 保留剩余的微秒部分
    }

    // 微秒级延时部分
    if (us > 0) {
#ifdef USE_BLOCKING_DELAY_US
        // 阻塞式延时：使用忙等待实现高精度微秒延时
        timer_enable(TIMER3);
        volatile uint32_t start_count;
        taskENTER_CRITICAL();
        start_count = global_count; // 获取起始计数
        taskEXIT_CRITICAL();

        while (global_count - start_count < us) {
            // 忙等待，注意这会占用 CPU
        }
        timer_disable(TIMER3);
#else
				us /= 2; //这个地方必须除以二freertos的api产生的开销会让1us变慢
			
        // 非阻塞式延时：使用任务通知机制
        TaskHandle_t current_task = xTaskGetCurrentTaskHandle();
        uint32_t target_time;

        // 计算目标时间，保护 global_count
        taskENTER_CRITICAL();
        target_time = global_count + us;
        taskEXIT_CRITICAL();

        // 在 delay_tasks 数组中寻找空闲槽位或当前任务的槽位
        int i;
        for (i = 0; i < MAX_DELAY_US_TASKS; i++) {
            if (delay_tasks[i].task_handle == NULL || delay_tasks[i].task_handle == current_task) {
                delay_tasks[i].task_handle = current_task;
                delay_tasks[i].target_time = target_time;
                delay_tasks[i].active = 1; // 设置任务为活跃状态
                break;
            }
        }

        // 如果没有可用槽位，记录错误并返回（可根据需求修改行为）
        if (i == MAX_DELAY_US_TASKS) {
            // 可以添加日志或错误处理
						LOG_I("delay_1us error");
            return;
        }
			
        // 启用定时器（如果尚未运行）
        taskENTER_CRITICAL();
        if (!timer_is_running) {
            timer_enable(TIMER3);
            timer_is_running = 1;
        }
        taskEXIT_CRITICAL();

        // 阻塞任务，等待延时完成
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
#endif
    }
}

/*!
    \brief      delay a time in milliseconds in polling mode
    \param[in]  count: count in milliseconds
    \param[out] none
    \retval     none
*/
void delay_1ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
