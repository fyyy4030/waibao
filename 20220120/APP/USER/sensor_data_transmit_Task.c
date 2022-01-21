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

/*��������485�豸���ݲɼ��ϱ�����(�����Ǵ����������)
          ��������485�ӻ����͹���������
			    ������յ�485�豸�ӻ��ϴ��Ĵ���������
					    �豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
					    �豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢	
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
		for(index=0x01;index<=0x1F;index++)//�ӻ�ID��Χ0x01~0x1F
		{
			if(0==send485_master_inquiry(index,FW485ID,FW232ID))//ѯ�ʳɹ� 
			{
				MY_PRINT("485�ӻ� %d������\r\n",index);
				if(flagSensorDataCome)//�������485�豸�ӻ��ϴ��Ĵ���������
				{
					MY_PRINT("485�ӻ� %d�������ݷ��͹���\r\n",index);
					flagSensorDataCome=0;//���־
					//У�����ݰ�
					if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) )//У��ɹ�
					{
						send485_ack(devID_master,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);//����Ӧ��
						
						falgPublicData=1;//���������ϱ�
					}
					else
					{
						send485_ack_err(devID_master,RS485rev.RevBuf[2],0x01);/*���ʹ�����*/
					}
				}		

				if(falgPublicData&&falgDownLoadFW)//�������ع̼��������ݴ���SD��
				{
					falgPublicData=0;
					xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
//					if(0==offline_data_record(&RS485rev.RevBuf[8],*(uint16_t*)&RS485rev.RevBuf[6]))
//						MY_PRINT("\r\n\r\n�ɹ�д��SD��\r\n\r\n\r\n");
//					else
//						MY_PRINT("\r\n\r\nд��ʧ��\r\n\r\n\r\n");									
				}	
				
				if(1==falgPublicData)//�ϱ�����
				{
					falgPublicData=0;
					if(flagServerConn)//�豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
					{
						uint16_t yuanshi_data_len=*(uint16_t*)&RS485rev.RevBuf[6]-TDSaoMa_LEN_MAX;//ԭʼ���ݳ��ȣ���ȥɨ��ǹ����
						char saoma[TDSaoMa_LEN_MAX]={0};
						memcpy(saoma,&RS485rev.RevBuf[8+yuanshi_data_len],TDSaoMa_LEN_MAX);
						LED1_ON;
						if(0==make_data_and_report(&RS485rev.RevBuf[8],yuanshi_data_len,saoma))
						{
							LED1_OFF;delay_microsecond(150*1000);	
							LED1_ON;delay_microsecond(150*1000);
							LED1_OFF;delay_microsecond(150*1000);								
						}
						else//ʧ�ܣ���¼��SD��
						{
							LED1_OFF;
							xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
//							if(0==offline_data_record(&RS485rev.RevBuf[8],*(uint16_t*)&RS485rev.RevBuf[6]))
//								MY_PRINT("\r\n\r\n�ɹ�д��SD��\r\n\r\n\r\n");
//							else
//								MY_PRINT("\r\n\r\nд��ʧ��\r\n\r\n\r\n");					
						}
					}
					else//�豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢		
					{
						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_485);
					}

				}				
				
			}		
			vTaskDelay(500);//��ѯ���500ms	
		}
		vTaskDelay(10);//���ܿ�ѭ��			
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL		
}

/*�����ߣ�232�豸���ݲɼ��ϱ�����(�����Ǵ����������)
          ��������232�ӻ����͹���������
 					���232�豸�ϴ��Ĵ��������ݣ�������ɺ�
					    �豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
					    �豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢	
MASTER_REV_232DEV_DATA_HANDLE_TASK_PRO     8
MASTER_REV_232DEV_DATA_HANDLE_STACK_SIZE   256
*/
void master_rev_232dev_data_handle_Task(void *pvParameters)
{
	MY_PRINT("master_rev_232dev_data_handle_Task coming!!\r\n");
	uint8_t flagSaoMa=0;
	uint8_t flag232date=0;//ֻ����flagIsSaomaSingle==2
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
//			memcpy(TDSaoMa,scannerrev.RevBuf,scannerrev.BufLen-2);//��������ֽ���\r\n
			LED4_ON;delay_microsecond(150*1000);
			LED4_OFF;delay_microsecond(150*1000);
			flagSaoMa=1;
		
		}		

		if(flagIsSaomaSingle==0)//0û��ɨ��ǹ,232������ֱ���ϱ�
		{
			if(flag232CeShiJieGuo&&(falgDownLoadFW||flagReport_ing))//�������ع̼����������ϱ�
			{
				flag232CeShiJieGuo=0;
				xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
			}
			if(flag232CeShiJieGuo)/*���ܵ�����232�豸�Ĵ���������*/
			{
				flag232CeShiJieGuo=0;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);
				if(flagServerConn)//�豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
				{
					MY_PRINT("\r\nģʽ��%d\r\n\r\n",flagIsSaomaSingle);
					xEventGroupSetBits(report_event_group, EVENT_REPORT_232);		
//					LED1_ON;
//					if(0==make_data_and_report(RS232rev.RevBuf,RS232rev.BufLen,TDSaoMa))
//					{
//						LED1_OFF;delay_microsecond(150*1000);	
//						LED1_ON;delay_microsecond(150*1000);
//						LED1_OFF;delay_microsecond(150*1000);	
//							
//					}
//					else//ʧ�ܣ���¼��SD��
//					{
//						LED1_OFF;
//						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
//					}
				}
				else//�豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢		
				{
					xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);
				}

			}			
		}
		else if(flagIsSaomaSingle==1)//1ֻ��ɨ��ǹ��ɨ��ǹ������ֱ���ϱ�
		{
			if(flagSaoMa&&falgDownLoadFW==0)//�����ϱ�ɨ��
			{
				flagSaoMa=0;
				MY_PRINT("\r\nģʽ��%d\r\n\r\n",flagIsSaomaSingle);
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
		else if(flagIsSaomaSingle==2)//2ɨ��ǹ+232��ɨ���232���ݶ��������ϱ�
		{
			if(flag232CeShiJieGuo)
			{
				flag232CeShiJieGuo=0;
				LED4_ON;delay_microsecond(150*1000);
				LED4_OFF;delay_microsecond(150*1000);				
				flag232date=1;
			}
			if(flagSaoMa&&flag232date&&(falgDownLoadFW||flagReport_ing))//�������ع̼����������ϱ�
			{
				flagSaoMa=0;
				flag232date=0;
				xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
			}
			if(flagSaoMa&&flag232date)//ɨ���232���ݶ��������ϱ�
			{
				flagSaoMa=0;
				flag232date=0;			
				if(flagServerConn)//�豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
				{
					MY_PRINT("\r\nģʽ��%d\r\n\r\n",flagIsSaomaSingle);
					xEventGroupSetBits(report_event_group, EVENT_REPORT_232);		
//					LED1_ON;
//					if(0==make_data_and_report(RS232rev.RevBuf,RS232rev.BufLen,TDSaoMa))
//					{
//						LED1_OFF;delay_microsecond(150*1000);	
//						LED1_ON;delay_microsecond(150*1000);
//						LED1_OFF;delay_microsecond(150*1000);	
//							
//					}
//					else//ʧ�ܣ���¼��SD��
//					{
//						LED1_OFF;
//						xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);					
//					}
				}
				else//�豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢		
				{
					xEventGroupSetBits(offline_data_event_group, HAVE_OFFLINE_DATA_232);
				}				
				
			}
			
		}
		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL		
}


#include "string.h"

/*��������232�豸���ݲɼ��ϱ�����(�����Ƿ������豸�Ĵӻ�)
          �ӻ�����232�ӻ����͹���������
					���232�豸�ϴ����������ݣ�������ɺ�ͨ��485����������������
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
			//�����ݰ�װ��232����֡��������saoma��,����RS232rev.RevBuf
			//flagsSendReady=1;
			
		}
			
		
		if(flagIsSaomaSingle==0)//0û��ɨ��ǹ,232������ֱ���ϱ�   OK!!!!
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
		else if(flagIsSaomaSingle==1)//1ֻ��ɨ��ǹ��ɨ��ǹ������ֱ���ϱ�
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
				//�����ݰ�װ��232����֡��������saoma����,����RS232rev.RevBuf
				//flagsSendReady=1;
			}
			
		}
		else if(flagIsSaomaSingle==2)//2ɨ��ǹ+232��ɨ���232���ݶ��������ϱ�   OK!!!!
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
			flagInquiry=0;//�յ���������������ѯ��
			MY_PRINT("485����ѯ��\r\n");
			//flag232CeShiJieGuo=1;//���ԣ��ֶ���1��Ч
			if(flagsSendReady)//���232�豸�ϴ����������ݣ�������ɺ󣬣�����Ҫ������ͨ��485����������������
			{
				flagsSendReady=0;
		
				memcpy(&RS232rev.RevBuf[RS232rev.BufLen],TDSaoMa,TDSaoMa_LEN_MAX);
				if(0==send485_sensor_data(devID_salve,devID_master,(uint8_t *)RS232rev.RevBuf,RS232rev.BufLen+TDSaoMa_LEN_MAX))//���ͳɹ����Է��Ѿ�����
				{
					MY_PRINT("���͸�485�����ɹ�\r\n\r\n");
					//LED4_ON;delay_microsecond(150*1000);
					//LED4_OFF;delay_microsecond(150*1000);
					memset(TDSaoMa,0,TDSaoMa_LEN_MAX);
				}
				else//�����Ӧ��Ӧ�𣬴���ʧ�ܣ��򡣡�����
				{
					MY_PRINT("���͸�485����ʧ��\r\n\r\n");
				}				
			}
			else if(*(int *)&RS485rev.RevBuf[8]>FW485ID)//����������Ҫ����485�̼�����
			{
				if(0==send485_frimware_update_notify(devID_salve,devID_master,0x01))
				{
					MY_PRINT("485�̼����������ͳɹ�\\r\n\r\n");
				}
				else
				{
					MY_PRINT("485�̼�����������ʧ��\\r\n\r\n");	
				}
									
			}
			else if(*(int *)&RS485rev.RevBuf[8+4]>FW232ID)//����������Ҫ����232�̼�����
			{
				if(0==send485_frimware_update_notify(devID_salve,devID_master,0x02))
				{MY_PRINT("232�̼����������ͳɹ�\\r\n\r\n");
					}
				else
				{MY_PRINT("232�̼�����������ʧ��\\r\n\r\n");
					}
			}				
			else//û���յ�232�豸��������ظ�Ӧ��
			{
				//�ظ�Ӧ���ʾ����
				send485_ack(devID_salve,RS485rev.RevBuf[2],RS485rev.RevBuf[4],RS485rev.RevBuf[5]);				
			}
			
		}

		vTaskDelay(10);//���ܿ�ѭ��
	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL		
}





















