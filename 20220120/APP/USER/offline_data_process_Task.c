#include "my_tasks.h"
#include "stdio.h"
#include "led.h"
#include "rs485.h"
#include "rs232.h"
#include "sd_operation.h"
#include "scanner.h"
#include "makedata.h"



static void delay_microsecond(uint32_t microsecond)
{
	microsecond *= 42;
	
	for(uint32_t i = 0; i < microsecond; i++)
	{
		;
	}
}

void offline_data_process_Task(void *pvParameters)
{
	MY_PRINT("offline_data_process_Task coming!!\r\n");
//	if(flagServerConn==DISCONNECT)
//	{
//		vTaskSuspend(NULL);//�����Լ�
//	}
	
	while(1)
	{
		//*�����ȴ��¼���־���������ݴ���
		EventBits_t bit=xEventGroupWaitBits(offline_data_event_group, HAVE_OFFLINE_DATA_485|HAVE_OFFLINE_DATA_232, pdTRUE, pdFALSE,portMAX_DELAY);
		if(bit==HAVE_OFFLINE_DATA_485)
		{
			xEventGroupClearBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
			
			MY_PRINT("�Ѵ�485���豸�յ���������ݴ洢������SD�������ı���ʽ�洢��һ����Ϣһ���ı�\r\n");
			//�Ѵ�232�豸/����485���豸�յ���������ݴ洢������SD�������ı���ʽ�洢����ʱ���Ⱥ�˳��洢��
			if(0==offline_data_record(&RS485rev.RevBuf[8],*(uint16_t*)&RS485rev.RevBuf[6]))
			{
				MY_PRINT("\r\n\r\n\r\n�ɹ�д��SD��\r\n\r\n\r\n");
			}
			else
			{
				MY_PRINT("\r\n\r\n\r\nд��ʧ��\r\n\r\n\r\n");
			}
			
			
		}
		else if(bit==HAVE_OFFLINE_DATA_232)
		{
			xEventGroupClearBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);
			MY_PRINT("�Ѵ�232�豸�յ���������ݴ洢������SD�������ı���ʽ�洢��һ����Ϣһ���ı�\r\n");
			memcpy(&RS232rev.RevBuf[RS232rev.BufLen],TDSaoMa,TDSaoMa_LEN_MAX);
			if(0==offline_data_record(RS232rev.RevBuf,  RS232rev.BufLen+TDSaoMa_LEN_MAX))
			{
				MY_PRINT("\r\n\r\n\r\n�ɹ�д��SD��\r\n\r\n\r\n");
			}
			else
			{
				MY_PRINT("\r\n\r\n\r\nд��ʧ��\r\n\r\n\r\n");
			}
		}
		
		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL
}



void report_Task(void *pvParameters)
{
	MY_PRINT("report_Task coming!!\r\n");
	

	while(1)
	{
		//*�����ȴ��¼���־���������ݴ���

		EventBits_t bit=xEventGroupWaitBits(report_event_group, EVENT_REPORT_232 | EVENT_REPORT_saoma | EVENT_REPORT_io, pdTRUE, pdFALSE,portMAX_DELAY);
		if(bit==EVENT_REPORT_232)
		{
			xEventGroupClearBits(report_event_group, EVENT_REPORT_232);
			flagReport_ing=1;
					LED1_ON;
					if(0==make_data_and_report(RS232rev.RevBuf,RS232rev.BufLen,TDSaoMa))
					{
						flagReport_ing=0;
						LED1_OFF;delay_microsecond(150*1000);	
						LED1_ON;delay_microsecond(150*1000);
						LED1_OFF;delay_microsecond(150*1000);	
							
					}
					else//ʧ�ܣ���¼��SD��
					{
						LED1_OFF;
						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
					}			
			flagReport_ing=0;
		}
		else if(bit==EVENT_REPORT_saoma)
		{
			xEventGroupClearBits(report_event_group, EVENT_REPORT_saoma);
			flagReport_ing=1;
				LED1_ON;
				if(0==make_data_and_report_saoma(TDSaoMa))
				{
					flagReport_ing=0;			
						LED1_OFF;delay_microsecond(150*1000);	
						LED1_ON;delay_microsecond(150*1000);
						LED1_OFF;delay_microsecond(150*1000);					
				}
				else
				{
					LED1_OFF;
				}			
			flagReport_ing=0;			
		}
		else if(bit==EVENT_REPORT_io)
		{
			xEventGroupClearBits(report_event_group, EVENT_REPORT_io);
//			flagReport_ing=1;
//			LED1_ON;
//			if(0==make_data_and_report_io(io_n,value))
//			{
			flagReport_ing=0;			
//					LED1_OFF;delay_microsecond(150*1000);	
//					LED1_ON;delay_microsecond(150*1000);
//					LED1_OFF;delay_microsecond(150*1000);					
//			}
//			else
//			{
//				LED1_OFF;
//			}				
//			flagReport_ing=0;			
		}	
	
	}
	
}

