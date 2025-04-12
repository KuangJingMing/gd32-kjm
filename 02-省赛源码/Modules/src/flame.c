#include "flame.h"

//连接 P9 端口

void FLAME_Init(void) {
  rcu_periph_clock_enable(FLAME_RTC);

  gpio_mode_set(FLAME_GPIO_Port, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, FLAME_Pin);
}

void FLAME_Test(void) {
  oled.show_str(30, 4, " 状态: ", 16);
  if (FLAME) {
    oled.show_str(70, 4, " 无火 ", 16);
  } else {
    oled.show_str(70, 4, " 有火 ", 16);
  }
}

uint8_t FLAME_Get(void) { return !FLAME; }

flame_i flame = {
    .init = FLAME_Init,
    .test = FLAME_Test,
    .get = FLAME_Get,
};
