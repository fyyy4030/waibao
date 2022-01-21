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
//	uint8_t get_network_sta=flagNetWork;//记录检测结果
//	
//	uint8_t mark_server_sta=flagServerConn;//记录服务器连接状态	
	
	MY_PRINT("network_status_check_Task coming!!\r\n");
	while(1)
	{
Check_NW:
		//2： ESP8266 Station 已连接 AP，获得 IP 地址
		//3： ESP8266 Station 已建立 TCP 或 UDP 传输
		//4： ESP8266 Station 断开网络连接
		//5： ESP8266 Station 未连接 AP
		if(flagNetWork==NETWORK_OFF)
		{
			if(_Wifi_Sta.sta==3)//ESP8266 Station 已建立 TCP 或 UDP 传输,此时使用的是WIFI进行网络通信
			{
				flagNetWork=NETWORK_ON;//设备有网标志
				flagNetWorkDev=DEV_WIFI;
				xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_ON);//触发有网事件
			}
			else//WIFI网络/与服务器断开了，此时使用的是4G进行网络通信
			{
				if(_4g_Sta.sta==3)//4G网络正常
				{
					flagNetWorkDev=DEV_4G;
					flagNetWork=NETWORK_ON;//设备有网标志
					xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_ON);//触发有网事件
				}
				else//WIFI 4G 都没有网络
				{
					
				}
			}			
		}

		if(flagNetWork==NETWORK_ON)
		{
			if(_Wifi_Sta.sta!=3 && _4g_Sta.sta!=3)//WIFI与4G都没有网络
			{
				flagNetWork=NETWORK_OFF;//设备断网标志
				flagNetWorkDev=DEV_NULL;
				xEventGroupSetBits(network_event_group, EVENT_OF_NETWORK_OFF);//触发断网事件
			}	
			//使用WIFI时发生WIFI断网
			if(flagNetWorkDev==DEV_WIFI && _Wifi_Sta.sta!=3)
			{
				flagNetWorkDev=DEV_NULL;
				flagNetWork=NETWORK_OFF;//当前设备网络断开标志
				goto Check_NW;
			}
			//使用4G时WIFI恢复网络
			if(flagNetWorkDev==DEV_4G && _Wifi_Sta.sta==3)
			{
				flagNetWorkDev=DEV_NULL;
				flagNetWork=NETWORK_OFF;//当前设备网络断开标志
				goto Check_NW;
			}
			
		}

		vTaskDelay(10);//不能空循环
	}
	
}

void network_status_change_handle_Task(void *pvParameters)
{
	EventBits_t bit;
	MY_PRINT("network_status_change_handle_Task coming!!\r\n");
	while(1)
	{
		//阻塞等待事件标志（网络恢复与断开事件），手动清标志
		bit=xEventGroupWaitBits(network_event_group, EVENT_OF_NETWORK_ON|EVENT_OF_NETWORK_OFF, pdFALSE, pdFALSE,portMAX_DELAY);
		if(bit==EVENT_OF_NETWORK_ON)//发生一次有网事件
		{
			xEventGroupClearBits(network_event_group, EVENT_OF_NETWORK_ON);//直到成功接入服务器并注册完成才清除有网事件标志

			flagServerConn=CONNECTED;
			//LED3常亮
			LED3_ON;
			//挂起“离线数据处理任务”
			//vTaskSuspend(offline_data_process_TaskHandle);
			/*检测本地SD卡是否有离线数据存储，有就上传到服务器，上传成功后删除离线数据。*/
			vTaskSuspend(master_rev_485dev_data_handle_TaskHandle);
			//uint8_t *pbuf=malloc(2048);
			uint16_t length=0;
			uint8_t result;
			uint8_t fn_buf[25]={0};
			while(1)
			{
				result=read_offline_data((uint8_t *)RS232rev.RevBuf,&length,fn_buf);
				if(0==result){
					 if(length<TDSaoMa_LEN_MAX)//错误的文件
					 {
						 MY_PRINT("SD卡数据格式有误\r\n");
						 offline_data_file_remove(fn_buf);
					 } 
					 else
					 {
							RS232rev.BufLen=length-TDSaoMa_LEN_MAX;//原始数据长度，出去扫描枪数据
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
							else if(RET==100||RET==101)//格式错误
							{
								MY_PRINT("SD卡数据格式有误\r\n");
								offline_data_file_remove(fn_buf);
							}
							else//失败，不删除
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
//				//记录保存在SD卡中的最新固件版本号,-1为没有该固件
//				sd_mark_firmware_version(-1,-1,true);//++++测试
//				//读取当前SD卡中保存的固件的版本
//				sd_read_firmware_version(&FW485ID,&FW232ID);	//++++测试					
		}
		else if(bit==EVENT_OF_NETWORK_OFF)//发生一次断网事件
		{
			xEventGroupClearBits(network_event_group, EVENT_OF_NETWORK_OFF);
			flagServerConn=DISCONNECT;//设备失去网络，肯定会断开了服务器
			//LED3熄灭
			LED3_OFF;
			//唤醒“离线数据处理任务”
			//vTaskResume(offline_data_process_TaskHandle);
			//记录断网时间点到本地SD卡 ，本次是因为设备网络断开 
			broken_network_record( );
		}
		
		vTaskDelay(10);//不能空循环
	}
}







