#ifndef __MY_TASKS_H__
#define __MY_TASKS_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "stm32f4xx.h"
#include "FreeRTOS.h"//这个头文件必须在task.h之前被包含进来，否则报错，
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




/*全局标志：网络状态************************************************************
            服务器连接状态
            SD卡就绪
						设备类型（带网络/非网络）
						485设备ID
*/
extern uint8_t flagNetWorkDev;//当前使用的网络设备
#define DEV_NULL 0
#define DEV_WIFI 1
#define DEV_4G 2
extern uint8_t flagNetWork;//使用WIFI,连上路由为1///使用4G,有网络为1
#define NETWORK_ON  1 //有网络
#define NETWORK_OFF 0 //无网络
extern uint8_t flagServerConn;
#define CONNECTED 1 //连接
#define DISCONNECT 0 //断开
extern uint8_t flagSDReady;//SD卡就绪标志
#define READY 1   //就绪
#define NOT_READY 0 //未检测到卡
extern uint8_t flagDevType;
#define MASTER 1 //主机，带网络功能
#define SALVER 0 //从机，无网络功能

extern uint8_t falgDownLoadFW;
extern uint8_t flagReport_ing;//正在上报数据标志

extern uint8_t flagIsSaomaSingle;

extern int FW485ID;//存在SD卡中的485固件的版本，-1表示没有存储到该固件
extern int FW232ID;//存在SD卡中的232固件的版本，-1表示没有存储到该固件

//网络配置信息
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
#define HAVE_OFFLINE_DATA_485 BIT0 //有485从机传输过来的离线数据要处理
#define HAVE_OFFLINE_DATA_232 BIT1 //有232设备离线数据要处理

extern EventGroupHandle_t firmware_event_group;
#define HAVE_NEW_FIRMWARE_485 BIT0
#define HAVE_NEW_FIRMWARE_232 BIT1
#define GET_NEW_485FIRMWARE_FROM_SERVER BIT2
#define GET_NEW_232FIRMWARE_FROM_SERVER BIT3



//******************************************************************************
/*任务11：上报数据任务

*/
#define REPORT_TASK_PRO     13
#define REPORT_TASK_STACK_SIZE   256*8//256+128
void report_Task(void *pvParameters);
extern TaskHandle_t report_TaskHandle;//任务句柄


//******************************************************************************
/*任务11：485设备轮询任务

*/
#define POLLING_TASK_PRO     12
#define POLLING_TASK_STACK_SIZE   128//256+128
void polling_Task(void *pvParameters);
extern TaskHandle_t polling_TaskHandle;//任务句柄

//--------------------------------------------------------------------------------------------------------------------


/*任务1：WIFI网络状态检测任务（仅支持带网络的主机）
	
*/
#define NETWORK_STATUS_CHECK_WIFI_TASK_PRO          6
#define NETWORK_STATUS_CHECK_WIFI_TASK_STACK_SIZE   256*2
void network_status_check_wifi_Task(void *pvParameters);
extern TaskHandle_t network_status_check_wifi_TaskHandle;//任务句柄

/*任务2：4G设备网络状态检测任务（仅支持带网络的主机）
	

*/
#define NETWORK_STATUS_CHECK_4G_TASK_PRO          5
#define NETWORK_STATUS_CHECK_4G_TASK_STACK_SIZE   256
void network_status_check_4g_Task(void *pvParameters);
extern TaskHandle_t network_status_check_4g_TaskHandle;//任务句柄

//#########################################################################
/*任务3：设备网络状态检测任务（仅支持带网络的主机）
					轮询a、b
					a.检测物联网卡的网络情况，
			           如果发生网络变化则触发相应事件标志
								 
					b.设备在有网的情况下，发生了服务器失联（原本是连接状态，变成了断开状态）,触发一次有网事件标志	进行重新接入服务器		

*/
#define NETWORK_STATUS_CHECK_TASK_PRO     4
#define NETWORK_STATUS_CHECK_TASK_STACK_SIZE   128
void network_status_check_Task(void *pvParameters);
extern TaskHandle_t network_status_check_TaskHandle;//任务句柄


/*任务4：设备网络状态变化处理任务（仅支持带网络的主机）
			   如果触发了一次有网事件
			       接入服务器直至成功
						 成功接入服务器后：
									设备先服务器进行注册（2035，2034接口）
									LED2常亮
									检测本地SD卡是否有离线数据存储，有就上传到服务器，上传成功后删除离线数据。
									挂起“离线数据处理任务”
							
			    如果触发了一次断网事件（与服务器断开）
									LED2熄灭
				          记录断网时间点到本地SD卡
									唤醒“离线数据处理任务”
*/
#define NETWORK_STATUS_CHANGE_HANDLE_TASK_PRO        3
#define NETWORK_STATUS_CHANGE_HANDLE_TASK_STACK_SIZE   256*5//不能改小
void network_status_change_handle_Task(void *pvParameters);
extern TaskHandle_t network_status_change_handle_TaskHandle;//任务句柄

//--------------------------------------------------------------------------------------------------------------------

/*任务5：离线数据处理任务：（仅支持带网络的主机）
							阻塞等待事件标志（离线数据处理）
             //把从232设备/或者485从设备收到的相关数据存储到本地SD卡。以文本形式存储，按时间先后顺序存储。
*/
#define OFFLINE_DATA_PROCESS_TASK_PRO         2
#define OFFLINE_DATA_PROCESS_TASK_STACK_SIZE   256*2//不能改小
void offline_data_process_Task(void *pvParameters);
extern TaskHandle_t offline_data_process_TaskHandle;//任务句柄

//--------------------------------------------------------------------------------------------------------------------


/*任务6：服务器下发的数据处理任务：（仅支持带网络的主机）
			a.服务器同步本地RTC
			b.从服务器获取升级文件到本地(SD卡)
*/
#define SERVER_DATA_PROCESS_TASK_PRO     7
#define SERVER_DATA_PROCESS_TASK_STACK_SIZE   256*4//不能改小
void server_data_process_Task(void *pvParameters);
extern TaskHandle_t server_data_process_TaskHandle;//任务句柄

//--------------------------------------------------------------------------------------------------------------------
/*任务7：主机升级固件传输任务：（仅用于主机）
				阻塞等待事件标志（新固件事件）
				①得到了新固件（不管是232固件还是485固件），通过485协议把升级固件发送到非网络设备的从机	
				②如果新固件类型是485固件  在SD卡记录有新固件的标志文件	                          
				 如果新固件类型是232固件  发送232固件传输事件给任务“firmware_transmit_to_232dev_Task”
*/
#define FIRMWARE_TRANSMIT_TO_485SALVE_TASK_PRO    8  
#define FIRMWARE_TRANSMIT_TO_485SALVE_TASK_STACK_SIZE   256*3
void firmware_transmit_to_salve_Task(void *pvParameters);
extern TaskHandle_t firmware_transmit_to_salve_TaskHandle;//任务句柄

/*任务S1：从机升级固件传输任务：（仅用于从机）
				等待主机与之进行固件传输
				得到了新固件，通过232协议把升级固件发送到232设备（传输协议甲方提供）
				如果新固件类型是485固件，在SD卡记录有新固件的标志文件
				如果新固件类型是232固件，发送232固件传输事件给任务“firmware_transmit_to_232dev_Task”
*/
#define FIRMWARE_RECEIVE_FROM_485MASTER_TASK_PRO     8  
#define FIRMWARE_RECEIVE_FROM_485MASTER_TASK_STACK_SIZE   256*5
void firmware_receive_from_master_Task(void *pvParameters);
extern TaskHandle_t firmware_receive_from_master_TaskHandle;//任务句柄

/*任务8：把新固件传输给232设备任务：
      阻塞等待232固件传输事件
			通过232协议把升级固件发送到232设备（传输协议甲方提供）
*/
#define FIRMWARE_TRANSMIT_TO_232DEV_TASK_PRO     9   
#define FIRMWARE_TRANSMIT_TO_232DEV_TASK_STACK_SIZE   256*4
void firmware_transmit_to_232dev_Task(void *pvParameters);
extern TaskHandle_t firmware_transmit_to_232dev_TaskHandle;//任务句柄


//--------------------------------------------------------------------------------------------------------------------

/*任务9：485设备数据采集上报任务：(本机是带网络的主机)
          主机处理485从机发送过来的数据
			    如果接收到485设备从机上传的传感器数据
					    设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
					    设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储					
*/
#define MASTER_REV_485DEV_DATA_HANDLE_TASK_PRO     10   
#define MASTER_REV_485DEV_DATA_HANDLE_STACK_SIZE   256*4+128//不能改小
void master_rev_485dev_data_handle_Task(void *pvParameters);
extern TaskHandle_t master_rev_485dev_data_handle_TaskHandle;//任务句柄
/*任务10：232设备数据采集上报任务：(本机是带网络的主机)
          主机处理232从机发送过来的数据
 					如果232设备上传的传感器数据，接收完成后
					    设备是与服务器连通状态：直接把数据上传至服务器（传输协议甲方提供）
					    设备是与服务器断开状态：发送事件标志（离线数据处理），把数据交给“离线数据处理任务”进行本地离线存储																							 
*/
#define MASTER_REV_232DEV_DATA_HANDLE_TASK_PRO     11   
#define MASTER_REV_232DEV_DATA_HANDLE_STACK_SIZE   256*8
void master_rev_232dev_data_handle_Task(void *pvParameters);
extern TaskHandle_t master_rev_232dev_data_handle_TaskHandle;//任务句柄

/*任务S2：数据采集上报任务：(本机是非网络设备的从机)
          从机处理232从机发送过来的数据
					如果232设备上传传感器数据，接收完成后，通过485传输给带网络的主机
*/	
#define SALVER_REV_232DEV_DATA_HANDLE_TASK_PRO    11   
#define SALVER_REV_232DEV_DATA_HANDLE_STACK_SIZE   256*6
void salver_rev_232dev_data_handle_Task(void *pvParameters);
extern TaskHandle_t salver_rev_232dev_data_handle_TaskHandle;//任务句柄
//--------------------------------------------------------------------------------------------------------------------




#ifdef __cplusplus
}
#endif

#endif
