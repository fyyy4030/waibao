#ifndef __WIFI_H__
#define __WIFI_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include "my_tasks.h"
#include "4G.h"
#include "LED.h"
/*
****************************************************************************************
* TYPEDEFS (���������ض���)
****************************************************************************************
*/
#define WIFI_BUF_MAX 1024//1024*1+200

 typedef struct
{
	uint8_t RevBuf[WIFI_BUF_MAX];
	uint16_t RevLen;
	uint8_t RevOver;
	uint16_t BufLen;
  uint8_t  type;
}TYPE_WIFI_U;

extern TYPE_WIFI_U wifi_rev; 



typedef struct 
{
	//2�� ESP8266 Station ������ AP����� IP ��ַ
  //3�� ESP8266 Station �ѽ��� TCP �� UDP ����
  //4�� ESP8266 Station �Ͽ���������
  //5�� ESP8266 Station δ���� AP
	uint8_t sta;
}TYPE_WIFI_STA;
extern TYPE_WIFI_STA _Wifi_Sta; 



/*
****************************************************************************************
* EXTERNAL VARIABLES (�ⲿ����)
****************************************************************************************
*/


/*
****************************************************************************************
* CONSTANTS (����)
****************************************************************************************
*/


/*
****************************************************************************************
* MACROS (�궨��)
****************************************************************************************
*/

#define WIFI_UART USART2

#define WIFI_UART_IRQn USART2_IRQn
#define WIFI_UART_IRQHandler USART2_IRQHandler

/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (����ȫ�ֺ���)
****************************************************************************************
*/
void wifi_uartConfig(uint32_t bond);
void wifi_tx_bytes( uint8_t* TxBuffer, uint16_t Length );
char wifi_init(void);

char wifi_jk2017_test();
char wifi_http_data_report(char TongDao_num,uint8_t *TongDaon,int TDCiShu,uint8_t *TDJZh,uint8_t *TDNZongHeGe);
bool  wifi_tcpip_get_connection_status(uint8_t *sta );

char wifi_connet_ap(uint8_t *ssid,uint8_t *pwd);
 char wifi_connet_server(uint8_t* tppe,uint8_t *remote_ip,int remote_port);

char wifi_http_check_firmware(int BanBenID,int XBanBenID,int *o_BanBenID,int *o_XBanBenID);
char wifi_http_get_firmware(int BanBenHaoId,char * IsSWJ,char *filename);

void exit_pass_throughMode(void);
#ifdef __cplusplus
}
#endif

#endif
