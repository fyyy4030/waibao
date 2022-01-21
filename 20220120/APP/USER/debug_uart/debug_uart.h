#ifndef __UART_H__
#define __UART_H__

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
}TYPE_DEBUG_U;

extern TYPE_DEBUG_U debug_rev; 

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

#define DEBUG_UART USART3

#define DEBUG_UART_IRQn USART3_IRQn
#define DEBUG_UART_IRQHandler //USART3_IRQHandler

/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (声明全局函数)
****************************************************************************************
*/
void debug_uartConfig(uint32_t bond);
void debug_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length );
#ifdef __cplusplus
}
#endif

#endif
