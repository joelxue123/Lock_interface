#include "userdata.h"
#include "eeprom.h"




/****1���ֽ�BCD��תHEX *********/

u8 u8_BCD_2_hex(u8 data)
{
  return ( ((data >> 4) & 0x0f) *10 + (data & 0x0f) );
}

/***2���ֽ�BCD��תHEX *********/

unsigned char u16_BCD_2_hex(short data)
{
  return ( u8_BCD_2_hex(data & 0x00ff)*100 +u8_BCD_2_hex( (data>>8) & 0x00ff));
}

/****1���ֽ�HEX��תBCD *********/

u16 u8_HEX_2_BCD(u8 data)
{
  return ( (data / 100) + ( ((((data %100) /10) <<4) + (data %10) ) << 8 ) );
}


/**********
*�������ƣ�lockdata_2_zigbeedata
*����;�����û�������Ϣ��ת����zigbee�û���Ϣ
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
       *zigbeedata = 0X01; //��ͨ�û�
      else
       *zigbeedata = 0X00; //����Ա����
    
    break;
  case lock_keytype:
      if( (*lockdata & 0xf0)  == 0x80)  //����
        *zigbeedata = 0x02;
      else if((*lockdata & 0xf0)  == 0x90) //ָ��
        *zigbeedata = 0x01;
      else if((*lockdata & 0xf0)  == 0xa0)  //��  
        *zigbeedata = 0x03;
      else
        *zigbeedata = 0x05;
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
*�������ƣ�zigbeedata_2_lockdata
*����;��zigbee�û�������Ϣ��ת�������û���Ϣ
**********/
void zigbeedata_2_lockdata(u8 zigbeedata_type,u8 *lockdata,u8 *zigbeedata)
{
  u16 temp;
  u8 i,len;
  u8 wap;
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
       *lockdata  = *lockdata | 0X80; //��ͨ�û�
      else
       *lockdata = *lockdata | 0X00; //����Ա����
      break;
  case zigbee_password:
      len = zigbeedata[0];
      for (i = 0;i < 7;i++)
              lockdata[i] &= 0x00;

      for (i = 0;i < 14 - len;i++)
      {
              if (( i) % 2 == 0)
                      lockdata[(i) / 2] |= 0x0f << 4;
              else
                      lockdata[(i) / 2] |= 0x0f;
      }

      for (i = 0;i < len;i++)
      {
              if ((14 - len - i) % 2 == 0)
                      lockdata[(14 - len + i) / 2] |= (zigbeedata[i + 1] - '0') << 4;
              else
                      lockdata[(14 - len + i) / 2] |= (zigbeedata[i + 1] - '0');
      }

      for (i = 0;i < 4;i++)
      {
              wap = lockdata[i];
              lockdata[i] = lockdata[6 - i];
              lockdata[6 - i] = wap;
      }
    break;
  default:
    break;
  }
}







