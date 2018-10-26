#include "protocol.h"
#include "global.h"
#include "userdata.h"
#include "command.h"
#include "bsp.h"


u8 exe_zigbee_cmd(u8 zigbee_cmd)
{
  u8  ret;
  
    if(zigbee_cmd==ZIGBEE_CMD_ACK)
    { 
      return 1;
    }
#if defined ZIGBEE_CMD_ADD_USER_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_ADD_USER)  //添加用户
    {
      ret = zigbee_adduser();
    }
#endif
#if defined ZIGBEE_CMD_ADD_USER_FUNC  
    else if(zigbee_cmd==ZIGBEE_CMD_DELETE_USER)  //删除用户
    {
      ret = zigbee_delete_user();
    }
#endif
#if defined ZIGBEE_CMD_MODIFY_USER_FUNC   
    else if(zigbee_cmd==ZIGBEE_CMD_MODIFY_USER) //修改用户
    {
      ret = zigbee_modify_user();
    }
#endif    
#if defined ZIGBEE_CMD_OPENNET_FAIL_FUNC    
    else if(zigbee_cmd==ZIGBEE_CMD_OPENNET_FAIL)
    {
      ret = zigbee_opennet_fail();
    }
#endif    
#if defined ZIGBEE_CMD_OPENNET_SUCEESS_FUNC  
    else if(zigbee_cmd==ZIGBEE_CMD_OPENNET_SUCEESS)
    {
      ret = zigbee_opennet_sucess();
    }
#endif    
#if defined ZIGBEE_CMD_OPENLOCK_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_OPENLOCK)
    {
      ret = zigbee_openlock(); 
    }
#endif    
#if defined ZIGBEE_CMD_CLOCK_SYNC_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_CLOCK_SYNC)
    {
      ret = zigbee_clock_sync();
    }
#endif    
#if defined ZIGBEE_CMD_INQURE_USERINFO_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_INQURE_USERINFO)
    {
      ret = zigbee_inqure_userinfo();
    }
#endif    
#if defined ZIGBEE_CMD_INQURE_LOCK_STATE_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_INQURE_LOCK_STATE)
    {
      ret = zigbee_inqure_lockstate();    
    }
#endif    
#if defined ZIGBEE_CMD_ADMIN_IDENTIFICATION_FUNC    
    else if(zigbee_cmd==ZIGBEE_CMD_ADMIN_IDENTIFICATION)
    {
       ret = zigbee_amdin_identification();
    }
#endif    
#if defined ZIGBEE_CMD_OPENNET_FAIL_2_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_OPENNET_FAIL_2)
    {
       ret = zigbee_opennet_fail_2();
    }
#endif
#if defined ZIGBEE_CMD_inqure_factory_id_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_inqure_factory_id)
    {
       ret = zigbee_inqure_factory_id();
       if(ret ==1)
       {
          transparently_flag =1;
          write_userdata2eeprom(transparently_flag_addr,&transparently_flag, 1);
       }
    }
#endif
#if defined ZIGBEE_CMD_ONLINE_INFO_FUNC 
    else if(zigbee_cmd==ZIGBEE_CMD_ONLINE_INFO)
    {
       ret = zigbee_online_info();
    }

#endif
    else 
    {  
      ret =1;
    }
    
    return ret;
}


u8 exe_lock_cmd(u8 lock_cmd)
{
  u8 ret;
  
    if(lock_cmd==LOCK_OPENNET)
    {
      ret = lock_opennet();
    }
#if    defined LOCK_CLOSENETFUNC
    else if(lock_cmd==LOCK_CLOSENET)
    {
      ret = lock_closenet();
    }
#endif    
    else if(lock_cmd==LOCK_LOCK_BE_OPENED)
    {
      ret =  lock_be_opened();
    }
#if defined LCOK_LOCK_BE_CLOSED_FUNC    
    else if(lock_cmd==LCOK_LOCK_BE_CLOSED)
    {
      ret =  lock_be_closed();
    } 
#endif    
#if defined LOCK_OPENLOCK_BYFORCE_FUNC     
    else if(lock_cmd==LOCK_OPENLOCK_BYFORCE)
    {
      ret = lock_openclock_byforce();
    }
#endif    
#if defined LOCK_LOGIN_SAFE_MODE_FUNC 
    else if(lock_cmd==LOCK_LOGIN_SAFE_MODE)
    {
      ret = lock_login_safemode();
    }
#endif    
    else if(lock_cmd==LOCK_LOGOUT_SAFE_MODE)
    {
#if defined(Nan_Jing)
      ret = lock_logout_safe_mode();
#else
      ret =1;
#endif
    }
    else if(lock_cmd==LOCK_LOGOUT_SETTING_MODE)
    {
      ret = lock_logout_settting_mode();
    }
#if defined LOCK_USER_ADDED_FUNC 
    else if(lock_cmd==LOCK_USER_ADDED)
    {
      ret =lock_add_user();
    }
#endif    
#if defined LOCK_STATE_UPDATA_FUNC    
    else if(lock_cmd==LOCK_STATE_UPDATA)
    {
      ret = lock_state_updata();
    }
#endif    
#if defined LOCK_delete_USER_FUNC
    else if(lock_cmd==LOCK_delete_USER)
    {
      ret =lock_delete_user();
    }  
#endif    
#if defined(LOCK_ALARM_FUNC) 
    else if(lock_cmd==LOCK_ALARM)
    {  
      ret =lock_alarm();     
    }
#endif     
    else if(lock_cmd==LOCK_illgal_user_REPORT) //非法开锁 锁上报
    {
      ret = lolck_illage_user_report();
    }
#if defined LOCK_FORMAT_FUNC    
    else if(lock_cmd==LOCK_FORMAT) //非法开锁 锁上报
    {
      ret = lock_format();
    }
#endif    
    else 
    {
      ret =1;
    }
    
    return ret;
}