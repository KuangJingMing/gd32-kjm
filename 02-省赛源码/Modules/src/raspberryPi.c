#include "raspberryPi.h"
#include "usart.h"
#include "stdarg.h"

//无此模块，已弃用

//连接P14端口
USART_Config raspberry_pi_usart_config;

void raspberry_init(void)
{
	raspberry_pi_usart_config.af_x = GPIO_AF_7;
	raspberry_pi_usart_config.baud = 9600;
	raspberry_pi_usart_config.nvic_irq = USART1_IRQn;
	raspberry_pi_usart_config.rx_pin = GPIO_PIN_3;
	raspberry_pi_usart_config.rx_port = GPIOA;
	raspberry_pi_usart_config.rx_rtc = RCU_GPIOA;
	raspberry_pi_usart_config.tx_pin = GPIO_PIN_2;
	raspberry_pi_usart_config.tx_port = GPIOA;
	raspberry_pi_usart_config.tx_rtc = RCU_GPIOA;
	raspberry_pi_usart_config.usartx_rtc = RCU_USART1;
	raspberry_pi_usart_config.usart_x = USART1;
	USART_Init(&raspberry_pi_usart_config);
}

void raspberry_print(const char *format, ...) {
	va_list args;
	va_start(args, format);
	USART_printf(&raspberry_pi_usart_config, format, args);
	va_end(args);
}

raspi_i raspi = 
{
	.init = raspberry_init,
	.print = raspberry_print,
};
