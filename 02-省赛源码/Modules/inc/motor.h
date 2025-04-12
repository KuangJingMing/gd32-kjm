#ifndef MOTOR_H__
#define MOTOR_H__

#include "stdint.h"

// 电机驱动 GPIO 和 定时器 配置
#define MOTOR_RCU  							RCU_GPIOA
#define MOTOR_TIMER_RCU					RCU_TIMER2 
#define MOTOR_TIMER             TIMER2
#define MOTOR_GPIO_PORT         GPIOA
#define MOTOR_IN1_PIN           GPIO_PIN_6
#define MOTOR_IN2_PIN           GPIO_PIN_7  
#define MOTOR_TIMER_CH1         TIMER_CH_0  // 将 CH2 定义为电机控制通道1
#define MOTOR_TIMER_CH2         TIMER_CH_1  // 将 CH3 定义为电机控制通道2

// 编码器 GPIO 配置
#define ENCODER_RCU 						RCU_GPIOA
#define ENCODER_GPIO_PORT       GPIOA
#define ENCODER_H1_PIN          GPIO_PIN_2 
#define ENCODER_H2_PIN          GPIO_PIN_3

// 编码器参数
#define ENCODER_PPR             1000 // 编码器每转脉冲数 (PPR)

typedef struct {
	void (*init)(void);
	void (*set_speed)(int16_t);
	float (*get_rpm)(void);
} motor_t;

void motor_init(void);  // 初始化电机
void motor_gpio_init(void);  // 初始化电机GPIO
void motor_timer_init(void);  // 初始化电机定时器
void encoder_init(void);  // 初始化编码器
void set_motor_speed(int16_t speed);  // 设置电机速度
float get_motor_speed(void);  // 获取电机转速

extern motor_t motor;
extern volatile int32_t encoder_count;

#endif
