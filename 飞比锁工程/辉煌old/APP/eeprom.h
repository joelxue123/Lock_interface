#ifndef __EEPROM_H
#define __EEPROM_H
#include "stm8l15x.h"


extern void EEPROM_multbyte_read(u32 start_address , u8 *data,u8 len);
void EEPROM_multbyte_write(u32 start_address , u8 *data,u8 len);

#endif