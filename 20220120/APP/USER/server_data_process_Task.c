#include "my_tasks.h"
#include "stdio.h"
#include "string.h"
#include "4G.h"
#include "wifi.h"
#include "sd_operation.h"


		

/*�����ģ��������·������ݴ������񣺣���֧�ִ�������豸��
			a.������ͬ������RTC
			b.�ӷ�������ȡ�����ļ�������(SD��)
*/
void server_data_process_Task(void *pvParameters)
{
	MY_PRINT("server_data_process_Task coming!!\r\n");
	uint32_t NotifyValue;
  BaseType_t err;
	while(1)
	{
	//�����ȴ��¼���־���������¹̼��¼������Զ����־
		EventBits_t bit=xEventGroupWaitBits(firmware_event_group, GET_NEW_485FIRMWARE_FROM_SERVER|GET_NEW_232FIRMWARE_FROM_SERVER, pdTRUE, pdFALSE,portMAX_DELAY);	
		err=xTaskNotifyWait((uint32_t   )0x00,              //���뺯����ʱ���������bit
                            (uint32_t   )0,         //�˳�������ʱ��������е�bit
                            (uint32_t*  )&NotifyValue,      //��������ֵ֪ͨ-�������̼��汾��
                            (TickType_t )5000);    //����ʱ��
	 if(err==pdTRUE) //��ȡ����֪ͨ�ɹ�	
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
				MY_PRINT("\r\n��ʼ�ӷ�����������λ���¹̼���SD��\r\n");//����ʱ�̼���base64���룩
				falgDownLoadFW=1;		 //��ʼ���ر�־
				char file[20]={0};
				char ret=1;
				if(	flagNetWorkDev==DEV_4G)
					ret=air724_4g_http_get_firmware(http_key.BanBenID,"true",file);
				else if(flagNetWorkDev==DEV_WIFI)
					ret=wifi_http_get_firmware(http_key.BanBenID,"true",file);
				if(0==ret){
					falgDownLoadFW=0;		//�������ر�־ 
					MY_PRINT("file name:%s\r\n",file);	
					if(strlen(file)>4){
						if(0==sd_change_base64_file(file,NULL)){
							MY_PRINT("�̼��������\r\n");
							if(0==sd_mark_firmware_version(NotifyValue,FW232ID,true)){
								FW485ID=NotifyValue;
							}
						}
					}
				}	
			}
			else if(bit==GET_NEW_232FIRMWARE_FROM_SERVER)
			{
				MY_PRINT("\r\n��ʼ�ӷ�����������λ���¹̼���SD��\r\n");//����ʱ�̼���base64���룩
				falgDownLoadFW=1;		 //��ʼ���ر�־
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
							MY_PRINT("�̼��������\r\n");
							//�����¹̼��汾��
							if(0==sd_mark_firmware_version(FW485ID,NotifyValue,true)){
								FW232ID=NotifyValue;
							}
						}
					}
				}	
				falgDownLoadFW=0;		//�������ر�־ 				
			}	
			falgDownLoadFW=0;		//�������ر�־ 
		 if(flagNetWorkDev==DEV_WIFI){
			vTaskResume(network_status_check_wifi_TaskHandle);//add ����WIFI��������	
		 }
		 else if(flagNetWorkDev==DEV_4G){
				vTaskResume(network_status_check_4g_TaskHandle);//add ����4G��������
				vTaskResume(network_status_check_wifi_TaskHandle);//add ����WIFI��������			 
		 }
		//vTaskResume(master_rev_485dev_data_handle_TaskHandle);	
		//vTaskResume(master_rev_232dev_data_handle_TaskHandle);	 
		// xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_ON);//���������¼�-Ϊ�˼���Ƿ������ݴ浽SD��
	
			
	 }		
	 else
	 {
		
	 }
		 
		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL
}






