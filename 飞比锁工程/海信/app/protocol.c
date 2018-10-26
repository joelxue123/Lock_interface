#include "protocol.h"
#include "global.h"
#include "userdata.h"
#include "command.h"


/***********
** �������ƣ�sum_check()
**������������У��
**��ڲ��� �� char * ��u8 len
**����ֵ  ����� sum
***********/

u8 sum_check(u8 *p,u8 len)
{
  u8 i;
  u8 sum=0;
  for(i=0;i<len;i++)
  {
    sum = sum +p[i];
  }
  return sum;
}

/*******��������֤****/
u8 check_code(u8 *buf)
{
  u16 data_len = 0;
  
  data_len = buf[1];

  if(  ( buf[data_len+2] )  != sum_check(&buf[0],data_len +2)  )
  {
    return 1;
  }
  else 
    return 0;
}

/**********
*�������ƣ�zigbee_protocal_component
**�������� ����֡��zigbeeЭ��
**���������� buf:������õ�֡��len:���ݳ���+������ cmd:Э������ ��data_buf :��������
**********/
u8  zigbee_protocal_component(u8 *buf,u16 len,u8 cmd,u8* data_buff)
{
  u8 i;
  u16 sum ;
  
  if(len < 1 || len > 65)
  return 0;
  
  buf[0]=0xFA;      //
  buf[1]=len;      //
  buf[2]=cmd;      //

  for(i=0;i<(len-1);i++)
    buf[3+i]=data_buff[i];      // 
 for(i=4;i<8;i++)
    buf[i]=0xff;
  
  sum = sum_check(&buf[0],len+2);
  buf[len+2]=sum ;    

  return (len+3);    

}


/**** У��zigbee����
*����ֵ �� 0 ��ʾ���� 1����ʾ��ȷ
*******/
u8 check_zigbee_cmd(u8 zigbee_cmd)
{
  u16 sum;
  u8 data_len;
  sum = (BFCT_protocol_Zigbee.receive_data[2] << 8) + BFCT_protocol_Zigbee.receive_data[1];
  data_len =  BFCT_protocol_Zigbee.receive_data[3];
  if(sum != sum_check(&(BFCT_protocol_Zigbee.receive_data[4]),data_len) )
  { /**********����У����� ************/
  data_len =0 ;
#if DEBUG    
  zigbee_erro =  zigbee_erro | sum_check_BIT;
#endif
  return 0;
  }
  if( BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR] != zigbee_cmd)   //��ȡ�������� != ZIGBEE_CMD_ACK)
  {
  data_len =0 ;
#if DEBUG
  zigbee_erro =  zigbee_erro | NACK_BIT;
#endif
  return 0;
  }
  return 1;
}

/*****����zigbee��� ����1 ��ʾ������ɣ����� 0��ʾ ���� ʧ��*******/
u8 send_zigbeecmd(u8 len,u8 zigbeecmd,u8 *data_buff )
{
  static u8 zigbee_moni_state=0;
  static u16 delay=0;
  u8 ret;
  
  switch(zigbee_moni_state)
  {
  case 0:
        BFCT_protocol_Zigbee.send_len = zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,len,zigbeecmd,data_buff); //data_buff = 0 0 0 0 0 0 0 0 0 0 0 0 
        zigbee_moni_state ++;
        delay=0;   
    break;
  case 1:
    if(BFCT_protocol_Zigbee.send_len ==0)
    {
      zigbee_moni_state++;
      delay =0;
      if(zigbeecmd == ZIGBEE_CMD_ACK)
      {
        zigbee_moni_state =0;
        return 1;
      }
    }
    break;
       
  case 2:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        
        BFCT_protocol_Zigbee.receive_flag = 0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        zigbee_moni_state=0;
        return 1;
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 1000){
         zigbee_moni_state=0;
         delay=0;         
         return 1;
       }
     }
     break;
  default:
    zigbee_moni_state =0;
    break;
  }
  return 0;
}



u8 convert_zigbee_cmd(void)
{
  u8 cmd=0;
  
  if(BFCT_protocol_Zigbee.receive_data[2] == 0x14){
      if(BFCT_protocol_Zigbee.receive_data[3] == 0x51 ) 
      {
          cmd =ZIGBEE_CMD_CLOCK_SYNC ;
      }
      if(BFCT_protocol_Zigbee.receive_data[3] == 0x52 ) 
      {
          cmd =ZIGBEE_CMD_CLOCK_SYNC ;
      }
       else if(BFCT_protocol_Zigbee.receive_data[3] == 0x53 ) 
      {
          cmd =ZIGBEE_CMD_OPENLOCK ;
      }
      else if(BFCT_protocol_Zigbee.receive_data[3] == 0x54)
      {
        
         if(BFCT_protocol_Zigbee.receive_data[11] == 0x04)
           cmd  = ZIGBEE_CMD_ADMIN_IDENTIFICATION;
         if(BFCT_protocol_Zigbee.receive_data[11] == 0x03)
           cmd  = ZIGBEE_CMD_INQURE_USERINFO;
         if(BFCT_protocol_Zigbee.receive_data[11] == 0x00)
           cmd  =ZIGBEE_CMD_ADD_USER;
         if(BFCT_protocol_Zigbee.receive_data[11] == 0x01)
           cmd  = ZIGBEE_CMD_DELETE_USER;
      }
  }
  else
  {
    cmd = BFCT_protocol_Zigbee.receive_data[2];
  }
  return cmd;
}

