#ifndef __FAN_H
#define __FAN_H

#include "gd32f4xx.h"

typedef struct fan_i {
	uint8_t speed;
	void (*init)(void);
	void (*set_speed)(uint16_t speed);
}	fan_i;

extern fan_i fan;

#endif
