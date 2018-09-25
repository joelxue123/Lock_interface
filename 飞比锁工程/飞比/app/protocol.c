#include "protocol.h"
#include "global.h"
#include "userdata.h"
#include "command.h"
#include "usart.h"
#include "bsp.h"

const u8 key[]={0x46,0x45,0x49,0x42,0x49,0x47};
u8 password[6]={1,2,3,4,5,6};
u8 npass[6];
u8 remote_open_flag = 0;
 u32 cmd_id = 1;

 
/***********
** 函数名称：xor_check()
**功能描述：和校验
**入口参数 ： char * ，u8 len
**返回值  异或检验和 xor
***********/

u8 xor_check(u8 *p,u16 len)
{
  u16 i;
  u8 xor=0;
  for(i=0;i<len;i++)
  {
    xor = xor^p[i];
  }
  return xor;
}

/***********
** 函数名称：xor_check()
**功能描述：和校验
**入口参数 ： char * ，u8 len
**返回值  异或检验和 xor
***********/
void for_cmd_id(u32 cmd_id,u8 cmd_type,u8 *buff)
{
  buff[0]=( cmd_id >> 24) & 0xff;      //命令字自增ID
  buff[1]=(cmd_id >> 16) & 0xff;      //命令字自增ID
  buff[2]=(cmd_id >> 8) & 0xff;     //命令字自增ID
  buff[3]=(cmd_id ) & 0xff;    //命令字自增ID
    
  if(cmd_type ==0)
     buff[4]=0x00;      //应答
  else{
     buff[4]=0x01;      //应答
  }
}

/***********
** 函数名称：encryption()
**功能描述：密码加密
**入口参数 ：
**返回值 
***********/

void encryption(void)
{
  u8 i;
  for(i=0;i<6;i++)
  {
    npass[i] =(u8) key[i] ^  password[i];
  }
}

/*******检验码验证****/
u8 check_code(u8 *buf)
{
  u16 data_len = 0;
  data_len = buf[1];
  if(buf[data_len +8] != (xor_check(buf,data_len +8)) )
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
  buf[0]=0xAA;      //帧头
  buf[1]=len;      //有效长度
  buf[2]=cmd;      //命令字

  for(i=0;i<len+4;i++){
     buf[3+i]=data_buff[i];      //应答  
  }
  buf[8+len]=xor_check(BFCT_protocol_Zigbee.send_data,len+8);     //长度
  buf[9+len]=0x55;      
  return (len+10); 
}


/**** 校验zigbee命令
*返回值 ： 0 表示错误。 1：表示正确
*******/
u8 check_zigbee_cmd(u8 zigbee_cmd)
{
  u8 data_len;
  data_len = BFCT_protocol_Zigbee.receive_data[1];
  if(BFCT_protocol_Zigbee.receive_data[data_len +8] != (xor_check(BFCT_protocol_Zigbee.receive_data,data_len +8)) )
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
          BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,len,zigbeecmd,data_buff);
          if(data_buff[4] == ACK){
          zigbee_moni_state ++;
          delay=0;   
          }
          else{
          BFCT_protocol_Lock.receive_enable=0;  //清除串口接收标志位
          BFCT_protocol_Lock.receive_len =0;
          BFCT_protocol_Zigbee.receive_enable = 1;
          BFCT_protocol_Zigbee.receive_len =0;
          
          BFCT_protocol_Zigbee.receive_flag =0;
          zigbee_moni_state =2;
          delay=0; 
          }
  
      break;
    case 1:
      if(BFCT_protocol_Zigbee.send_len ==0)
      {
        zigbee_moni_state =0;
        delay =0;
        return 1;
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 500){
         zigbee_moni_state=0;
         delay=0;
         lock_erro =  lock_erro | TIMEOUT_BIT;
         return 1;
       }
     }
      break;
         
    case 2:
        if(BFCT_protocol_Zigbee.receive_flag == 1)
        {
          BFCT_protocol_Lock.receive_enable=0;  //清除串口接收标志位
          BFCT_protocol_Lock.receive_len =0;
          BFCT_protocol_Zigbee.receive_enable = 0;
          
          BFCT_protocol_Zigbee.receive_flag = 0;
          ret = check_zigbee_cmd(zigbeecmd);
          zigbee_moni_state=0;
          return 1;
        }
       if(t_1ms)
       {
         delay++;
         if(delay > 1000){
           zigbee_moni_state=0;
           delay=0;
  #if DEBUG             
           zigbee_erro =  zigbee_erro | TIMEOUT_BIT;
  #endif
           return 1;
         }
       }
       break;
    default:
      break;
    }
    return 0;
}

/**********
**函数描述 ： 南京模块 回复 ACK 流程
**********/
u8 zigbee_usart_send(void)
{
    static u8 delay=0;
    if(t_1ms){
      delay++;
      if(delay ==2)
          wake_up_in(SET);
      if(delay ==20)   
          wake_up_in(RESET);
      if(delay ==40)
      {
        enableInterrupts();    //似乎没啥用，忘记了为啥放到这里了
        UART1_Init(ZIGBEE_BAUD); //PC5,PC6为串口,与zigbee模块通讯
        send_hex(BFCT_protocol_Zigbee.send_data,BFCT_protocol_Zigbee.send_len); // 发送数据

      } 
      if(delay ==44)
      {
        delay =0;
        BFCT_protocol_Zigbee.receive_enable =1; //使能zigbee模块 接收功能
        BFCT_protocol_Lock.receive_enable =0;   //失能lock串口接收 
        BFCT_protocol_Zigbee.receive_len =0;
        BFCT_protocol_Zigbee.receive_flag =0;
        return 1;
      }
      if(delay > 100)
      {
        delay =0;
      }
    }  
   return 0;
}  