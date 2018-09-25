#ifndef __USERDATA_H
#define __USERDATA_H
#include "stm8l15x.h"

#define LOW_BATTERY_LEVEL  30


/*** 锁用户数据的各个地址 *******/

#define lock_user_No_addr (0x1000)  //用户编号地址 
#define lock_user_No_len (2)  //用户编号长度
#define lock_username_addr (0x1002)  //用户名国际编码地址
#define lock_username_len (8)   //用户名国际编码长度
#define lock_user_attribute_addr (0x100a) //用户属性地址
#define lock_user_attribute_len (1) //用户属性长度
#define lock_start_validtime_addr  (0x100b) //用户生效开始时间
#define lock_start_validtime_len  (3) //用户有效起始时间
#define lock_end_validtime_addr  (0x100e) //用户有效结束时间
#define lock_end_validtime_len     (3) //用户有效结束时间
#define lock_everyday_validtime_addr  (0x1011) //用户每天生效开始时间
#define lock_everyday_validtime_len      (12) //用户每天有效起始时间
#define lock_everyweek_validtime_addr  (0x1023) //用户每周有效结束时间
#define lock_everyweek_validtime_len      (1) //用户每周有效结束时间
#define lock_allowed_counts_addr       (0x1024) //用户允许的开门次数地址
#define lock_allowed_counts_len           (2) //用户允许的开门次数的长度 
#define lock_type_addr                 (0x1026) //电子钥匙的种类
#define lock_type_len                    (1) //电子钥匙的种类
#define lock_psw_addr                 (0x1027) //钥匙数据的地址
#define lock_psw_len                     (7)//钥匙数据的长度
#define auto_hand_closelock_addr            (0x102e)//自动手动关锁地址
#define auto_hand_closelock_len               (1)//自动手动关锁长度
#define openlock_holdtime_addr            (0x102f) //开锁保持时间的地址
#define openlock_holdtime_len               (1) //开锁保持时间的地址
#define sound_volume_addr                (0x1030) //音量地址
#define sound_volume_len                    (1) //音量地址
#define chinese_english_language_addr     (0x1031) //中文英文切换地址 
#define chinese_english_language_len         (1) //中文英文切换长度 
#define wireless_open_close_addr       (0x1032) //无线开关
#define wireless_open_close_len            (1) //无线开关
#define lock_authority_addr            (0x1033) //开锁权限地址
#define lock_authority_len               (1) //开锁权限地址
#define lock_clock_addr            (0x1034) //锁内时间地址
#define lock_clock_len                  (7) //锁内时间长度
#define lock_battery_level_addr        (0x103b) //电池1电量百分比
#define lock_battery1_len                  (1) //电池1电量百分比
#define lock_battery_leve2_addr        (0x103c) //电池2电量百分比
#define lock_battery2_len                  (1) //电池2电量百分比
#define lock_switch_state_addr      (0x103d) //开关状态的地址                                                                                                                    
#define lock_switch_state_len            (2) //开关状态的地址   


//*********临时用户的编号**********/
#define casual_work_addr      (0x1040) //开关状态的地址

/*** zigbee用户数据的各个地址 *******/
#define zigbee_clock_addr (0x1080)
#define zigbee_clock_len (6)
#define zigbee_user_attribute_addr (0x1086) //用户属性地址
#define zigbee_user_attribute_len (1) //用户属性地址
#define zigbee_user_No_addr (0x1087)  //用户编号地址 
#define zigbee_user_No_len  (1) //用户编号长度
#define zigbee_psw_len_addr     (0x1088)  //密码长度的地址
#define zigbee_psw_len_len     (0x01)  //密码长度的长度
#define zigbee_psw_addr  (0x1089) //密码地址
#define zigbee_psw_len    (10) /*********需要从falsh中读取 ***********/
#define zigbee_psw_start_validtime_addr  (0x1095) //密码有效起始时间
#define zigbee_psw_start_validtime_len  (6) //密码有效起始时间
#define zigbee_psw_end_validtime_addr   (0x109b)  //密码有效截至时间
#define zigbee_psw_end_validtime_len   (6)  //密码有效截至时间









/*****锁数据类型****************/
enum {
 lock_user_No =0 ,
 lock_username =1,
 lock_user_attribute =2,
 lock_keytype =3,
 lock_password=4,
 lcok_power_level_state =5,
 lock_square_tongue_locked_state = 6,
 lock_anti_lock_state =7,
 lock_motor_state = 8


};

/*****Zigbee数据类型****************/

enum {
 zigbee_user_No =0 ,
 zigbee_username =1,
 zigbee_user_attribute =2,
 zigbee_clock =3,
 zigbee_password = 4
 

};








/******库函数**********/
void write_userdata2eeprom(u16 addr, u8 *ptr, u8 len);
void read_userdata4eeprom(u16 addr, u8 *ptr, u8 len);
u8 u8_BCD_2_hex(u8 data);
unsigned char u16_BCD_2_hex(short data);
u16 u8_HEX_2_BCD(u8 data);
void lockdata_2_zigbeedata( u8 lockdata_type,u8 *lockdata,u8 *zigbeedata);
void zigbeedata_2_lockdata(u8 zigbeedata_type,u8 *lockdata,u8 *zigbeedata);
u8 u8_HEX_2_byte_BCD(u8 data);

#endif 