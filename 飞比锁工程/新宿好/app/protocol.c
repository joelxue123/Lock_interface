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

/*******��������֤****/
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
  
  buf[0]=0xF5;      //
  buf[3]=len;      //
  buf[4]=cmd;      //

  for(i=0;i<(len-1);i++)
    buf[5+i]=data_buff[i];      // 

  for(i =(len+4); i<32; i++)
    buf[i]=0xff;  
  
  sum = sum_check(&buf[4],len);
  buf[1]=sum & 0x00ff;       //֡ͷ
  buf[2]=( (sum & 0xff00) >> 8);      //֡ͷ
  return ((u8)32);    

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



/********͸��Э�� *************/
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
          if( (BFCT_protocol_Zigbee.receive_data[0] == 0xf5) && (BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR] == 0xa8) )
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
            if(BFCT_protocol_Lock.receive_data[LOCK_CMD_ADDR] == LOCK_OPENNET || BFCT_protocol_Lock.receive_data[LOCK_CMD_ADDR] == LOCK_CLOSENET )
            {
              return 2;
            }
             BFCT_protocol_Lock.receive_enable =0;
             USART1->CR2 &= (uint8_t)~(USART_CR2_TEN);
             GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,�͵�ƽ���
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