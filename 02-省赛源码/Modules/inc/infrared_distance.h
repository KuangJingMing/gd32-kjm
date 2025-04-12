#ifndef __INFRARED_DISTANCE_H__
#define __INFRARED_DISTANCE_H__


typedef struct {
	void (*init)(void);
	float (*get_distance)(void);
} infrared_distance_t;

extern infrared_distance_t infrared_distance;

void Infrared_Distance_Init(void);
float Infrared_Distance_Get_Value(void);

#endif 
