/*
 * ds18b20.h
 *
 * created: 10/1/2025
 *  author: 
 */

#ifndef _DS18B20_H
#define _DS18B20_H

#ifdef __cplusplus
extern "C" {
#endif

#include "syslib.h"

#define DS18B20_PIN GPIO_PIN_15
#define DS18B20_PORT GPIOD
#define DS18B20_RTC RCU_GPIOD

typedef struct ds18b20_t {
    unsigned char (*init)(void);
    float (*get_temperature)(void);
} ds18b20_t;

extern ds18b20_t ds18b20;

unsigned char DS18B20_Initialize(void);    // 初始化 DS18B20
float DS18B20_GetTemperature(void);

void DS18B20_SetPinOutput(int value);
void DS18B20_SetPinInput(void);
void DS18B20_SetPinAsOutput(void);
void DS18B20_Reset(void);
unsigned char DS18B20_CheckPresence(void);

#ifdef __cplusplus
}
#endif

#endif // _DS18B20_H
