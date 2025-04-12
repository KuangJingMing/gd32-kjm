#ifndef __SERVO_H__
#define __SERVO_H__

#include "stdint.h"

void Servo_Init(void);
void Servo_Set_Angle(int16_t duty);

typedef struct servo_t {
	void (*init)(void);
	void (*set_angle)(int16_t);
} servo_t;

extern servo_t servo;

#endif 
