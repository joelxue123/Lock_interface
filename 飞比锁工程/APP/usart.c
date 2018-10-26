#include "usart.h"
#include "global.h"
#include "eeprom.h"
#include "bsp.h"
#include "stm8l15x_iwdg.h"
/**********************
** ��������: ���ڳ�ʼ��
**������ baudrate ������
**********************/
void UART1_Init(unsigned char remap)
{ 
  
   SYSCFG->RMPCR1 &= 0xcf;
  if(remap == 0x01)
  {
      GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Fast); //TXD
      GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_In_PU_No_IT);      //RXD
     SYSCFG->RMPCR1 |= 0x10;
  }
  else
  {
     SYSCFG->RMPCR1 |= 0x20;
  }
        CLK->PCKENR1 |= (uint8_t)((uint8_t)1 << ((uint8_t)0x05)); //����ʱ��
	USART1->CR2 &= ~(USART_CR2_REN|USART_CR2_TEN);//disable receiver and transmision
	USART1->SR=0;//TC=0;
	USART1->BRR2 = BRR2_data;
	USART1->BRR1 = BRR1_data;
        if(remap ==1)
        {
          USART1->BRR2 = 0x02;
          USART1->BRR1 = 0x68;
        }
	USART1->CR1=0;//R8=T8=0;USART enable;8 Data bits,ildle line wake up,parity disable,even parity,parity interrupt disable;
	USART1->CR3=0;//STOP bit=1 bits.USART_CK disable,
	USART1->CR2=(USART_CR2_REN|USART_CR2_RIEN|USART_CR2_TEN);//enable transmision
#if defined(Hui_huang) 
      ITC->ISPR8 &=(uint8_t)(~(uint8_t)(0x03U << ((28 % 4U) * 2U)));
    ITC->ISPR8 |= (uint8_t)((uint8_t)(ITC_PriorityLevel_2) << ((28 % 4U) * 2U));
#endif
  USART1->CR1 &= (uint8_t)(~USART_CR1_USARTD); /**< USART Enable */
}

/**********************
** ��������: ���ڷ�������
**������ char *p ���͵�����, u8 len ���͵��ֽ���
**********************/
void send_hex(u8 *p,u8 len)
{
  u8 i;
  for(i=0;i<len;i++){
    
    if ((WWDG->CR & 0x7F) < WWDG->WR) //С�ڴ���ֵ����ι��  
       WWDG->CR |= 0x7F; //����ι�� 
    
    while( (USART1->SR & (uint8_t)USART_FLAG_TXE) == 0);       
    USART_SendData8(USART1 , (u8)p[i]);             //���ͼĴ���д������     
  }
  while( (USART1->SR & (uint8_t)USART_FLAG_TXE) == 0);   //�ȴ����ݷ������
}