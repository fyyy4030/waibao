#include "my_tasks.h"
#include "stdio.h"
#include "string.h"
#include "rs485.h"
#include "rs232.h"
#include "4G.h"
#include "makedata.h"
#include "led.h"
#include "scanner.h"
#include "sd_operation.h"
#include "io.h"
static void delay_microsecond(uint32_t microsecond)
{
	microsecond *= 42;
	
	for(uint32_t i = 0; i < microsecond; i++)
	{
		;
	}
}

/*任务六：485设备数据采集上报任务：(本机是带网络的主机)
          主机处理485从机发送过来的数据
			    如果接收到485设备从机上传的传感器数据
					    设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
					    设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储	
MASTER_REV_485DEV_DATA_HANDLE_TASK_PRO     7
MASTER_REV_485DEV_DATA_HANDLE_STACK_SIZE   256							
*/

void master_rev_485dev_data_handle_Task(void *pvParameters)
{
	
	uint8_t falgPublicData=0;
	uint8_t index=0x01;
	MY_PRINT("master_rev_485dev_data_handle_Task coming!!\r\n");
	while(1)
	{
		for(index=0x01;index<=0x1F;index++)//从机ID范围0x01~0x1F
		{
			if(0==send485_master_inquiry(index,FW485ID,FW232ID))//询问成功 
			{
				MY_PRINT("485从机 %d号在线\r\n",index);
				if(flagSensorDataCome)//如果接收485设备从机上传的传感器数据
				{
					MY_PRINT("485从机 %d号有数据发送过来\r\n",index);
					flagSensorDataCome=0;//清标志
					//校验数据包
					if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) )//校验成功
					{
						send485_ack(devID_master,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//发送应答
						
						falgPublicData=1;//触发数据上报
					}
					else
					{
						send485_ack_err(devID_master,RS485rev.RevBuf[2],0x01);/*发送错误反馈*/
					}
				}		

				if(falgPublicData&&falgDownLoadFW)//正在下载固件，把数据存在SD卡
				{
					falgPublicData=0;
					xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
//					if(0==offline_data_record(&RS485rev.RevBuf[8],*(uint16_t*)&RS485rev.RevBuf[6]))
//						MY_PRINT("\r\n\r\n成功写入SD卡\r\n\r\n\r\n");
//					else
//						MY_PRINT("\r\n\r\n写入失败\r\n\r\n\r\n");									
				}	
				
				if(1==falgPublicData)//上报数据
				{
					falgPublicData=0;
					if(flagServerConn)//设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
					{
						uint16_t yuanshi_data_len=*(uint16_t*)&RS485rev.RevBuf[6]-TDSaoMa_LEN_MAX;//原始数据长度，出去扫描枪数据
						char saoma[TDSaoMa_LEN_MAX]={0};
						memcpy(saoma,&RS485rev.RevBuf[8+yuanshi_data_len],TDSaoMa_LEN_MAX);
						LED1_ON;
						if(0==make_data_and_report(&RS485rev.RevBuf[8],yuanshi_data_len,saoma))
						{
							LED1_OFF;delay_microsecond(150*1000);	
							LED1_ON;delay_microsecond(150*1000);
							LED1_OFF;delay_microsecond(150*1000);								
						}
						else//失败，记录到SD卡
						{
							LED1_OFF;
							xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
//							if(0==offline_data_record(&RS485rev.RevBuf[8],*(uint16_t*)&RS485rev.RevBuf[6]))
//								MY_PRINT("\r\n\r\n成功写入SD卡\r\n\r\n\r\n");
//							else
//								MY_PRINT("\r\n\r\n写入失败\r\n\r\n\r\n");					
						}
					}
					else//设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储		
					{
						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
					}

				}				
				
			}		
			vTaskDelay(500);//轮询间隔500ms	
		}
		vTaskDelay(10);//不能空循环			
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL		
}

/*任务七：232设备数据采集上报任务：(本机是带网络的主机)
          主机处理232从机发送过来的数据
 					如果232设备上传的传感器数据，接收完成后
					    设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
					    设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储	
MASTER_REV_232DEV_DATA_HANDLE_TASK_PRO     8
MASTER_REV_232DEV_DATA_HANDLE_STACK_SIZE   256
*/
void master_rev_232dev_data_handle_Task(void *pvParameters)
{
	MY_PRINT("master_rev_232dev_data_handle_Task coming!!\r\n");
	uint8_t flagSaoMa=0;
	uint8_t flag232date=0;//只用在flagIsSaomaSingle==2
	while(1)
	{
		if(flagIO1||flagIO2)
		{
			LED4_ON;delay_microsecond(150*1000);
			LED4_OFF;delay_microsecond(150*1000);
			char io_n=0;char value=0;
			if(flagIO1)
			{
				flagIO1=0;io_n=1;value=1;
			}	
			else if(flagIO2)
			{
				flagIO2=0;io_n=2;	value=2;
			}
//			xEventGroupSetBits(report_event_group, EVENT_REPORT_io);		
			LED1_ON;
			if(0==make_data_and_report_io(io_n,value))
			{
					LED1_OFF;delay_microsecond(150*1000);	
					LED1_ON;delay_microsecond(150*1000);
					LED1_OFF;delay_microsecond(150*1000);					
			}
			else
			{
				LED1_OFF;
			}			
			
		}
		if(scannerrev.RevOver)
		{
			scannerrev.RevOver=0;
//			memset(TDSaoMa,0,sizeof(TDSaoMa));
//			memcpy(TDSaoMa,scannerrev.RevBuf,scannerrev.BufLen-2);//最后两个字节是\r\n
			LED4_ON;delay_microsecond(150*1000);
			LED4_OFF;delay_microsecond(150*1000);
			flagSaoMa=1;
		
		}		

		if(flagIsSaomaSingle==0)//0没有扫码枪,232来数据直接上报
		{
			if(flag232CeShiJieGuo&&(falgDownLoadFW||flagReport_ing))//正在下载固件或者正在上报
			{
				flag232CeShiJieGuo=0;
				xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
			}
			if(flag232CeShiJieGuo)/*接受到来自232设备的传感器数据*/
			{
				flag232CeShiJieGuo=0;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);
				if(flagServerConn)//设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
				{
					MY_PRINT("\r\n模式：%d\r\n\r\n",flagIsSaomaSingle);
					xEventGroupSetBits(report_event_group, EVENT_REPORT_232);		
//					LED1_ON;
//					if(0==make_data_and_report(RS232rev.RevBuf,RS232rev.BufLen,TDSaoMa))
//					{
//						LED1_OFF;delay_microsecond(150*1000);	
//						LED1_ON;delay_microsecond(150*1000);
//						LED1_OFF;delay_microsecond(150*1000);	
//							
//					}
//					else//失败，记录到SD卡
//					{
//						LED1_OFF;
//						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
//					}
				}
				else//设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储		
				{
					xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);
				}

			}			
		}
		else if(flagIsSaomaSingle==1)//1只有扫码枪，扫码枪来数据直接上报
		{
			if(flagSaoMa&&falgDownLoadFW==0)//单独上报扫码
			{
				flagSaoMa=0;
				MY_PRINT("\r\n模式：%d\r\n\r\n",flagIsSaomaSingle);
				xEventGroupSetBits(report_event_group, EVENT_REPORT_saoma);		
//				LED1_ON;
//				if(0==make_data_and_report_saoma(TDSaoMa))
//				{
//						LED1_OFF;delay_microsecond(150*1000);	
//						LED1_ON;delay_microsecond(150*1000);
//						LED1_OFF;delay_microsecond(150*1000);					
//				}
//				else
//				{
//					LED1_OFF;
//				}
			}
		}
		else if(flagIsSaomaSingle==2)//2扫码枪+232，扫码和232数据都有了再上报
		{
			if(flag232CeShiJieGuo)
			{
				flag232CeShiJieGuo=0;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);				
				flag232date=1;
			}
			if(flagSaoMa&&flag232date&&(falgDownLoadFW||flagReport_ing))//正在下载固件或者正在上报
			{
				flagSaoMa=0;
				flag232date=0;
				xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
			}
			if(flagSaoMa&&flag232date)//扫码和232数据都有了再上报
			{
				flagSaoMa=0;
				flag232date=0;			
				if(flagServerConn)//设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
				{
					MY_PRINT("\r\n模式：%d\r\n\r\n",flagIsSaomaSingle);
					xEventGroupSetBits(report_event_group, EVENT_REPORT_232);		
//					LED1_ON;
//					if(0==make_data_and_report(RS232rev.RevBuf,RS232rev.BufLen,TDSaoMa))
//					{
//						LED1_OFF;delay_microsecond(150*1000);	
//						LED1_ON;delay_microsecond(150*1000);
//						LED1_OFF;delay_microsecond(150*1000);	
//							
//					}
//					else//失败，记录到SD卡
//					{
//						LED1_OFF;
//						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
//					}
				}
				else//设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储		
				{
					xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);
				}				
				
			}
			
		}
		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL		
}


#include "string.h"

/*任务六：232设备数据采集上报任务：(本机是非网络设备的从机)
          从机处理232从机发送过来的数据
					如果232设备上传传感器数据，接收完成后，通过485传输给带网络的主机
*/	
void salver_rev_232dev_data_handle_Task(void *pvParameters)
{
	MY_PRINT("salver_rev_232dev_data_handle_Task coming!!\r\n");
	uint8_t flagSaoMa=0;
	uint8_t flag232data=0;
	uint8_t flagsSendReady=0;
	while(1)
	{
		if(flagIO1||flagIO2)
		{
			LED4_ON;delay_microsecond(150*1000);
			LED4_OFF;delay_microsecond(150*1000);
			char io_n=0;char value=0;
			if(flagIO1)
			{
				flagIO1=0;io_n=1;value=1;
			}	
			else if(flagIO2)
			{
				flagIO2=0;io_n=2;	value=2;
			}
			//把数据包装成232数据帧（不用组saoma）,存在RS232rev.RevBuf
			//flagsSendReady=1;
			
		}
			
		
		if(flagIsSaomaSingle==0)//0没有扫码枪,232来数据直接上报   OK!!!!
		{
			if(flag232CeShiJieGuo)
			{
				flag232CeShiJieGuo=0;
				flag232data=1;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);
			}			
			if(flag232data==1)
			{
				flag232data=0;
				flagsSendReady=1;
			}
		}
		else if(flagIsSaomaSingle==1)//1只有扫码枪，扫码枪来数据直接上报
		{
			if(scannerrev.RevOver)
			{
				scannerrev.RevOver=0;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);
				flagSaoMa=1;
			}			
			if(flagSaoMa==1)
			{
				//把数据包装成232数据帧（不用组saoma），,存在RS232rev.RevBuf
				//flagsSendReady=1;
			}
			
		}
		else if(flagIsSaomaSingle==2)//2扫码枪+232，扫码和232数据都有了再上报   OK!!!!
		{
			if(flag232CeShiJieGuo)
			{
				flag232CeShiJieGuo=0;
				flag232data=1;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);
			}	
			if(scannerrev.RevOver)
			{
				scannerrev.RevOver=0;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);
				flagSaoMa=1;
			}		
			if(flagSaoMa==1&&flag232data==1)
			{
				flagSaoMa=0;flag232data=0;
				flagsSendReady=1;
			}
		}		
		
		
		if(flagInquiry)
		{
			flagInquiry=0;//收到来自主机的在线询问
			MY_PRINT("485主机询问\r\n");
			//flag232CeShiJieGuo=1;//测试，手动置1生效
			if(flagsSendReady)//如果232设备上传传感器数据，接收完成后，（不需要解析）通过485传输给带网络的主机
			{
				flagsSendReady=0;
		
				memcpy(&RS232rev.RevBuf[RS232rev.BufLen],TDSaoMa,TDSaoMa_LEN_MAX);
				if(0==send485_sensor_data(devID_salve,devID_master,(uint8_t *)RS232rev.RevBuf,RS232rev.BufLen+TDSaoMa_LEN_MAX))//发送成功，对方已经接收
				{
					MY_PRINT("发送给485主机成功\r\n\r\n");
					//LED4_ON;delay_microsecond(150*1000);
					//LED4_OFF;delay_microsecond(150*1000);
					memset(TDSaoMa,0,TDSaoMa_LEN_MAX);
				}
				else//如果对应无应答，传输失败，则。。。。
				{
					MY_PRINT("发送给485主机失败\r\n\r\n");
				}				
			}
			else if(*(int *)&RS485rev.RevBuf[8]>FW485ID)//告诉主机需要传输485固件过来
			{
				if(0==send485_frimware_update_notify(devID_salve,devID_master,0x01))
				{
					MY_PRINT("485固件更新请求发送成功\\r\n\r\n");
				}
				else
				{
					MY_PRINT("485固件更新请求发送失败\\r\n\r\n");	
				}
									
			}
			else if(*(int *)&RS485rev.RevBuf[8+4]>FW232ID)//告诉主机需要传输232固件过来
			{
				if(0==send485_frimware_update_notify(devID_salve,devID_master,0x02))
				{MY_PRINT("232固件更新请求发送成功\\r\n\r\n");
					}
				else
				{MY_PRINT("232固件更新请求发送失败\\r\n\r\n");
					}
			}				
			else//没有收到232设备的数据则回复应答
			{
				//回复应答表示在线
				send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);				
			}
			
		}

		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL		
}





















