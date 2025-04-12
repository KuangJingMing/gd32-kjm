#ifndef RASPBERRY_PI_H__
#define RASPBERRY_PI_H__

typedef struct raspi_i
{
	void (*init)(void);
	void (*print)(const char *format, ...);
} raspi_i;

extern raspi_i raspi;

#endif 
