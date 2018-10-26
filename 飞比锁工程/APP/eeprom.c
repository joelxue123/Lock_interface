#include "eeprom.h"
#include "stm8l15x_iwdg.h"
/******
*函数名 EEPROM_multbyte_write
*函数描述：读EEPROM的字节
*参数： start_address ：起始地址 。 0x1000开始， 256个字节
*data,写入的缓冲区   。 len :写入的长度
**********/
void EEPROM_multbyte_write(u32 start_address , u8 *data,u8 len)
{
  u8 count;
 // FLASH_SetProgrammingTime(FLASH_ProgramTime_TProg); //设定编程时间为标准编程时间
  FLASH->CR1 &= (uint8_t)(~FLASH_CR1_FIX);
  FLASH->CR1 |= (uint8_t)FLASH_ProgramTime_TProg;
 //MASS 密钥，解除EEPROM的保护
 // FLASH_Unlock(FLASH_MemType_Data);
    FLASH->DUKR = FLASH_RASS_KEY2; /* Warning: keys are reversed on data memory !!! */
    FLASH->DUKR = FLASH_RASS_KEY1;

  //向一块写入128个字节数据
  for(count = 0; count < len ; count++)
  {
    *((unsigned char *)(start_address + count)) = data[count];
    if ((WWDG->CR & 0x7F) < WWDG->WR) //小于窗口值才能喂狗  
    WWDG->CR |= 0x7F; //重新喂狗 
  }
// while(FLASH_GetFlagStatus(FLASH_FLAG_WR_PG_DIS) == 1);
  while(FLASH->IAPSR  & (uint8_t)FLASH_FLAG_WR_PG_DIS);

//  while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == 1);
  while(FLASH->IAPSR  & (uint8_t)FLASH_FLAG_EOP);
  if((WWDG->CR & 0x7F) < WWDG->WR) //小于窗口值才能喂狗  
    WWDG->CR |= 0x7F; //重新喂狗 
}

/******
*函数名 EEPROM_multbyte_read
*函数描述：读EEPROM的字节
*参数： start_address ：起始地址 。 0x1000开始， 256个字节
*data,读到的缓冲区   。 len :读取的长度
**********/
void EEPROM_multbyte_read(u32 start_address , u8 *data,u8 len)
{
  u8 count;

  //读N个字节
  for(count = 0; count < len ; count++)
  {
    data[count] = *((unsigned char *)(start_address + count));
  }
        
}


/**********
*函数名称：write_userdata2eeprom
*保存用户数据到EEPROM
*参数：addr： eeprom中写入的地址 
* prt : 要写入的数据缓冲区
*len :要写入的长度
*返回： 无
***********/

void write_userdata2eeprom(u16 addr, u8 *ptr, u8 len)
{
  EEPROM_multbyte_write(addr , ptr,len);
}


/**********
*函数名称:read_userdata4eeprom
*从EEPROM读取用户信息到ptr
*参数：addr： eeprom中读入的地址 
* prt : 要读出的数据缓冲区
*len :要读出的长度
*返回：无
***********/

void read_userdata4eeprom(u16 addr, u8 *ptr, u8 len)
{
  EEPROM_multbyte_read(addr,ptr, len);
}
