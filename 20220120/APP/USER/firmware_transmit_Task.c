#include "my_tasks.h"
#include "stdio.h"
#include "rs485.h"
#include "rs232.h"
#include "sd_operation.h"
#include "makedata.h"
#include "debug_uart.h"
/*任务五：主机升级固件传输任务：（仅用于主机）
				阻塞等待事件标志（新固件事件）
				①得到了新固件（不管是232固件还是485固件），通过485协议把升级固件发送到非网络设备的从机	
				②如果新固件类型是485固件  在SD卡记录有新固件的标志文件	                          
				 如果新固件类型是232固件  发送232固件传输事件给任务“firmware_transmit_to_232dev_Task”
*/
void firmware_transmit_to_salve_Task(void *pvParameters)
{
	MY_PRINT("firmware_transmit_to_salve_Task coming!!\r\n");
	uint8_t frimware_type=0;//固件类型
	while(1)
	{
		if(flagSlaveUpdateFW )///从机请求主机传输固件标志
		{
			flagSlaveUpdateFW=0;
			
			uint8_t frimware_type=RS485rev.RevBuf[8];//固件类型
			if(frimware_type==0x01)
			{
				MY_PRINT("从机需要更新485固件\r\n");
			}
			else if(frimware_type==0x02)
			{
				MY_PRINT("从机需要更新232固件\r\n");
			}
			//挂起其他任务
		 if(flagNetWorkDev==DEV_WIFI){
			vTaskSuspend(network_status_check_wifi_TaskHandle);			 
		 }
		 else if(flagNetWorkDev==DEV_4G){
			vTaskSuspend(network_status_check_4g_TaskHandle);
			vTaskSuspend(network_status_check_wifi_TaskHandle);					 
		 }
		 vTaskSuspend(master_rev_485dev_data_handle_TaskHandle);	
		 vTaskSuspend(master_rev_232dev_data_handle_TaskHandle);
	   //falgDownLoadFW=1;
		 
			char ret;
		  int firmware_size=0;
		  char file[15]={0};
			if(frimware_type==0x01)
				strcpy(file,"485fw.bin");
			else if(frimware_type==0x02)
				strcpy(file,"232fw.bin");
			if(0==sd_read_firmware_size(file,&firmware_size,true)) 
			{
				ret = send485_firmware_update_start(devID_master,RS485rev.RevBuf[2],0x02,firmware_size);//
				if(ret==0)
				{ //循环分批发送固件
					uint8_t rev_buf[510]={0};
					int start_addr=0;
					UINT br=0;
					uint16_t trans_cnt=0;//传输次数
					while(1)
					{
						if(0==sd_read_firmware(file,rev_buf,start_addr,500,&br,true))
						{
							if(0==send485_firmware_data(devID_master,RS485rev.RevBuf[2],rev_buf,br))
							{
								trans_cnt++;
								start_addr+=br;
								if(br<500 || start_addr==firmware_size)//发送完毕
								{
									break;
								}
								memset(rev_buf,0,sizeof(rev_buf));
							}
							else
							{
								break;
							}
							
						}
						else
						{
						 break;
						}
					}
					if(start_addr==firmware_size)//传输完全
					{
						MY_PRINT("从机更新固件完成\r\n");
						if(frimware_type==0x01)
							send485_firmware_update_end(devID_master,RS485rev.RevBuf[2],trans_cnt,FW485ID);
						else if(frimware_type==0x02)
							send485_firmware_update_end(devID_master,RS485rev.RevBuf[2],trans_cnt,FW232ID);
					
					}
					
				}
			}
			
			
			//唤醒其他任务
			if(flagNetWorkDev==DEV_WIFI){
				vTaskResume(network_status_check_wifi_TaskHandle);//add 唤醒WIFI网络任务	
			 }
			 else if(flagNetWorkDev==DEV_4G){
					vTaskResume(network_status_check_4g_TaskHandle);//add 唤醒4G网络任务
					vTaskResume(network_status_check_wifi_TaskHandle);//add 唤醒WIFI网络任务			 
			 }
			vTaskResume(master_rev_485dev_data_handle_TaskHandle);	
			vTaskResume(master_rev_232dev_data_handle_TaskHandle);	
			//falgDownLoadFW=0; 
			 
			flagSlaveUpdateFW=0;
		}
		
		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL
	
}


#include "crc16.h"

/*任务五：从机升级固件传输任务：（仅用于从机）
				等待主机与之进行固件传输
				得到了新固件，通过232协议把升级固件发送到232设备（传输协议甲方提供）
				如果新固件类型是485固件，在SD卡记录有新固件的标志文件
				如果新固件类型是232固件，发送232固件传输事件给任务“firmware_transmit_to_232dev_Task”
*/
void firmware_receive_from_master_Task(void *pvParameters)
{
	MY_PRINT("firmware_receive_from_master_Task coming!!\r\n");
	uint8_t frimware_type=0;//固件类型
	uint16_t frimware_size=0;//固件大小
	uint8_t times=0;
	char file[10]={0};
  char ret=0;
	int CRC32=0;
	int count=0;
	while(1)
	{
			if(flagFirmwareStart)
			{
				flagFirmwareStart=0;
				count=0;CRC32=0;
				//校验数据包
				if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){//校验成功
					frimware_type=RS485rev.RevBuf[8];//固件类型
					frimware_size=*(uint16_t *)&RS485rev.RevBuf[9];//固件大小
					if(frimware_type==0x01)
						strcpy(file,"485fw.bin");
					else if(frimware_type==0x02)
						strcpy(file,"232fw.bin");
					
					MY_PRINT("\r\n创建%s\r\n",file);
					ret=0;//ret=sd_create_firmware_file(file);
					if(0==ret){
						send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//发送应答
						times=0;
					}
					else{
						send485_ack_err(devID_salve,RS485rev.RevBuf[2],0x04);/*发送错误反馈*/
					}
					
				}
				else{
					send485_ack_err(devID_salve,RS485rev.RevBuf[2],0x01);/*发送错误反馈*/
				}
			}
			if(flagFirmwareData)
			{
				flagFirmwareData=0;
				//校验数据包
				if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){//校验成功
					times++;
					//把固件写入SD卡中
					CRC32 = calc_crc32(CRC32, &RS485rev.RevBuf[8], *(uint16_t *)&RS485rev.RevBuf[6]);//校验固件
					MY_DEBUG_TX( &RS485rev.RevBuf[8],*(uint16_t *)&RS485rev.RevBuf[6]);
					count+=*(uint16_t *)&RS485rev.RevBuf[6];
					ret=sd_write_firmware_file(file,&RS485rev.RevBuf[8],*(uint16_t *)&RS485rev.RevBuf[6]);
					if(0==ret){
						send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//发送应答
					}
					else{
						send485_ack_err(devID_salve,RS485rev.RevBuf[2],0x04);/*发送错误反馈*/
					}
				}
				else{
					send485_ack_err(devID_master,RS485rev.RevBuf[2],0x01);/*发送错误反馈*/
				}				
			}
			
			if(flagFirmwareEnd)
			{
				flagFirmwareEnd=0;
				//校验数据包
				if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){//校验成功
					if(times==*(uint16_t *)&RS485rev.RevBuf[8] && frimware_size){
						send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//发送应答
						MY_PRINT("\r\nget size;%d\r\n",count);
						MY_PRINT("\r\nCRC32 value:%X\r\n",CRC32);
						//记录版本到SD卡
						if(frimware_type==0x01){//485固件
							MY_PRINT("\r\n485固件版本：%d\r\n",*(int *)&RS485rev.RevBuf[10]);
							if(0==sd_mark_firmware_version(*(int *)&RS485rev.RevBuf[10],FW232ID,true))
							{
								FW485ID=*(int *)&RS485rev.RevBuf[10];
							}
						}
						else if(frimware_type==0x02)//232固件
						{
							MY_PRINT("\r\n232固件版本：%d\r\n",*(int *)&RS485rev.RevBuf[10]);
							if(0==sd_mark_firmware_version(FW485ID,*(int *)&RS485rev.RevBuf[10],true))
							{
								FW232ID=*(int *)&RS485rev.RevBuf[10];
							}
						}
					}
					else{
						send485_ack_err(devID_master,RS485rev.RevBuf[2],0x04);/*发送错误反馈*/

					}
				}
				else{
					send485_ack_err(devID_master,RS485rev.RevBuf[2],0x01);/*发送错误反馈*/
	
				}					
			}		
		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL	
}


/*任务六：把新固件传输给232设备任务：
      阻塞等待232固件传输事件
			通过232协议把升级固件发送到232设备（传输协议甲方提供）
*/
void firmware_transmit_to_232dev_Task(void *pvParameters)
{
	MY_PRINT("firmware_transmit_to_232dev_Task coming!!\r\n");
	int cnt=0;
	char ret=0;
	int firmware_size=0;//固件大小

	while(1)
	{
		if(flag232UpDate)
		{
			flag232UpDate=0;
			MY_PRINT("\r\n232板子请求更新固件\r\n");
			//挂起其他任务
		 if(flagNetWorkDev==DEV_WIFI){
			vTaskSuspend(network_status_check_wifi_TaskHandle);			 
		 }
		 else if(flagNetWorkDev==DEV_4G){
			vTaskSuspend(network_status_check_4g_TaskHandle);
			vTaskSuspend(network_status_check_wifi_TaskHandle);					 
		 }
		 vTaskSuspend(master_rev_485dev_data_handle_TaskHandle);	
		 vTaskSuspend(master_rev_232dev_data_handle_TaskHandle);			
			
			char file[]={"232fw.bin"};
			//查找SD卡中是否有"232fw.bin文件"
			if(0==sd_read_firmware_size(file,&firmware_size,true))
			{
				uint8_t rev_buf[1030]={0};
				int start_addr=0;
				UINT br=0;
				while(1)
				{
					if(0==sd_read_firmware(file,rev_buf,start_addr,1024,&br,true))
					{
						start_addr+=br;
						if(br<1024 || start_addr==firmware_size)//最后一笔
						{
							//固件传输给232板子，发送最后一笔
							//MY_PRINT("\r\n固件传输给232板子，发送最后一笔\r\n");
							if(0==send_rs2332_updata_firmware_data_end(rev_buf,br))
							{
								//固件传输给232板子，发送固件大小
								if(firmware_size==start_addr)
								{
									if(0==send_rs2332_updata_firmware_size(firmware_size))
									{
										//等待连续3次争取响应
										if(0==rs232_wait_updata_ok( )) 
										{
											MY_PRINT("\r\n232设备更新完成\r\n");
											//sd_mark_dev_firmware_version(int BanBenID,int XBanBenID,true);//不用记录也行
										}
									}
									else
									{
										MY_PRINT("\r\n传输失败 3\r\n");
										break;
									}										
								}
								else{
									MY_PRINT("\r\n固件大小不正确\r\n");
								}
								break;
							}	
							else
							{
								MY_PRINT("\r\n传输失败 2\r\n");
								break;//传输失败
							}
						}			
						//固件传输给232板子，发送中
						//MY_PRINT("\r\n固件传输给232板子，发送中\r\n");
						if(0!=send_rs2332_updata_firmware_data_ing(rev_buf,br))	
						{
							MY_PRINT("\r\n传输失败 1\r\n");
							break;//传输失败
						}							
						memset(rev_buf,0,sizeof(rev_buf));
					}
					else
					{
						MY_PRINT("\r\n文件读取失败\r\n");
						break;//文件读取失败
					}		
				}
			}
			else
			{
				MY_PRINT("\r\n没有232固件\r\n");
			}
			//唤醒其他任务
			if(flagNetWorkDev==DEV_WIFI){
				vTaskResume(network_status_check_wifi_TaskHandle);//add 唤醒WIFI网络任务	
			 }
			 else if(flagNetWorkDev==DEV_4G){
					vTaskResume(network_status_check_4g_TaskHandle);//add 唤醒4G网络任务
					vTaskResume(network_status_check_wifi_TaskHandle);//add 唤醒WIFI网络任务			 
			 }
			vTaskResume(master_rev_485dev_data_handle_TaskHandle);	
			vTaskResume(master_rev_232dev_data_handle_TaskHandle);			
			flag232UpDate=0;
		}

	
		vTaskDelay(10);//不能空循环
	}
	vTaskDelete(NULL);//删除任务，这里由于是自尽所以用NULL	
}




