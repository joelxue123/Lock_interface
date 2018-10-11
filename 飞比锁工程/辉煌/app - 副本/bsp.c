#include "stm8l15x.h"
#include "bsp.h"
#include "global.h"
#include "protocol.h"
#include "usart.h"

/***********
** 函数名称：wake_up_in()
**功能描述：唤醒模块函数
**入口参数 ：cmd 
SET,为高电平
RESET为低电平
**返回值 
***********/
void wake_up_in(u8 GPIO_BitVal)
{
  
   GPIO_WriteBit(GPIOC , GPIO_Pin_4 ,(BitAction)GPIO_BitVal);  //设置PB0为高电平，唤醒模块
}

/***********
** 函数名称：wake_up_in()
**功能描述：唤醒模块函数
**入口参数 ：cmd  
**GPIO_Mode_In_FL_No_IT
**
**返回值 
***********/
void config_wake_up_out(u8 GPIO_Mode)
{
  GPIO_Init(GPIOB,GPIO_Pin_0,(GPIO_Mode_TypeDef)GPIO_Mode); //PB0, 浮空输入,带中断 
}
/*****模块唤醒后，获取zigbee的唤醒引脚的高低电平 **********/
u8 get_wakeup_out_pin_state()   //获取wakeup_out pin的状态
{
  return GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0); 
}


//////////***********所有的收发，都是对本模块而已，对应对方模块则是相反的****************//

/***模块唤醒以后，获取rx引脚的电平**********/
u8 get_lock_rx_pin()  //获取本模块rx引脚
{
  return GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3); //PA3,rx, 浮空输入,带中断
}
/*** 设置锁的rx引脚，*********/
void config_lock_rx_pin(u8 GPIO_Mode)  //获取本模块rx引脚
{
  GPIO_Init(GPIOA,GPIO_Pin_3,(GPIO_Mode_TypeDef)GPIO_Mode);  //PA3,rx, 浮空输入,带中断
}

/**********
*函数描述 ： 配置模块rx引脚的 功能
*GPIO_Mode_Out_PP_High_Fast
********/
void config_lock_tx_pin(u8 GPIO_Mode)
{
  GPIO_Init(GPIOA, GPIO_Pin_2, (GPIO_Mode_TypeDef)GPIO_Mode); //TXD
}
/**********
*函数描述 ： 设置模块引脚的高低电平
********/
void set_lock_tx_pin(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_2, (BitAction)GPIO_BitVal); //TXD
}

/*******************************************************************************
**函数名称：void TIM2_Init()     Name: void TIM2_Init()
**功能描述：初始化定时器2
**入口参数：无
**输出：无
*******************************************************************************/
void TIM2_Init(u16 prescaler)
{
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM2 , ENABLE);              //使能定时器2时钟
  TIM2_TimeBaseInit(TIM2_Prescaler_1 , TIM2_CounterMode_Up , prescaler);    //设置定时器2为1分频，向上计数，计数值为16000即为1毫秒的计数值
}

void TIM3_Init(u16 period)
{
  TIM3_Cmd(DISABLE);  
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM3 , ENABLE);              //使能定时器3时钟
  TIM3_TimeBaseInit(TIM2_Prescaler_128 , TIM3_CounterMode_Up , period);    //设置定时器3为1分频，向上计数，计数值为16000即为1毫秒的计数值
  
    ITC->ISPR6 &= (uint8_t)(~(uint8_t)(0x03U << ((21% 4U) * 2U)));
    ITC->ISPR6 |=(uint8_t)((uint8_t)(ITC_PriorityLevel_1) << ((21 % 4U) * 2U));
    
  TIM3_ITConfig(TIM3_IT_Update , ENABLE);     //使能向上计数溢出中断
  TIM3_ARRPreloadConfig(ENABLE);  //使能定时器3自动重载功能    
  
  TIM3_Cmd(DISABLE);               //启动定时器3开始计数
}
/*******************************************************************************
**函数名称：void IOInit()     Name: void IOInit()
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
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
  GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //PA2,tx,低电平输出
  GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast); //Wakeup_IN  
}

/*******************************************************************************
**函数名称：LED2
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
*******************************************************************************/
void LED2(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOC , GPIO_Pin_1 ,(BitAction)GPIO_BitVal);  //设置PB0为高电平，唤醒模块
}


void init_pin_interrupt(void)
{
    u16 delay=0; 
    u8 ret; 


    disableInterrupts();

    UART1_Init(ZIGBEE_BAUD);//波特率 设置成 19200
    USART1->CR2 &= (uint8_t)~(USART_CR2_TEN);
    
    BFCT_protocol_Lock.receive_enable=0;
    BFCT_protocol_Zigbee.receive_enable =1;
    BFCT_protocol_Zigbee.receive_len =0;

    GPIOA->DDR = 0xff;
    GPIOA->ODR = 0x00;
    GPIOB->DDR = 0xff;
    GPIOB->ODR = 0x00;

//    GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode_In_FL_IT); //PC4, 浮空输入,带中断 
    GPIO_Init(GPIOC,GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,低电平输出
//    GPIO_Init(GPIOC,GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,低电平输出
    
 //   GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast); //Wakeup_IN  

    
    GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,低电平输出
    GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_In_FL_IT); //PA3,rx, 浮空输入,带中断

    
    for(delay=0;delay<5000;delay++);//延时一会儿，等待IO口稳定
    EXTI->CR1=0xff;
    EXTI->CR2=0xff;
    EXTI->CR3=0XFF;
    EXTI->SR1=0XFF;
    EXTI->SR2=0XFF;  
    

    enableInterrupts();	
    if(Wfi_Mode)
       wfi();
    else{
      GPIO_Init(GPIOC,GPIO_Pin_6, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,低电平输出
       halt();  //进入睡眠
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

