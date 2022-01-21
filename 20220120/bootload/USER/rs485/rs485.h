#ifndef __RS485_H__
#define __RS485_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "stm32f4xx.h"

/*
****************************************************************************************
* TYPEDEFS (���������ض���)
****************************************************************************************
*/


typedef struct
{
	uint8_t RevBuf[1024];
	uint16_t RevLen;
	uint8_t RevOver;
	uint16_t BufLen;
  uint8_t  type;
}TYPE_RS485_U;

extern TYPE_RS485_U RS485rev; 


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
#define RS485_UART USART6

#define RS485_UART_IRQn USART6_IRQn
#define RS485_UART_IRQHandler USART6_IRQHandler


#define RS485_GPIO_CLOCK_BUS      AHB1ENR
#define RS485_GPIO_CLOCK_BUS_BIT  2
//#define RS485_GPIO GPIOC
//#define RS485_GPIO_PIN 8

//#define SEND_RS485_MODE   GPIO_SetBits( RS485_GPIO,  GPIO_Pin_8 )//��������-��
//#define REV_RS485_MODE    GPIO_ResetBits( RS485_GPIO,GPIO_Pin_8 )  //��������-��


/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (����ȫ�ֺ���)
****************************************************************************************
*/

void rs485_uart_config(uint32_t bond);
void rs485_uart_tx_bytes( uint8_t* TxBuffer, uint8_t Length );
void send_rs485_commond(uint8_t *cmd,uint8_t len);
uint16_t crc16_list(uint8_t *puchmsg, uint16_t usdatalen);
#ifdef __cplusplus
}
#endif

#endif
