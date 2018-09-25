#include "stm8l15x.h"
#include "global.h"
#include "eeprom.h"
#include "usart.h"
#include "protocol.h"
#include "command.h"
#include "bsp.h"
#include "userdata.h"

u8 G_receive_data[USART_LEN]={0};
u8 G_send_data[100]={0};

BFCT_protocol BFCT_protocol_Zigbee;
BFCT_protocol BFCT_protocol_Lock;

u8 USART_process_flag;
u16 data3_tc;
u8 t_1ms;

u8 zigbee_send_cmd;

u8 lock_interrupt =0;
u8 zigbee_interrupt =0;

u8 Zigbee_process_done =1;
u8 Lock_process_done =1;

u8 Zigbee_processing_flag =0;
u8 Lock_processing_flag =0;


u8 lock_state =0;
u8 zigbee_state =0;

u8 zigbee_erro=0;
u8 lock_erro = 0;

u16 sys_timer=0;
u16 deadline = 0;

#if defined(Jun_He)
u8 transparently_flag = 0;
#endif

#if defined(Hui_huang)
u8 Wfi_Mode = 0;

static u8 delete_user_process_flag = 0;
#endif
/**********************
** ��������: ���
**********************/
void led(void)
{
#if 1
   static u8 led_on=0;
   static  u16 led_timer=0;
   if(t_1ms)
   {
    led_timer++;
    
    if(led_timer == 1000){
      led_on=1-led_on;
      led_timer=0;
    }
    if(led_on)
       LED2(RESET); //���� LED
    else
       LED2(SET);   //�ر�LED
   }
#endif 
}


/**********
**�������� �� ʱ�������,1ms����ж�
**********/
void os_time(void) 
{
    if( ( TIM2_GetFlagStatus(TIM2_FLAG_Update) != RESET) ) //�ȴ������Ƿ�ﵽ1����
    {
      t_1ms=1;
      TIM2_ClearFlag(TIM2_FLAG_Update);
    }else t_1ms=0;
}

/**********************
*�����ͺ���
*
************************/
u8 lock_usart_send(void)
{
   static u16 delay=0;

   
    if(t_1ms){
       delay++;
       if(1==delay)
       {
          USART_Cmd(USART1 , DISABLE);
          GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //TXD
       }
       
       if(20 ==delay)
       {
          GPIOA->ODR |= (uint8_t)GPIO_Pin_2;//���ô���Ϊ�ߵ�ƽ
       } 
       if(delay < 1000 && delay > 20)
       {
          if(  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3) )
          {
              UART1_RemapInit(9600);//������ ���ó� 9600
              send_hex(BFCT_protocol_Lock.send_data,BFCT_protocol_Lock.send_len); // ��������
              BFCT_protocol_Lock.receive_enable =1;
              delay =0;
              for(delay=0;delay<1000;delay++);
              
              USART1->CR2 &= (uint8_t)~(USART_CR2_TEN);
              GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TXD
              delay=0;  
              return 1;
          }
       }
       if(delay > 1000)
       {
          delay=0;
          return 2;
       }
    }
       return 0;
}

/**********
**�������� �� ��zigbee�յ�������Э�� ת�г� ��Э��
**********/
u8 zigbee_convert2lock(void)
{
    u8 ret;
    
    /************
    **�Ͼ�ģ��,Э��Ӧ���ʽ

    ***************/
    /****
    ��zigbee����������ݣ���䵽lock����������
    ****/
  
  switch(zigbee_state)
  {
    case 0:
        BFCT_protocol_Zigbee.receive_flag =0;
        BFCT_protocol_Lock.receive_enable =0;  //�ر������ڽ��ܺ���
        BFCT_protocol_Zigbee.receive_enable =0; //�ر�zigbee���ڽ��ܺ���
        
        BFCT_protocol_Zigbee.command=BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR]; //��ȡ��������

        if( check_code(BFCT_protocol_Zigbee.receive_data) )
        { /**********����У����� ************/
#if defined(DEADLINE)
          if(deadline > 10000)
          {
            return 2;
          }
#endif 
          zigbee_state =10;
        }else {
         zigbee_state ++;
        }
        
    break;
    case 1:
        ret = exe_zigbee_cmd(BFCT_protocol_Zigbee.command);
        if(ret == 1)
        {
            zigbee_state =10;
        }
#if defined(DEADLINE)        
        else if(ret == 2)
          {
            zigbee_state = 0;
            return 2;
          }
#endif
    break;

  case 10:
       Zigbee_process_done =1;
       zigbee_state =0;
       return 1;
     
      break;
    default: 
    break;
    
  }
  
  return 0;
}


/***
**�����ж�������Э�飬ת����zigbeeģʽ
**/
u8 lock_convert2zigbee(void)
{
  u16 data_len;
  u8 ret;

  switch(lock_state )
  {
    case 0:
        BFCT_protocol_Lock.receive_flag =0; 
        BFCT_protocol_Lock.receive_enable =0;  //�ر������ڽ��ܺ���
        BFCT_protocol_Zigbee.receive_enable =0; //�ر�zigbee���ڽ��ܺ���
        data_len = BFCT_protocol_Lock.receive_data[1] + (BFCT_protocol_Lock.receive_data[2] << 8); //��ȡ���ݳ���
        BFCT_protocol_Lock.command=BFCT_protocol_Lock.receive_data[LOCK_CMD_ADDR]; //��ȡ��������
        if(BFCT_protocol_Lock.receive_data[data_len +3] != crc_8n(BFCT_protocol_Lock.receive_data,data_len +3) )
        {    /** ����У�����**/
          data_len =0 ;
          lock_state = 10;
        }
        else {
          data_len =0;
         lock_state ++;
        }
    break;
   
   case 1:
      ret = exe_lock_cmd(BFCT_protocol_Lock.command);
      if(ret ==1)
      {
        lock_state =10;
      }
#if defined(DEADLINE)
      else if(ret ==2)
      {
        lock_state =0;
        return 2;
      }
#endif
     break;
      
    case 10:
      lock_state =0;
      Lock_process_done=1;
      return 1;
    break;
    
    default:
    break;
  }
  return 0;
}

/********************
    *���ڳ�ʱ��������
    **********************/
void uart_receiving_timeout(void)
{
  if( t_1ms==1 ) //10ms��ʱ��,��ʱ��ÿ10ms,����˺���
  {
    data3_tc++;
    if( data3_tc > (UART_TIMEOUT+1) ) //��ʱ���ӵ� 300ms,ֹͣ����
    {
      data3_tc= (UART_TIMEOUT+1);  //��Ϊȫ�ֱ�������Ҫ����ʹ��
    }
    if( data3_tc == UART_TIMEOUT ) //��ʱ280ms��ʱ��,��ʾ������ֹ
    {
      if( (BFCT_protocol_Zigbee.receive_enable == 1) && BFCT_protocol_Zigbee.receive_flag != 1)
         BFCT_protocol_Zigbee.receive_flag = 1;
      if( (BFCT_protocol_Lock.receive_enable == 1) && BFCT_protocol_Lock.receive_flag != 1)
         BFCT_protocol_Lock.receive_flag = 1;          
    }
  }
}

/**********
**�������� �� ����zigbee���ݴ���
**********/
void USART_process(void)
{
  
    u8 res;
/****************
 *�жϴ��ڽ�����ɽ������
*****************/
    if(BFCT_protocol_Zigbee.receive_flag == 1)
    { 
      data3_tc =(UART_TIMEOUT+1);
      if( Lock_processing_flag ==0 )/****ֻ����һ��Э�����������******/
      {
        deadline = 10000;
        Zigbee_processing_flag =1;  //����zigbee����  
      }
    }
    else if(BFCT_protocol_Lock.receive_flag==1)
    {
      USART1->CR2 &= (uint8_t)~(USART_CR2_TEN);
      GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,�͵�ƽ���
      
      data3_tc =(UART_TIMEOUT+1);
      if( Zigbee_processing_flag ==0)/****ֻ����һ��Э�����������******/
      {
        deadline = 10000;
        Lock_processing_flag  = 1; //��������������
      }
    }  

/****************
 *Zigbee����������
*****************/
    if(Zigbee_processing_flag ==1)
    {
     res = zigbee_convert2lock();
     if(res==1)
     {
        Zigbee_processing_flag =0;
        Zigbee_process_done =1;
     }
#if defined(DEADLINE)
     else if(res == 2)
     {
        Zigbee_processing_flag = 0;
     }
#endif
    }
/****************
 *������������
*****************/
    if(Lock_processing_flag ==1)
    {
      res =lock_convert2zigbee();
      if(res==1)
      {
          Lock_processing_flag=0;
          Lock_process_done=1;
      }
#if defined(DEADLINE)
      else if(res ==2)
      {  
          Lock_process_done=1;
          Lock_processing_flag=0;
      }
#endif      
    }

}
    
/**************
    *�����ڷ��ͺ���
***********/
void lock_uart_send_session(void)
{
  u8 res =0;
  if(BFCT_protocol_Lock.send_len)
  {
    res = lock_usart_send();
   if(res==1)
   {
    BFCT_protocol_Lock.send_len =0;
   }  
   else if(res ==2)
   {
    BFCT_protocol_Lock.send_fail=1;
    BFCT_protocol_Lock.send_len =0;
   }
  }
}
/**************
    *zigbee���ڷ��ͺ���
***********/
void zigbee_uart_send_session(void)
{
  u8 res;
  if(BFCT_protocol_Zigbee.send_len)
  {
    res =zigbee_usart_send();
   if(res ==1)
   {
    BFCT_protocol_Zigbee.send_len =0;
   }  
  }
}

/***********************
***��ʱ,ɾ���û��߳�
*********************/
#if defined(Hui_huang) 
void delete_user_process(void)
{
  u8 ret;
  if(Wfi_Mode)
  {
     if( Zigbee_Send_timeout ==1)
     {
        Zigbee_Send_timeout--;
        wake_up_in(SET);
     }
     if(Sleep_time == 0)
     { 
       if(Lock_processing_flag ||Zigbee_processing_flag || Zigbee_Send_timeout )
       return ;
       USART_process_flag =0;
       delete_user_process_flag =1;
       Zigbee_process_done = 0;
       Lock_process_done =0;
       sys_timer = 1000;
       deadline = sys_timer +1000;
       ret =  delete_user();
       if(ret)
       {
         USART_process_flag = 1;
         delete_user_process_flag = 0;
         wake_up_in(SET);
         Wfi_Mode = 0;
         deadline = sys_timer ;
         TIM3_Cmd(ENABLE);  
        }
     }
  } 
}
#endif

/*********���������� *****************/             

void main()
{
  volatile  u16 i,j;
  static  u16 delay=0;
  u8 ret;

   disableInterrupts();   //�ر�ϵͳ���ж�
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1); //�ڲ�ʱ��Ϊ1��Ƶ = 16Mhz 
  
  deadline =1;

  TIM2_Init(16000);
  TIM2->CR1 |= TIM_CR1_ARPE;  //ʹ�ܶ�ʱ��2�Զ����ع���    
  TIM2->CR1 |= TIM_CR1_CEN;
  
#if defined(Hui_huang) 
  TIM3_Init(65535);
#endif
  IOInit();
  
  for(i=0;i<3000;i++)
    for(j=0;j<1000;j++);
  
#if defined(Jun_He) 
  read_userdata4eeprom(transparently_flag_addr,&transparently_flag, 1);
#endif  
  
  BFCT_protocol_Zigbee.receive_data = G_receive_data ;
  BFCT_protocol_Zigbee.send_data = G_send_data ;
  
  BFCT_protocol_Lock.receive_data = G_receive_data ;
  BFCT_protocol_Lock.send_data = G_send_data ;
  
  USART_process_flag =1;
  data3_tc =(UART_TIMEOUT+1);
  while(1)
  {

   led();  //���
   lock_uart_send_session();
   zigbee_uart_send_session();
   uart_receiving_timeout();

   if(USART_process_flag)
   {
      USART_process();
   }
   
#if defined(Jun_He) 
  if(transparently_flag)
  {
    USART_process_flag = 0;
    ret = transparently_process();
    if(ret ==1)
    {
      sys_timer = deadline;
    }
    else if(ret ==2)
    {
      sys_timer = deadline;
      transparently_flag =0;
      USART_process_flag =1;
      write_userdata2eeprom(transparently_flag_addr,&transparently_flag, 1);
    }
  }
#endif   
#if defined(Hui_huang) 
    delete_user_process();
#endif
   
   if((Zigbee_process_done | Lock_process_done) | (sys_timer >= deadline) )
   {
      delay =0;
      lock_state=0;
      zigbee_state =0;
      sys_timer=0;
      
      zigbee_erro =0;
      lock_erro =0;
      
      Zigbee_process_done =0;  //���������ɱ�־λ
      Lock_process_done =0;
      
      Zigbee_processing_flag =0;  //������ڴ���ʹ�ܺ���
      Lock_processing_flag =0;
      
      BFCT_protocol_Lock.receive_enable=0;  //������ڽ��ձ�־λ
      BFCT_protocol_Lock.receive_len =0;
      
      lock_interrupt = 0;
      zigbee_interrupt = 0;
     
      data3_tc =(UART_TIMEOUT+1);
      
      USART_Cmd(USART1 , DISABLE);

      init_pin_interrupt();//����͹���ģʽ

      deadline = 1;
      if(BFCT_protocol_Zigbee.receive_len || zigbee_interrupt){
        config_lock_rx_pin(GPIO_Mode_In_FL_No_IT); //���û�������Ϊ ���жϵĸ������� ��������Ϊ��ģ�黽������
         deadline = 300;
      }
      if(lock_interrupt)
         deadline = 300;   
      
   }
  if(lock_interrupt)
  {
      delay ++;  
      if(delay ==1){
        config_lock_rx_pin(GPIO_Mode_In_FL_No_IT); //���û�������Ϊ ���жϵĸ������� ��������Ϊ��ģ�黽������
        config_wake_up_out(GPIO_Mode_In_FL_No_IT); //���û�������Ϊ ���жϵĸ������� 
      }
      if( delay > 0  && delay < 9)
      {
        if( get_lock_rx_pin())
        {  }
        else
        { 
          sys_timer = deadline; 
          delay =0; 
        }
        
      }
      if(delay ==9) {

        delay =0;
        lock_interrupt =0;
        UART1_RemapInit(9600);//������ ���ó� 9600
     //   config_lock_tx_pin(GPIO_Mode_Out_PP_High_Fast); //TXD
        GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TXD
        BFCT_protocol_Lock.receive_enable=1;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Lock.receive_len =0;
   //     enableInterrupts();
      }
  }
   
  else if(zigbee_interrupt)
  {   
      config_lock_rx_pin(GPIO_Mode_In_FL_No_IT); //���û�������Ϊ ���жϵĸ������� ��������Ϊ��ģ�黽������
      config_wake_up_out(GPIO_Mode_In_FL_No_IT); //���û�������Ϊ ���жϵĸ������� 
      zigbee_interrupt = 0;
      UART1_Init(ZIGBEE_BAUD);//������ ���ó� 19200
      BFCT_protocol_Lock.receive_enable=0;
      BFCT_protocol_Zigbee.receive_enable =1;
      BFCT_protocol_Zigbee.receive_len =0;
  //    enableInterrupts();
  }

    if(t_1ms)
      sys_timer ++;
    
    os_time();   //ϵͳʱ��
  }
  
}