#include "lock.h"

// PD15 连接至P7端口

static void Lock_Init(void) {
  rcu_periph_clock_enable(LOCK_RTC);
  gpio_mode_set(LOCK_GPIO_Port, GPIO_MODE_OUTPUT, GPIO_PUPD_PULLDOWN, LOCK_Pin);
  gpio_output_options_set(LOCK_GPIO_Port, GPIO_OTYPE_PP, GPIO_OSPEED_200MHZ,
                          LOCK_Pin);
  gpio_bit_write(LOCK_GPIO_Port, LOCK_Pin, RESET);
}

static void Lock_Control(unsigned char level) {
  if (level) {
    gpio_bit_write(LOCK_GPIO_Port, LOCK_Pin, SET);
  } else {
    gpio_bit_write(LOCK_GPIO_Port, LOCK_Pin, RESET);
  }
}

lock_i lock = {
    .init = Lock_Init,
    .control = Lock_Control,
		.state = 0,
};
