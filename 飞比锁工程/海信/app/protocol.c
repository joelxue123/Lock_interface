#include "protocol.h"
#include "global.h"
#include "userdata.h"
#include "command.h"


/***********
** 函数名称：sum_check()
**功能描述：和校验
**入口参数 ： char * ，u8 len
**返回值  检验和 sum
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

/*******检验码验证****/
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
*函数名称：zigbee_protocal_component
**函数描述 ：组帧，zigbee协议
**参数描述： buf:储存组好的帧，len:数据长度+命令子 cmd:协议命令 。data_buf :数据内容
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


/**** 校验zigbee命令
*返回值 ： 0 表示错误。 1：表示正确
*******/
u8 check_zigbee_cmd(u8 zigbee_cmd)
{
  u16 sum;
  u8 data_len;
  sum = (BFCT_protocol_Zigbee.receive_data[2] << 8) + BFCT_protocol_Zigbee.receive_data[1];
  data_len =  BFCT_protocol_Zigbee.receive_data[3];
  if(sum != sum_check(&(BFCT_protocol_Zigbee.receive_data[4]),data_len) )
  { /**********数据校验错误 ************/
  data_len =0 ;
#if DEBUG    
  zigbee_erro =  zigbee_erro | sum_check_BIT;
#endif
  return 0;
  }
  if( BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR] != zigbee_cmd)   //获取锁的命令 != ZIGBEE_CMD_ACK)
  {
  data_len =0 ;
#if DEBUG
  zigbee_erro =  zigbee_erro | NACK_BIT;
#endif
  return 0;
  }
  return 1;
}

/*****发送zigbee命令， 返回1 表示发送完成，发送 0表示 发送 失败*******/
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

