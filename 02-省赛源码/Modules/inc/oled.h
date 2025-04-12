#ifndef __OLED_H_
#define __OLED_H_

#include "gd32f4xx.h"

#define I2C_OLED 0x78
#define IIC_OLED_CMD 0x00
#define IIC_OLED_DATA 0x40

typedef struct oled_i {
  void (*init)(void);
  void (*clear)(void);
  void (*show_char)(uint16_t x, uint16_t y, uint8_t num, uint8_t wsize);
  void (*show_string)(uint16_t x, uint16_t y, char *p, uint8_t wsize);
  void (*show_int32_num)(uint16_t x, uint16_t y, int32_t num, uint8_t len,
                         uint8_t wsize);
  void (*draw_font16)(uint16_t x, uint16_t y, char *s);
  void (*draw_font32)(uint16_t x, uint16_t y, char *s);
  void (*show_str)(uint16_t x, uint16_t y, char *str, uint8_t wsize);
	void (*draw_bmp)(unsigned char, unsigned char, unsigned char, unsigned char, unsigned char[]);
	void (*print)(uint16_t, uint16_t, uint8_t, char *, ...);
} oled_i;

extern oled_i oled;

#endif
