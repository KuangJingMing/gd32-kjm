#ifndef SMOKE_H__
#define SMOKE_H__

void Smoke_Init(void);
int Smoke_Get_Val(void);
void Smoke_Test(void);

typedef struct smoke_t {
	void (*init)(void);
	void (*test)(void);
	int (*get_val)(void);
} smoke_t;

extern smoke_t smoke;

#endif 
