#ifndef __BSP_H
#define __BSP_H
#include "stm8l15x.h"

#define Hai_xin

#define ZIGBEE_BAUD (9600)
#define BRR1_data (0x68)
#define BRR2_data (0x02)

void wake_up_in(u8 GPIO_BitVal);
void config_wake_up_out(u8 GPIO_Mode);
u8 get_wakeup_out_pin_state();   //获取wakeup_out pin的状态
u8 get_lock_rx_pin();  //获取本模块rx引脚
void config_lock_rx_pin(u8 GPIO_Mode);  //获取本模块rx引脚
void config_lock_tx_pin(u8 GPIO_Mode);
void set_lock_tx_pin(u8 GPIO_BitVal);
void IOInit();
void init_pin_interrupt(void);
u8 zigbee_usart_send(void);
void LED2(u8 GPIO_BitVal);


#endif 