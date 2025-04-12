#include "infrared_pyroelectric.h"
#include "oled.h"

//连接到开发板P9端口

ir_t ir = {
	.init = IR_Init,
	.get_state = IR_Get_State,
	.test = IR_Test,
};

void IR_Init(void)
{
	rcu_periph_clock_enable(IR_RTC);

	gpio_mode_set(IR_GPIO_Port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, IR_Pin);
}

void IR_Test(void)
{
	if(IR){ 
		oled.show_str(25,4," 状态: 无人 ",16);
	}else{
		oled.show_str(25,4," 状态: 有人  ",16);
	}
	
}

uint8_t IR_Get_State(void) {
	return (uint8_t)IR;
}

