#include "stm8l15x.h"
#include "bsp.h"
#include "global.h"
#include "usart.h"

/***********
** 函数名称：wake_up_in()
**功能描述：唤醒模块函数
**入口参数 ：cmd 
SET,为高电平
RESET为低电平
**返回值 
***********/
inline void wake_up_in(u8 GPIO_BitVal)
{
   GPIO_WriteBit(GPIOC , GPIO_Pin_4 ,GPIO_BitVal);  //设置PB0为高电平，唤醒模块
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
  GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode); //PB0, 浮空输入,带中断 
}
/*****模块唤醒后，获取zigbee的唤醒引脚的高低电平 **********/
u8 get_wakeup_out_pin_state()   //获取wakeup_out pin的状态
{
  return (GPIOB->IDR & (uint8_t)GPIO_Pin_0);
}


//////////***********所有的收发，都是对本模块而已，对应对方模块则是相反的****************//

/***模块唤醒以后，获取rx引脚的电平**********/
u8 get_lock_rx_pin()  //获取本模块rx引脚
{
  return (GPIOA->IDR & (uint8_t)GPIO_Pin_3);
}
/*** 设置锁的rx引脚，*********/
void config_lock_rx_pin(u8 GPIO_Mode)  //获取本模块rx引脚
{
  GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode);  //PA3,rx, 浮空输入,带中断
}

/**********
*函数描述 ： 配置模块rx引脚的 功能
*GPIO_Mode_Out_PP_High_Fast
********/
void config_lock_tx_pin(u8 GPIO_Mode)
{
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode); //TXD
}
/**********
*函数描述 ： 设置模块引脚的高低电平
********/
void set_lock_tx_pin(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOA, GPIO_Pin_2, GPIO_BitVal); //TXD
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

/*******************************************************************************
**函数名称：void IOInit()     Name: void IOInit()
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
*******************************************************************************/
void IOInit()
{

    GPIOB->DDR = 0xff;
    GPIOB->ODR = 0x00;
    GPIOD->DDR = 0xff;
    GPIOD->ODR = 0x00;
    
    GPIOC->DDR = 0xFf;
    GPIOC->ODR = 0x10;

    
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TX_REMAP
  GPIO_Init(GPIOC, GPIO_Pin_1, GPIO_Mode_Out_PP_High_Fast); //Wakeup_IN  



}

/*******************************************************************************
**函数名称：LED2
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
*******************************************************************************/
void LED2(u8 GPIO_BitVal)
{
  GPIO_WriteBit(GPIOC , GPIO_Pin_1 ,GPIO_BitVal);  //设置PB0为高电平，唤醒模块
}


void init_pin_interrupt(void)
{
    u16 delay=0; 
    
    disableInterrupts();
#if 0
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortC , DISABLE); //把USART1串口模块默认的PC5(TX) , PC6(RX)引脚
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortA , DISABLE); //把USART1串口模块默认的PC5(TX) , PC6(RX)引脚，重映射到PA2(TX) ,PA3(RX)
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1 , DISABLE); //使能USART1时钟
#endif
    GPIO_Init(GPIOB,GPIO_Pin_0,GPIO_Mode_In_FL_IT); //PC4, 浮空输入,带中断 
    GPIO_Init(GPIOC,GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,低电平输出
    GPIO_Init(GPIOC, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Fast); //Wakeup_IN 
    
    GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,低电平输出
    GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_In_FL_IT); //PA3,rx, 浮空输入,带中断

    
    for(delay=0;delay<5000;delay++);//延时一会儿，等待IO口稳定
    EXTI->CR1=0xff;
    EXTI->CR2=0xff;
    EXTI->CR3=0XFF;
    EXTI->SR1=0XFF;
    EXTI->SR2=0XFF;  
    enableInterrupts();	
    
    halt();  //进入睡眠

}

/**********
**函数描述 ： 南京模块 回复 ACK 流程
**********/
u8 zigbee_usart_send(void)
{
    static u8 delay=0;
    if(t_1ms){
      delay++;
      if(delay ==1)
          wake_up_in(SET);
      if(delay ==20)   
          wake_up_in(RESET);
      if(delay ==40)
      {
        enableInterrupts();    //似乎没啥用，忘记了为啥放到这里了
        UART1_Init(ZIGBEE_BAUD); //PC5,PC6为串口,与zigbee模块通讯
        wake_up_in(SET);
        send_hex(BFCT_protocol_Zigbee.send_data,BFCT_protocol_Zigbee.send_len); // 发送数据

      } 
      if(delay ==42)
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


