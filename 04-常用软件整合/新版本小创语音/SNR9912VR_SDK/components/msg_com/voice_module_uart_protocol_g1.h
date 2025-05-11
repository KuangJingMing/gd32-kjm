#ifndef _COM_TASK_
#define _COM_TASK_
#include "sdk_default_config.h"

#if (UART_PROTOCOL_VER == 1)

//--defined 
#define UART_HEADER0    0xA5
#define UART_HEADER1    0xFA
#define UART_END        0xFB
#if USER_UART_USE_UART1
#if ((DEBUG_PRINTF_FUNC_EN == 0) && (AUTOTEST_PRINTF_FUNC_EN == 1))
    #define UART2_BAUDRATE_DEFAULT UART_BaudRate115200
#else
	#define UART2_BAUDRATE_DEFAULT UART_BaudRate9600
#endif
#else
    #define UART2_BAUDRATE_DEFAULT B9600
#endif
//--define use by rx irq
#define SOP_STATE1      0x00
#define SOP_STATE2      0x01
#define DATA_STATE     0x02

//--CI send cmd.
typedef enum
{
    SN_TX_START =   0x80, //start   
	SN_TX_ASR	    ,  
    SN_TX_UNWAKEN	    , 
    SN_TX_VER,
    SN_TX_REV,
    SN_TX_END       ,      //--end
}CMD_SN_SEND;
//--CI recive cmd.
typedef enum
{
    SN_RX_START	    =   0,//--start
	SN_RX_ECHO_EN	    ,
    SN_RX_CHK_EN        ,
    SN_RX_PLAY_INDEX    ,
    SN_RX_RESET    ,
    SN_RX_UNWAKE_CMD    ,
    SN_RX_MUTE,
    SN_RX_VOLSET ,
    SN_RX_GETVER,
    SN_RX_REV,
    SN_RX_WAKEN,
    SN_RX_CHANGE_GROUP,
    SN_RX_END   ,//--end
}CMD_SN_RECIVED;

typedef struct
{
    unsigned char header0;//header
    unsigned char header1;
    unsigned char id;
    unsigned char cmd;
    unsigned char data0;
    unsigned char data1;
    unsigned char chksum;//check sum= add "header0~dta1"
    unsigned char end; 
}sys_msg_com_data_t;

////////////////////////////////////
typedef struct{
  //  char Baudrat;//目前的baudrate  1:9600  2:14400 3:19200 4:56000 5:38400 6:57600 7:115200 其他数值，115200
    char echo;//是否回复收到的数据 
    char vol_set;//上位机通知调整音量
    char chk_enable;//使用校验
    char play_by_uart; //是否是串口发送的播报, 1的话则可直接播报
}UART_USER_SETTING_Typedef;

extern UART_USER_SETTING_Typedef gs_uar2_user;
extern void uart_int_rx_handle(unsigned char rxdata);
extern void userapp_deal_com_msg(sys_msg_com_data_t *com_data);
extern void uart_send_AsrResult(unsigned int index,float score);
extern void uart_send_exit_wakeup(void);
extern int uart_communicate_init(void);

#endif //(UART_PROTOCOL_VER == 1)

#endif