#ifndef __RTC_H__
#define __RTC_H__

#include "stdint.h"

// 时间结构体
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} RTC_Time;

void rtc_clock_config(void);
void rtc_set_time(RTC_Time new_time);
RTC_Time rtc_get_time(void);

#endif
