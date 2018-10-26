#ifndef __GLOBAL_H
#define __GLOBAL_H
#include "stm8l15x.h"

#define ON 1
#define OFF 0

#define Re_map ((u8)1)

extern u16 sys_timer;
extern u16 deadline ;
extern u8 Wfi_Mode;

#define Wakeup_OUT_PORT GPIOB  
#define Wakeup_OUT_PIN  GPIO_Pin_0

#define Wakeup_IN_PORT GPIOC
#define Wakeup_IN_PIN  GPIO_Pin_4

#define TX_REMAP_PORT GPIOA 
#define TX_REMAP_PIN GPIO_Pin_2

#define DEBUG 0

extern u8 G_receive_data[];
extern u8 G_send_data[];

typedef struct
{
    u8 receive_len;
    u8* receive_data;
    u8 send_len;
    u8* send_data; 
    u8 receive_flag;
    u8 receive_fail;
    u8 send_fail;
    u8 receive_enable;
    u8 command;
} BFCT_protocol;



/*********接收数组的长度***************/
#define USART_LEN 200


/* 保存全局变量的地址，eeprom*/
#define battery_percent_addr  0x1000   


extern BFCT_protocol BFCT_protocol_Lock;
extern BFCT_protocol BFCT_protocol_Zigbee;

#define SUCCESS  0x00
#define FAIL     0xff

#define UPLOAD   0x00  
#define DOWNLOAD 0X01


/******锁协议错误标准 ***********/
#define crc_8n_ERRO_BIT 0X01
#define CMD_LOST_BIT    0X02
#define LOCK_BUSY_BIT   0X04
/******锁协议错误标准 ***********/


/******Zigbee协议错误标准 ***********/
#define sum_check_BIT       0X01
#define NACK_BIT           0x02
#define TIMEOUT_BIT        0x08 
/******Zigbee协议错误标准 ***********/

extern u8 lock_interrupt;
extern u8 zigbee_interrupt ;

extern u8 Zigbee_process_done;
extern u8 Lock_process_done;

extern u8 Zigbee_processing_flag ;
extern u8 Lock_processing_flag ;

extern u8 lock_erro;
extern u8 zigbee_erro;

extern u8 t_1ms;
extern u16 data3_tc;
extern u8 transparently_flag;
#endif



