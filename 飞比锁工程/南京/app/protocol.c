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

u8 sum_check(const u8 *p,u16 len)
{
  u16 i;
  u8 sum=0;
  for(i=0;i<len;i++)
  {
    sum = sum +p[i];
  }
  return sum;
}

/*******检验码验证****/
u8 check_code(const u8 *buf)
{
  u16 data_len = 0;
  
  data_len = (buf[2] << 8) + buf[3];

  if(buf[data_len +3] != sum_check(buf,data_len +3) )
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
u8  zigbee_protocal_component(u8 *buf,u16 len,u8 cmd,const u8* data_buff)
{
  u8 i;
  
  if(len < 1 || len > 65)
  return 0;
  
  buf[0]=0xA1;      //帧头
  buf[1]=cmd;      //命令字
  buf[2]=(len >>8) &0xff;      //长度
  buf[3]=len & 0xff;      //命令字自增ID
  
  for(i=0;i<len-1;i++){
     buf[4+i]=data_buff[i];      //应答  
  }
  buf[3+len]=sum_check(buf,len+3);     //长度
  return (len+4); 
}


/**** 校验zigbee命令
*返回值 ： 0 表示错误。 1：表示正确
*******/
u8 check_zigbee_cmd(u8 zigbee_cmd)
{
  if( check_code(BFCT_protocol_Zigbee.receive_data) )
  { /**********数据校验错误 ************/
  return 0;
  }
  if( BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR] != zigbee_cmd)   //获取锁的命令 != ZIGBEE_CMD_ACK)
  {
    return 0;
  }
  return 1;
}

/*****发送zigbee命令， 返回1 表示发送完成，发送 0表示 发送 失败*******/
u8 send_zigbeecmd(u8 len,u8 zigbeecmd,u8 *data_buff )
{
  static u8 zigbee_moni_state=0;
  static u16 delay=0;
  
  switch(zigbee_moni_state)
  {
  case 0:
        BFCT_protocol_Zigbee.send_len = zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,len,zigbeecmd,data_buff); //data_buff = 0 0 0 0 0 0 0 0 0 0 0 0 
        if(zigbeecmd == ZIGBEE_CMD_ACK){
        zigbee_moni_state ++;
        }
        else{
        BFCT_protocol_Zigbee.receive_enable=1;
        BFCT_protocol_Zigbee.receive_len =0;
        BFCT_protocol_Zigbee.receive_flag =0;
        zigbee_moni_state =2;
        }

    break;
  case 1:
    if(BFCT_protocol_Zigbee.send_len ==0)
    {
      zigbee_moni_state =0;
      delay =0;
      return 1;
    }
    break;
       
  case 2:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        
        BFCT_protocol_Zigbee.receive_flag = 0;
  //      ret = check_zigbee_cmd(ZIGBEE_CMD_ACK);
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