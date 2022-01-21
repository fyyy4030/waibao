#include "my_tasks.h"
#include "stdio.h"
#include "string.h"
#include "4G.h"
#include "wifi.h"
#include "sd_operation.h"


		

/*任务四：服务器下发的数据处理任务：（仅支持带网络的设备）
			a.服务器同步本地RTC
			b.从服务器获取升级文件到本地(SD卡)
*/
void server_data_process_Task(void *pvParameters)
{
	MY_PRINT("server_data_process_Task coming!!\r\n");
	uint32_t NotifyValue;
  BaseType_t err;
	while(1)
	{
	//阻塞等待事件标志（服务器新固件事件），自动清标志
		EventBits_t bit=xEventGroupWaitBits(firmware_event_group, GET_NEW_485FIRMWARE_FROM_SERVER|GET_NEW_232FIRMWARE_FROM_SERVER, pdTRUE, pdFALSE,portMAX_DELAY);	
		err=xTaskNotifyWait((uint32_t   )0x00,              //进入函数的时候不清除任务bit
                            (uint32_t   )0,         //退出函数的时候清除所有的bit
                            (uint32_t*  )&NotifyValue,      //保存任务通知值-服务器固件版本号
                            (TickType_t )5000);    //阻塞时间
	 if(err==pdTRUE) //获取任务通知成功	
	 {
		 if(flagNetWorkDev==DEV_WIFI){
			vTaskSuspend(network_status_check_wifi_TaskHandle);			 
		 }
		 else if(flagNetWorkDev==DEV_4G){
			vTaskSuspend(network_status_check_4g_TaskHandle);
			vTaskSuspend(network_status_check_wifi_TaskHandle);					 
		 }
		 //vTaskSuspend(master_rev_485dev_data_handle_TaskHandle);	
		 //vTaskSuspend(master_rev_232dev_data_handle_TaskHandle);	
			

			if(bit==GET_NEW_485FIRMWARE_FROM_SERVER)
			{
				MY_PRINT("\r\n开始从服务器下载上位机新固件到SD卡\r\n");//（此时固件是base64编码）
				falgDownLoadFW=1;		 //开始下载标志
				char file[20]={0};
				char ret=1;
				if(	flagNetWorkDev==DEV_4G)
					ret=air724_4g_http_get_firmware(http_key.BanBenID,"true",file);
				else if(flagNetWorkDev==DEV_WIFI)
					ret=wifi_http_get_firmware(http_key.BanBenID,"true",file);
				if(0==ret){
					falgDownLoadFW=0;		//结束下载标志 
					MY_PRINT("file name:%s\r\n",file);	
					if(strlen(file)>4){
						if(0==sd_change_base64_file(file,NULL)){
							MY_PRINT("固件解码完成\r\n");
							if(0==sd_mark_firmware_version(NotifyValue,FW232ID,true)){
								FW485ID=NotifyValue;
							}
						}
					}
				}	
			}
			else if(bit==GET_NEW_232FIRMWARE_FROM_SERVER)
			{
				MY_PRINT("\r\n开始从服务器下载下位机新固件到SD卡\r\n");//（此时固件是base64编码）
				falgDownLoadFW=1;		 //开始下载标志
				char file[20]={0};
				char ret=1;
				if(	flagNetWorkDev==DEV_4G)
					ret=air724_4g_http_get_firmware(http_key.XBanBenID,"false",file);
				else if(flagNetWorkDev==DEV_WIFI)
					ret=wifi_http_get_firmware(http_key.XBanBenID,"false",file);
				if(0==ret){
					
					MY_PRINT("file name:%s\r\n",file);	
					if(strlen(file)>4){
						if(0==sd_change_base64_file(file,NULL)){
							MY_PRINT("固件解码完成\r\n");
							//保存新固件版本号
							if(0==sd_mark_firmware_version(FW485ID,NotifyValue,true)){
								FW232ID=NotifyValue;
							}
						}
					}
				}	
				falgDownLoadFW=0;		//结束下载标志 				
			}	
			falgDownLoadFW=0;		//结束下载标志 
		 if(flagNetWorkDev==DEV_WIFI){
			vTaskResume(network_status_check_wifi_TaskHandle);//add 唤醒WIFI网络任务	
		 }
		 else if(flagNetWorkDev==DEV_4G){
				vTaskResume(network_status_check_4g_TaskHandle);//add 唤醒4G网络任务
				vTaskResume(network_status_check_wifi_TaskHandle);//add 唤醒WIFI网络任务			 
		 }
		//vTaskResume(master_rev_485dev_data_handle_TaskHandle);	
		//vTaskResume(master_rev_232dev_data_handle_TaskHandle);	 
		// xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_ON);//触发有网事件-为了检测是否有数据存到SD卡
	
			
	 }		
	 else
	 {
		
	 }
		 
		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL
}






