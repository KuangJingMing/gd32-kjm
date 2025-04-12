#include "servo.h"
#include "timer.h"

//连接开发板 P6端口

servo_t servo = {
	.init = Servo_Init,
	.set_angle = Servo_Set_Angle,
};

void Servo_Init(void) {
	TIMER4_init(200-1, 16800-1);
  pwm_gpio_config();

}

void Servo_Set_Angle(int16_t duty) {
	uint8_t Duty_cycle;
	if((duty<-90)||(duty>90))
		return;
	Duty_cycle=15+duty/9;
	timer_channel_output_pulse_value_config(TIMER4,TIMER_CH_0,Duty_cycle);
}
