#include "ch455g.h"
#include "iic.h"
#include "oled.h"
#include "stdio.h"
#include "stdlib.h"

// 连接p15端口

//此模块按下会触发 ch455g_callback 回调

static unsigned char keyPressCounts[MAX_KEYS] = {0}; // 初始化计数器数组

IIC_Config ch455g_i2c_init_struct;

keyboard_i keyboard = {
    .init = CH455G_Init,
    .read = Read_Keyboard,
    .get_press_count = Get_Key_Press_Count,
};

static void CH455_Write(unsigned short cmd);

static void CH455G_Init(void) {
  ch455g_i2c_init_struct.scl_pin = GPIO_PIN_4;
  ch455g_i2c_init_struct.scl_port = GPIOA;
  ch455g_i2c_init_struct.scl_rtc = RCU_GPIOA;
  ch455g_i2c_init_struct.sda_pin = GPIO_PIN_5;
  ch455g_i2c_init_struct.sda_port = GPIOA;
  ch455g_i2c_init_struct.sda_rtc = RCU_GPIOA;
  IIC_Init(&ch455g_i2c_init_struct);
  CH455_Write(CH455_8SEG_ON); // 开启显示和键盘
}

static void CH455_Write(unsigned short cmd) // 写命令
{
  IIC_Start(&ch455g_i2c_init_struct); // 启动总线
  IIC_Send_Byte(&ch455g_i2c_init_struct,
                ((unsigned char)(cmd >> 7) & CH455_I2C_MASK) | CH455_I2C_ADDR);
  while (IIC_Wait_Ack(&ch455g_i2c_init_struct))
    ;
  IIC_Send_Byte(&ch455g_i2c_init_struct, (unsigned char)cmd); // 发送数据
  while (IIC_Wait_Ack(&ch455g_i2c_init_struct))
    ;
  IIC_Stop(&ch455g_i2c_init_struct); // 结束总线
}

static unsigned char CH455_Read(void) // 读取按键
{
  unsigned char keycode;
  IIC_Start(&ch455g_i2c_init_struct); // 启动总线
  IIC_Send_Byte(&ch455g_i2c_init_struct,
                (unsigned char)((CH455_GET_KEY >> 7) & CH455_I2C_MASK) | 0x01 |
                    CH455_I2C_ADDR);
  while (IIC_Wait_Ack(&ch455g_i2c_init_struct))
    ;
  keycode = IIC_Read_Byte(&ch455g_i2c_init_struct, 0); // 读取数据
  IIC_Stop(&ch455g_i2c_init_struct);                   // 结束总线
  return keycode;
}

// 这里要轮询读取按键
unsigned char Read_Keyboard(void) {
    static uint8_t read_flag = 0;
    
    static const unsigned char keyMap[MAX_KEYS] = {
        0, 0, 0, 0,  1,  5,  9,  13,
        0, 0, 0, 0,  2,  6,  10, 14,
        0, 0, 0, 0,  3,  7,  11, 15,
        0, 0, 0, 0,  4,  8,  12, 16
    };

    static unsigned char lastKeyID = 0;
    unsigned char keyID = CH455_Read();
    if (keyID != 0 && keyID != lastKeyID) { // 检测按键抬起
        unsigned char temp = keyMap[keyID];
        if (temp != 0) { // 如果按键有效
            if (read_flag == 1) {
                keyPressCounts[temp]++;
                if (keyboard.ch455g_callback != NULL && keyID < MAX_KEYS)  {
                    keyboard.ch455g_callback(keyMap[keyID]); //触发回调
                }
            }
            if (read_flag == 0) read_flag = 1;
        }
    }

    lastKeyID = keyID; // 更新上一次的按键状态
    return keyID;
}

// 获取按键按下次数
unsigned char Get_Key_Press_Count(unsigned char key) {
    if (key < MAX_KEYS) {
        return keyPressCounts[key];
    }
    return 0; // 如果按键无效，返回0
}


//添加数字到 temp_buffer 
void Ch455g_Util_Process_Key_Number(int key_num, char *temp_buffer, size_t buffer_size) {
    const char map[16] = {
        '0', '1', '2', '3',
        '4', '5', '6', '7',
        '8', '9', 'C', 'A',
        '.', 'N', 'N', 'N'
    };
    switch (map[key_num - 1]) {
        case 'N':
            // 什么都不做
            break;
        case 'C':
            // 取消一位
            if (strlen(temp_buffer) > 0) {
                temp_buffer[strlen(temp_buffer) - 1] = '\0'; // 删除最后一个字符
            }
            break;
        case 'A':
            // 删除全部
            temp_buffer[0] = '\0'; // 清空字符串
            break;
        default:
            // 数字添加到 buffer
            if (strlen(temp_buffer) < buffer_size - 1) { // 确保不越界
                size_t len = strlen(temp_buffer);
                temp_buffer[len] = map[key_num - 1]; // 添加字符
                temp_buffer[len + 1] = '\0'; // 结束字符串
            }
            break;
    }
}
