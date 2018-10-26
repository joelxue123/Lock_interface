#include "eeprom.h"
#include "stm8l15x_iwdg.h"
/******
*������ EEPROM_multbyte_write
*������������EEPROM���ֽ�
*������ start_address ����ʼ��ַ �� 0x1000��ʼ�� 256���ֽ�
*data,д��Ļ�����   �� len :д��ĳ���
**********/
void EEPROM_multbyte_write(u32 start_address , u8 *data,u8 len)
{
  u8 count;
 // FLASH_SetProgrammingTime(FLASH_ProgramTime_TProg); //�趨���ʱ��Ϊ��׼���ʱ��
  FLASH->CR1 &= (uint8_t)(~FLASH_CR1_FIX);
  FLASH->CR1 |= (uint8_t)FLASH_ProgramTime_TProg;
 //MASS ��Կ�����EEPROM�ı���
 // FLASH_Unlock(FLASH_MemType_Data);
    FLASH->DUKR = FLASH_RASS_KEY2; /* Warning: keys are reversed on data memory !!! */
    FLASH->DUKR = FLASH_RASS_KEY1;

  //��һ��д��128���ֽ�����
  for(count = 0; count < len ; count++)
  {
    *((unsigned char *)(start_address + count)) = data[count];
    if ((WWDG->CR & 0x7F) < WWDG->WR) //С�ڴ���ֵ����ι��  
    WWDG->CR |= 0x7F; //����ι�� 
  }
// while(FLASH_GetFlagStatus(FLASH_FLAG_WR_PG_DIS) == 1);
  while(FLASH->IAPSR  & (uint8_t)FLASH_FLAG_WR_PG_DIS);

//  while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == 1);
  while(FLASH->IAPSR  & (uint8_t)FLASH_FLAG_EOP);
  if((WWDG->CR & 0x7F) < WWDG->WR) //С�ڴ���ֵ����ι��  
    WWDG->CR |= 0x7F; //����ι�� 
}

/******
*������ EEPROM_multbyte_read
*������������EEPROM���ֽ�
*������ start_address ����ʼ��ַ �� 0x1000��ʼ�� 256���ֽ�
*data,�����Ļ�����   �� len :��ȡ�ĳ���
**********/
void EEPROM_multbyte_read(u32 start_address , u8 *data,u8 len)
{
  u8 count;

  //��N���ֽ�
  for(count = 0; count < len ; count++)
  {
    data[count] = *((unsigned char *)(start_address + count));
  }
        
}


/**********
*�������ƣ�write_userdata2eeprom
*�����û����ݵ�EEPROM
*������addr�� eeprom��д��ĵ�ַ 
* prt : Ҫд������ݻ�����
*len :Ҫд��ĳ���
*���أ� ��
***********/

void write_userdata2eeprom(u16 addr, u8 *ptr, u8 len)
{
  EEPROM_multbyte_write(addr , ptr,len);
}


/**********
*��������:read_userdata4eeprom
*��EEPROM��ȡ�û���Ϣ��ptr
*������addr�� eeprom�ж���ĵ�ַ 
* prt : Ҫ���������ݻ�����
*len :Ҫ�����ĳ���
*���أ���
***********/

void read_userdata4eeprom(u16 addr, u8 *ptr, u8 len)
{
  EEPROM_multbyte_read(addr,ptr, len);
}
