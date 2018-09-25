#include "stm8l15x.h"
#include "global.h"


u8 key[]={0x46,0x45,0x49,0x42,0x47};
u8 password[6]={1,2,3,4,5,6};
u8 npass[6];

u8 G_receive_data[100]={0xAA ,0x0A, 0x40, 00, 00, 00, 0x01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xE1, 0x55};
u8 G_send_data[100]={0xAA ,0x0A, 0x40, 00, 00, 00, 0x01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 0xE1, 0x55};

BFCT_protocol BFCT_protocol_Zigbee;
BFCT_protocol BFCT_protocol_Lock;

u8 data3_tc;
u8 timeout_flag;
u8 t_1ms;

u8 zigbee_send_cmd;

#define ON 1
#define OFF 1
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

/**************************************************************************
**函数名称：void crc_8n()     Name: void crc_8n()
**功能描述 检验和
**入口参数：无
**输出：无
**************************************************************************/

u8 crc_8n(unsigned char *ptr,unsigned char n)
{
	unsigned char j=8;
	unsigned char cbit,out;
	unsigned char crc=0x79,crc_a=0,crc_b=0;	
	//*取移位的位*/	
	do{
		crc_a=*ptr;
		ptr++;
		j=8;
		cbit=1;
		do{
			crc_b=crc_a;
			crc_b=crc_b^crc;
			out=crc_b&cbit;
		//	crc_b=crc_b>>1;
			if(out!=0)
                        {
                                crc=crc^0X18;
                                crc=crc>>1;
                                crc|=0x80;
                        }else
                                crc=crc>>1;
			crc_a=crc_a>>1;
		//	cbit=cbit<<1;
		}while(--j);
	}while(--n);
	return crc;
}

/***********
** 函数名称：sum_check()
**功能描述：和校验
**入口参数 ： char * ，u8 len
**返回值  检验和 sum
***********/

u8 sum_check(char *p,u16 len)
{
  u16 i;
  u8 sum=0;
  for(i=0;i<len;i++)
  {
    sum = sum +p[i];
  }
  return sum;
}
/***********
** 函数名称：xor_check()
**功能描述：和校验
**入口参数 ： char * ，u8 len
**返回值  异或检验和 xor
***********/

u8 xor_check(char *p,u16 len)
{
  u16 i;
  u8 xor=0;
  for(i=0;i<len;i++)
  {
    xor = xor^p[i];
  }
  return xor;
}

/*******************************************************************************
**函数名称：void TIM2_Init()     Name: void TIM2_Init()
**功能描述：初始化定时器2
**入口参数：无
**输出：无
*******************************************************************************/
void TIM2_Init()
{
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM2 , ENABLE);              //使能定时器2时钟
  TIM2_TimeBaseInit(TIM2_Prescaler_1 , TIM2_CounterMode_Up , 16000);    //设置定时器2为1分频，向上计数，计数值为16000即为1毫秒的计数值
}

/*******************************************************************************
**函数名称：void TIM2_DelayMs(unsigned int ms)
**功能描述：定时器2参进行精确延时，最小为1毫秒，最大65535
**入口参数：unsigned int ms     1=< ms <= 65535 
**输出：无
*******************************************************************************/
void TIM2_DelayMs(unsigned int ms)
{ 
  TIM2_ARRPreloadConfig(ENABLE);  //使能定时器2自动重载功能    
  TIM2_Cmd(ENABLE);               //启动定时器2开始计数
  while(ms--)
  {
    while( TIM2_GetFlagStatus(TIM2_FLAG_Update) == RESET); //等待计数是否达到1毫秒
    TIM2_ClearFlag(TIM2_FLAG_Update);  //计数完成1毫秒，清除相应的标志
  }
  TIM2_Cmd(DISABLE);                   //延时全部结束，关闭定时器2
}
/*******************************************************************************
**函数名称：void All_LEDInit()     Name: void All_LEDInit()
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
*******************************************************************************/
void All_LEDInit()
{
  GPIO_Init(GPIOB , GPIO_Pin_2 , GPIO_Mode_Out_PP_Low_Fast);  //初始化LED1，设置PB_2为快速输出模式
  GPIO_Init(GPIOB , GPIO_Pin_1 , GPIO_Mode_Out_PP_Low_Fast);  //初始化LED2，设置PB_1为快速输出模式
  GPIO_Init(GPIOB , GPIO_Pin_6 , GPIO_Mode_Out_PP_Low_Fast);  //初始化LED3，设置PB_6为快速输出模式
}
/*******************************************************************************
**函数名称：LED2
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
*******************************************************************************/
void LED2(u8 cmd)
{
  if(cmd == ON)
    GPIO_WriteBit(GPIOB , GPIO_Pin_1 ,RESET);  //设置PB1输出低电平 ，使LED2点亮  
  else
    GPIO_WriteBit(GPIOB , GPIO_Pin_1 ,SET);  //设置PB1输出低电平 ，使LED2点亮  
}
/*******************************************************************************
**函数名称：LED3
**功能描述：初始化所有LED灯IO口
**入口参数：无
**输出：无
*******************************************************************************/
void LED3(u8 cmd)
{
    if(cmd == ON)
    GPIO_WriteBit(GPIOB , GPIO_Pin_6 ,RESET);  //设置PB1输出低电平 ，使LED3点亮  
  else
    GPIO_WriteBit(GPIOB , GPIO_Pin_6 ,SET);  //设置PB1输出低电平 ，使LED3点亮 
}

/**********************
** 函数描述: 串口初始化
**参数， baudrate 波特率
**********************/
void UART1_Init(unsigned int baudrate)
{ 
    //GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TXD
  SYSCFG_REMAPDeInit();
  SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortC , ENABLE); //把USART1串口模块默认的PC5(TX) , PC6(RX)引脚，重映射到PA2(TX) ,PA3(RX)
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1 , ENABLE); //使能USART1时钟
  USART_Init(USART1,                //设置USART1
            baudrate,               //波特率设置
            USART_WordLength_8b,    //数据长度设为8位
            USART_StopBits_1,       //1位停止位
            USART_Parity_No,        //无校验
            (USART_Mode_Rx | USART_Mode_Tx));  //设置为发送接收双模式
  
  USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);
  USART_Cmd(USART1 , ENABLE);
}

/**********************
** 函数描述: 串口重新映射
**参数， baudrate 波特率
**********************/
void UART1_RemapInit(unsigned int baudrate)
{ 
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TXD
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_In_PU_No_IT);      //RXD
  
  SYSCFG_REMAPDeInit();
  SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortA , ENABLE); //把USART1串口模块默认的PC5(TX) , PC6(RX)引脚，重映射到PA2(TX) ,PA3(RX)
  
  CLK_PeripheralClockConfig(CLK_Peripheral_USART1 , ENABLE); //使能USART1时钟

  USART_Init(USART1,                //设置USART1
            baudrate,               //波特率设置
            USART_WordLength_8b,    //数据长度设为8位
            USART_StopBits_1,       //1位停止位
            USART_Parity_No,        //无校验
            (USART_Mode_Rx | USART_Mode_Tx));  //设置为发送接收双模式
  
  USART_ITConfig(USART1, USART_IT_RXNE , ENABLE);
   
  USART_Cmd(USART1 , ENABLE);
  
}
/**********************
** 函数描述: 串口发送数据
**参数， char *p 发送的数组, u8 len 发送的字节数
**********************/
void send_hex(char *p,u8 len)
{
  u8 i;
  for(i=0;i<len;i++){
    while(USART_GetFlagStatus(USART1 , USART_FLAG_TXE) == 0);       
    USART_SendData8(USART1 , (u8)p[i]);             //向发送寄存器写入数据     
  }
}

/**********
**函数描述 ： zigbee收到的数据协议,组合成锁的协议形式,只用作命令应答
**********/
void  lock_protocal_component_ACK(u8 cmd,u8 state)
{
    BFCT_protocol_Lock.send_data[0]=0x02;      //帧头
    
    BFCT_protocol_Lock.send_data[1]=0x06;      //长度
    BFCT_protocol_Lock.send_data[2]=0x00;     //长度
    BFCT_protocol_Lock.send_data[3]=cmd;     //命令字
    BFCT_protocol_Lock.send_data[4]=state;      //状态
    BFCT_protocol_Lock.send_data[5]=crc_8n(BFCT_protocol_Lock.send_data,5);  //检验
    BFCT_protocol_Lock.send_data[6]=0xff;     //长度
    
}
/**********
**函数描述 ：zigbee收到的数据协议,组合成锁的协议形式,只用作命令
**********/
void  lock_protocal_component(u16 len,u8 cmd,u8* data_buff)
{
    u16 i;
    BFCT_protocol_Lock.send_data[0]=0x02;      //帧头
    
    BFCT_protocol_Lock.send_data[1]=len &0x0f;      //长度
    BFCT_protocol_Lock.send_data[2]=(len >> 4)&0x0f;     //长度
    BFCT_protocol_Lock.send_data[3]=cmd;     //命令字
    for(i=0;i<len-1;i++)
    {
       BFCT_protocol_Lock.send_data[4+i] = data_buff[i];
    }
    BFCT_protocol_Lock.send_data[len+4]=crc_8n(BFCT_protocol_Lock.send_data,len+3);  //检验
    BFCT_protocol_Lock.send_data[len+5]=0xff;     //帧尾
    
    send_hex(BFCT_protocol_Lock.send_data,BFCT_protocol_Lock.send_len+6); // 发送数据
}


/***
** 重新组合zigbe协议
**/
void  zigbee_protocal_component(u8 len,u8 cmd,u32 cmd_id,u8* data_buff)
{
  u8 i;
  BFCT_protocol_Zigbee.send_data[0]=0xAA;      //帧头
  BFCT_protocol_Zigbee.send_data[1]=len;      //有效长度
  BFCT_protocol_Zigbee.send_data[2]=cmd;      //命令字
  BFCT_protocol_Zigbee.send_data[3]=cmd_id & 0x0f;      //命令字自增ID
  BFCT_protocol_Zigbee.send_data[4]=(cmd_id >> 4) & 0x0f;      //命令字自增ID
  BFCT_protocol_Zigbee.send_data[5]=(cmd_id >> 8) & 0x0f;     //命令字自增ID
  BFCT_protocol_Zigbee.send_data[6]=(cmd_id >> 12) & 0x0f;    //命令字自增ID
  BFCT_protocol_Zigbee.send_data[7]=0x00;      //应答
  
  for(i=0;i<len;i++){
     BFCT_protocol_Zigbee.send_data[8+i]=data_buff[i];      //应答  
  }
  BFCT_protocol_Zigbee.send_data[8+len]=xor_check(BFCT_protocol_Zigbee.send_data,len+8);     //长度
  BFCT_protocol_Zigbee.send_data[9+len]=0x55;      //添加用户控制字段
  BFCT_protocol_Zigbee.send_len =len+10; 
}


/**********
**函数描述 ： **模块协议解析
**********/
void lock_protocal_analysis(void)
{

  
}
/**********
**函数描述 ： 讲zigbee收到的数据协议 转行成 锁协议
**********/
void zigbee_convert2lock(void)
{


}


/**********
**函数描述 ： 时间节拍器,1ms溢出判断
**********/
void os_time(void) 
{
    if( ( TIM2_GetFlagStatus(TIM2_FLAG_Update) != RESET) ) //等待计数是否达到1毫秒
    {
      t_1ms=1;
      TIM2_ClearFlag(TIM2_FLAG_Update);
    }else t_1ms=0;
  //////////////////////////////////////////////// 实时时钟
 
 
  
}

/**********************
** 函数描述: 点灯
**********************/
void led(void)
{
   static u8 led_on=0;
   static  u16 timer=0;
   if(t_1ms)
   {
    timer++;
    
    if(timer == 2000){
      led_on=1-led_on;
      timer=0;
    }
    
    if(led_on)
      GPIO_WriteBit(GPIOB , GPIO_Pin_2 ,RESET);  //设置PB2输出低电平 ，使LED1点亮  
    else
      GPIO_WriteBit(GPIOB , GPIO_Pin_2 ,SET);  //设置PB2输出低电平 ，使LED1点亮 
   }
}

void init_pin_interrupt(void)
{
  disableInterrupts();
 // GPIO_Init(GPIOC,GPIO_Pin_6,GPIO_Mode_In_FL_IT); //PC6,rx, 浮空输入,带中断
  GPIO_Init(GPIOC,GPIO_Pin_5, GPIO_Mode_Out_PP_Low_Fast); //PC5,tx,低电平输出
   GPIO_WriteBit(GPIOC , GPIO_Pin_5 ,SET);  //设置PB2输出低电平 ，使LED1点亮 
  
  GPIO_Init(GPIOA,GPIO_Pin_3,GPIO_Mode_In_FL_IT); //PA3,rx, 浮空输入,带中断
  GPIO_Init(GPIOA,GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //PA2,tx,低电平输出
  
    EXTI->CR1=0xff;
    EXTI->CR2=0xff;
    EXTI->CR3=0XFF;
    EXTI->SR1=0XFF;
    EXTI->SR2=0XFF;  
    enableInterrupts();	

    halt();
    
}

void main()
{
   static  u16 timer=0;
  disableInterrupts();   //关闭系统总中断
  CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1); //内部时钟为1分频 = 16Mhz 
  All_LEDInit();
  TIM2_Init();
  TIM2_ARRPreloadConfig(ENABLE);  //使能定时器2自动重载功能    
  TIM2_Cmd(ENABLE); 
  GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TXD
  GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_In_PU_No_IT);      //RXD
  GPIO_Init(GPIOC, GPIO_Pin_5, GPIO_Mode_Out_PP_High_Fast); //TXD
  GPIO_Init(GPIOC, GPIO_Pin_6, GPIO_Mode_In_PU_No_IT);      //RXD
  
  GPIO_Init(GPIOB, GPIO_Pin_0, GPIO_Mode_Out_PP_Low_Fast); //TXD  
  
  UART1_Init(57600);
  enableInterrupts();   //使能系统总中断

  BFCT_protocol_Zigbee.receive_enable = 1;
  BFCT_protocol_Zigbee.receive_flag = 0;
  BFCT_protocol_Zigbee.receive_data = G_receive_data ;
  BFCT_protocol_Zigbee.send_data = G_send_data ;
  
  BFCT_protocol_Lock.receive_enable=0;
  BFCT_protocol_Lock.receive_len =0;
  BFCT_protocol_Lock.receive_flag = 0;
  BFCT_protocol_Lock.receive_data = G_receive_data ;
  BFCT_protocol_Lock.send_data = G_send_data ;
 
  while(1)
  {
 //  led();  //点灯
   if(t_1ms)
   {
        timer++;
        
        if(timer ==800)
        {
          GPIO_WriteBit(GPIOB , GPIO_Pin_0 ,SET);  //设置PB2输出低电平 ，使LED1点亮 
        }
        if(timer ==830)
        {
          
          timer =0;
            SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortC , ENABLE); //把USART1串口模块默认的PC5(TX) , PC6(RX)引脚
            send_hex(G_send_data,20);
            GPIO_WriteBit(GPIOB , GPIO_Pin_0 ,RESET);  //设置PB2输出低电平 ，使LED1点亮 
        }
     }
   if(BFCT_protocol_Zigbee.receive_flag == 1)
   {
    
     SYSCFG_REMAPPinConfig(REMAP_Pin_USART1TxRxPortA , ENABLE); //把USART1串口模块默认的PC5(TX) , PC6(RX)引脚，重映射到PA2(TX) ,PA3(RX)
     send_hex(G_receive_data,BFCT_protocol_Zigbee.receive_len);
     BFCT_protocol_Zigbee.receive_enable =1;
     BFCT_protocol_Zigbee.receive_flag =0;
     BFCT_protocol_Zigbee.receive_len =0;
   
   }
   os_time();   //系统时钟

  }
  
}