#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "stm8l15x.h"


#define  protocal_header (0xf5)
#define  protocal_tail (0x55)
#define protocal_len  (100 )

/*******串口接收函数超时时间*********/

#define UART_TIMEOUT (20)




/********* zigbee 模块协议 宏定义 **************/
#define ZIGBEE_CMD_ADDR 0x04

#define CMD_CRC_ERRO 1
#define CMD_LEN_ERRO 1
#define ZIGBEE_CMD_ACK 0x00

#define ZIGBEE_CMD_ADD_USER 0x13  
#define ZIGBEE_CMD_DELETE_USER 0x14  
#define ZIGBEE_CMD_MODIFY_USER 0x15  

#define ZIGBEE_CMD_OPENNET_FAIL 0xA8
#define ZIGBEE_CMD_OPENNET_SUCEESS 0xA0
#define ZIGBEE_CMD_OPENNET_FAIL_2 0x94
#define ZIGBEE_CMD_OPENLOCK 0x18
#define ZIGBEE_CMD_OPENLOCK_BYFORCE 0x22
#define ZIGBEE_CMD_LOCAL_OPEN_LOCK 0x80
#define ZIGBEE_CMD_OPENNET 0xa0
#define ZIGBEE_CMD_CLOSENET 0xA8
#define ZIGBEE_CMD_inqure_factory_id 0xB1
#define ZIGBEE_CMD_LOW_POWER 0x30
#define ZIGBEE_CMD_CLOCK_SYNC 0x10
#define ZIGBEE_CMD_ADMIN_IDENTIFICATION 0x11
#define ZIGBEE_CMD_INQURE_USERINFO 0x12
#define ZIGBEE_CMD_INQURE_LOCK_STATE 0x17
#define ZIGBEE_CMD_ADDUSER_FROM_LOCK 0x33

#define ACK_ZIGBEE_ILLAGEL_USER_REPORT 0x20
#define ACK_ZIGBEE_CMD_INQURE_LOCK_STATE  0x3B
#define ACK_ZIGBEE_CMD_LOW_POWER 0X30
#define ACK_ZIGBEE_CMD_UPLOAD 0X3D
#define ACK_ZIGBEE_CMD_DELETE_USER  0X34
#define ACK_ZIGBEE_MODIFY_USERINFO 0x35
#define ACK_ZIGBEE_FORMAT 0x36
#define ACK_ZIGBEE_CMD_LOCAL_OPEN_LOCK 0x80
#define ACK_ZIGBEE_CMD_OPENNET_SUCESS 0x40
#define ACK_ZIGBEE_CMD_OPENLOCK 0x54
/********* zigbee 模块协议 宏定义 **************/


/******锁串口命令地址字节*******/
#define UART_CMD_ADDR 1
#define DATA_ADDR 4
#define LOCK_CMD_ADDR 0x03
#define LOCK_ACK_RES_ADDR 0x04
/******* 锁协议宏定义 ************************/
#define LOCK_DELETE_USER 0x02
#define LOCK_INQUIRY_USER_INFO 0x12
#define LOCK_LOCK_BE_OPENED 0X15
#define LOCK_OPENNET 0X1C
#define LOCK_CLOSENET 0X1D
#define LOCK_OPENLOCK 0x06
#define LOCK_OPENLOCK_BYFORCE 0x1A
#define LOCK_LOGIN_SAFE_MODE 0x1B
#define LOCK_LOGOUT_SAFE_MODE 0x9B
#define LOCK_LOGOUT_SETTING_MODE 0x26
#define LOCK_ALARM 0X1E

#define LOCK_USER_ADDED 0x21
#define LOCK_FORMAT 0x23
#define LCOK_LOCK_BE_CLOSED 0X16
#define LOCK_STATE_UPDATA 0X17
#define LOCK_illgal_user_REPORT 0x18
#define LOCK_delete_USER 0x24
#define LOCK_ADD_USER_FROM_WIRELESS 0X01
#define LOCK_CLOCK_CALIBRATION 0X08
#define LOCK_CMD_INQURE_LOCK_STATE 0x09

#define ACK_LOCK_INQUIRY_USER_INFO 0x92
#define ACK_LOCK_CMD_INQURE_LOCK_STATE  0X89
#define ACK_LOCK_OPENLOCK 0X86
#define ACK_OPENLOCK_BYFORCE 0X9a
#define ACK_LOCK_OPENNET 0X9c

/******* 锁协议宏定义 ************************/

/******函数对外接口 **********************/
u8 exe_zigbee_cmd(u8 zigbee_cmd);
u8 exe_lock_cmd(u8 lock_cmd);

u8 send_zigbeecmd(u8 len,u8 zigbeecmd,u8 *data_buff );
u8 check_zigbee_cmd(u8 zigbee_cmd);
u8  zigbee_protocal_component(u8 *buf,u16 len,u8 cmd,u8* data_buff);
u8 check_code(u8 *buf);
u16 sum_check(u8 *p,u8 len);
u8 transparently_process(void);





/******ZIGBEE模块功能 **********************/
// #define ZIGBEE_CMD_ADD_USER_FUNC
// #define ZIGBEE_CMD_DELETE_USER_FUNC
// #define ZIGBEE_CMD_MODIFY_USER_FUNC
// #define ZIGBEE_CMD_OPENNET_FAIL_FUNC
// #define ZIGBEE_CMD_OPENNET_SUCEESS_FUNC
// #define ZIGBEE_CMD_OPENLOCK_FUNC
// #define ZIGBEE_CMD_CLOCK_SYNC_FUNC
// #define ZIGBEE_CMD_INQURE_USERINFO_FUNC
// #define ZIGBEE_CMD_INQURE_LOCK_STATE_FUNC
// #define ZIGBEE_CMD_ADMIN_IDENTIFICATION_FUNC
// #define ZIGBEE_CMD_OPENNET_FAIL_2_FUNC
#define ZIGBEE_CMD_inqure_factory_id_FUNC
// #define ZIGBEE_CMD_ONLINE_INFO_FUNC


/******LOCK模块功能 **********************/

#define LOCK_OPENNET_FUNC
// #define LOCK_CLOSENETFUNC
// #define LOCK_LOCK_BE_OPENED_FUNC
// #define LCOK_LOCK_BE_CLOSED_FUNC
// #define LOCK_OPENLOCK_BYFORCE_FUNC
// #define LOCK_LOGIN_SAFE_MODE_FUNC
// #define LOCK_LOGOUT_SAFE_MODE_FUNC
// #define LOCK_LOGOUT_SETTING_MODE_FUNC
// #define LOCK_USER_ADDED_FUNC
// #define LOCK_STATE_UPDATA_FUNC
//  #define LOCK_delete_USER_FUNC
// #define LOCK_illgal_user_REPORT_FUNC
// #define LOCK_FORMAT_FUNC
// #define LOCK_ALARM_FUNC


/***********宏定义函数****/

#define  ACK_zigbee_openlock_composite_data(cmd,len)  { \
         cmd =  ZIGBEE_CMD_ACK; \
         len =0x01; \
} 
#define  ACK_zigbee_openlock_fail_composite_data(cmd,len)  { \
         len = 0x01; cmd = 0x0D ;}


#endif 