/*
 * mlx90614.h
 *
 * created: 11/1/2025
 *  author:
 */

#ifndef _MLX90614_H
#define _MLX90614_H

#ifdef __cplusplus
extern "C"
{
#endif

#define ACK 0
#define NACK 1 // 不应答或否定的应答

#define SA 0x00           // 从机地址，单个MLX90614时地址为0x00,多个时地址默认为0x5a
#define RAM_ACCESS 0x00    // RAM access command
#define EEPROM_ACCESS 0x20 // EEPROM access command
#define RAM_TOBJ1 0x07     // To1 address in the eeprom
#define RAM_TA 0x06

typedef struct mlx90614_t {
    void (*init)(void);
    float (*get_temp)(void);
} mlx90614_t;

extern mlx90614_t mlx90614;

void SMBus_Init(void);
float SMBus_ReadTemp(void);


#ifdef __cplusplus
}
#endif

#endif // _MLX90614_H
