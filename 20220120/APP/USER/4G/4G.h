#ifndef __4G_H__
#define __4G_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "stm32f4xx.h"
#include "makedata.h"
#include "stdbool.h"
/*
****************************************************************************************
* TYPEDEFS (���������ض���)
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
	//2�� ESP8266 Station ������ AP����� IP ��ַ
  //3�� ESP8266 Station �ѽ��� TCP �� UDP ����
  //4�� ESP8266 Station �Ͽ���������
  //5�� ESP8266 Station δ���� AP
	uint8_t sta;
}TYPE_4G_STA;
extern TYPE_4G_STA _4g_Sta; 




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

#define AIR724_4G_UART    USART1

#define AIR724_4G_UART_IRQn   USART1_IRQn
#define AIR724_4G_UART_IRQHandler   USART1_IRQHandler

/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (����ȫ�ֺ���)
****************************************************************************************
*/
void air724_4g_uartConfig(uint32_t bond);
void air724_4g_tx_bytes( uint8_t* TxBuffer, uint16_t Length );



char network_4g(bool isOS);
//���4G�������
char air724_4g_network_check();
//�������������
char air724_4g_connect_to_http_server();

//0�ɹ�������������س���ID��CJID��   1û�гɹ����HTTP��Ӧ    2�������Ӧ������false 
//char air724_4g_http_jk2035_get(void);

//ע���豸�ӿ�-setp2-�ӿڷ���2034�ӿ�
//����1ע�� 0��ע�ᣨ�ϱ����ݣ�
//char air724_4g_http_jk2034_zhuce_get(void);
//char air724_4g_http_jk2034_report_get(void);


char air724_4g_http_data_report(char TongDao_num,uint8_t *TongDaon,int TDCiShu,uint8_t *TDJZh,uint8_t *TDNZongHeGe);


char air724_4g_http_check_firmware(int BanBenID,int XBanBenID,int *o_BanBenID,int *o_XBanBenID);
char air724_4g_http_get_firmware(int BanBenHaoId,char * IsSWJ,char *filename);
#ifdef __cplusplus
}
#endif

#endif
