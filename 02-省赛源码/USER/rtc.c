#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "rtc.h"
#include <stdio.h>


// 当前RTC时间
static RTC_Time current_time = {2025, 3, 5, 9, 35, 1};

// 定义定时器句柄
TimerHandle_t xTimer = NULL;

// 闰年判断函数
int is_leap_year(uint16_t year) {
    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        return 1; // 闰年
    }
    return 0; // 平年
}

// 获取一个月的天数
uint8_t get_days_in_month(uint8_t month, uint16_t year) {
    // 月份的天数：1, 3, 5, 7, 8, 10, 12 是31天
    uint8_t days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    // 如果是2月，考虑闰年
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    
    return days_in_month[month];
}

// 定时器回调函数，每秒调用一次
void vTimerCallback(TimerHandle_t xTimer) {
    // 增加秒数
    current_time.second++;

    // 处理秒的进位
    if (current_time.second >= 60) {
        current_time.second = 0;
        current_time.minute++;

        // 处理分钟的进位
        if (current_time.minute >= 60) {
            current_time.minute = 0;
            current_time.hour++;

            // 处理小时的进位
            if (current_time.hour >= 24) {
                current_time.hour = 0;
                current_time.day++;

                // 获取当前月份的最大天数
                uint8_t days_in_month = get_days_in_month(current_time.month, current_time.year);

                // 处理日期的进位
                if (current_time.day > days_in_month) {
                    current_time.day = 1;
                    current_time.month++;

                    // 处理月份的进位
                    if (current_time.month > 12) {
                        current_time.month = 1;
                        current_time.year++;
                    }
                }
            }
        }
    }
}

// RTC配置函数
void rtc_clock_config(void) {
    // 创建定时器，每秒触发一次
    xTimer = xTimerCreate("RTC_Timer",           // 定时器名称
                          pdMS_TO_TICKS(1000),   // 定时器周期：1000ms = 1秒
                          pdTRUE,                // 自动重载
                          0,                     // 定时器ID
                          vTimerCallback);       // 回调函数

    if (xTimer != NULL) {
        // 启动定时器
        xTimerStart(xTimer, 0);
    } else {
        // 定时器创建失败
        printf("Failed to create RTC timer!\n");
    }
}

// 获取当前时间接口
RTC_Time rtc_get_time(void) {
    return current_time;
}

// 设置当前时间接口
void rtc_set_time(RTC_Time new_time) {
    current_time = new_time;
    printf("Time set to: %04d-%02d-%02d %02d:%02d:%02d\n", current_time.year, current_time.month, current_time.day,
						current_time.hour, current_time.minute, current_time.second);
}
