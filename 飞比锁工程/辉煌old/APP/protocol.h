#ifndef __PROTOCOL_H
#define __PROTOCOL_H
#include "stm8l15x.h"



#define ZIGBEE_BAUD 57600


#define UART_CMD_ADDR 1
#define DATA_ADDR 4


/*******zigbee命令定义********/
enum {
  
  face_recognition_sucess = 0x00,
  fingerprint_unlock_sucess = 0x01,
  password_unlock_sucess =0x02,
  IC_card_unlock_sucess =0x03,
  key_unlock_sucess = 0x04,
  remote_unlock_sucess =0x05,
  telecontroller_unlock_sucess = 0x06,
  dynamic_password_unlock_sucess = 0x07,
  set_up_dynamic_password_success =0x08,
  set_up_face_recognition_sucess = 0x09,
  set_up_fingerprint_unlock_sucess = 0x0a,
  set_up_password_unlock_sucess = 0x0b,
  set_up_IC_card_unlock_sucess = 0x0c,
  set_up_telecontroller_unlock_sucess = 0x0d,
  set_up_network_sucess = 0x0e,
  cancel_face_recognition_sucess = 0x0f,
  cancel_fingerprint_unlock_sucess = 0x10,
  cancel_password_unlock_sucess = 0x11,
  cancel_IC_card_unlock_sucess = 0x12,
  cancel_telecontroller_unlock_sucess = 0x13,
  cancel_network_sucess = 0x14,
  factory_reset_sucess = 0x15,
  accessory_lock_lowpower_alarm = 0x16,
  close_lock_without_antilock = 0x17,
  duress_do_alarm = 0x18,
  low_power_alarm = 0x19,
  pry_lock =0x1a,
  illegal_user = 0x1b,
  trial_erro_alarm = 0x1c,
  lock_not_closed = 0x1d,
  ring_bell = 0x1e,
  close_lock_and_antilock = 0x1f,
  scaning_module = 0x20,
  set_up_linkage = 0x21,
  cancell_linkage = 0x22,
  check_module_if_online = 0x23
 
};


/********* zigbee 模块协议 宏定义 **************/
#define ZIGBEE_CMD_ADDR 0x01

#define CMD_CRC_ERRO 1
#define CMD_LEN_ERRO 1
#define ZIGBEE_CMD_ACK 0x00

#define ZIGBEE_CMD_ADD_USER 0x13  
#define ZIGBEE_CMD_DELETE_USER 0x14  
#define ZIGBEE_CMD_MODIFY_USER 0x15  

#define ZIGBEE_CMD_OPENNET_FAIL 0x95
#define ZIGBEE_CMD_OPENNET_SUCEESS 0x93

#define ZIGBEE_CMD_OPENLOCK 0x18
#define ZIGBEE_CMD_OPENLOCK_BYFORCE 0x22
#define ZIGBEE_CMD_LOCAL_OPEN_LOCK 0x80
#define ZIGBEE_CMD_OPENNET 0x40
#define ZIGBEE_CMD_CLOSENET 0x41
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

/******* 锁协议宏定义 ************************/

#define LOCK_CMD_ADDR 0x03

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






#endif 