#include "userdata.h"
#include "eeprom.h"




/****1个字节BCD码转HEX *********/

u8 u8_BCD_2_hex(u8 data)
{
  return ( ((data >> 4) & 0x0f) *10 + (data & 0x0f) );
}

/***2个字节BCD码转HEX *********/

unsigned char u16_BCD_2_hex(short data)
{
  return ( u8_BCD_2_hex(data & 0x00ff)*100 +u8_BCD_2_hex( (data>>8) & 0x00ff));
}

/****1个字节HEX码转BCD *********/

u16 u8_HEX_2_BCD(u8 data)
{
  return ( (data / 100) + ( ((((data %100) /10) <<4) + (data %10) ) << 8 ) );
}

u8 u8_HEX_2_byte_BCD(u8 data)
{
  return ( (data/10 <<4) + (data %10) );
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
    
      if(*lockdata & 0x80)
       *zigbeedata = 0X01; //普通用户
      else
       *zigbeedata = 0X00; //管理员属性
    
    break;
  case lock_keytype:
      if( (*lockdata & 0xf0)  == 0x80)
        *zigbeedata = 0x01;
      else if((*lockdata & 0xf0)  == 0x90)
        *zigbeedata = 0x03;
      else if((*lockdata & 0xf0)  == 0xa0)
        *zigbeedata = 0x02;
      else
        *zigbeedata = 0xff;
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
      if(*zigbeedata == 0)
        temp = 0;
      else {
       temp = u8_HEX_2_BCD(*zigbeedata);
       }
      *((u16*)lockdata) =temp;
    break;
  case zigbee_user_attribute:
      *lockdata = *lockdata &0xfe;
      if(*zigbeedata == 0X01)
       *lockdata  = *lockdata | 0X80; //普通用户
      else
       *lockdata = *lockdata & 0X7f; //管理员属性
      break;
  case zigbee_password:
      lockdata[0] = (zigbeedata[4]<<4) +  zigbeedata[5];
      lockdata[1] = (zigbeedata[2]<<4) +  zigbeedata[3];
      lockdata[2] = (zigbeedata[0]<<4) +  zigbeedata[1];
    break;
  default:
    break;
  }
}







