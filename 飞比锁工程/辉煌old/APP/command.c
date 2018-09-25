#include "stm8l15x.h"
#include "global.h"
#include "eeprom.h"
#include "usart.h"
#include "protocol.h"
#include "command.h"
#include "userdata.h"
#include "bsp.h"
#include "stm8l15x_gpio.h"

u8 zigbee_moni_state=0;
u16 zigbee_delay=0;

const u8 key[]={0};
 u8 password[12]={1,2,3,4,5,6};
u8 npass[6];
u8 data_buff[60]={0};

/**************************************************************************
**�������ƣ�void crc_8n()     Name: void crc_8n()
**�������� �����
**��ڲ�������
**�������
**************************************************************************/

u8 crc_8n(unsigned char *ptr,unsigned char n)
{
	unsigned char j=8;
	unsigned char cbit,out;
	unsigned char crc=0x79,crc_a=0,crc_b=0;	
	//*ȡ��λ��λ*/	
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
** �������ƣ�sum_check()
**������������У��
**��ڲ��� �� char * ��u8 len
**����ֵ  ����� sum
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
/***********
** �������ƣ�xor_check()
**������������У��
**��ڲ��� �� char * ��u8 len
**����ֵ  ������� xor
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
** �������ƣ�encryption()
**�����������������
**��ڲ��� ��
**����ֵ 
***********/

void encryption(void)
{
  u8 i;
  for(i=0;i<6;i++)
  {
    npass[i] =(u8) key[i] ^  password[i];
  }
}

/**********
*�������ƣ�lock_protocal_component
**�������� ����֡��lockЭ��
**���������� buf:������õ�֡��len:���ݳ���+������ cmd:Э������ ��data_buf :��������
**********/
u8  lock_protocal_component(u8*buf, u16 len,u8 cmd,u8* data_buff)
{
    u16 i;
    buf[0]=0x02;      //֡ͷ
    buf[1]=len &0xFf;      //����
    buf[2]=(len >> 8)&0xFf;     //����
    buf[3]=cmd;     //������
    for(i=0;i<len-1;i++)
    {
       buf[4+i] = data_buff[i];
    }
    buf[len+3]=crc_8n(buf,len+3);  //����
    buf[len+4]=0xff;     //֡β
    return (5+len);
}


/**********
*�������ƣ�zigbee_protocal_component
**�������� ����֡��zigbeeЭ��
**���������� buf:������õ�֡��len:���ݳ���+������ cmd:Э������ ��data_buf :��������
**********/
u8  zigbee_protocal_component(u8 *buf,u8 cmd,u8* data_buff)
{
  u8 i;
  
  buf[0]=0x7e;      //֡ͷ
  for(i=0;i<6;i++){
     buf[1+i]=data_buff[i];      //Ӧ��  
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
  
  buf[15]=sum_check(buf,15);     //����
  return (u8)(16); 
}

/**** У��������
*����ֵ �� 0 ��ʾ���� 1����ʾ��ȷ
*******/
u8 check_lock_cmd(u8 lock_cmd)
{
  u8 data_len=0;

  data_len = BFCT_protocol_Zigbee.receive_data[1] + (BFCT_protocol_Zigbee.receive_data[2] << 8); //��ȡ���ݳ���
  if(BFCT_protocol_Zigbee.receive_data[data_len +3] != crc_8n(BFCT_protocol_Zigbee.receive_data,data_len +3) )
  {  /**********����У����� ************/
    data_len =0 ;
#if DEBUG        
    lock_erro =  lock_erro | crc_8n_ERRO_BIT; 
#endif
    return 0;
  } 
  else if(BFCT_protocol_Zigbee.receive_data[3] != lock_cmd)
  {
#if DEBUG    
    lock_erro =  lock_erro | crc_8n_ERRO_BIT;
#endif
    return 0;
  }

  return 1;
}
/**** У��zigbee����
*����ֵ �� 0 ��ʾ���� 1����ʾ��ȷ
*******/
u8 check_zigbee_cmd(u8 zigbee_cmd)
{
  u8 data_len;
  data_len = (BFCT_protocol_Zigbee.receive_data[2] << 8) + BFCT_protocol_Zigbee.receive_data[3];
  if(BFCT_protocol_Zigbee.receive_data[data_len +3] != sum_check(BFCT_protocol_Zigbee.receive_data,data_len +3) )
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




/********
* ����������
*����ֵ�� 0 Ϊ��ɣ�1�����
************/
u8 lock_opennet()
{
  static u8 lock_moni_state=0;
  static u16 delay=0;
  u8 ret ;
  switch(lock_moni_state)
  {
    
  case 0:
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,check_module_if_online, data_buff);
        BFCT_protocol_Zigbee.receive_flag =0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        delay = 0;
        lock_moni_state ++;
    break;
  case 1:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        BFCT_protocol_Zigbee.receive_flag = 0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        if(BFCT_protocol_Zigbee.receive_data[3] == 0x01){
          delay = 0;
          lock_moni_state ++;
        }
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 1000){
         lock_moni_state=0;
         delay=0;
       }
     }
    break;
  case 2:
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        
        BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,set_up_network_sucess, data_buff);
        BFCT_protocol_Zigbee.receive_flag =0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        delay=0;
        sys_timer =0;
        lock_moni_state++;
    break;
  case 3:
     if(t_1ms)
     {
       delay++;
       if(delay > 9000){
         lock_moni_state=0;
         delay=0;
         return 1;
       }
     }
     break;
  default:
    break;
  }
  return 0;
}


/********
* �ر���������
*����ֵ�� 0 Ϊ��ɣ�1�����
************/
u8 lock_closenet(void)
{
return 1;
  
}

/*****���߸�ʽ������*********/
u8 lock_format(void)
{
return 1;
}

/*****���밲ȫģʽ���� *******/
u8 lock_login_safemode()
{
  return 1;
}
//*******�˳���ȫģʽ ************/
u8 lock_logout_safe_mode()
{
  return 1;
}
/**********�˳�����ģʽ*********/
u8 lock_logout_settting_mode(void)
{

    return 1;
}

/********ǿ�ƿ�������*********/
u8 lock_openclock_byforce()
{
  return 1;
}
/******�ϴ���״̬********/
u8 lock_state_updata()
{
  return 1;
}

/******��������û�����*******************/
u8 lock_add_user()
{ 
 return 1; 
}

u8 lock_be_opened(void)
{
  static u8 lock_moni_state=0;
  static u16 delay=0;
  static u8 key_type,battery1_level,battery2_level;
  u8 ret ;

  switch(lock_moni_state)
  {
  case 0:
          battery1_level = BFCT_protocol_Lock.receive_data[4];  
          battery2_level = BFCT_protocol_Lock.receive_data[5]; 
          key_type = BFCT_protocol_Lock.receive_data[9]; 
          
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,check_module_if_online, data_buff);
        BFCT_protocol_Zigbee.receive_flag =0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        delay = 0;
        lock_moni_state ++;
    break;
  case 1:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        BFCT_protocol_Zigbee.receive_flag = 0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        if(BFCT_protocol_Zigbee.receive_data[3] == 0x01){
          delay = 0;
          sys_timer =0;
          lock_moni_state ++;
        }
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 1000){
         lock_moni_state=0;
         delay=0;
       }
     }
    break;
  case 2:
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        switch(key_type)
        {
          case 0x80: 
               BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,password_unlock_sucess, data_buff);
            break;
          case 0x90:
               BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,fingerprint_unlock_sucess, data_buff);
            break;
          default:
               BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,IC_card_unlock_sucess, data_buff);
            break;
        }

        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        delay = 0;
        lock_moni_state ++;
    break;
  case 3:
    if(BFCT_protocol_Zigbee.send_len ==0)
    {
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        USART1->CR2=0;//disable transmision and receiver
        USART_Cmd(USART1 , DISABLE);
        delay = 0;
        lock_moni_state++;
    }
    break;
  case 4:
     if(t_1ms)
     {
       delay++;
       if(delay > 9000)
       {
         if(battery1_level + battery2_level < 50)
         {
             zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,low_power_alarm, data_buff);
             BFCT_protocol_Zigbee.send_len = 0x10;
            BFCT_protocol_Zigbee.receive_enable=0;
            BFCT_protocol_Zigbee.receive_len =0;
            delay = 0;
            sys_timer =0;
            lock_moni_state ++;
         }
         else
         {
            lock_moni_state = 0;
            delay=0;
            return 1;
         }
       }
     }
    break;
  case 5:
    if(BFCT_protocol_Zigbee.send_len ==0)
    {
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        USART1->CR2=0;//disable transmision and receiver
        USART_Cmd(USART1 , DISABLE);
        delay = 0;
        lock_moni_state++;
    }
    break;
  case 6:
     if(t_1ms)
     {
       delay++;
       if(delay > 9000){
         lock_moni_state = 0;
         delay=0;
         return 1;
       }
     }
    break;
  default:
    break;
  }
  return 0;
}
/********�Ƿ��û����ϱ�����**********/
u8 lolck_illage_user_report(void)
{
return 1;
}

/*****ɾ���û�**********/
u8 lock_delete_user(void)
{
return 1;
}

/*****���忪��******/
u8 lock_alarm(void)
{
  static u8 lock_moni_state=0;
  static u16 delay=0;
  u8 ret ,i;
  static u8 sys_time_counter = 0;
  switch(lock_moni_state)
  {
    
  case 0:
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,check_module_if_online, data_buff);
        BFCT_protocol_Zigbee.receive_flag =0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        delay = 0;
        lock_moni_state ++;
    break;
  case 1:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        BFCT_protocol_Zigbee.receive_flag = 0;
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        if(BFCT_protocol_Zigbee.receive_data[3] == 0x01){
          delay = 0;
          lock_moni_state ++;
        }
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 1000){
         lock_moni_state=0;
         delay=0;
       }
     }
    break;
  case 2:
       BFCT_protocol_Lock.receive_enable=0;
       BFCT_protocol_Zigbee.receive_enable =1;
       BFCT_protocol_Zigbee.receive_len =0;
       delay=0; 
       sys_timer =0;
       BFCT_protocol_Zigbee.receive_flag = 0;
       lock_moni_state++;
  case 3:
      if(BFCT_protocol_Zigbee.receive_flag == 1)
      {
        BFCT_protocol_Zigbee.receive_flag = 0;

        if(BFCT_protocol_Zigbee.receive_data[2] == 0xa1)
        {
          BFCT_protocol_Zigbee.receive_enable=0;
          BFCT_protocol_Zigbee.receive_len =0;
          delay = 0;
          sys_timer = 0;
          sys_time_counter =0;
          lock_moni_state ++;
        }
        else if(BFCT_protocol_Zigbee.receive_data[2] == 0xa2)
        {
          BFCT_protocol_Zigbee.receive_enable=1;
          BFCT_protocol_Zigbee.receive_len =0;
          for(i=0;i<BFCT_protocol_Zigbee.receive_data[7];i++)
            password[i]=BFCT_protocol_Zigbee.receive_data[8+i];
        }
        else
        {
         BFCT_protocol_Lock.receive_enable=0;
         BFCT_protocol_Zigbee.receive_enable =1;
         BFCT_protocol_Zigbee.receive_len =0;
        }
      }
     if(t_1ms)
     {
       delay++;
       if(delay > 9000){
         delay=0;
         sys_timer = 0;
         sys_time_counter ++;
       }
     }
     if(sys_time_counter == 10)
     {
        sys_time_counter = 0;
        delay = 0;
        lock_moni_state =0;
        return 1;
     }
     break;
     case 4:
        BFCT_protocol_Lock.send_len = lock_protocal_component(BFCT_protocol_Lock.send_data,0x01,LOCK_OPENLOCK,data_buff);
        BFCT_protocol_Lock.receive_enable=0;
        BFCT_protocol_Zigbee.receive_enable=0;
        delay=0;
        lock_moni_state++;
      break; 
   case 5:
       if( BFCT_protocol_Lock.send_len ==0) //�ȴ��������
       {
          BFCT_protocol_Lock.receive_enable=1;
          BFCT_protocol_Lock.receive_len =0;
          BFCT_protocol_Lock.receive_flag = 0;
          lock_moni_state++;
       }

      if(BFCT_protocol_Lock.send_fail==1)
      {
        BFCT_protocol_Lock.send_fail =0;
        delay = 0;
        lock_moni_state=0;
        return 1;
      }
    break;
   case 6:
     if(BFCT_protocol_Lock.receive_flag==1)
     {
        GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //TXD
        BFCT_protocol_Lock.receive_flag=0;
        ret = check_lock_cmd(ACK_LOCK_OPENLOCK);
        if(ret ==1)
        { 
          lock_moni_state = 10;
          delay = 0;
        }
        else
        {
          delay =0;
          lock_moni_state=0;
          return 1;
        }    
     }
     if(t_1ms)
     {
       delay++;
       if(delay >= (u16)1000){
          delay =0;
          lock_moni_state=0;
          return 1;
       }
     }
     break;
  case 10:
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        BFCT_protocol_Zigbee.send_len =  zigbee_protocal_component(BFCT_protocol_Zigbee.send_data,remote_unlock_sucess, data_buff);
   //     BFCT_protocol_Zigbee.receive_flag =0;
        lock_moni_state ++;
    break;
  case 11:
    if(BFCT_protocol_Zigbee.send_len ==0)
    {
        BFCT_protocol_Zigbee.receive_enable=0;
        BFCT_protocol_Zigbee.receive_len =0;
        USART1->CR2=0;//disable transmision and receiver
        USART_Cmd(USART1 , DISABLE);
        delay = 0;
        lock_moni_state++;
    }
    break;
  case 12:
     if(t_1ms)
     {
       delay++;
       if(delay > 9000){
         lock_moni_state = 0;
         delay=0;
         return 1;
       }
     }
    break;
     
  default:
    break;
  }
  return 0;
}

/********************zigbee�·����� **********************************/
/****************
*���������� ��������
************************/
u8 zigbee_openlock(void)
{
return 1;
}




/*****zigbee������ʧ�� *******/
u8 zigbee_opennet_fail()
{
return 1;
}

/*****zigbee ����û� *******/
u8 zigbee_adduser(void)
{
 
  return 1;
}
/*****zigbee ɾ���û� *******/
u8 zigbee_delete_user(void)
{
  return 1;                                      
}
/*****zigbee ��������û� *******/
u8 zigbee_modify_user(void)
{
  return 1;
}
/*****zigbee ������ɹ� *******/
u8 zigbee_opennet_sucess(void)
{
return 1;
}
/*****zigbee ʱ��ͬ�� *******/
u8 zigbee_clock_sync()
{
  return 1;
}
/*****zigbee ��ѯ�û���Ϣ *******/
u8 zigbee_inqure_userinfo(void)
{

  return 1;
}

/*****zigbee ��ѯ��״̬ *******/
u8 zigbee_inqure_lockstate(void)
{
return 1;
}

u8 zigbee_amdin_identification(void)
{
return 1;
}
