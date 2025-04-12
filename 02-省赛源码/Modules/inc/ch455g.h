#ifndef __CH455G_H
#define __CH455G_H

#include "gd32f4xx.h"
#include "math.h"
#include "oled.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "systick.h"

#define MAX_KEYS 32

// ��ȡ������������
#define CH455_GET_KEY 0x0700 // ��ȡ����,���ذ�������

#define CH455_BIT_ENABLE 0x01  // ����/�ر�λ
#define CH455_BIT_SLEEP 0x04   // ˯�߿���λ
#define CH455_BIT_7SEG 0x08    // 7�ο���λ
#define CH455_BIT_INTENS1 0x10 // 1������
#define CH455_BIT_INTENS2 0x20 // 2������
#define CH455_BIT_INTENS3 0x30 // 3������
#define CH455_BIT_INTENS4 0x40 // 4������
#define CH455_BIT_INTENS5 0x50 // 5������
#define CH455_BIT_INTENS6 0x60 // 6������
#define CH455_BIT_INTENS7 0x70 // 7������
#define CH455_BIT_INTENS8 0x00 // 8������

#define CH455_SYSOFF 0x0400                            // �ر���ʾ���رռ���
#define CH455_SYSON (CH455_SYSOFF | CH455_BIT_ENABLE)  // ������ʾ������
#define CH455_SLEEPOFF CH455_SYSOFF                    // �ر�˯��
#define CH455_SLEEPON (CH455_SYSOFF | CH455_BIT_SLEEP) // ����˯��
#define CH455_7SEG_ON (CH455_SYSON | CH455_BIT_7SEG)   // �����߶�ģʽ
#define CH455_8SEG_ON (CH455_SYSON | 0x00)             // �����˶�ģʽ
#define CH455_SYSON_4                                                          \
  (CH455_SYSON | CH455_BIT_INTENS4) // ������ʾ�����̡�4������
#define CH455_SYSON_8                                                          \
  (CH455_SYSON | CH455_BIT_INTENS8) // ������ʾ�����̡�8������

// CH455�ӿڶ���
#define CH455_I2C_ADDR 0x40 // CH455�ĵ�ַ
#define CH455_I2C_MASK 0x3E // CH455�ĸ��ֽ���������

typedef struct keyboard_i {
  void (*init)(void);
  uint8_t (*read)(void);
  uint8_t (*get_press_count)(uint8_t);
  void (*ch455g_callback)(uint8_t);
} keyboard_i;

extern keyboard_i keyboard;

static void CH455G_Init(void);
unsigned char Read_Keyboard(void);
unsigned char Get_Key_Press_Count(unsigned char key);

#endif
