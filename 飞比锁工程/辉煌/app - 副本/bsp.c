#include "stm8l15x.h"
#include "bsp.h"
#include "global.h"
#include "protocol.h"
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
  
   GPIO_WriteBit(GPIOC , GPIO_Pin_4 ,(BitAction)GPIO_BitVal);  //����PB0Ϊ�ߵ�ƽ������ģ��
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
  GPIO_Init(GPIOB,GPIO_Pin_0,(GPIO_Mode_TypeDef)GPIO_Mode); //PB0, ��������,���ж� 
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
  GPIO_Init(GPIOA,GPIO_Pin_3,(GPIO_Mode_TypeDef)GPIO_Mode);  //PA3,rx, ��������,���ж�
}

/**********
*�������� �� ����ģ��rx���ŵ� ����
*GPIO_Mode_Out_PP_High_Fast
********/
void config_lock_tx_pin(u8 GPIO_Mode)
{
  GPIO_Init(GPIOA, GPIO_Pin_2, (GPIO_Mode_TypeDef)GPIO_Mode); //TXD
}
/**********
*�������� �� ����ģ�����ŵĸߵ͵�ƽ
********/
void set_lock_tx_pin(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)GPIO_BitVal); //TXD
}

/*******************************************************************************
**�������ƣ�void TIM2_Init()     Name: void TIM2_Init()
**������������ʼ����ʱ��2
**��ڲ�������
**�������
*******************************************************************************/
void TIM2_Init(u16 prescaler)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM2 , ENABLE);              //ʹ�ܶ�ʱ��2ʱ��
  TIM2_TimeBaseInit(TIM2_Prescaler_1 , TIM2_CounterMode_Up , prescaler);    //���ö�ʱ��2Ϊ1��Ƶ�����ϼ���������ֵΪ16000��Ϊ1����ļ���ֵ
}

void TIM3_Init(u16 period)
{
  TIM3_Cmd(DISABLE);  
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM3 , ENABLE);              //ʹ�ܶ�ʱ��3ʱ��
  TIM3_TimeBaseInit(TIM2_Prescaler_128 , TIM3_CounterMode_Up , period);    //���ö�ʱ��3Ϊ1��Ƶ�����ϼ���������ֵΪ16000��Ϊ1����ļ���ֵ
  
    ITC->ISPR6 &= (uint8_t)(~(uint8_t)(0x03U << ((21% 4U) * 2U)));
    ITC->ISPR6 |=(uint8_t)((uint8_t)(ITC_PriorityLevel_1) << ((21 % 4U) * 2U));
    
  TIM3_ITConfig(TIM3_IT_Update , ENABLE);     //ʹ�����ϼ�������ж�
  TIM3_ARRPreloadConfig(ENABLE);  //ʹ�ܶ�ʱ��3�Զ����ع���    
  
  TIM3_Cmd(DISABLE);               //������ʱ��3��ʼ����
}
/*******************************************************************************
**�������ƣ�void IOInit()     Name: void IOInit()
**������������ʼ������LED��IO��
**��ڲ�������
**�������
*******************************************************************************/
void IOInit()
{
    GPIOA->DDR = 0xff;
    GPIOA->ODR = 0x04;
    GPIOB->DDR = 0xff;
    GPIOB->ODR = 0x00;
    GPIOC->DDR = 0xff;
    GPIOC->ODR = 0x10;
    GPIOD->DDR = 0xff;
    GPIOD->ODR = 0x00;
  GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //PA2,tx,�͵�ƽ���
  GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast); //Wakeup_IN  
}

/*******************************************************************************
**�������ƣ�LED2
**������������ʼ������LED��IO��
**��ڲ�������
**�������
*******************************************************************************/
void LED2(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOC , GPIO_Pin_1 ,(BitAction)GPIO_BitVal);  //����PB0Ϊ�ߵ�ƽ������ģ��
}


void init_pin_interrupt(void)
{
    u16 delay=0; 
    u8 ret; 


    disableInterrupts();

    UART1_Init(ZIGBEE_BAUD);//������ ���ó� 19200
    USART1->CR2 &= (uint8_t)~(USART_CR2_TEN);
    
    BFCT_protocol_Lock.receive_enable=0;
    BFCT_protocol_Zigbee.receive_enable =1;
    BFCT_protocol_Zigbee.receive_len =0;

    GPIOA->DDR = 0xff;
    GPIOA->ODR = 0x00;
    GPIOB->DDR = 0xff;
    GPIOB->ODR = 0x00;

//    GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode_In_FL_IT); //PC4, ��������,���ж� 
    GPIO_Init(GPIOC,GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,�͵�ƽ���
//    GPIO_Init(GPIOC,GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,�͵�ƽ���
    
 //   GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast); //Wakeup_IN  

    
    GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,�͵�ƽ���
    GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_In_FL_IT); //PA3,rx, ��������,���ж�

    
    for(delay=0;delay<5000;delay++);//��ʱһ������ȴ�IO���ȶ�
    EXTI->CR1=0xff;
    EXTI->CR2=0xff;
    EXTI->CR3=0XFF;
    EXTI->SR1=0XFF;
    EXTI->SR2=0XFF;  
    

    enableInterrupts();	
    if(Wfi_Mode)
       wfi();
    else{
      GPIO_Init(GPIOC,GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,�͵�ƽ���
       halt();  //����˯��
    }

}


void delay_s(u8 time)
{
  volatile u16 i,j,k;
  for(k=0;k<time;k++){
    for(i =0;i<1000;i++)
     for(j=0;j<1000;j++);
  }
}

