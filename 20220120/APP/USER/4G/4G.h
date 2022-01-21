#ifndef __4G_H__
#define __4G_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "stm32f4xx.h"
#include "makedata.h"
#include "stdbool.h"
/*
****************************************************************************************
* TYPEDEFS (数据类型重定义)
****************************************************************************************
*/

#define _4G_BUF_MAX 1024//1024*3
 typedef struct
{
	uint8_t RevBuf[_4G_BUF_MAX];
	uint16_t RevLen;
	uint8_t RevOver;
	uint16_t BufLen;
  uint8_t  type;
}TYPE_AIR724_4G_U;

extern TYPE_AIR724_4G_U air724_4g_rev; 


typedef struct 
{
	//2： ESP8266 Station 已连接 AP，获得 IP 地址
  //3： ESP8266 Station 已建立 TCP 或 UDP 传输
  //4： ESP8266 Station 断开网络连接
  //5： ESP8266 Station 未连接 AP
	uint8_t sta;
}TYPE_4G_STA;
extern TYPE_4G_STA _4g_Sta; 




/*
****************************************************************************************
* EXTERNAL VARIABLES (外部变量)
****************************************************************************************
*/


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

#define AIR724_4G_UART    USART1

#define AIR724_4G_UART_IRQn   USART1_IRQn
#define AIR724_4G_UART_IRQHandler   USART1_IRQHandler

/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (声明全局函数)
****************************************************************************************
*/
void air724_4g_uartConfig(uint32_t bond);
void air724_4g_tx_bytes( uint8_t* TxBuffer, uint16_t Length );



char network_4g(bool isOS);
//检测4G网络情况
char air724_4g_network_check();
//接入服务器函数
char air724_4g_connect_to_http_server();

//0成功并输出参数返回厂家ID（CJID）   1没有成功获得HTTP响应    2获得了响应，但是false 
//char air724_4g_http_jk2035_get(void);

//注册设备接口-setp2-接口访问2034接口
//参数1注册 0不注册（上报数据）
//char air724_4g_http_jk2034_zhuce_get(void);
//char air724_4g_http_jk2034_report_get(void);


char air724_4g_http_data_report(char TongDao_num,uint8_t *TongDaon,int TDCiShu,uint8_t *TDJZh,uint8_t *TDNZongHeGe);


char air724_4g_http_check_firmware(int BanBenID,int XBanBenID,int *o_BanBenID,int *o_XBanBenID);
char air724_4g_http_get_firmware(int BanBenHaoId,char * IsSWJ,char *filename);
#ifdef __cplusplus
}
#endif

#endif
