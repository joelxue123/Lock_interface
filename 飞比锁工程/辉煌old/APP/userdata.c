#include "userdata.h"
#include "eeprom.h"


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


/****1个字节BCD码转HEX *********/

u8 u8_BCD_2_hex(u8 data)
{
  return ( ((data >> 4) & 0x0f) *10 + (data & 0x0f) );
}

/***2个字节BCD码转HEX *********/

unsigned char u16_BCD_2_hex(short data)
{
  return ( u8_BCD_2_hex(data & 0x00ff) +u8_BCD_2_hex( (data>>8) & 0x00ff) *100);
}

/****1个字节HEX码转BCD *********/

u16 u8_HEX_2_BCD(u8 data)
{
  return ( ((data / 100) << 8) + ( ((data %100) /10) <<4) + (data %10) );
}


/**********
*函数名称：lockdata_2_zigbeedata
*描述;把锁用户数据信息，转化成zigbee用户信息
**********/
void lockdata_2_zigbeedata( u8 lockdata_type,u8 *lockdata,u8 *zigbeedata)
{
  u16 temp ;
  u8 i;
  switch(lockdata_type)
  {
  case lock_user_No:
      temp =*((u16*)lockdata);
      *zigbeedata=u16_BCD_2_hex(temp);
    break;
  case lock_username:
      for(i=0;i<8;i++)
      {
        *(lockdata+i) = *(zigbeedata+i);
      }
    break;  
    
  case lock_user_attribute:
    
      if(*lockdata & 0x01)
       *zigbeedata = 0X01; //普通用户
      else
       *zigbeedata = 0X00; //管理员属性
    
    break;
  case lock_keytype:
      
    break;
  case lock_password:  
     break;
    
  case lcok_power_level_state:
      if( *lockdata & 0x80)
        *zigbeedata = 1;
    break;
  case lock_square_tongue_locked_state:
      if( *lockdata & 0x20)
        *zigbeedata = 1;
    break;
  case lock_anti_lock_state:
      if( *lockdata & 0x10)
        *zigbeedata = 1;
    break;
  case lock_motor_state:
      if( *lockdata & 0x02)
        *zigbeedata = 1;
    break;
    
  case 100:
    break;
    
  default:
    break;
    
  }
}

/**********
*函数名称：zigbeedata_2_lockdata
*描述;把zigbee用户数据信息，转化成锁用户信息
**********/
void zigbeedata_2_lockdata(u8 zigbeedata_type,u8 *lockdata,u8 *zigbeedata)
{
  u16 temp;
  switch(zigbeedata_type)
  {
    case zigbee_user_No:
      temp = u8_HEX_2_BCD(*zigbeedata);
      *((u16*)lockdata) =temp;
    break;
  case zigbee_user_attribute:
      *lockdata = *lockdata &0xfe;
      if(*zigbeedata == 0X01)
       *lockdata  = *lockdata | 0X01; //普通用户
      else
       *lockdata = *lockdata | 0X00; //管理员属性
      break;
  case zigbee_password:
    break;
  default:
    break;
  }
}







