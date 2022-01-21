#ifndef __MY_TASKS_H__
#define __MY_TASKS_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"//���ͷ�ļ�������task.h֮ǰ���������������򱨴�
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

#include "my_print.h"


//Register Bits{{
#define BIT31   0x80000000
#define BIT30   0x40000000
#define BIT29   0x20000000
#define BIT28   0x10000000
#define BIT27   0x08000000
#define BIT26   0x04000000
#define BIT25   0x02000000
#define BIT24   0x01000000
#define BIT23   0x00800000
#define BIT22   0x00400000
#define BIT21   0x00200000
#define BIT20   0x00100000
#define BIT19   0x00080000
#define BIT18   0x00040000
#define BIT17   0x00020000
#define BIT16   0x00010000
#define BIT15   0x00008000
#define BIT14   0x00004000
#define BIT13   0x00002000
#define BIT12   0x00001000
#define BIT11   0x00000800
#define BIT10   0x00000400
#define BIT9     0x00000200
#define BIT8     0x00000100
#define BIT7     0x00000080
#define BIT6     0x00000040
#define BIT5     0x00000020
#define BIT4     0x00000010
#define BIT3     0x00000008
#define BIT2     0x00000004
#define BIT1     0x00000002
#define BIT0     0x00000001
//}}
#define TRUE  1
#define FLASE 0




/*ȫ�ֱ�־������״̬************************************************************
            ����������״̬
            SD������
						�豸���ͣ�������/�����磩
						485�豸ID
*/
extern uint8_t flagNetWorkDev;//��ǰʹ�õ������豸
#define DEV_NULL 0
#define DEV_WIFI 1
#define DEV_4G 2
extern uint8_t flagNetWork;//ʹ��WIFI,����·��Ϊ1///ʹ��4G,������Ϊ1
#define NETWORK_ON  1 //������
#define NETWORK_OFF 0 //������
extern uint8_t flagServerConn;
#define CONNECTED 1 //����
#define DISCONNECT 0 //�Ͽ�
extern uint8_t flagSDReady;//SD��������־
#define READY 1   //����
#define NOT_READY 0 //δ��⵽��
extern uint8_t flagDevType;
#define MASTER 1 //�����������繦��
#define SALVER 0 //�ӻ��������繦��

extern uint8_t falgDownLoadFW;
extern uint8_t flagReport_ing;//�����ϱ����ݱ�־

extern uint8_t flagIsSaomaSingle;

extern int FW485ID;//����SD���е�485�̼��İ汾��-1��ʾû�д洢���ù̼�
extern int FW232ID;//����SD���е�232�̼��İ汾��-1��ʾû�д洢���ù̼�

//����������Ϣ
extern char   WIFISSID[30];     
extern char   WIFIPWD[30];       
extern char   SERVER_IP[30];      
extern int   SERVER_PORT;   
extern char   URL[40];  

//******************************************************************************
 
 
extern EventGroupHandle_t report_event_group;
#define EVENT_REPORT_232 BIT0
#define EVENT_REPORT_saoma BIT1
#define EVENT_REPORT_io BIT2

extern EventGroupHandle_t network_event_group;
#define EVENT_OF_NETWORK_ON BIT0
#define EVENT_OF_NETWORK_OFF BIT1

extern EventGroupHandle_t offline_data_event_group;
#define HAVE_OFFLINE_DATA_485 BIT0 //��485�ӻ������������������Ҫ����
#define HAVE_OFFLINE_DATA_232 BIT1 //��232�豸��������Ҫ����

extern EventGroupHandle_t firmware_event_group;
#define HAVE_NEW_FIRMWARE_485 BIT0
#define HAVE_NEW_FIRMWARE_232 BIT1
#define GET_NEW_485FIRMWARE_FROM_SERVER BIT2
#define GET_NEW_232FIRMWARE_FROM_SERVER BIT3



//******************************************************************************
/*����11���ϱ���������

*/
#define REPORT_TASK_PRO     13
#define REPORT_TASK_STACK_SIZE   256*8//256+128
void report_Task(void *pvParameters);
extern TaskHandle_t report_TaskHandle;//������


//******************************************************************************
/*����11��485�豸��ѯ����

*/
#define POLLING_TASK_PRO     12
#define POLLING_TASK_STACK_SIZE   128//256+128
void polling_Task(void *pvParameters);
extern TaskHandle_t polling_TaskHandle;//������

//--------------------------------------------------------------------------------------------------------------------


/*����1��WIFI����״̬������񣨽�֧�ִ������������
	
*/
#define NETWORK_STATUS_CHECK_WIFI_TASK_PRO          6
#define NETWORK_STATUS_CHECK_WIFI_TASK_STACK_SIZE   256*2
void network_status_check_wifi_Task(void *pvParameters);
extern TaskHandle_t network_status_check_wifi_TaskHandle;//������

/*����2��4G�豸����״̬������񣨽�֧�ִ������������
	

*/
#define NETWORK_STATUS_CHECK_4G_TASK_PRO          5
#define NETWORK_STATUS_CHECK_4G_TASK_STACK_SIZE   256
void network_status_check_4g_Task(void *pvParameters);
extern TaskHandle_t network_status_check_4g_TaskHandle;//������

//#########################################################################
/*����3���豸����״̬������񣨽�֧�ִ������������
					��ѯa��b
					a.����������������������
			           �����������仯�򴥷���Ӧ�¼���־
								 
					b.�豸������������£������˷�����ʧ����ԭ��������״̬������˶Ͽ�״̬��,����һ�������¼���־	�������½��������		

*/
#define NETWORK_STATUS_CHECK_TASK_PRO     4
#define NETWORK_STATUS_CHECK_TASK_STACK_SIZE   128
void network_status_check_Task(void *pvParameters);
extern TaskHandle_t network_status_check_TaskHandle;//������


/*����4���豸����״̬�仯�������񣨽�֧�ִ������������
			   ���������һ�������¼�
			       ���������ֱ���ɹ�
						 �ɹ������������
									�豸�ȷ���������ע�ᣨ2035��2034�ӿڣ�
									LED2����
									��Ȿ��SD���Ƿ����������ݴ洢���о��ϴ������������ϴ��ɹ���ɾ���������ݡ�
									�����������ݴ�������
							
			    ���������һ�ζ����¼�����������Ͽ���
									LED2Ϩ��
				          ��¼����ʱ��㵽����SD��
									���ѡ��������ݴ�������
*/
#define NETWORK_STATUS_CHANGE_HANDLE_TASK_PRO        3
#define NETWORK_STATUS_CHANGE_HANDLE_TASK_STACK_SIZE   256*5//���ܸ�С
void network_status_change_handle_Task(void *pvParameters);
extern TaskHandle_t network_status_change_handle_TaskHandle;//������

//--------------------------------------------------------------------------------------------------------------------

/*����5���������ݴ������񣺣���֧�ִ������������
							�����ȴ��¼���־���������ݴ���
             //�Ѵ�232�豸/����485���豸�յ���������ݴ洢������SD�������ı���ʽ�洢����ʱ���Ⱥ�˳��洢��
*/
#define OFFLINE_DATA_PROCESS_TASK_PRO         2
#define OFFLINE_DATA_PROCESS_TASK_STACK_SIZE   256*2//���ܸ�С
void offline_data_process_Task(void *pvParameters);
extern TaskHandle_t offline_data_process_TaskHandle;//������

//--------------------------------------------------------------------------------------------------------------------


/*����6���������·������ݴ������񣺣���֧�ִ������������
			a.������ͬ������RTC
			b.�ӷ�������ȡ�����ļ�������(SD��)
*/
#define SERVER_DATA_PROCESS_TASK_PRO     7
#define SERVER_DATA_PROCESS_TASK_STACK_SIZE   256*4//���ܸ�С
void server_data_process_Task(void *pvParameters);
extern TaskHandle_t server_data_process_TaskHandle;//������

//--------------------------------------------------------------------------------------------------------------------
/*����7�����������̼��������񣺣�������������
				�����ȴ��¼���־���¹̼��¼���
				�ٵõ����¹̼���������232�̼�����485�̼�����ͨ��485Э��������̼����͵��������豸�Ĵӻ�	
				������¹̼�������485�̼�  ��SD����¼���¹̼��ı�־�ļ�	                          
				 ����¹̼�������232�̼�  ����232�̼������¼�������firmware_transmit_to_232dev_Task��
*/
#define FIRMWARE_TRANSMIT_TO_485SALVE_TASK_PRO    8  
#define FIRMWARE_TRANSMIT_TO_485SALVE_TASK_STACK_SIZE   256*3
void firmware_transmit_to_salve_Task(void *pvParameters);
extern TaskHandle_t firmware_transmit_to_salve_TaskHandle;//������

/*����S1���ӻ������̼��������񣺣������ڴӻ���
				�ȴ�������֮���й̼�����
				�õ����¹̼���ͨ��232Э��������̼����͵�232�豸������Э��׷��ṩ��
				����¹̼�������485�̼�����SD����¼���¹̼��ı�־�ļ�
				����¹̼�������232�̼�������232�̼������¼�������firmware_transmit_to_232dev_Task��
*/
#define FIRMWARE_RECEIVE_FROM_485MASTER_TASK_PRO     8  
#define FIRMWARE_RECEIVE_FROM_485MASTER_TASK_STACK_SIZE   256*5
void firmware_receive_from_master_Task(void *pvParameters);
extern TaskHandle_t firmware_receive_from_master_TaskHandle;//������

/*����8�����¹̼������232�豸����
      �����ȴ�232�̼������¼�
			ͨ��232Э��������̼����͵�232�豸������Э��׷��ṩ��
*/
#define FIRMWARE_TRANSMIT_TO_232DEV_TASK_PRO     9   
#define FIRMWARE_TRANSMIT_TO_232DEV_TASK_STACK_SIZE   256*4
void firmware_transmit_to_232dev_Task(void *pvParameters);
extern TaskHandle_t firmware_transmit_to_232dev_TaskHandle;//������


//--------------------------------------------------------------------------------------------------------------------

/*����9��485�豸���ݲɼ��ϱ�����(�����Ǵ����������)
          ��������485�ӻ����͹���������
			    ������յ�485�豸�ӻ��ϴ��Ĵ���������
					    �豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
					    �豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢					
*/
#define MASTER_REV_485DEV_DATA_HANDLE_TASK_PRO     10   
#define MASTER_REV_485DEV_DATA_HANDLE_STACK_SIZE   256*4+128//���ܸ�С
void master_rev_485dev_data_handle_Task(void *pvParameters);
extern TaskHandle_t master_rev_485dev_data_handle_TaskHandle;//������
/*����10��232�豸���ݲɼ��ϱ�����(�����Ǵ����������)
          ��������232�ӻ����͹���������
 					���232�豸�ϴ��Ĵ��������ݣ�������ɺ�
					    �豸�����������ͨ״̬��ֱ�Ӱ������ϴ���������������Э��׷��ṩ��
					    �豸����������Ͽ�״̬�������¼���־���������ݴ����������ݽ������������ݴ������񡱽��б������ߴ洢																							 
*/
#define MASTER_REV_232DEV_DATA_HANDLE_TASK_PRO     11   
#define MASTER_REV_232DEV_DATA_HANDLE_STACK_SIZE   256*8
void master_rev_232dev_data_handle_Task(void *pvParameters);
extern TaskHandle_t master_rev_232dev_data_handle_TaskHandle;//������

/*����S2�����ݲɼ��ϱ�����(�����Ƿ������豸�Ĵӻ�)
          �ӻ�����232�ӻ����͹���������
					���232�豸�ϴ����������ݣ�������ɺ�ͨ��485����������������
*/	
#define SALVER_REV_232DEV_DATA_HANDLE_TASK_PRO    11   
#define SALVER_REV_232DEV_DATA_HANDLE_STACK_SIZE   256*6
void salver_rev_232dev_data_handle_Task(void *pvParameters);
extern TaskHandle_t salver_rev_232dev_data_handle_TaskHandle;//������
//--------------------------------------------------------------------------------------------------------------------




#ifdef __cplusplus
}
#endif

#endif
