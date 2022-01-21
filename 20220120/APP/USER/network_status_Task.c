#include "my_tasks.h"
#include "stdio.h"
#include "led.h"
#include "4G.h"
#include "wifi.h"
#include "debug_uart.h"
#include "sd_operation.h"
#include "makedata.h"
#include "scanner.h"

static void delay_microsecond(uint32_t microsecond)
{
	microsecond *= 42;
	
	for(uint32_t i = 0; i < microsecond; i++)
	{
		;
	}
}

void network_status_check_Task(void *pvParameters)
{
//	uint8_t get_network_sta=flagNetWork;//��¼�����
//	
//	uint8_t mark_server_sta=flagServerConn;//��¼����������״̬	
	
	MY_PRINT("network_status_check_Task coming!!\r\n");
	while(1)
	{
Check_NW:
		//2�� ESP8266 Station ������ AP����� IP ��ַ
		//3�� ESP8266 Station �ѽ��� TCP �� UDP ����
		//4�� ESP8266 Station �Ͽ���������
		//5�� ESP8266 Station δ���� AP
		if(flagNetWork==NETWORK_OFF)
		{
			if(_Wifi_Sta.sta==3)//ESP8266 Station �ѽ��� TCP �� UDP ����,��ʱʹ�õ���WIFI��������ͨ��
			{
				flagNetWork=NETWORK_ON;//�豸������־
				flagNetWorkDev=DEV_WIFI;
				xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_ON);//���������¼�
			}
			else//WIFI����/��������Ͽ��ˣ���ʱʹ�õ���4G��������ͨ��
			{
				if(_4g_Sta.sta==3)//4G��������
				{
					flagNetWorkDev=DEV_4G;
					flagNetWork=NETWORK_ON;//�豸������־
					xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_ON);//���������¼�
				}
				else//WIFI 4G ��û������
				{
					
				}
			}			
		}

		if(flagNetWork==NETWORK_ON)
		{
			if(_Wifi_Sta.sta!=3 && _4g_Sta.sta!=3)//WIFI��4G��û������
			{
				flagNetWork=NETWORK_OFF;//�豸������־
				flagNetWorkDev=DEV_NULL;
				xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_OFF);//���������¼�
			}	
			//ʹ��WIFIʱ����WIFI����
			if(flagNetWorkDev==DEV_WIFI && _Wifi_Sta.sta!=3)
			{
				flagNetWorkDev=DEV_NULL;
				flagNetWork=NETWORK_OFF;//��ǰ�豸����Ͽ���־
				goto Check_NW;
			}
			//ʹ��4GʱWIFI�ָ�����
			if(flagNetWorkDev==DEV_4G && _Wifi_Sta.sta==3)
			{
				flagNetWorkDev=DEV_NULL;
				flagNetWork=NETWORK_OFF;//��ǰ�豸����Ͽ���־
				goto Check_NW;
			}
			
		}

		vTaskDelay(10);//���ܿ�ѭ��
	}
	
}

void network_status_change_handle_Task(void *pvParameters)
{
	EventBits_t bit;
	MY_PRINT("network_status_change_handle_Task coming!!\r\n");
	while(1)
	{
		//�����ȴ��¼���־������ָ���Ͽ��¼������ֶ����־
		bit=xEventGroupWaitBits(network_event_group, EVENT_OF_NETWORK_ON|EVENT_OF_NETWORK_OFF, pdFALSE, pdFALSE,portMAX_DELAY);
		if(bit==EVENT_OF_NETWORK_ON)//����һ�������¼�
		{
			xEventGroupClearBits(network_event_group, EVENT_OF_NETWORK_ON);//ֱ���ɹ������������ע����ɲ���������¼���־

			flagServerConn=CONNECTED;
			//LED3����
			LED3_ON;
			//�����������ݴ�������
			//vTaskSuspend(offline_data_process_TaskHandle);
			/*��Ȿ��SD���Ƿ����������ݴ洢���о��ϴ������������ϴ��ɹ���ɾ���������ݡ�*/
			vTaskSuspend(master_rev_485dev_data_handle_TaskHandle);
			//uint8_t *pbuf=malloc(2048);
			uint16_t length=0;
			uint8_t result;
			uint8_t fn_buf[25]={0};
			while(1)
			{
				result=read_offline_data((uint8_t *)RS232rev.RevBuf,&length,fn_buf);
				if(0==result){
					 if(length<TDSaoMa_LEN_MAX)//������ļ�
					 {
						 MY_PRINT("SD�����ݸ�ʽ����\r\n");
						 offline_data_file_remove(fn_buf);
					 } 
					 else
					 {
							RS232rev.BufLen=length-TDSaoMa_LEN_MAX;//ԭʼ���ݳ��ȣ���ȥɨ��ǹ����
							//char saoma[TDSaoMa_LEN_MAX]={0};
							memcpy(TDSaoMa,&RS232rev.RevBuf[RS232rev.BufLen],TDSaoMa_LEN_MAX);
							LED1_ON;
							//if(0==make_data_and_report(pbuf,yuanshi_data_len,TDSaoMa))
						 char RET=make_data_and_report(RS232rev.RevBuf,RS232rev.BufLen,TDSaoMa);
						 if(0==RET)
							{
								offline_data_file_remove(fn_buf);
								LED1_OFF;delay_microsecond(150*1000);	
								LED1_ON;delay_microsecond(150*1000);
								LED1_OFF;delay_microsecond(150*1000);	
							}
							else if(RET==100||RET==101)//��ʽ����
							{
								MY_PRINT("SD�����ݸ�ʽ����\r\n");
								offline_data_file_remove(fn_buf);
							}
							else//ʧ�ܣ���ɾ��
							{
								LED1_OFF;
								break;
							}					 
					 }

				}	
				else{
					break;
				}
				vTaskDelay(500);
				
			}
			//free(pbuf);
			vTaskResume(master_rev_485dev_data_handle_TaskHandle);
//				//��¼������SD���е����¹̼��汾��,-1Ϊû�иù̼�
//				sd_mark_firmware_version(-1,-1,true);//++++����
//				//��ȡ��ǰSD���б���Ĺ̼��İ汾
//				sd_read_firmware_version(&FW485ID,&FW232ID);	//++++����					
		}
		else if(bit==EVENT_OF_NETWORK_OFF)//����һ�ζ����¼�
		{
			xEventGroupClearBits(network_event_group, EVENT_OF_NETWORK_OFF);
			flagServerConn=DISCONNECT;//�豸ʧȥ���磬�϶���Ͽ��˷�����
			//LED3Ϩ��
			LED3_OFF;
			//���ѡ��������ݴ�������
			//vTaskResume(offline_data_process_TaskHandle);
			//��¼����ʱ��㵽����SD�� ����������Ϊ�豸����Ͽ� 
			broken_network_record( );
		}
		
		vTaskDelay(10);//���ܿ�ѭ��
	}
}







