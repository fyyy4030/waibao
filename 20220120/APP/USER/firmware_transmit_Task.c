#include "my_tasks.h"
#include "stdio.h"
#include "rs485.h"
#include "rs232.h"
#include "sd_operation.h"
#include "makedata.h"
#include "debug_uart.h"
/*�����壺���������̼��������񣺣�������������
				�����ȴ��¼���־���¹̼��¼���
				�ٵõ����¹̼���������232�̼�����485�̼�����ͨ��485Э��������̼����͵��������豸�Ĵӻ�	
				������¹̼�������485�̼�  ��SD����¼���¹̼��ı�־�ļ�	                          
				 ����¹̼�������232�̼�  ����232�̼������¼�������firmware_transmit_to_232dev_Task��
*/
void firmware_transmit_to_salve_Task(void *pvParameters)
{
	MY_PRINT("firmware_transmit_to_salve_Task coming!!\r\n");
	uint8_t frimware_type=0;//�̼�����
	while(1)
	{
		if(flagSlaveUpdateFW )///�ӻ�������������̼���־
		{
			flagSlaveUpdateFW=0;
			
			uint8_t frimware_type=RS485rev.RevBuf[8];//�̼�����
			if(frimware_type==0x01)
			{
				MY_PRINT("�ӻ���Ҫ����485�̼�\r\n");
			}
			else if(frimware_type==0x02)
			{
				MY_PRINT("�ӻ���Ҫ����232�̼�\r\n");
			}
			//������������
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
				{ //ѭ���������͹̼�
					uint8_t rev_buf[510]={0};
					int start_addr=0;
					UINT br=0;
					uint16_t trans_cnt=0;//�������
					while(1)
					{
						if(0==sd_read_firmware(file,rev_buf,start_addr,500,&br,true))
						{
							if(0==send485_firmware_data(devID_master,RS485rev.RevBuf[2],rev_buf,br))
							{
								trans_cnt++;
								start_addr+=br;
								if(br<500 || start_addr==firmware_size)//�������
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
					if(start_addr==firmware_size)//������ȫ
					{
						MY_PRINT("�ӻ����¹̼����\r\n");
						if(frimware_type==0x01)
							send485_firmware_update_end(devID_master,RS485rev.RevBuf[2],trans_cnt,FW485ID);
						else if(frimware_type==0x02)
							send485_firmware_update_end(devID_master,RS485rev.RevBuf[2],trans_cnt,FW232ID);
					
					}
					
				}
			}
			
			
			//������������
			if(flagNetWorkDev==DEV_WIFI){
				vTaskResume(network_status_check_wifi_TaskHandle);//add ����WIFI��������	
			 }
			 else if(flagNetWorkDev==DEV_4G){
					vTaskResume(network_status_check_4g_TaskHandle);//add ����4G��������
					vTaskResume(network_status_check_wifi_TaskHandle);//add ����WIFI��������			 
			 }
			vTaskResume(master_rev_485dev_data_handle_TaskHandle);	
			vTaskResume(master_rev_232dev_data_handle_TaskHandle);	
			//falgDownLoadFW=0; 
			 
			flagSlaveUpdateFW=0;
		}
		
		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL
	
}


#include "crc16.h"

/*�����壺�ӻ������̼��������񣺣������ڴӻ���
				�ȴ�������֮���й̼�����
				�õ����¹̼���ͨ��232Э��������̼����͵�232�豸������Э��׷��ṩ��
				����¹̼�������485�̼�����SD����¼���¹̼��ı�־�ļ�
				����¹̼�������232�̼�������232�̼������¼�������firmware_transmit_to_232dev_Task��
*/
void firmware_receive_from_master_Task(void *pvParameters)
{
	MY_PRINT("firmware_receive_from_master_Task coming!!\r\n");
	uint8_t frimware_type=0;//�̼�����
	uint16_t frimware_size=0;//�̼���С
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
				//У�����ݰ�
				if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){//У��ɹ�
					frimware_type=RS485rev.RevBuf[8];//�̼�����
					frimware_size=*(uint16_t *)&RS485rev.RevBuf[9];//�̼���С
					if(frimware_type==0x01)
						strcpy(file,"485fw.bin");
					else if(frimware_type==0x02)
						strcpy(file,"232fw.bin");
					
					MY_PRINT("\r\n����%s\r\n",file);
					ret=0;//ret=sd_create_firmware_file(file);
					if(0==ret){
						send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//����Ӧ��
						times=0;
					}
					else{
						send485_ack_err(devID_salve,RS485rev.RevBuf[2],0x04);/*���ʹ�����*/
					}
					
				}
				else{
					send485_ack_err(devID_salve,RS485rev.RevBuf[2],0x01);/*���ʹ�����*/
				}
			}
			if(flagFirmwareData)
			{
				flagFirmwareData=0;
				//У�����ݰ�
				if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){//У��ɹ�
					times++;
					//�ѹ̼�д��SD����
					CRC32 = calc_crc32(CRC32, &RS485rev.RevBuf[8], *(uint16_t *)&RS485rev.RevBuf[6]);//У��̼�
					MY_DEBUG_TX( &RS485rev.RevBuf[8],*(uint16_t *)&RS485rev.RevBuf[6]);
					count+=*(uint16_t *)&RS485rev.RevBuf[6];
					ret=sd_write_firmware_file(file,&RS485rev.RevBuf[8],*(uint16_t *)&RS485rev.RevBuf[6]);
					if(0==ret){
						send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//����Ӧ��
					}
					else{
						send485_ack_err(devID_salve,RS485rev.RevBuf[2],0x04);/*���ʹ�����*/
					}
				}
				else{
					send485_ack_err(devID_master,RS485rev.RevBuf[2],0x01);/*���ʹ�����*/
				}				
			}
			
			if(flagFirmwareEnd)
			{
				flagFirmwareEnd=0;
				//У�����ݰ�
				if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){//У��ɹ�
					if(times==*(uint16_t *)&RS485rev.RevBuf[8] && frimware_size){
						send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//����Ӧ��
						MY_PRINT("\r\nget size;%d\r\n",count);
						MY_PRINT("\r\nCRC32 value:%X\r\n",CRC32);
						//��¼�汾��SD��
						if(frimware_type==0x01){//485�̼�
							MY_PRINT("\r\n485�̼��汾��%d\r\n",*(int *)&RS485rev.RevBuf[10]);
							if(0==sd_mark_firmware_version(*(int *)&RS485rev.RevBuf[10],FW232ID,true))
							{
								FW485ID=*(int *)&RS485rev.RevBuf[10];
							}
						}
						else if(frimware_type==0x02)//232�̼�
						{
							MY_PRINT("\r\n232�̼��汾��%d\r\n",*(int *)&RS485rev.RevBuf[10]);
							if(0==sd_mark_firmware_version(FW485ID,*(int *)&RS485rev.RevBuf[10],true))
							{
								FW232ID=*(int *)&RS485rev.RevBuf[10];
							}
						}
					}
					else{
						send485_ack_err(devID_master,RS485rev.RevBuf[2],0x04);/*���ʹ�����*/

					}
				}
				else{
					send485_ack_err(devID_master,RS485rev.RevBuf[2],0x01);/*���ʹ�����*/
	
				}					
			}		
		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL	
}


/*�����������¹̼������232�豸����
      �����ȴ�232�̼������¼�
			ͨ��232Э��������̼����͵�232�豸������Э��׷��ṩ��
*/
void firmware_transmit_to_232dev_Task(void *pvParameters)
{
	MY_PRINT("firmware_transmit_to_232dev_Task coming!!\r\n");
	int cnt=0;
	char ret=0;
	int firmware_size=0;//�̼���С

	while(1)
	{
		if(flag232UpDate)
		{
			flag232UpDate=0;
			MY_PRINT("\r\n232����������¹̼�\r\n");
			//������������
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
			//����SD�����Ƿ���"232fw.bin�ļ�"
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
						if(br<1024 || start_addr==firmware_size)//���һ��
						{
							//�̼������232���ӣ��������һ��
							//MY_PRINT("\r\n�̼������232���ӣ��������һ��\r\n");
							if(0==send_rs2332_updata_firmware_data_end(rev_buf,br))
							{
								//�̼������232���ӣ����͹̼���С
								if(firmware_size==start_addr)
								{
									if(0==send_rs2332_updata_firmware_size(firmware_size))
									{
										//�ȴ�����3����ȡ��Ӧ
										if(0==rs232_wait_updata_ok( )) 
										{
											MY_PRINT("\r\n232�豸�������\r\n");
											//sd_mark_dev_firmware_version(int BanBenID,int XBanBenID,true);//���ü�¼Ҳ��
										}
									}
									else
									{
										MY_PRINT("\r\n����ʧ�� 3\r\n");
										break;
									}										
								}
								else{
									MY_PRINT("\r\n�̼���С����ȷ\r\n");
								}
								break;
							}	
							else
							{
								MY_PRINT("\r\n����ʧ�� 2\r\n");
								break;//����ʧ��
							}
						}			
						//�̼������232���ӣ�������
						//MY_PRINT("\r\n�̼������232���ӣ�������\r\n");
						if(0!=send_rs2332_updata_firmware_data_ing(rev_buf,br))	
						{
							MY_PRINT("\r\n����ʧ�� 1\r\n");
							break;//����ʧ��
						}							
						memset(rev_buf,0,sizeof(rev_buf));
					}
					else
					{
						MY_PRINT("\r\n�ļ���ȡʧ��\r\n");
						break;//�ļ���ȡʧ��
					}		
				}
			}
			else
			{
				MY_PRINT("\r\nû��232�̼�\r\n");
			}
			//������������
			if(flagNetWorkDev==DEV_WIFI){
				vTaskResume(network_status_check_wifi_TaskHandle);//add ����WIFI��������	
			 }
			 else if(flagNetWorkDev==DEV_4G){
					vTaskResume(network_status_check_4g_TaskHandle);//add ����4G��������
					vTaskResume(network_status_check_wifi_TaskHandle);//add ����WIFI��������			 
			 }
			vTaskResume(master_rev_485dev_data_handle_TaskHandle);	
			vTaskResume(master_rev_232dev_data_handle_TaskHandle);			
			flag232UpDate=0;
		}

	
		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL	
}




