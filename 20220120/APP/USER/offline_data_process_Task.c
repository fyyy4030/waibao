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
//		vTaskSuspend(NULL);//挂起自己
//	}
	
	while(1)
	{
		//*阻塞等待事件标志（离线数据处理）
		EventBits_t bit=xEventGroupWaitBits(offline_data_event_group, HAVE_OFFLINE_DATA_485|HAVE_OFFLINE_DATA_232, pdTRUE, pdFALSE,portMAX_DELAY);
		if(bit==HAVE_OFFLINE_DATA_485)
		{
			xEventGroupClearBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
			
			MY_PRINT("把从485从设备收到的相关数据存储到本地SD卡。以文本形式存储，一条消息一个文本\r\n");
			//把从232设备/或者485从设备收到的相关数据存储到本地SD卡。以文本形式存储，按时间先后顺序存储。
			if(0==offline_data_record(&RS485rev.RevBuf[8],*(uint16_t*)&RS485rev.RevBuf[6]))
			{
				MY_PRINT("\r\n\r\n\r\n成功写入SD卡\r\n\r\n\r\n");
			}
			else
			{
				MY_PRINT("\r\n\r\n\r\n写入失败\r\n\r\n\r\n");
			}
			
			
		}
		else if(bit==HAVE_OFFLINE_DATA_232)
		{
			xEventGroupClearBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);
			MY_PRINT("把从232设备收到的相关数据存储到本地SD卡。以文本形式存储，一条消息一个文本\r\n");
			memcpy(&RS232rev.RevBuf[RS232rev.BufLen],TDSaoMa,TDSaoMa_LEN_MAX);
			if(0==offline_data_record(RS232rev.RevBuf,  RS232rev.BufLen+TDSaoMa_LEN_MAX))
			{
				MY_PRINT("\r\n\r\n\r\n成功写入SD卡\r\n\r\n\r\n");
			}
			else
			{
				MY_PRINT("\r\n\r\n\r\n写入失败\r\n\r\n\r\n");
			}
		}
		
		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL
}



void report_Task(void *pvParameters)
{
	MY_PRINT("report_Task coming!!\r\n");
	

	while(1)
	{
		//*阻塞等待事件标志（离线数据处理）

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
					else//失败，记录到SD卡
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

