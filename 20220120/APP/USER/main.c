#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f4xx.h"
#include "led.h"
#include "debug_uart.h"
#include "rtc.h"
#include "sd_operation.h"
#include "wifi.h"
#include "4G.h"
#include "rs485.h"
#include "rs232.h"
#include "scanner.h"
#include "makedata.h"
#include "FreeRTOS.h"//���ͷ�ļ�������task.h֮ǰ���������������򱨴�
#include "task.h"
#include "my_tasks.h"
#include "my_print.h"
#include "io.h"




#define FIRMWARE_VERSION 1

#define BOOTLOAD_ADDR 0x08000000  //����������01
#define APP_ADDR      0x08008000  //����������23456


/*ȫ�ֱ�־������״̬************************************************************
            ����������״̬
            SD������
						�豸���ͣ�������/�����磩
				
*/
uint8_t flagNetWorkDev=DEV_NULL;//�����豸 1-WIFI  2-4G
uint8_t flagNetWork=NETWORK_OFF;//ʹ��WIFI,����·��Ϊ1///ʹ��4G,������Ϊ1
uint8_t flagServerConn=DISCONNECT;
uint8_t flagSDReady=NOT_READY;//SD��������־
uint8_t flagDevType=MASTER;//0-485�ӻ�   1-485����
uint8_t falgDownLoadFW=0;
uint8_t flagReport_ing=0;//�����ϱ����ݱ�־
uint8_t flagIsSaomaSingle=0;////0û��ɨ��ǹ  1ֻ��ɨ��ǹ  2ɨ��ǹ+232


int FW485ID=-1;//����SD���е�485�̼��İ汾��-1��ʾû�д洢���ù̼�
int FW232ID=-1;//����SD���е�232�̼��İ汾��-1��ʾû�д洢���ù̼�
//����������Ϣ
char   WIFISSID[30]={0};     
char   WIFIPWD[30]={0};     
char   SERVER_IP[30]={0};     //   "8.134.112.231"    
int   SERVER_PORT=0;          //  8099 
char   URL[40]={0};    //   "/ShuJu/QingQiu"


FATFS       this_fs;//�ļ�ϵͳ
//******************************************************************************
//�������
TaskHandle_t report_TaskHandle;//������
TaskHandle_t polling_TaskHandle;//������
TaskHandle_t network_status_check_wifi_TaskHandle;//������
TaskHandle_t network_status_check_4g_TaskHandle;//������
TaskHandle_t network_status_check_TaskHandle;//������
TaskHandle_t network_status_change_handle_TaskHandle;//������

TaskHandle_t offline_data_process_TaskHandle;//������

TaskHandle_t server_data_process_TaskHandle;//������

TaskHandle_t firmware_transmit_to_salve_TaskHandle;//������
TaskHandle_t firmware_receive_from_master_TaskHandle;//������
TaskHandle_t firmware_transmit_to_232dev_TaskHandle;//������

TaskHandle_t master_rev_485dev_data_handle_TaskHandle;//������
TaskHandle_t master_rev_232dev_data_handle_TaskHandle;//������
TaskHandle_t salver_rev_232dev_data_handle_TaskHandle;//������

EventGroupHandle_t report_event_group;//�¼���־��
EventGroupHandle_t network_event_group;//�¼���־��
EventGroupHandle_t offline_data_event_group;//�¼���־��
EventGroupHandle_t firmware_event_group;//�¼���־��

SemaphoreHandle_t Semaphore_wifi = NULL;//WIFIר�õĻ����ź���
SemaphoreHandle_t Semaphore_4g = NULL;//4Gר�õĻ����ź���
SemaphoreHandle_t Semaphore_SDCARD = NULL;//sd��ר�õĻ����ź���
//******************************************************************************

static char read_485dev_addr(void)
{
	//1-PD4 2-PD3 3-PD1 4-PD0 5-PA15
	GPIO_InitTypeDef GPIO_InitStruct;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_1|GPIO_Pin_0;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_Init(GPIOD, &GPIO_InitStruct);	
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	char addr=0;
	if( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_4) )
		addr|=1<<0;
	if( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_3) )
		addr|=1<<1;	
	if( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1) )
		addr|=1<<2;	
	if( GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_0) )
		addr|=1<<3;	
	if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_15) )
		addr|=1<<4;		
	return addr;
}

static void delay_microsecond(uint32_t microsecond)
{
	microsecond *= 42;
	
	for(uint32_t i = 0; i < microsecond; i++)
	{
		;
	}
}


int main(void)
{
	SCB->VTOR = APP_ADDR;

	LED_Init( );
	air724_4g_uartConfig(115200);
	rs232_uart_config(115200);//debug_uartConfig(115200);
	rs485_uart_config(115200);
	wifi_uartConfig(115200);
	RTC__Init( );
	IO_tongdao_init( );

//	while(1)
//	{
//		if(wifi_rev.RevOver)
//		{
//			wifi_rev.RevOver=0;
//			MY_DEBUG_TX(wifi_rev.RevBuf,wifi_rev.BufLen);
//			printf("\r\n#\r\n");
//		}
//		
//		if(RS232rev.RevOver)
//		{
//			RS232rev.RevOver=0;
//			wifi_tx_bytes(RS232rev.RevBuf,RS232rev.BufLen);			
//		}
//	}
	int SaoMaBaud=115200;
	//SD����ʼ��
	if(SD_OK==SD_Init( ))
	{
			
			FRESULT fresult;
			
			fresult= f_mount (&this_fs,"0:",1);//��SD�����ص��ļ�ϵͳ��
			if(FR_OK!=fresult)
			{
				flagSDReady=NOT_READY;
				MY_PRINT("f_mout error ,check sd card\r\n");
			}
			else 
			{
				flagSDReady=READY;
				//����·��
				sd_read_path( );
				
				
				//��ȡ������Ϣ SBBH  CJID
				if(0==sd_read_defaultconfig((char *)&flagIsSaomaSingle,http_key.SBBH,&http_key.CJID,&SaoMaBaud,http_key.TDJZh,http_key.ZaXiang))
				{
					MY_PRINT("*********flagIsSaomaSingle:%d\r\n",flagIsSaomaSingle);//0û��ɨ��ǹ  1ֻ��ɨ��ǹ  2ɨ��ǹ+232
					MY_PRINT("*********SBBH:%s\r\n",http_key.SBBH);
					MY_PRINT("*********CJID:%d\r\n",http_key.CJID);
					http_key.TDJZh[sizeof(http_key.TDJZh)-1]='\0';
					MY_PRINT("*********TDJZh:%s\r\n",http_key.TDJZh);
					http_key.ZaXiang[sizeof(http_key.ZaXiang)-1]='\0';
					MY_PRINT("*********ZaXiang:%s\r\n",http_key.ZaXiang);
				}
				//��ȡ��ǰ�豸����汾��
				if(0!=sd_read_dev_firmware_version(&http_key.BanBenID,&http_key.XBanBenID))
				{
					//��ֹSD������
					http_key.BanBenID=1;//��ǰ��λ��������汾��
					http_key.XBanBenID=0;//��ǰ���ӵ���λ��������汾��	
				}
				//��¼������SD���е����¹̼��汾��,-1Ϊû�иù̼�
				//sd_mark_firmware_version(-1,-1,false);//+++����
				
				//��ȡ��ǰSD���б���Ĺ̼��İ汾
				sd_read_firmware_version(&FW485ID,&FW232ID);
			}
	}
	else
	{
		MY_PRINT("sd_init err\r\n");
	}
	
	scanner_232UARTConfig(SaoMaBaud);
	
	
	/*����豸��485��������485�ӻ�*/
	char ADDR=read_485dev_addr( );
	MY_PRINT("�豸��ַ��0X%X",ADDR);
	if(ADDR==0x00)
	{
		flagDevType=MASTER;
		devID_master=ADDR;//��������ID,�̶�0x00
	}
	else if(ADDR>=0x01 && ADDR<=0x1E){
		flagDevType=SALVER;
		devID_salve=ADDR;//�ӻ�����ID 0x02~0x1E
		//FW232ID=1;//����
	}
	else
	{
		MY_PRINT("�벦������ѡ����ȷ���豸��ַ\r\n");
		while(1)
		{
		LED1_ON;LED2_ON;LED3_ON;LED4_ON;
		delay_microsecond(1000*200);
		LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;
		delay_microsecond(1000*200);
		}
	}
	if(flagDevType==MASTER)
	{
		//��ȡ����������Ϣ
		sd_read_network_config(WIFISSID,WIFIPWD,SERVER_IP,&SERVER_PORT,URL);
		MY_PRINT("WIFISSID:%s\r\n",WIFISSID);
		MY_PRINT("WIFIPWD:%s\r\n",WIFIPWD);
		MY_PRINT("SERVER_IP:%s\r\n",SERVER_IP);
		MY_PRINT("SERVER_PORT:%d\r\n",SERVER_PORT);
		MY_PRINT("URL:%s\r\n",URL);
		//����Ӳ����ʼ��	
		//wifi_init( );
		//network_4g( );
				

	}

	/*���񴴽�*/
	Semaphore_SDCARD=xSemaphoreCreateMutex();
	xTaskCreate(polling_Task, "polling_Task", POLLING_TASK_STACK_SIZE, NULL, POLLING_TASK_PRO, &polling_TaskHandle);	
	vTaskStartScheduler() ;//����������ȣ�֮�����ڴ��еĴ���/�����ǿ�ʼ���У��������ȼ���С
	
	while(1)
	{
		
	}
}


/*��������485�豸��ѯ����
*/
void polling_Task(void *pvParameters)
{
	MY_PRINT("polling_Task coming!!\r\n");
	
	if(MASTER==flagDevType){
		Semaphore_wifi = xSemaphoreCreateMutex();
		Semaphore_4g = xSemaphoreCreateMutex();
		report_event_group = xEventGroupCreate();//��̬����һ���¼���־��
		network_event_group = xEventGroupCreate();//��̬����һ���¼���־��
		offline_data_event_group = xEventGroupCreate();//��̬����һ���¼���־��
		firmware_event_group = xEventGroupCreate();//��̬����һ���¼���־��
		
		
		xTaskCreate(report_Task, "report_Task", 
			REPORT_TASK_STACK_SIZE, NULL, REPORT_TASK_PRO, &report_TaskHandle);
		
		xTaskCreate(network_status_check_Task, "network_status_check_Task", 
			NETWORK_STATUS_CHECK_TASK_STACK_SIZE, NULL, NETWORK_STATUS_CHECK_TASK_PRO, &network_status_check_TaskHandle);

		xTaskCreate(network_status_change_handle_Task, "network_status_change_handle_Task",
			NETWORK_STATUS_CHANGE_HANDLE_TASK_STACK_SIZE, NULL, NETWORK_STATUS_CHANGE_HANDLE_TASK_PRO, &network_status_change_handle_TaskHandle);

		xTaskCreate(offline_data_process_Task, "offline_data_process_Task",
			OFFLINE_DATA_PROCESS_TASK_STACK_SIZE, NULL, OFFLINE_DATA_PROCESS_TASK_PRO, &offline_data_process_TaskHandle);


		xTaskCreate(network_status_check_4g_Task, "network_status_check_4g_Task", 
			NETWORK_STATUS_CHECK_4G_TASK_STACK_SIZE, NULL, NETWORK_STATUS_CHECK_4G_TASK_PRO, &network_status_check_4g_TaskHandle);

		xTaskCreate(master_rev_485dev_data_handle_Task, "master_rev_485dev_data_handle_Task", MASTER_REV_485DEV_DATA_HANDLE_STACK_SIZE, NULL, 
			MASTER_REV_485DEV_DATA_HANDLE_TASK_PRO, &master_rev_485dev_data_handle_TaskHandle);	

		xTaskCreate(master_rev_232dev_data_handle_Task, "master_rev_232dev_data_handle_Task", MASTER_REV_232DEV_DATA_HANDLE_STACK_SIZE, NULL, 
			MASTER_REV_232DEV_DATA_HANDLE_TASK_PRO, &master_rev_232dev_data_handle_TaskHandle);
	
					
		xTaskCreate(network_status_check_wifi_Task, "network_status_check_wifi_Task", 
			NETWORK_STATUS_CHECK_WIFI_TASK_STACK_SIZE, NULL, NETWORK_STATUS_CHECK_WIFI_TASK_PRO, &network_status_check_wifi_TaskHandle);
			
		xTaskCreate(server_data_process_Task, "server_data_process_Task", 
			SERVER_DATA_PROCESS_TASK_STACK_SIZE, NULL, SERVER_DATA_PROCESS_TASK_PRO, &server_data_process_TaskHandle);	
		
		xTaskCreate(firmware_transmit_to_salve_Task, "firmware_transmit_to_salve_Task", 
			FIRMWARE_TRANSMIT_TO_485SALVE_TASK_STACK_SIZE, NULL, FIRMWARE_TRANSMIT_TO_485SALVE_TASK_PRO, &firmware_transmit_to_salve_TaskHandle);	
		
		xTaskCreate(firmware_transmit_to_232dev_Task, "firmware_transmit_to_232dev_Task", 
			FIRMWARE_TRANSMIT_TO_232DEV_TASK_STACK_SIZE, NULL, FIRMWARE_TRANSMIT_TO_232DEV_TASK_PRO, &firmware_transmit_to_232dev_TaskHandle);	
	}
	else if(SALVER==flagDevType)
	{
		xTaskCreate(firmware_receive_from_master_Task, "firmware_receive_from_master_Task", 
			FIRMWARE_RECEIVE_FROM_485MASTER_TASK_STACK_SIZE, NULL, FIRMWARE_RECEIVE_FROM_485MASTER_TASK_PRO, &firmware_receive_from_master_TaskHandle);	
		
		xTaskCreate(firmware_transmit_to_232dev_Task, "firmware_transmit_to_232dev_Task", 
			FIRMWARE_TRANSMIT_TO_232DEV_TASK_STACK_SIZE, NULL, FIRMWARE_TRANSMIT_TO_232DEV_TASK_PRO, &firmware_transmit_to_232dev_TaskHandle);	
		
		xTaskCreate(salver_rev_232dev_data_handle_Task, "salver_rev_232dev_data_handle_Task", SALVER_REV_232DEV_DATA_HANDLE_STACK_SIZE, NULL, 
			SALVER_REV_232DEV_DATA_HANDLE_TASK_PRO, &salver_rev_232dev_data_handle_TaskHandle);		
	}	
	
//+++����	
//	char cnt=0;
//	while(1)
//	{
//		LED3_ON;
//		vTaskDelay(500);
//		LED3_OFF;
//		vTaskDelay(500);		
//		if(++cnt>=10)
//		{
//			cnt=0;
//			flag232UpDate=1;
//			break;
//		}
//	}
	while(1)
	{
		if(falgDownLoadFW)
		{
			LED3_OFF;
			vTaskDelay(200);
			LED3_ON;
			vTaskDelay(200);			
		}
		else
		{
			LED2_ON;
			vTaskDelay(500);
			LED2_OFF;
			vTaskDelay(500);			
		}

	}
	vTaskDelete(NULL);//ɾ�����������������Ծ�������NULL
}






