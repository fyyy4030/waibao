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
void rs485_uart_tx_bytes( uint8_t* TxBuffer, uint8_t Length );
void send_rs485_commond(uint8_t *cmd,uint8_t len);
uint16_t crc16_list(uint8_t *puchmsg, uint16_t usdatalen);
#ifdef __cplusplus
}
#endif

#endif
