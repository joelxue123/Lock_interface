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
u8 Allowed_Sleep =1;
u8 Sleep_time = 0;
u8 Zigbee_Send_timeout;
/***********
** 函数名称：sum_check()
**功能描述：和校验
**入口参数 ： char * ，u8 len
**返回值  检验和 sum
***********/

u8 sum_check(u8 *p,u16 len)
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
u8 check_code(u8 *buf)
{
  u16 data_len = 0;
  data_len = buf[1];
  if(buf[data_len +2] != (sum_check(buf,data_len +2)) )
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
  
  buf[0]=0x7e;      //帧头
  for(i=0;i<6;i++){
     buf[1+i]=data_buff[i];      //应答  
  }
  switch(cmd)
  {
    case face_recognition_sucess:
      buf[7] = 0xa0;buf[8] = 0xa0;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case fingerprint_unlock_sucess:
      buf[7] = 0x80;buf[8] = 0x80;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case password_unlock_sucess:
      buf[7] = 0x40;buf[8] = 0x40;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case IC_card_unlock_sucess:
      buf[7] = 0x20;buf[8] = 0x20;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case key_unlock_sucess:
      buf[7] = 0x10;buf[8] = 0x10;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case remote_unlock_sucess:
      buf[7] = 0x08;buf[8] = 0x08;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case telecontroller_unlock_sucess:
      buf[7] = 0x04;buf[8] = 0x04;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case dynamic_password_unlock_sucess:
      buf[7] = 0x02;buf[8] = 0x02;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_dynamic_password_success:
      buf[7] = 0x01;buf[8] = 0x01;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_face_recognition_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0xa0;buf[10] = 0xa0;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_fingerprint_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x80;buf[10] = 0x80;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_password_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x40;buf[10] = 0x40;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_IC_card_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x20;buf[10] = 0x20;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_telecontroller_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x10;buf[10] = 0x10;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case set_up_network_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x08;buf[10] = 0x08;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case cancel_face_recognition_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0xa0;buf[12] = 0xa0;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case cancel_fingerprint_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x80;buf[12] = 0x80;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case cancel_password_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x40;buf[12] = 0x40;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case cancel_IC_card_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x20;buf[12] = 0x20;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case cancel_telecontroller_unlock_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x10;buf[12] = 0x10;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case cancel_network_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x08;buf[12] = 0x08;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case factory_reset_sucess:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x04;buf[12] = 0x04;buf[13] = 0x00;buf[14] = 0xf8;
      break;
    case accessory_lock_lowpower_alarm:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0xc0;buf[14] = 0xf8;
      break;
    case close_lock_without_antilock:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0xa0;buf[14] = 0xf8;
      break;
    case duress_do_alarm:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x80;buf[14] = 0xf8;
      break;
    case low_power_alarm:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x40;buf[14] = 0xf8;
      break;
    case pry_lock:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x20;buf[14] = 0xf8;
      break;
    case illegal_user:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x10;buf[14] = 0xf8;
      break;
    case trial_erro_alarm:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x08;buf[14] = 0xf8;
      break;
    case lock_not_closed:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x04;buf[14] = 0xf8;
      break;
    case ring_bell:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x02;buf[14] = 0xf8;
      break;
    case close_lock_and_antilock:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x01;buf[14] = 0xf8;
      break;
    case scaning_module:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x05;buf[14] = 0xf8;
      break;
    case set_up_linkage:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x06;buf[14] = 0xf8;
      break;
    case cancell_linkage:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x09;buf[14] = 0xf8;
      break;
    case check_module_if_online:
      buf[7] = 0x00;buf[8] = 0x00;buf[9] = 0x00;buf[10] = 0x00;buf[11] = 0x00;buf[12] = 0x00;buf[13] = 0x0b;buf[14] = 0xf8;
      break;
      
    default:
      break;
  }
  
  buf[15]=sum_check(buf,15);     //长度
  return (u8)(16); 
}

u8 expect_cmd(u8 cmd)
{
  u8 ret;
  switch(cmd)
  {
    case check_module_if_online:
        ret = 0xac;
      break;
    default:
      break;
  }
  return ret;
}

/**** 校验zigbee命令
*返回值 ： 0 表示错误。 1：表示正确
*******/
u8 check_zigbee_cmd(u8 zigbee_cmd)
{

  if( check_code(BFCT_protocol_Zigbee.receive_data) )
  { /**********数据校验错误 ************/
#if DEBUG    
  zigbee_erro =  zigbee_erro | sum_check_BIT;
#endif
  return 0;
  }
  if( BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR] != expect_cmd(zigbee_cmd) )   //获取锁的命令 != ZIGBEE_CMD_ACK)
  {
#if DEBUG
  zigbee_erro =  zigbee_erro | NACK_BIT;
#endif
  return 0;
  }
  return 1;
}

/******等待命令返回*******/
u8 wait_cmd_return(u8 cmd)
{
  u8 ret = 0;
  
  switch(cmd)
  {
   case check_module_if_online:
     ret =1;
     break;
  default:
    ret =0;
     break;
  }
  
  return ret;
  
}


/*****发送zigbee命令， 返回1 表示发送完成，发送 0表示 发送 失败***

* 返回结果 1：发送完成。 2:xiumian
****/
u8 send_zigbeecmd(u8 len,u8 zigbeecmd,u8 *data_buff )
{
    static u8 zigbee_moni_state=0;
    static u16 delay=0;
    static u8 erro_num = 0;
    u8 ret;
    
    switch(zigbee_moni_state)
    {
    case 0:
        BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,0x01,check_module_if_online, data_buff);
        BFCT_protocol_Zigbee.receive_flag =0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        delay = 0;
        erro_num = 0;
        zigbee_moni_state ++;
      break;
    case 1:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        BFCT_protocol_Zigbee.receive_flag = 0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        if(BFCT_protocol_Zigbee.receive_data[3] == 0x01){
          delay = 0;
          zigbee_moni_state ++;
        }
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 1000){
         erro_num ++;
         zigbee_moni_state=0;
         delay=0;
         if(erro_num > 10){
           erro_num = 0;
           return 1;
         }
           
       }
     }
      break;
    case 2:   
          if(zigbeecmd == check_module_if_online )
          {
            delay = 0;
            zigbee_moni_state = 0;
            return 2;
          }
          BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,len,zigbeecmd,data_buff);

          BFCT_protocol_Lock.receive_enable=0;  //清除串口接收标志位
          BFCT_protocol_Lock.receive_len =0;
          BFCT_protocol_Zigbee.receive_enable = 1;
          BFCT_protocol_Zigbee.receive_len =0;
          delay=0;
          zigbee_moni_state++;
      break;
    case 3:
      if(BFCT_protocol_Zigbee.send_len ==0)
      {
        zigbee_moni_state = 10;
        delay =0;
        sys_timer = 0;
        if(wait_cmd_return(zigbeecmd)){
            delay = 0;
            zigbee_moni_state = 0;
            return 2;
        }
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
         
       
    case 10:
      zigbee_moni_state = 0;
       delay=0;
       return 1;
         
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