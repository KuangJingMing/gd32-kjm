/*
 * hmc5883l.h
 *
 * created: 7/1/2025
 *  author: 邝
 */

#ifndef _HMC5883L_H
#define _HMC5883L_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gd32f4xx_it.h"

typedef enum {
    DIRECTION_NORTH = 0,    // 北
    DIRECTION_NORTHEAST,    // 东北
    DIRECTION_EAST,         // 东
    DIRECTION_SOUTHEAST,    // 东南
    DIRECTION_SOUTH,        // 南
    DIRECTION_SOUTHWEST,    // 西南
    DIRECTION_WEST,         // 西
    DIRECTION_NORTHWEST     // 西北
} CompassDirection;

typedef struct hmc5883l {
		uint8_t output_str[255];
    void (*init)(void);
    uint8_t (*show_compass)(void);
    double (*get_angle)(void);
} hmc5883l_t;

extern hmc5883l_t hmc5883l;

void HMC5883L_Init(void);
void HMC5883L_READ(int16_t *x, int16_t *y);
void HCM5883L_Init(void);
double HCM5883L_Get_Angle(void);
uint8_t Show_CompassVal(void);

#ifdef __cplusplus
}
#endif

#endif // _HMC5883L_H
