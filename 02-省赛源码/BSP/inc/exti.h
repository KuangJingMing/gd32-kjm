#ifndef __EXTI_H
#define __EXTI_H

#include "gd32f4xx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ultrasonic.h"

void my_EXTI_Init(void);

extern volatile uint32_t start_time;
extern volatile uint32_t pulse_width;
extern volatile uint8_t echo_start ;


#endif 

