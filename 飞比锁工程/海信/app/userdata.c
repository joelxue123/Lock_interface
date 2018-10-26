#include "userdata.h"
#include "eeprom.h"


/***********

0x5685c180

*******/


#define YearLength(yr) ((u16)(IsLeapYear(yr) ? 366 : 365))
#define IsLeapYear(yr) (!((yr) % 400) || (((yr) % 100) && !((yr) % 4)))
#define BEGYEAR (1970)
#define DAY (86400)
static u8 monthLength( u8 lpyr, u8 mon )
{
  u8 days = 31;
  if ( mon == 1 ) // feb
  {
  days = ( 28 + lpyr );
  }
  else
  {
  if ( mon > 6 ) // aug-dec
  {
  mon--;
  }
  if ( mon & 1 )
  {
  days = 30;
  }
}
return ( days );
}

void osal_converUtctime(UTCTimeStruct *tm,u32 sectime)
{
  u16 day = sectime % (u32)DAY;
  tm->seconds = day % (u8)60;
  tm->minutes = (day %(u16)3600 ) /(u8)60;
  tm->hour = day/(u16)3600 +8;
  
  u16  numDays = sectime / (u32)DAY;
  
  tm->year = (u16)BEGYEAR;
  while ( numDays >= YearLength( tm->year ) )
  {
    numDays -= YearLength( tm->year );
    tm->year++;
  }
  tm->month = 1;
  while ( numDays >= monthLength( IsLeapYear( tm->year ), tm->month ) )
  {
    numDays -= monthLength( IsLeapYear( tm->year ), tm->month );
    tm->month++;
  }
  tm->year = tm->year - (u16)BEGYEAR -30;
  tm->day = numDays+1;
}

/*****����ʱ��ת����UTC******/
u32 osal_convertUTC(UTCTimeStruct *tm)
{
  u32 minitues;
  
  minitues = (tm->hour * 60) + tm->minutes;
  
  u16 days;
  days = tm->day;
  
  signed char month = tm->month;
  while ( --month >= 0 )
  {
    days += monthLength( IsLeapYear( tm->year ), month );
  }
  
  u16 year = tm->year;
  while ( --year >= (u16)2016 )
  {
    days += YearLength( year );
  }
  minitues += (u32)days*(1440);
  return minitues;
}



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

u8 u8_HEX_2_byte_BCD(u8 data)
{
  return ( (data/10 <<4) + (data %10) );
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

    
  case lock_keytype:
      if( (*lockdata & 0xf0)  == 0x80)  //����
        *zigbeedata = 0x00;
      else if((*lockdata & 0xf0)  == 0x90) //ָ��
        *zigbeedata = 0x04;
      else if((*lockdata & 0xf0)  == 0xa0)  //��  
        *zigbeedata = 0x03;
      else
        *zigbeedata = 0x01;
    break;
  case lock_anti_lock_state:
      if( *lockdata & 0x10)
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

  case zigbee_password:
      lockdata[0] = (zigbeedata[4]<<4) +  zigbeedata[5];
      lockdata[1] = (zigbeedata[2]<<4) +  zigbeedata[3];
      lockdata[2] = (zigbeedata[0]<<4) +  zigbeedata[1];
    break;
  default:
    break;
  }
}






