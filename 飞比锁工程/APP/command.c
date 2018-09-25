#include "stm8l15x.h"
#include "global.h"
#include "eeprom.h"
#include "usart.h"
#include "protocol.h"
#include "command.h"
#include "userdata.h"
#include "bsp.h"

u8 zigbee_moni_state=0;
u16 zigbee_delay=0;


u16 casual_work_No =0;


u8 data_buff[66]={0};

#if defined(RING_BELL)
static u8 ring_num=0;
#endif
 
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



/**********
*函数名称：lock_protocal_component
**函数描述 ：组帧，lock协议
**参数描述： buf:储存组好的帧，len:数据长度+命令子 cmd:协议命令 。data_buf :数据内容
**********/
u8  lock_protocal_component(u8*buf, u16 len,u8 cmd,u8* data_buff)
{
    u16 i;
    buf[0]=0x02;      //帧头
    buf[1]=len &0xFf;      //长度
    buf[2]=(len >> 8)&0xFf;     //长度
    buf[3]=cmd;     //命令字
    for(i=0;i<len-1;i++)
    {
       buf[4+i] = data_buff[i];
    }
    buf[len+3]=crc_8n(buf,len+3);  //检验
    buf[len+4]=0xff;     //帧尾
    return (5+len);
}




/**** 校验锁命令
*返回值 ： 0 表示错误。 1：表示正确
*******/
u8 check_lock_cmd(u8 lock_cmd)
{
  u8 data_len=0;

  data_len = BFCT_protocol_Zigbee.receive_data[1] + (BFCT_protocol_Zigbee.receive_data[2] << 8); //获取数据长度
  if(BFCT_protocol_Zigbee.receive_data[data_len +3] != crc_8n(BFCT_protocol_Zigbee.receive_data,data_len +3) )
  {  /**********数据校验错误 ************/
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




/*****发送lcok命令， 返回1 表示发送完成，发送 0表示 继续发送，发送1表示成功，发送2表示 失败*******/
u8 send_lcokcmd(u8 len,u8 zigbeecmd,u8 *data_buff )
{
  static u8 lock_moni_state =0;
  
  switch(lock_moni_state)
  {
   case 0:
      BFCT_protocol_Lock.send_len = lock_protocal_component(BFCT_protocol_Lock.send_data,len,zigbeecmd,data_buff);
      BFCT_protocol_Lock.receive_enable=0;
      BFCT_protocol_Zigbee.receive_enable=0;
      lock_moni_state++;
    break; 
  case 1:
    if(BFCT_protocol_Lock.send_fail==1)
    {
      BFCT_protocol_Lock.send_fail =0;
      lock_moni_state =0;
      return 2;
    }
    else if( BFCT_protocol_Lock.send_len ==0) //等待发送完成
     {
        BFCT_protocol_Lock.receive_enable=1;
        BFCT_protocol_Lock.receive_len =0;
        lock_moni_state =0 ;
        return 1;
     }
  break;
  default:
  break;
  }
  return 0;
}

 /*****
 *函数名称：get_lock_message
  *返回值：0: 未完成，继续。 1:成功，返回。 2： 失败，返回
 ********/ 
  
u8  get_lock_message(void)
{
  static u16 zigbee_delay =0;
  u8 ret =0;
  if(BFCT_protocol_Lock.receive_flag==1)
  {
    GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //TXD
    BFCT_protocol_Lock.receive_flag=0;
    zigbee_delay = 0;
    ret =1;
  }
 if(t_1ms)
 {
   zigbee_delay++;
   if(zigbee_delay >= (u16)2000){
      zigbee_delay =0;
      ret =2;
   }
 }
  return ret;
}
/********
* 打开网络流程
*返回值： 0 为完成，1：完成
************/
u8 lock_opennet()
{
  static u8 lock_moni_state =0,len=0,cmd;
  u8 ret;
  
  switch(lock_moni_state)
  {  
     case 0:
#if defined(Fei_Bi)
    for_cmd_id(cmd_id,REQUEST,data_buff);
    cmd_id++;
    data_buff[5+0] = 0;data_buff[5+1] = 0;data_buff[5+2] = 0;data_buff[5+3] = 0;data_buff[5+4] = 0;data_buff[5+5] = 0;
    data_buff[5+6] = 0;data_buff[5+7] = 0;data_buff[5+8] = 0;data_buff[5+9] = 0;data_buff[5+10] = 0;
    len = 0x0a;
    cmd = ZIGBEE_CMD_OPENNET;
#elif defined(Nan_Jing)
    len = 0x01;
    cmd = ZIGBEE_CMD_OPENNET;
#elif   defined(Jun_He)
    data_buff[0] = 0x88;data_buff[1] = 0x01;data_buff[2] = 0x9a;data_buff[3] = 0x48;data_buff[4] = 0x02;
    len = 0x06;
    cmd = ZIGBEE_CMD_OPENNET;
#elif defined(Hui_huang)
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        sys_timer =0;
        cmd = set_up_network_sucess;
#endif
    
    lock_moni_state++;
    break;
  case 1:
      ret = send_zigbeecmd(len,cmd,data_buff);
      if(ret == 1) {
        lock_moni_state ++;
      }
    break;
  case 2:
#if defined(GET_TIME)
    for_cmd_id(cmd_id,REQUEST,data_buff);
    cmd_id++;
    data_buff[5+0] = 0;data_buff[5+1] = 0;data_buff[5+2] = 0;data_buff[5+3] = 0;data_buff[5+4] = 0;data_buff[5+5] = 0;
    data_buff[5+6] = 0;data_buff[5+7] = 0;data_buff[5+8] = 0;data_buff[5+9] = 0;data_buff[5+10] = 0;
    len = 0x0a;
    cmd = 0x85;
    lock_moni_state++;
    break;
  case 3:
    ret = send_zigbeecmd(len,cmd,data_buff);
    if(ret == 1) {
      lock_moni_state =0;
      return 1;
    }
    break;
#else
#if defined(Hui_huang)
    Wfi_Mode =1;
    Zigbee_Send_timeout = 10;
    TIM3_Cmd(ENABLE); 
#endif    
    lock_moni_state =0;
    return 1;
    break;
#endif    

  default:
    lock_moni_state =0;
    break;
  }
  return 0;
}
/*****锁被关闭 ******/
u8 lock_be_closed(void)
{
  return 1;
}


/*****删除锁中的用户******/
u8 delete_user(void)
{
  static u8 lock_moni_state=0;
  static u16 delay =0;
  u8 ret;
  static u8 try_erro_num = 0;
  switch(lock_moni_state)
  {
  case 0:
      if(casual_work_No == (u16)0x999)  
      {
         data_buff[0] = 0X99;
         data_buff[1] = 0X09;
         data_buff[2] = 0Xff;
         delay =0;
         try_erro_num = 0;
         lock_moni_state ++;
      }
      else
        return 1;
    break;
  case 1:
    ret = send_lcokcmd(0x04,LOCK_DELETE_USER,data_buff );
    if(ret ==1)
    {
      delay =0;
      BFCT_protocol_Lock.receive_flag =0;
      lock_moni_state++;

    }
    else if(ret ==2)
    {
    }
    break;
  case 2:
    if(try_erro_num > 1)
    {
      try_erro_num = 0;
      lock_moni_state++;
    }
    else 
    {
       if(t_1ms)
       {
         delay++;
         if(delay > 500){
            lock_moni_state --;
            delay =0;
            try_erro_num ++;
         }
       }
    }
    break;
 
  case 3:
        ret = get_lock_message();
       if(ret == 1)
       {
          GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //TXD
          BFCT_protocol_Lock.receive_flag=0;
          ret = check_lock_cmd(0x82);
          if(ret ==1)
          { 
            if(BFCT_protocol_Lock.receive_data[4] == 0x00){
              casual_work_No = 0x0000;
         //     write_userdata2eeprom( casual_work_addr,(u8 *)&casual_work_No,2);
            }
            lock_moni_state =0;
            return 1;
          }
       }
       else if(ret ==2)
       {
            lock_moni_state =0;
            return 1;
       }
    break;
  default:
    break;
    
  }
  return 0;

}
/********
* 关闭网络流程
*返回值： 0 为完成，1：完成
************/
u8 lock_closenet(void) 
{
  u8 ret=0;
  ret = send_zigbeecmd(0x01,ZIGBEE_CMD_CLOSENET,data_buff);
  return ret;
  
}

/*****锁具格式化操作*********/
u8 lock_format(void)
{
  u8 ret;
  ret = send_zigbeecmd(0x01,ACK_ZIGBEE_FORMAT,data_buff);
  return ret;
}

/*****进入安全模式流程 *******/
u8 lock_login_safemode()
{
#if defined(RING_BELL)
      ring_num =0;
#endif
  return 1;
}
//*******退出安全模式 ************
u8 lock_logout_safe_mode()
{
  return 1;
}

#if defined(RING_BELL)   
/****门铃功能******/
u8  lock_alarm(void)
{
  static u8 lock_moni_state = 0,len =0,cmd=0;
  u8 ret=0 ;
  switch(lock_moni_state)
  {
    case 0:
#if defined(RING_BELL)
    ring_num++;
    if(ring_num < 1)
      return 1;
    else
      ring_num =0;
#endif

#if defined(Fei_Bi)
      for_cmd_id(cmd_id,REQUEST,data_buff);
      cmd_id++;
      data_buff[5+0] = 0;data_buff[5+1] = 0;data_buff[5+2] = 0;data_buff[5+3] = 0;data_buff[5+4] = 0;data_buff[5+5] = 0;
      data_buff[5+6] = 0;data_buff[5+7] = 0;data_buff[5+8] = 0;data_buff[5+9] = 0;data_buff[5+10] = 0;
      cmd = ZIGBEE_CMD_WAKEUP_MODULE;
      len = 0x0a;
#elif defined(Hui_huang)
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 
        cmd = check_module_if_online;
#endif
      lock_moni_state++;
      break;
    case 1:
        ret = send_zigbeecmd(len,cmd,data_buff);
        if(ret == 1) {
          lock_moni_state =0;
        }
#if defined(Hui_huang)
        else if(ret == 2)
        {
          Wfi_Mode=1;
          Sleep_time = 40;
          TIM3_Cmd(ENABLE);   
          lock_moni_state = 0;
          BFCT_protocol_Zigbee.receive_enable = 1;
          BFCT_protocol_Zigbee.receive_len =0;    
        }
#endif
      break;
    default:
      break;
  }
  return ret;
}
#endif


/**********退出设置模式*********/
u8 lock_logout_settting_mode(void)
{
    static u8 lock_moni_state=0;
    static u16 delay=0;
    u8 ret;

    switch(lock_moni_state)
    {
      case 0:
          read_userdata4eeprom(zigbee_clock_addr, data_buff, 6);
          if(data_buff[0] |data_buff[1] | data_buff[2])
          {
            lock_moni_state++;
            delay =0;
          }
          else
            return 1;
      break;
  case 1:
       if(t_1ms)
       {
         delay++;
         if(delay >= (u16)300){
            delay =0;
            lock_moni_state++;
         }
       }
    break;

      case 2:
          ret = send_lcokcmd(0x08,LOCK_CLOCK_CALIBRATION,data_buff );
          if(ret ==1)
          {
            GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_Low_Fast); //TXD
            data_buff[0]=0; data_buff[1]=0; data_buff[2]=0; data_buff[3]=0; data_buff[4]=0; data_buff[5]=0; data_buff[6]=0; 
            write_userdata2eeprom(zigbee_clock_addr, data_buff, 6);
            delay =0;
            lock_moni_state = 0;
            return 1;
          }
          else if(ret ==2)
          {
            goto clear;
          }
      break;
    default:
    break; 

    }
    return 0;
clear:
      delay =0;
      lock_moni_state =0;
      return 1;
}

/********强制开锁报警*********/
u8 lock_openclock_byforce()
{
  return 1;
}
/******上传锁状态********/
u8 lock_state_updata()
{
  return 1;
}

/******从锁添加用户流程*******************/
u8 lock_add_user()
{
  static u8 lock_moni_state=0;
  u8 ret,ret1;
  
  switch(lock_moni_state)
  {
  case 0:
    
      lockdata_2_zigbeedata (lock_user_attribute,&(BFCT_protocol_Lock.receive_data[14]),&data_buff[0]); //用户属性 信息
      lockdata_2_zigbeedata (lock_user_No,&(BFCT_protocol_Lock.receive_data[4]),&data_buff[1]); //用户编号信息
      lockdata_2_zigbeedata(lock_password,&(BFCT_protocol_Lock.receive_data[37]),&data_buff[5]);   //密码长度  
      lock_moni_state++;

    break;
  case 1:
      ret = send_zigbeecmd(0x03,ZIGBEE_CMD_ADDUSER_FROM_LOCK,data_buff);
      if(ret)
      {
         if(zigbee_erro){
            lock_moni_state =0;
            return 1;
         }
         else{
           lock_moni_state ++; 
         }
      }
    break;
  case 2:
       data_buff[2] = 0x01;  // 开锁方式
       data_buff[3] = 0x06;  //  密码编号 
       lock_moni_state++;
    break;
  case 3:
      ret1 = send_zigbeecmd(0X0b,ACK_ZIGBEE_MODIFY_USERINFO,data_buff);
      if(ret1)
      {
        lock_moni_state=0; 
        return 1;
      }
    break;  
  default:
    lock_moni_state=0; 
    break;
  }
  return 0;
}

u8 lock_be_opened(void)
{
  static u8 lock_moni_state=0,len =0,cmd =0;
  static u16 delay=0;
  u8 ret;
  static u8 battery1_level =0, battery2_level =0,user_No=0;   
  
  
  switch(lock_moni_state)
  {
    case 0:
#if defined(RING_BELL)
      ring_num =0;
#endif
        battery1_level = BFCT_protocol_Lock.receive_data[4];  
        battery2_level = BFCT_protocol_Lock.receive_data[5];    
         
        user_No      = BFCT_protocol_Lock.receive_data[8]; 
       
#if defined(Fei_Bi)
        for_cmd_id(cmd_id,REQUEST,data_buff);
        cmd_id++;
        if(remote_open_flag == 1){
            data_buff[5+0] = 0 ; data_buff[5+1] = BFCT_protocol_Lock.receive_data[8];  //用户编号
            data_buff[5+2] = BFCT_protocol_Lock.receive_data[9]& 0x0f;data_buff[5+3] = 0;data_buff[5+4] = 0x05;data_buff[5+5] = 0;
            data_buff[5+6] = 0;data_buff[5+7] = 0;data_buff[5+8] = 0;data_buff[5+9] = 0;data_buff[5+10] = 0;
            len = 0x0a;
            remote_open_flag =0;
            cmd = ACK_ZIGBEE_CMD_OPENLOCK;
        }
        else{
            data_buff[5+0] = BFCT_protocol_Lock.receive_data[8] ; data_buff[5+1] = BFCT_protocol_Lock.receive_data[9]& 0x0f;  //用户编号
            data_buff[5+3] = 0x04;
            lockdata_2_zigbeedata(lock_keytype,&BFCT_protocol_Lock.receive_data[9],&data_buff[5+2]);
            if(battery1_level + battery2_level < 140)
              data_buff[5+3] = 0x02;
            else if(battery1_level + battery2_level < 100)
              data_buff[5+3] = 0x03;
            else if(battery1_level + battery2_level < 50)
              data_buff[5+3] = 0x04;
            
            data_buff[5+4] = 0x05;
            data_buff[5+5] = BFCT_protocol_Lock.receive_data[19];
            data_buff[5+6] = BFCT_protocol_Lock.receive_data[20];
            data_buff[5+7] = BFCT_protocol_Lock.receive_data[21];
            data_buff[5+8] = BFCT_protocol_Lock.receive_data[22];
            data_buff[5+9] = BFCT_protocol_Lock.receive_data[19];
            data_buff[5+10] = BFCT_protocol_Lock.receive_data[19];
            len = 0x0a;
            cmd = ACK_ZIGBEE_CMD_LOCAL_OPEN_LOCK;
        }
#elif defined(Nan_Jing)
   
        data_buff[0] =0x01;//   //用户属性
        data_buff[1] = u8_BCD_2_hex(user_No);  //用户编号
        data_buff[2] =0x01;  // 开锁方式
        data_buff[3] = 0x01;  //  密码编号 
        len = 0x05; cmd = ACK_ZIGBEE_CMD_UPLOAD;
#elif defined(Hui_huang)
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00;      
        lockdata_2_zigbeedata(lock_keytype,&BFCT_protocol_Lock.receive_data[9],&cmd);
         
   
#endif
        BFCT_protocol_Zigbee.receive_len =0;
        BFCT_protocol_Zigbee.receive_enable=1;
        BFCT_protocol_Lock.receive_enable=0;
        BFCT_protocol_Zigbee.receive_flag =0;
        delay=0;
        lock_moni_state++;
    break;
    case 1:
      ret = send_zigbeecmd(len,cmd,data_buff);
      if(ret)
      {
         if(zigbee_erro){
            delay =0;
            lock_moni_state =0;
            return 1;
         }
         lock_moni_state++; 
      }
    break;
  case 2:
      if( (battery1_level +battery2_level)< 50 )
      {
#if defined(Fei_Bi)
        for_cmd_id(cmd_id,REQUEST,data_buff);
        data_buff[5+0] = 0;data_buff[5+1] = u8_BCD_2_hex(user_No);  //用户编号
        data_buff[5+2] = 0;data_buff[5+3] = 0;data_buff[5+4] = 0;data_buff[5+5] = 5;
        data_buff[5+6] = 0;data_buff[5+7] = 0;data_buff[5+8] = 0;data_buff[5+9] = 0;data_buff[5+10] = 0;
        len = 0x0a;
        cmd = ACK_ZIGBEE_CMD_LOW_POWER;
#elif defined(Nan_Jing)
        len =0x01;  cmd = ACK_ZIGBEE_CMD_LOW_POWER;
#elif defined(Hui_huang)
        cmd = 0x19;
#endif
        
        delay = 0;
        lock_moni_state ++;
      }
      else
      {
#if defined(Hui_huang)
         Wfi_Mode =1;
         Zigbee_Send_timeout = 10;
         TIM3_Cmd(ENABLE);  
#endif         
        delay =0;
        lock_moni_state =0;
        return 1;
      }
    break;
    case 3:
         if(t_1ms)
         {
           delay++;
           if(delay == 800)
           {
              BFCT_protocol_Zigbee.receive_len =0;
              BFCT_protocol_Zigbee.receive_enable=1;
              BFCT_protocol_Lock.receive_enable=0;
              BFCT_protocol_Zigbee.receive_flag =0;
              delay=0;
              lock_moni_state++;
           }
         }
    break;
    case 4:
      ret = send_zigbeecmd(len,cmd,data_buff);
      if(ret)
      {
         delay = 0;
         lock_moni_state=0; 
#if defined(Hui_huang)
         Wfi_Mode =1;
         Zigbee_Send_timeout = 10;
         TIM3_Cmd(ENABLE);  
#endif
         return 1;

      }
    break;  
    default:
    break;
  }
  return 0;
}
/********非法用户锁上报命令**********/
u8 lolck_illage_user_report(void)
{
  
  static u8 lock_moni_state =0,len=0,cmd;
  u8 ret;
  
  switch(lock_moni_state)
  {
  case 0:
#if defined(Fei_Bi)
    for_cmd_id(cmd_id,REQUEST,data_buff);
    cmd_id++;
    data_buff[5+0] = 0;data_buff[5+1] = 0;data_buff[5+2] = 0;data_buff[5+3] = 0;data_buff[5+4] = 0;data_buff[5+5] = 0;
    data_buff[5+6] = 0;data_buff[5+7] = 0;data_buff[5+8] = 0;data_buff[5+9] = 0;data_buff[5+10] = 0;
    len = 0x0a;
#elif defined(Nan_Jing)
    len = 0x01;
#endif
    cmd = ACK_ZIGBEE_ILLAGEL_USER_REPORT;
    lock_moni_state++;
    break;
  case 1:
      ret = send_zigbeecmd(len,cmd,data_buff);
      if(ret == 1) {
        lock_moni_state =0;
      }
    break;
  default:
    break;
  }
  return ret;
}

/*****删除用户**********/
u8 lock_delete_user(void)
{
  static u8 lock_moni_state=0;
  u8 ret;  
  
  switch(lock_moni_state)
  {
    case 0:   
        lockdata_2_zigbeedata (lock_user_attribute,&(BFCT_protocol_Lock.receive_data[14]),&data_buff[0]); //用户属性 信息
        lockdata_2_zigbeedata( lock_user_No,&BFCT_protocol_Lock.receive_data[4], &data_buff[1]);
        lock_moni_state++;
      break;
  case 1:
      ret = send_zigbeecmd(0x03,ACK_ZIGBEE_CMD_DELETE_USER,data_buff);
      if(ret)
      {
        lock_moni_state = 0;
          return 1; 
      }
      break;
  default:
    break;
  }
  return 0;
}



/********************zigbee下发命令 **********************************/
/****************
*函数描述： 开锁流程
************************/
u8 zigbee_openlock(void)
{
   static u8 len=0 ,cmd =0; 
  u8 ret1, ret,i;
 static u8 zigbee_moni_state1 =0;
  switch(zigbee_moni_state1)
    {
      case 0:
#if defined(Fei_Bi)
        ACK_zigbee_openlock_composite_data(cmd,len)
#elif defined(Nan_Jing)
        ACK_zigbee_openlock_composite_data(cmd,len);
#endif
#if  defined(Hui_huang)
        data_buff[0] = 0x12;
        data_buff[1] = 0x34; 
        data_buff[2] = 0x56;
        data_buff[3] = 0x78; 
        data_buff[4] = 0x00; 
        data_buff[5] = 0x00; 

        data_buff[0] = 0x80;
        zigbeedata_2_lockdata(zigbee_password,&data_buff[1],&BFCT_protocol_Zigbee.receive_data[7]);
        
        zigbee_moni_state1 = 2;
#else        
        zigbee_moni_state1++;
#endif    
      break;
      case 1:
        
         ret = send_zigbeecmd(len,cmd,data_buff);
         if(ret)
         {
           if(1)
           {
#if defined(Nan_Jing)
              data_buff[0] = 0x80;
              zigbeedata_2_lockdata(zigbee_password,&data_buff[1],&BFCT_protocol_Zigbee.receive_data[6]);
              for(i=4;i<8;i++)
                data_buff[i] = 0xff;
              zigbee_moni_state1++;     
#elif defined(Fei_Bi)
              for(i=0;i<6;i++){
                password[i] = BFCT_protocol_Zigbee.receive_data[8+i];
              }
              encryption();
              data_buff[0] = 0x80;
              zigbeedata_2_lockdata(zigbee_password,&data_buff[1],npass);
              for(i=4;i<8;i++)
                data_buff[i] = 0xff;
              zigbee_moni_state1 ++;
#endif

           }
           else 
           {
              BFCT_protocol_Zigbee.receive_flag = 0;
              BFCT_protocol_Lock.receive_enable=0;
              BFCT_protocol_Zigbee.receive_enable=0;
              len =0x01;
              cmd = 0x02; 
              zigbee_moni_state1 =10;
           }
         }
        break;
    case 2:
         ret = send_lcokcmd(0x09,41,data_buff );
         if(ret == 1)
         {
            BFCT_protocol_Lock.receive_flag=0;
            zigbee_moni_state1++;
         }
         else if(ret == 2)
         {  
            BFCT_protocol_Zigbee.receive_enable=1;
            BFCT_protocol_Lock.receive_enable=0;
#if defined(Nan_Jing)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);

#endif   
            zigbee_moni_state1 = 10;    
          } 

      break;
    case 3:
       ret1 = get_lock_message();
       if(ret1 ==1)
       {
          ret = check_lock_cmd(0xA9);
          if(ret ==1)
          { 
#if  defined(Fei_Bi)
            remote_open_flag =1; 
#endif
            if(BFCT_protocol_Lock.receive_data[LOCK_ACK_RES_ADDR] == 0)
            {
              zigbee_moni_state1++;
            }
            else
            {
#if defined(Nan_Jing)
              ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
              ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Hui_huang)
               wake_up_in(SET);
#endif            
             
              BFCT_protocol_Zigbee.receive_enable=1;
              BFCT_protocol_Lock.receive_enable=0;
              zigbee_moni_state1=10;
            }
          }
          else
          {
#if defined(Nan_Jing)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Hui_huang)
               wake_up_in(SET);
#endif            
            BFCT_protocol_Zigbee.receive_enable=1;
            BFCT_protocol_Lock.receive_enable=0;
            zigbee_moni_state1=10;
          }
       }
       else if(ret1 ==2)
       {
#if defined(Nan_Jing)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Hui_huang)
               wake_up_in(SET);
#endif  
          zigbee_moni_state1=10;
       }
      break;
  
       case 4:
         ret = send_lcokcmd(0x01,LOCK_OPENLOCK,data_buff );
         if(ret == 1)
         {
            BFCT_protocol_Lock.receive_flag=0;
            zigbee_moni_state1++;
         }
         else if(ret == 2)
         {  
            BFCT_protocol_Zigbee.receive_enable=1;
            BFCT_protocol_Lock.receive_enable=0;
#if defined(Nan_Jing)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Hui_huang)
               wake_up_in(SET);

#endif   
            zigbee_moni_state1 = 10;    
          } 
      break;
     case 5:
       ret1 = get_lock_message();
       if(ret1 ==1)
       {
          ret = check_lock_cmd(ACK_LOCK_OPENLOCK);
          if(ret ==1)
          { 
#if  defined(Hui_huang)
               Sleep_time =0;
               wake_up_in(SET);
#endif   
            zigbee_moni_state1 = 0;
            return 1;
          }
          else
          {
#if defined(Nan_Jing)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Hui_huang)
               wake_up_in(SET);
#endif            
            BFCT_protocol_Zigbee.receive_enable=1;
            BFCT_protocol_Lock.receive_enable=0;
            zigbee_moni_state1=10;
          }
       }
       else if(ret1 ==2)
       {
#if defined(Nan_Jing)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Fei_Bi)
            ACK_zigbee_openlock_fail_composite_data(cmd,len);
#elif  defined(Hui_huang)
               wake_up_in(SET);
#endif  
          zigbee_moni_state1=10;
       }
       break;
         
       case 10:
#if defined(Hui_huang)
          zigbee_moni_state1 =0;
          return 1;
#endif
          ret = send_zigbeecmd(len,cmd,data_buff);
          if(ret == 1) {
            zigbee_moni_state1 =0;
            cmd =0;
            len =0;
            return 1;
          }
         break;  
         
       default:
         zigbee_moni_state1 =0;
         return 1;
       break;
    }
  return 0;
}

/*****zigbee打开网络失败 *******/
u8 zigbee_opennet_fail_2(void)
{
  u8 ret;
  
  ret = send_zigbeecmd(0X01,ZIGBEE_CMD_ACK,data_buff);
  return ret;
}
/*****zigbee在线提醒 *******/
#if defined(Fei_Bi)
u8 zigbee_online_info(void)
{
  static u8 len,cmd;
  u8 ret =0;
  switch(zigbee_moni_state)
  {
  case 0:
#if defined(Fei_Bi) 
    cmd = ZIGBEE_CMD_ONLINE_INFO;
    cmd_id = *((u32*)&BFCT_protocol_Zigbee.receive_data[3]);
    for_cmd_id(cmd_id,ACK,data_buff);
    len =1;
#endif
    zigbee_moni_state ++;
    break;
  case 1:
     ret = send_zigbeecmd(len,cmd,data_buff);
     if(ret ==1) zigbee_moni_state =0;
    break;
  default:
    break; 
  }

  return ret;
}
#endif
/*****zigbee打开网络失败 *******/
u8 zigbee_opennet_fail()
{
  u8 ret;
  
  ret = send_zigbeecmd(0X01,ZIGBEE_CMD_ACK,data_buff);
  return ret;
}

/***修改用户，增加用户通用这个函数 *******/
u8 add_user_process(void)
{
  u8 ret,i,j;
  static u8 user_No,user_attribute,function;
  static u8 User_No_exist =1;
  static u8 try_erro = 0;
  static u8 zigbee_moni_state = 0;
  switch(zigbee_moni_state)
  {
  case 0:
    for(i=15;i<33;i++)
      data_buff[i] = 0xff;
    data_buff[18] = 0x00;
    data_buff[19] = 0x00;
        data_buff[31] = 0x01;
        data_buff[32] = 0x00;
      User_No_exist =1;
#if defined(Nan_Jing)
      function = BFCT_protocol_Zigbee.receive_data[ZIGBEE_CMD_ADDR];
      data_buff[0] = 0x3f;
      user_attribute  = BFCT_protocol_Zigbee.receive_data[4];
      user_No = BFCT_protocol_Zigbee.receive_data[5];
      zigbeedata_2_lockdata(zigbee_user_No,&data_buff[1],&user_No);
      zigbeedata_2_lockdata(zigbee_user_attribute,&data_buff[11],&BFCT_protocol_Zigbee.receive_data[4]);
      data_buff[11] |= 0x7f;
      if(user_attribute == 0x03)
      {
        data_buff[1] = 0x99;
        data_buff[2] = 0x09;
        
 //       casual_work_No = 0x999;
  //      write_userdata2eeprom ( casual_work_addr,(u8 *)&casual_work_No,2);
      }
      
      data_buff[33]=0x80;
      if(function ==0x13)
      zigbeedata_2_lockdata(zigbee_password,&data_buff[34],&BFCT_protocol_Zigbee.receive_data[7]);
      else
      zigbeedata_2_lockdata(zigbee_password,&data_buff[34],&BFCT_protocol_Zigbee.receive_data[8]);
      for(i=37;i<41;i++)
         data_buff[i]=0xff;
      
      if( (function == 0x13) && (user_attribute != 0x03) )
        User_No_exist = 0;
#elif defined(Hui_huang)
    data_buff[0] = 0x3f;
    data_buff[11] = 0xff;
    data_buff[33]=0x80;
    zigbeedata_2_lockdata(zigbee_password,&data_buff[34],&BFCT_protocol_Zigbee.receive_data[8]);
    for(i=37;i<41;i++)
        data_buff[i]=0xff;
    
    casual_work_No = 0x999;
    data_buff[1] = 0x99;
    data_buff[2] = 0x09;
    data_buff[31] = 0x01;
    data_buff[32] = 0x00;
    

#endif      
      zigbee_moni_state++;
    break;
    
  case 1:
#if  defined(NO_reply)  
      ret =1;
#else 
       ret = send_zigbeecmd(0X01,ZIGBEE_CMD_ACK,data_buff);
#endif
       if(ret)
       {
          zigbee_delay = 0;
          if(User_No_exist ==1) 
             zigbee_moni_state = 4;
          else
            zigbee_moni_state = 2;
      }
    break;
  case 2:
       ret = send_lcokcmd(0x01,39,data_buff );
       if(ret == 1)
       {
          BFCT_protocol_Lock.receive_flag=0;
          zigbee_delay =0;
          user_No =0;
          zigbee_moni_state++;
       }
       else if(ret == 2)
       {
          goto clear;
       }
      break;
      

  case 3:
      ret = get_lock_message();
      if(ret ==1)
      {
        for(i =0;i<100;i++)
        {
          for(j=0;j<8;j++)
          {
            if (BFCT_protocol_Zigbee.receive_data[5+i] & (1<<(7-j)) )
            {
              user_No ++;
            }
            else
              break;
            
          }
          if (j < 8)
          break;
        } 
        
        zigbeedata_2_lockdata(zigbee_user_No,&data_buff[1],&user_No);
        zigbee_delay =0;
        zigbee_moni_state++; 
      }
      else if(ret ==2)
      {
          goto clear;
      }
      break;
   case 4:
       ret = send_lcokcmd(0x2a,0x01,data_buff );
       if(ret == 1)
       {
          BFCT_protocol_Lock.receive_flag=0;
          zigbee_delay =0;
          try_erro = 0;
          zigbee_moni_state++;
       }
       else if(ret == 2)
       {
          try_erro ++;
          if(try_erro >= 3)
          {
              try_erro = 0;
              BFCT_protocol_Lock.receive_flag=0;
              zigbee_delay =0;
              zigbee_moni_state++;
          }
       }
  break;
  case 5:
    
      ret = get_lock_message();
      if(ret ==1)
      {
        BFCT_protocol_Lock.receive_flag=0;
        ret = 1; //check_lock_cmd(0x81);
        if(ret ==1)
        { 
#if defined(Nan_Jing)
          if( function == 0x13){
            if(user_attribute == 0x03){
              data_buff[0] = 0x01;data_buff[1] = 0x03;data_buff[2] = 0x33;data_buff[3] = 0x01;data_buff[4] = 0x06;}
            else {
              data_buff[0] = 0x01;data_buff[1] = user_attribute;data_buff[2] = user_No;data_buff[3] = 0x01;data_buff[4] = 0x06;} 
          }
          else
          {
            data_buff[0] = 0x03;data_buff[1] = user_attribute;data_buff[2] = user_No;data_buff[3] = 0x01;data_buff[4] = 0x06;
          }
#endif          
            zigbee_delay =0;
            zigbee_moni_state =10;
        }
        else
        {
          goto clear;
        }  
      }
      else if(ret ==2)
      {
          goto clear;
      }
   break;      
  case 10:      //动态密码
#if  defined(NO_reply)   
       ret =1;
#else
       ret = send_zigbeecmd(0X0c,0x50,data_buff);
#endif
       if(ret)
       {
#if defined(Hui_huang)
          wake_up_in(SET);
          Sleep_time = 60;
          Wfi_Mode =1;
          TIM3_Cmd(ENABLE);               //启动定时器3开始计数
#endif
         zigbee_moni_state = 0;    
         return 1;
       }
    break;
  default:
    zigbee_moni_state = 0;
    break; 
  }
  return 0;
clear :
    zigbee_delay = 0;
    zigbee_moni_state =0;
    return 1;
  
}

/*****zigbee 添加用户 *******/
u8 zigbee_adduser(void)
{
  u8 ret;
#if  defined( Nan_Jing) | defined(Hui_huang)
  ret = add_user_process();
  return ret;
#else
  return 1;
#endif
}
/*****zigbee 删除用户 *******/
u8 zigbee_delete_user(void)
{
  u8 ret;
  static  u8  zigbee_moni_state =0;
  static u8 user_attribute =0,user_No =0;
  static u8 len,cmd;
  switch(zigbee_moni_state)
    {
      case 0:
           ret = send_zigbeecmd(0x01,ZIGBEE_CMD_ACK,data_buff);
           if(ret)
           {
             if(zigbee_erro){
                zigbee_moni_state =0;
                return 1;
             }
             user_attribute = BFCT_protocol_Zigbee.receive_data[4];
             user_No = BFCT_protocol_Zigbee.receive_data[5];
             zigbeedata_2_lockdata(zigbee_user_No,&data_buff[0],&(BFCT_protocol_Zigbee.receive_data[5]));
             data_buff[2] = 0XFF;
             zigbee_moni_state++;         
           }
        break;
      
       case 1:
       ret = send_lcokcmd(0x04,LOCK_DELETE_USER,data_buff );
       if(ret == 1)
       {
          BFCT_protocol_Lock.receive_flag=0;
          zigbee_moni_state++;
       }
       else if(ret == 2)
       {
         len =0x01;
         cmd = 0x0a;
          BFCT_protocol_Zigbee.receive_enable=1;
          BFCT_protocol_Lock.receive_enable=0;
          zigbee_moni_state=10;
       }
        break; 
     case 2:
      ret = get_lock_message();
      if(ret ==1)
      {
          ret = check_lock_cmd(0x82);
          if(ret ==1)
          { 
            if(BFCT_protocol_Lock.receive_data[4] == 0x00){
              data_buff[0] = 0x02;data_buff[1] = user_attribute; data_buff[2] = user_No;
              len = 0x04;
              cmd = 0x50; 
            }
            else
              
              len = 0x01;
              cmd = 0x0a;  
              BFCT_protocol_Zigbee.receive_flag =0;
              BFCT_protocol_Zigbee.receive_enable=1;
              BFCT_protocol_Lock.receive_enable=0;
              zigbee_moni_state = 10;
          }
          else
          {
             len = 0x01;
            cmd = 0x0a;
            BFCT_protocol_Zigbee.receive_enable=1;
            BFCT_protocol_Lock.receive_enable=0;
            zigbee_moni_state=10;
          }    
       }
      else if(ret ==2)
      {
          len = 0x01;
          cmd = 0x0a;
          BFCT_protocol_Zigbee.receive_enable=1;
          BFCT_protocol_Lock.receive_enable=0;
          zigbee_moni_state=10;
      }
       break;
         
       case 10:
          ret = send_zigbeecmd(len,cmd,data_buff);
          if(ret ==1)
          {
              zigbee_moni_state =0;
              return 1;
          }
         break;  
         
       default:
         zigbee_moni_state =0;
       break;
    }
  return 0;                                      
}
/*****zigbee 本地添加用户 *******/
u8 zigbee_modify_user(void)
{
  u8 ret;
  ret = add_user_process();
  return ret;
}
/*****zigbee 打开网络成功 *******/
u8 zigbee_opennet_sucess()
{
  static u8 len,cmd;
  u8 ret;
  switch(zigbee_moni_state)
  {
  case 0:
#if defined(Fei_Bi) 
    cmd = ZIGBEE_CMD_OPENNET_SUCEESS;
    cmd_id = *((u32*)&BFCT_protocol_Zigbee.receive_data[3]);
    for_cmd_id(cmd_id,ACK,data_buff);
    len =1;
#elif defined(Nan_Jing)
    cmd = ZIGBEE_CMD_ACK;
    len =1;
#endif
    zigbee_moni_state ++;
    break;
  case 1:
     ret = send_zigbeecmd(len,cmd,data_buff);
     if(ret ==1) zigbee_moni_state =0;
    break;
  default:
    break; 
  }

  return ret;
}
/*****zigbee 时间同步 *******/
u8 zigbee_clock_sync()
{
  static u8 len=0,cmd =0;
  u8 ret,i;

  switch(zigbee_moni_state)
  {
  case 0:
#if defined(Fei_Bi)   
    data_buff[0]=(u8_HEX_2_byte_BCD(BFCT_protocol_Zigbee.receive_data[14]));
    data_buff[1]=(u8_HEX_2_byte_BCD(BFCT_protocol_Zigbee.receive_data[13]));
    data_buff[2]=(u8_HEX_2_byte_BCD(BFCT_protocol_Zigbee.receive_data[12]));
    data_buff[3]=(u8_HEX_2_byte_BCD(BFCT_protocol_Zigbee.receive_data[11]));
    data_buff[4]=(u8_HEX_2_byte_BCD(BFCT_protocol_Zigbee.receive_data[10]));
    data_buff[5]=u8_HEX_2_byte_BCD (((BFCT_protocol_Zigbee.receive_data[9]<<8)+BFCT_protocol_Zigbee.receive_data[8])%2000);
    write_userdata2eeprom(zigbee_clock_addr,data_buff,6);
    zigbee_moni_state ++;
        cmd=0x62;
        cmd_id=*((u32*)&BFCT_protocol_Zigbee.receive_data[3]);
        for_cmd_id(cmd_id,ACK,data_buff);
        cmd_id++;
        len =0x01;
#elif defined(Nan_Jing)  
    for(i=0;i<6;i++)
    {
       data_buff[i]= u8_HEX_2_byte_BCD(BFCT_protocol_Zigbee.receive_data[9-i]); //秒
    }
    write_userdata2eeprom( zigbee_clock_addr,data_buff,6); //向flash 写时钟
    cmd = ZIGBEE_CMD_ACK;
    len = 0X01;
    zigbee_moni_state ++;
#endif
    break;
    
  case 1:
       ret = send_zigbeecmd(len,cmd,data_buff);
       if(ret)
       {
         zigbee_moni_state = 0;    
         return 1;
       }
    break;
  default:
    break; 
  }
  return 0;
}
/*****zigbee 查询用户信息 *******/
u8 zigbee_inqure_userinfo(void)
{
  u8 ret,i;
  static u8 admin_No=0,ordinary_No=0,admin_No_exist =1,ordinary_No_exist;
  static u8 get_mes_num = 0,user_attri = 0;
  static u8 j=0;
  static u8 len,cmd;
  static u8 *buf;

  switch(zigbee_moni_state)
  {
  case 0:
       ret = send_zigbeecmd(0X01,ZIGBEE_CMD_ACK,data_buff);
       if(ret)
       {
         data_buff[0]=0;  data_buff[1]=0;  data_buff[2]=3;   
         user_attri = BFCT_protocol_Zigbee.receive_data[4]; //
         zigbee_moni_state++;
       }
    break;
    
   case 1:
      zigbee_delay=0;
      BFCT_protocol_Lock.receive_enable=0;
      BFCT_protocol_Zigbee.receive_enable=0;
      USART_Cmd(USART1 , DISABLE);
      zigbee_moni_state++;
    break; 
  case 2:
      ret = send_lcokcmd(0x04,LOCK_INQUIRY_USER_INFO,data_buff );
      if(ret == 1)
      {
        data3_tc = 101;
        zigbee_delay =0;
        zigbee_moni_state++;
      }
      else if(ret == 2)
      {
        BFCT_protocol_Zigbee.receive_enable=1;
        BFCT_protocol_Lock.receive_enable=0;
         data_buff[0] = 0xee;
        len = 0x02;
        cmd = 0x3a;
        get_mes_num = 0;
        zigbee_delay =0;
        zigbee_moni_state=0x05;   
      } 

  break;
  case 3:
      ret = get_lock_message();
      if(ret ==1)
      {
        ordinary_No =0;
        admin_No =0;
        ret = check_lock_cmd(ACK_LOCK_INQUIRY_USER_INFO);
        if(ret ==1)
        { 
          get_mes_num = BFCT_protocol_Lock.receive_data[4];
          for(i=0; i< BFCT_protocol_Lock.receive_data[4];i++)
          {
            if( BFCT_protocol_Lock.receive_data[i*33+15] & 0x80)
            {
              ordinary_No++;
              data_buff[51-ordinary_No*10] = 0x06;
              data_buff[51+1-ordinary_No*10]= u16_BCD_2_hex( *(u16*)&BFCT_protocol_Lock.receive_data[i*33+5]);
              data_buff[51+2-ordinary_No*10]=1;data_buff[51+3-ordinary_No*10]=1;
              data_buff[51+4-ordinary_No*10]=i;data_buff[51+5-ordinary_No*10]=1;
               data_buff[51+6-ordinary_No*10]=1;data_buff[51+7-ordinary_No*10]=i;
                data_buff[51+8-ordinary_No*10]=1;data_buff[51+9-ordinary_No*10]=1;
            }
            else
            {
              data_buff[1+admin_No*10] = 0x06;
              data_buff[2+admin_No*10]=u16_BCD_2_hex( *(u16*)&BFCT_protocol_Lock.receive_data[i*33+5]);
              data_buff[3+admin_No*10]=1;data_buff[4+admin_No*10]=1;
              data_buff[5+admin_No*10]=i;data_buff[6+admin_No*10]=0;
              data_buff[7+admin_No*10]=0;data_buff[8+admin_No*10]=0;
              data_buff[9+admin_No*10]=admin_No;data_buff[10+admin_No*10]=ordinary_No;
              admin_No++;
              
            }
          }
            
          BFCT_protocol_Zigbee.receive_enable=1;
          BFCT_protocol_Lock.receive_enable=0;
          zigbee_delay =0;
          zigbee_moni_state++;
        }
        else
        {        
          data_buff[0] = 0xee;
          len = 0x02;
          cmd = 0x38;
          get_mes_num = 0;
          BFCT_protocol_Zigbee.receive_enable=1;
          BFCT_protocol_Lock.receive_enable=0;
          zigbee_delay =0;
          zigbee_moni_state = 0x05;
        }    
     }
     else if(ret ==2)
     {
        data_buff[0] = 0xee;
        len = 0x02;
        cmd = 0x38;
        get_mes_num = 0;
        BFCT_protocol_Zigbee.receive_enable=1;
        BFCT_protocol_Lock.receive_enable=0;
        zigbee_delay =0;
        zigbee_moni_state = 0x05;

     }
   break;
    case 4:
      if(user_attri == 0)
      {
        if(admin_No >0)
        {
           data_buff[0] = admin_No;
           admin_No_exist =1;
              len = admin_No*10+2;
              cmd = 0x38;
              buf = data_buff;
           ordinary_No =0;
        }
        else
        {
          if(get_mes_num < 3) {
            if(admin_No_exist)
            {
              j = 0;
             get_mes_num =0;
             admin_No_exist =0;
             zigbee_delay =0;
             zigbee_moni_state = 0;
             return 1;
            }else {
              data_buff[0] = 0xee;
              len = 0x02;
              cmd = 0x38;
              buf = data_buff;
            }
          }
          else
          {
             zigbee_delay =0;
             zigbee_moni_state = 6;
             return 0;
          }
        }
      }
      else if(user_attri == 1)
      {
          if(ordinary_No >0) 
          {
             ordinary_No_exist =1;
             data_buff[50 - ordinary_No*10 ] = ordinary_No;
            len = ordinary_No*10+2;
            cmd = 0x39;
            buf = &data_buff[50 - ordinary_No*10];
          }
          else{
            if(get_mes_num < 3) {
              if(ordinary_No_exist){
                 j = 0;
                 get_mes_num =0;
                 ordinary_No_exist =0;
                 zigbee_delay =0;
                 zigbee_moni_state = 0;
                 return 1;
              }else{
                   data_buff[0] = 0xee;
                  len = 0x02;
                  cmd = 0x39;
                  buf = data_buff;
              }
            }
            else
            {
               zigbee_delay =0;
               zigbee_moni_state = 6;
               return 0;
            }
          }
      }
      else
      {
          data_buff[0] = 0xee;
          len = 0x02;
          cmd = 0x3a;
          buf = data_buff;
      }
      zigbee_delay =0;
      zigbee_moni_state++;
      data3_tc = 101; 
      BFCT_protocol_Zigbee.receive_flag =0;
    break;

    case 5:
        ret = send_zigbeecmd(len,cmd,buf);
        if(ret ==1)
        {
          if(get_mes_num ==3 )
          {
              zigbee_moni_state++;
          }
          else
          {
              j = 0;
              get_mes_num =0;
              zigbee_delay = 0;
              zigbee_moni_state =0;
              return 1;
          }
          if( user_attri  == 0x02)
          {
              j = 0;
              get_mes_num =0;
              zigbee_delay = 0;
              zigbee_moni_state =0;
              return 1;
          }
        }
    break;
  case 6:
        if(t_1ms)
       {
         zigbee_delay++;
         if(zigbee_delay > 300){
           j++;
           data_buff[0]=3*j;  data_buff[1]=0;  data_buff[2]=3; 
           zigbee_moni_state =0x01;
           zigbee_delay=0;
         }
       }    
    break;
        
  default:
    zigbee_moni_state = 0;  
    break; 
  }
  return 0;
}

/*****zigbee 查询锁状态 *******/
u8 zigbee_inqure_lockstate()
{
  u8 ret;

  switch(zigbee_moni_state)
  {
  case 0:
       ret = send_zigbeecmd(0X01,ZIGBEE_CMD_ACK,data_buff);
       if(ret)
       {
         zigbee_delay =0;
         zigbee_moni_state++;
       }
    break;
    
   case 1:
       ret = send_lcokcmd(0x01,LOCK_CMD_INQURE_LOCK_STATE,data_buff );
       if(ret == 1)
       {
          BFCT_protocol_Lock.receive_flag=0;
          zigbee_delay =0;
          zigbee_moni_state++;
       }
       else if(ret == 2)
       {
        zigbee_delay =0;
        zigbee_moni_state=0;
        return 1;
       }

    break; 
  case 2:
      ret = get_lock_message();
      if(ret ==1)
      {
          ret = check_lock_cmd(ACK_LOCK_CMD_INQURE_LOCK_STATE);
          if(ret ==1)
          { 
            lockdata_2_zigbeedata(lcok_power_level_state,&(BFCT_protocol_Lock.receive_data[20]),&data_buff[0]); // power_level_state
            lockdata_2_zigbeedata(lock_anti_lock_state,&(BFCT_protocol_Lock.receive_data[20]),&data_buff[1]); // anti_lock_state
            lockdata_2_zigbeedata(lock_motor_state,&(BFCT_protocol_Lock.receive_data[20]),&data_buff[2]); // system_locked_state
            lockdata_2_zigbeedata(lock_square_tongue_locked_state,&(BFCT_protocol_Lock.receive_data[20]),&data_buff[3]); // square_tongue_locked_state

            zigbee_delay =0;
            zigbee_moni_state++;
          }
          else
          {
            zigbee_delay =0;
            zigbee_moni_state = 0;
            return 1;
          }    
      }
      else if(ret ==2)
      {
          zigbee_moni_state = 0;
          return 1;
      }
   break;
    case 3:
         ret = send_zigbeecmd(0x05,0x3B,data_buff);
         if(ret)
         {
            zigbee_delay = 0;
            zigbee_moni_state =0;
            return 1;
         }
    break;
        
  default:
    break; 
  }
  return 0;
}

u8 zigbee_amdin_identification(void)
{
  u8 ret,i;
  static u8 zigbee_moni_state=0;
  static u8 len=0,cmd;
    switch(zigbee_moni_state)
    {
    case 0:
     ret = send_zigbeecmd(0X01,ZIGBEE_CMD_ACK,data_buff);
     if(ret)
     {
       zigbee_moni_state ++;    
     }
    break;
    
    case 1:
        data_buff[0] = 0x80;
        zigbeedata_2_lockdata(zigbee_password,&data_buff[1],&BFCT_protocol_Zigbee.receive_data[6]);
        zigbee_delay = 0;
        for(i=4;i<8;i++)
        data_buff[i] = 0xff;
        zigbee_moni_state++;
    break;
    case 2:
       ret = send_lcokcmd(0x09,41,data_buff );
       if(ret == 1)
       {
          BFCT_protocol_Lock.receive_flag=0;
          zigbee_moni_state++;
       }
       else if(ret == 2)
       {  
          cmd = 0x05;
          len = 0x01;
          zigbee_moni_state =10; 
        } 
    
    break;
    case 3:
       ret = get_lock_message();
       if(ret ==1)
       {
          ret = check_lock_cmd(0xA9);
          if(ret ==1)
          { 
            if(BFCT_protocol_Lock.receive_data[LOCK_ACK_RES_ADDR] == 0)
            {
              cmd = 0x05;
            }
            else
            {
              cmd = 0x05;
            }
          }
          else
          {
            cmd = 0x05;
          }
       }
       else if(ret ==2)
       {
          cmd = 0x05;
       }
       
       len = 0x01;
       zigbee_moni_state=10;
       break;
    case 10:
       ret = send_zigbeecmd(len,cmd,data_buff);
       if(ret)
       {
         zigbee_moni_state = 0;    
         return 1;
       }
    break;
      
  default:
    zigbee_moni_state =0 ;
    break; 
  }
  return 0;
}


#if defined(Jun_He)
/*********查询锁状态,主要获得锁协议的类型************/
u8 zigbee_inqure_factory_id(void)
{
  static u8 state=0;
  static u8 len,cmd;
  u8 ret;
  switch(state)
  {
  case 0:
       data_buff[0] = 0x01;data_buff[1] = 0x02;
       cmd = 0xb1;
       len = 0x03;
       state++;
    break;
  case 1:
       ret = send_zigbeecmd(len,cmd,data_buff);
       if(ret)
       {
         zigbee_moni_state = 0;    
         return 1;
       }
    break;
    
    
  default:
    state = 0;
    break;
  }
  
  return 0;
}
#endif