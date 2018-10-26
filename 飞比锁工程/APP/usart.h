#ifndef __USART_H
#define __USART_H
#include "stm8l15x.h"
#include "global.h"
void UART1_Init(unsigned char remap);

void send_hex(u8 *p,u8 len);
 
 
#endif