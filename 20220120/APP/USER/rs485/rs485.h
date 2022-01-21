#ifndef __RS485_H__
#define __RS485_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "stm32f4xx.h"

/*
****************************************************************************************
* TYPEDEFS (数据类型重定义)
****************************************************************************************
*/

#define  MAX_DATA_LENGTH     1000          //最大字节数量  

//数据帧结构
struct DataFrame{
	                                                          //（0、1是帧头）
	unsigned char        sender_id;                           // 2  -发送方ID
	unsigned char        receiver_id;                         // 3  -接收方方ID
  unsigned char        commond;                           // 4-命令字
	unsigned char        serial_number;                     // 5  -序列号
  unsigned short int   dataLength;                        //6,7 -数据长度         
  unsigned char       data[MAX_DATA_LENGTH];             //原始数据数据缓冲区 
  //unsigned short int  check;                             //8+N+1  8+N+2 CRC16校验码
                                                         //8+N+3  帧尾巴
};




typedef struct
{
	uint8_t RevBuf[1024];
	uint16_t RevLen;
	uint8_t RevOver;
	uint16_t BufLen;
  uint8_t  type;
}TYPE_RS485_U;

extern TYPE_RS485_U RS485rev; 



extern uint8_t devID_master;
extern uint8_t devID_salve;
/*
****************************************************************************************
* EXTERNAL VARIABLES (外部变量)
****************************************************************************************
*/
extern uint8_t flagSensorDataCome;
extern uint8_t flagFirmwareStart;//主机给从机传输的固件开始传输标志
extern uint8_t flagFirmwareData;//主机给从机传输的固件内容标志
extern uint8_t flagFirmwareEnd;//主机给从机传输的固件传输完成标志
extern uint8_t flagInquiry;//主机向从机询问标志
extern uint8_t flagSlaveUpdateFW;//从机请求主机传输固件标志
/*全局标志：网络状态************************************************************
            服务器连接状态
            SD卡就绪
						设备类型（带网络/非网络）
*/
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
//******************************************************************************
/*
****************************************************************************************
* CONSTANTS (常量)
****************************************************************************************
*/


/*
****************************************************************************************
* MACROS (宏定义)
****************************************************************************************
*/
#define RS485_UART USART6

#define RS485_UART_IRQn USART6_IRQn
#define RS485_UART_IRQHandler USART6_IRQHandler


#define RS485_GPIO_CLOCK_BUS      AHB1ENR
#define RS485_GPIO_CLOCK_BUS_BIT  2
//#define RS485_GPIO GPIOC
//#define RS485_GPIO_PIN 8

//#define SEND_RS485_MODE   GPIO_SetBits( RS485_GPIO,  GPIO_Pin_8 )//主机发送-高
//#define REV_RS485_MODE    GPIO_ResetBits( RS485_GPIO,GPIO_Pin_8 )  //主机接收-低


/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (声明全局函数)
****************************************************************************************
*/

void rs485_uart_config(uint32_t bond);
void rs485_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length );
void send_rs485_commond(uint8_t *cmd,uint16_t len);


char check_rev_data(uint8_t *frame,  uint16_t frameLen);

/*提示接收方将要进行固件升级，做好接收准备
sender_id发送方ID
receiver_id接收方ID
firmware_type 固件类型 (0x01：232设备 ; 0x02:485设备)
firmware_size 固件字节大小
返回0发送完成并且收到对方正确回应
*/
char send485_firmware_update_start(uint8_t sender_id,uint8_t receiver_id,uint8_t firmware_type,uint16_t firmware_size);

/*固件传输，单次传输最多携带1000字节数据
sender_id发送方ID
receiver_id接收方ID
firmware_data 指向固件数据 
length 本次传输的固件数据长度
返回0发送完成并且收到对方正确回应
*/
char send485_firmware_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* firmware_data,uint16_t length);

/*提示接收方固件传输完成
sender_id发送方ID
receiver_id接收方ID
firmware_type 固件类型 (0x01：232设备 ; 0x02:485设备)
trans_cnt 固件分传次数
返回0发送完成并且收到对方正确回应
*/
char send485_firmware_update_end(uint8_t sender_id,uint8_t receiver_id,uint16_t trans_cnt,int version);

/*传输传感器采集数据
sender_id发送方ID
receiver_id接收方ID
sensor_data 指向传感器数据 
length 传感器数据长度
返回0发送完成并且收到对方正确回应
*/
char send485_sensor_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* sensor_data,uint16_t length);

//主机向从机发送在线询问数据包
char send485_master_inquiry(uint8_t salve_id,int fw485ID,int fw232ID);

char send485_frimware_update_notify(uint8_t sender_id,uint8_t receiver_id,uint8_t fw_type);

/*发送响应*/
char send485_ack(uint8_t sender_id,uint8_t receiver_id,uint8_t cmd,uint8_t serial_number);

/*发送错误反馈*/
char send485_ack_err(uint8_t sender_id,uint8_t receiver_id,uint8_t err_code);
#ifdef __cplusplus
}
#endif

#endif
