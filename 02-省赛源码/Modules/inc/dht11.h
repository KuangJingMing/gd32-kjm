#ifndef __DHT11_H
#define __DHT11_H

#include "gd32f4xx.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "systick.h"
#include "oled.h"

#define DQ_RTC 			RCU_GPIOC
#define DQ_GPIO_Port	GPIOC
#define DQ_Pin 			GPIO_PIN_7

//IO��������
#define DHT11_IO_IN()    DQ_IN()
#define DHT11_IO_OUT()   DQ_OUT()

////IO��������			  					    			   
#define	DHT11_DQ_OUT_1 	gpio_bit_write(DQ_GPIO_Port,DQ_Pin,SET)	
#define	DHT11_DQ_OUT_0 	gpio_bit_write(DQ_GPIO_Port,DQ_Pin,RESET)
#define	DHT11_DQ_IN()  	gpio_input_bit_get(DQ_GPIO_Port,DQ_Pin)

typedef struct dht11_i {
	uint16_t (*get_temperature)(void);
	uint16_t (*get_humidity)(void);
	void (*init)(void);
	void (*read)(void);
	void (*test)(void);
} dht11_i;


void DHT11_Update(void);
uint8_t DHT11_Read_Data(uint16_t *temp,uint16_t *humi);	//��ȡ��ʪ��
uint8_t DHT11_Read_Byte(void);				//����һ���ֽ�
uint8_t DHT11_Read_Bit(void);				//����һ��λ
uint8_t DHT11_Check(void);					//����Ƿ����DHT11
void DHT11_Rst(void);						//��λDHT11   
void DQ_OUT(void);
void DQ_IN(void);
void DHT11_WHILE(void);
void DHT11_Init(void);
void DHT11_Test(void);

extern dht11_i dht11;

#endif 
