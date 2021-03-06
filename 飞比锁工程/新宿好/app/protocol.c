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

u16 sum_check(u8 *p,u8 len)
{
  u8 i;
  u16 sum=0;
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
  
  data_len = buf[3];

  if(  ((buf[2]<<8) + buf[1])  != sum_check(&buf[4],data_len)  )
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
  
  if(len < 1)
  return 0;
  
  buf[0]=0xF5;      //
  buf[3]=len;      //
  buf[4]=cmd;      //

  for(i=0;i<(len-1);i++)
    buf[5+i]=data_buff[i];      // 

  for(i =(len+4); i<32; i++)
    buf[i]=0xff;  
  
  sum = sum_check(&buf[4],len);
  buf[1]=sum & 0x00ff;       //帧头
  buf[2]=( (sum & 0xff00) >> 8);      //帧头
  return ((u8)32);    

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
      zigbee_moni_state =0;
      delay =0;
      return 1;
    }
    break;
       
  case 2:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        
        BFCT_protocol_Zigbee.receive_flag = 0;
        ret = check_zigbee_cmd(ZIGBEE_CMD_ACK);
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
         zigbee_erro =  zigbee_erro | TIMEOUT_BIT;
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



/********透传协议 *************/
u8 transparently_process(void)
{
  static u8 state=0;
    switch(state)
    {
     case 0:
        if(BFCT_protocol_Zigbee.receive_flag == 1)
        {
          BFCT_protocol_Zigbee.receive_enable =0;
          deadline = 10000;
          BFCT_protocol_Zigbee.receive_flag =0;
          if( (BFCT_protocol_Zigbee.receive_data[0] == 0xf5) && (BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR] == 0xa8))
          {
              return 2;  
          }
          else
          {
             BFCT_protocol_Lock.send_data = G_receive_data;
             BFCT_protocol_Lock.send_len = BFCT_protocol_Zigbee.receive_len;
             state++;
          }
        }
        if(BFCT_protocol_Lock.receive_flag == 1)
        {
             BFCT_protocol_Lock.receive_enable =0;
             USART1->CR2 &= (uint8_t)~(USART_CR2_TEN);
             GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,低电平输出
             deadline = 10000;
             BFCT_protocol_Lock.receive_flag =0;
             BFCT_protocol_Zigbee.send_data = G_receive_data;
             BFCT_protocol_Zigbee.send_len = BFCT_protocol_Lock.receive_len;
             state = 2;
        }
       break;
    case 1:
         if(BFCT_protocol_Lock.send_len == 0)
         {
           state =0;
           return 1;
         }
      break;
    case 2:
         if(BFCT_protocol_Zigbee.send_len == 0)
         {
           state =0;
           return 1;
         }
      break;
    default:
      state = 0;
      break;
    }
    return 0;
}