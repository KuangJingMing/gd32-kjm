/*
 * c1016.h
 *
 * created: 20/12/2024
 *  author: 
 */

#ifndef _C1016_H
#define _C1016_H

#ifdef __cplusplus
extern "C" {
#endif

#define C1016_PIN GPIO_PIN_15
#define C1016_PORT GPIOA
#define C1016_RCU RCU_GPIOA

#include "stdint.h"
#include "stdbool.h"

/*********************
 *      颜色常量定义
 *********************/
#define C1016_COLOR_MODE_RESPIRE 1
#define C1016_COLOR_MODE_QUAICT_RESPIRE 2
#define C1016_COLOR_MODE_NORMALLY_OPEN 3
#define C1016_COLOR_MODE_NORMAL_CLOSE 4
#define C1016_COLOR_MODE_GRADUALLY_OPEN 5
#define C1016_COLOR_MODE_GRADUALLY_CLOSE 6
#define C1016_COLOR_MODE_SLOW_RESPIRE 7

#define C1016_COLOR_GREEN 1
#define C1016_COLOR_RED 2
#define C1016_COLOR_BLUE 4
#define C1016_COLOR_PURPLE 6     // 紫色 (红 + 蓝)
#define C1016_COLOR_CYAN 5       // 青色 (绿 + 蓝)
#define C1016_COLOR_YELLOW 3     // 黄色 (红 + 绿)
#define C1016_COLOR_WHITE 7      // 白色 (红 + 绿 + 蓝)

#define C1016_MAX_ID_SIZE 50

#define RX_BUFFER_SIZE 256

/*********************
 *      颜色结构体
 *********************/
typedef struct {
    uint8_t color_mode;
    uint8_t color_start_value;
    uint8_t color_end_value;
    uint8_t color_loop;
} C1016_COLOR_Struct;

extern C1016_COLOR_Struct c1016_color;

typedef struct c1016_t {
    uint8_t enroll_state;
    uint8_t identify_id;
    void (*show_log)(const char *, ...);
    void (*init)(void);
    void (*enroll)(void);
    void (*identfify)(void);
    void (*delete_id)(uint16_t, uint16_t);
    void (*show_format_ids)(void);
} c1016_t;

extern c1016_t c1016;

void C1016_Init(void);
void C1016_SetColor(C1016_COLOR_Struct *p);
void C1016_cmd_Delete(uint16_t start, uint16_t end);
void C1016_Enroll(void);
void C1016_Identify(void);
void C1016_Print_Format_Ids(void);

//下面是注册和识别用的判断函数
bool check_id_registered(void);
bool capture_fingerprint_image(const char* image_stage, int buffer_index);
bool save_fingerprint_image(int buffer_index);
bool merge_fingerprint_images(void);
bool save_fingerprint_template(void);
bool C1016_search_fingerprint(uint16_t start, uint16_t end);

extern unsigned char CMD_GET_STATU[26];
extern unsigned char CMD_FINGER_DETECT[26];
extern unsigned char CMD_GET_IMAGE[26];
extern unsigned char CMD_GENERATE[26];
extern unsigned char CMD_MERGE[26];
extern unsigned char CMD_STORE_CHAR[26];
extern unsigned char CMD_DEL_CHAR[26];
extern unsigned char CMD_SEARCH[26];
extern unsigned char CMD_VERIFY[26];
extern unsigned char CMD_ENTER_IAP_MODEX[26];
extern volatile uint8_t c1016_rx_buffer[RX_BUFFER_SIZE];

#ifdef __cplusplus
}
#endif

#endif // _C1016_H
