#ifndef __ALCOHOL_H__
#define __ALCOHOL_H__

#include "stdint.h"

void Alcohol_Test(void);
void Alcohol_Init(void);
int  Alcohol_Get_Value(void);

typedef struct alcohol_t {
	void (*init)(void);
	void (*test)(void);
	int (*get_val)(void);
} alcohol_t;

extern alcohol_t alcohol;

#endif 
