#include "fan.h"
#include "timer.h"

// 连接至P6端口

void Fan_Init(void) {
  pwm_gpio_config();
  TIMER4_init(167, 999);
}

void Fan_Set_Speed(uint16_t duty) {
  if (duty > 1000) {
    duty = 1000;
  }
  timer_channel_output_pulse_value_config(TIMER4, TIMER_CH_0, duty);
}

fan_i fan = {
    .init = Fan_Init,
    .set_speed = Fan_Set_Speed,
		.speed = 0,
};
