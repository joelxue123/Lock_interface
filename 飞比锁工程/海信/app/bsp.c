#include "stm8l15x.h"
#include "bsp.h"
#include "global.h"
#include "usart.h"

/***********
** �������ƣ�wake_up_in()
**��������������ģ�麯��
**��ڲ��� ��cmd 
SET,Ϊ�ߵ�ƽ
RESETΪ�͵�ƽ
**����ֵ 
***********/
void wake_up_in(u8 GPIO_BitVal)
{
  
   GPIO_WriteBit(GPIOC , GPIO_Pin_4 ,GPIO_BitVal);  //����PB0Ϊ�ߵ�ƽ������ģ��
}

/***********
** �������ƣ�wake_up_in()
**��������������ģ�麯��
**��ڲ��� ��cmd  
**GPIO_Mode_In_FL_No_IT
**
**����ֵ 
***********/
void config_wake_up_out(u8 GPIO_Mode)
{
  GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode); //PB0, ��������,���ж� 
}
/*****ģ�黽�Ѻ󣬻�ȡzigbee�Ļ������ŵĸߵ͵�ƽ **********/
u8 get_wakeup_out_pin_state()   //��ȡwakeup_out pin��״̬
{
  return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0); 
}


//////////***********���е��շ������ǶԱ�ģ����ѣ���Ӧ�Է�ģ�������෴��****************//

/***ģ�黽���Ժ󣬻�ȡrx���ŵĵ�ƽ**********/
u8 get_lock_rx_pin()  //��ȡ��ģ��rx����
{
  return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3); //PA3,rx, ��������,���ж�
}
/*** ��������rx���ţ�*********/
void config_lock_rx_pin(u8 GPIO_Mode)  //��ȡ��ģ��rx����
{
  GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode);  //PA3,rx, ��������,���ж�
}

/**********
*�������� �� ����ģ��rx���ŵ� ����
*GPIO_Mode_Out_PP_High_Fast
********/
void config_lock_tx_pin(u8 GPIO_Mode)
{
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode); //TXD
}
/**********
*�������� �� ����ģ�����ŵĸߵ͵�ƽ
********/
void set_lock_tx_pin(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_2, GPIO_BitVal); //TXD
}



/*******************************************************************************
**�������ƣ�void IOInit()     Name: void IOInit()
**������������ʼ������LED��IO��
**��ڲ�������
**�������
*******************************************************************************/
void IOInit()
{
    GPIOB->DDR = 0xff;
    GPIOB->ODR = 0x00;
    GPIOD->DDR = 0xff;
    GPIOD->ODR = 0x00;
    
    GPIOC->DDR = 0xFf;
    GPIOC->ODR = 0x10;
 
    GPIOB->DDR = 0xff;
    GPIOB->ODR = 0x04;
//  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TX_REMAP

}

/*******************************************************************************
**�������ƣ�LED2
**������������ʼ������LED��IO��
**��ڲ�������
**�������
*******************************************************************************/
void LED2(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOC , GPIO_Pin_1 ,GPIO_BitVal);  //����PB0Ϊ�ߵ�ƽ������ģ��
}


void init_pin_interrupt(void)
{
    u16 delay=0; 
    
    disableInterrupts();
    USART1->CR2=0;//disable transmision and receiver
    USART_Cmd(USART1 , DISABLE);
#if 0
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortC , DISABLE); //��USART1����ģ��Ĭ�ϵ�PC5(TX) , PC6(RX)����
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortA , DISABLE); //��USART1����ģ��Ĭ�ϵ�PC5(TX) , PC6(RX)���ţ���ӳ�䵽PA2(TX) ,PA3(RX)
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1 , DISABLE); //ʹ��USART1ʱ��
#endif
    GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode_In_FL_IT); //PC4, ��������,���ж� 
    GPIO_Init(GPIOC,GPIO_Pin_6, GPIO_Mode_In_FL_No_IT); //PC5,tx,�͵�ƽ���
//    GPIO_Init(GPIOC,GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,�͵�ƽ���
    GPIO_Init(GPIOC,GPIO_Pin_5, GPIO_Mode_In_FL_No_IT); //PC5,tx,�͵�ƽ���
    GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_Low_Fast); //Wakeup_IN 
    
    GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,�͵�ƽ���
    GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_In_FL_IT); //PA3,rx, ��������,���ж�

    
    for(delay=0;delay<5000;delay++);//��ʱһ������ȴ�IO���ȶ�
    EXTI->CR1=0xff;
    EXTI->CR2=0xff;
    EXTI->CR3=0XFF;
    EXTI->SR1=0XFF;
    EXTI->SR2=0XFF;  
    enableInterrupts();	
    
    
    halt();  //����˯��

}

/**********
**�������� �� �Ͼ�ģ�� �ظ� ACK ����
**********/
u8 zigbee_usart_send(void)
{
    static u8 delay=0;
    if(t_1ms){
      delay++;
      if(delay ==1){
          enableInterrupts();    //�ƺ�ûɶ�ã�������Ϊɶ�ŵ�������
          UART1_Init(!Re_map); //PC5,PC6Ϊ����,��zigbeeģ��ͨѶ
          wake_up_in(RESET);
      }
      if(delay ==20)   
          wake_up_in(SET);
      if(delay ==40)
      {
        send_hex(BFCT_protocol_Zigbee.send_data,BFCT_protocol_Zigbee.send_len); // ��������

      } 
      if(delay ==45)
      {
        delay =0;
        BFCT_protocol_Zigbee.receive_enable =1; //ʹ��zigbeeģ�� ���չ���
        BFCT_protocol_Lock.receive_enable =0;   //ʧ��lock���ڽ��� 
        BFCT_protocol_Zigbee.receive_len =0;
        BFCT_protocol_Zigbee.receive_flag =0;
         wake_up_in(RESET);
        return 1;
      }
      if(delay > 100)
      {
        delay =0;
      }
    }  
   return 0;
}  


