

/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "debug_uart.h"
#include "stdio.h"
#include "string.h"

/*
****************************************************************************************
* CONSTANTS (常量定义)
****************************************************************************************
*/


/*
****************************************************************************************
* TYPEDEFS (类型定义)
****************************************************************************************
*/


/*
****************************************************************************************
* LOCAL VARIABLES (静态变量)
****************************************************************************************
*/


/*
****************************************************************************************
* LOCAL FUNCTIONS DECLARE (静态函数声明)
****************************************************************************************
*/


/*
****************************************************************************************
* LOCAL FUNCTIONS (静态函数)
****************************************************************************************
*/


/*
****************************************************************************************
* PUBLIC FUNCTIONS (全局函数)
****************************************************************************************
*/

/*
****************************************************************************************
* Function: debug_uartConfig
* Description: None
* Input: None
* Output: None
* Return: None
* Author: weihaoMo
* Others: None
* Date of completion: 2020-09-01
* Date of last modify: 2019-09-01
****************************************************************************************
*/
void debug_uartConfig(uint32_t bond)
{
	//GPIO配置
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOD, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//配置参数结构体
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOD,  GPIO_PinSource8,  GPIO_AF_USART3 );
	GPIO_PinAFConfig (GPIOD,  GPIO_PinSource9,  GPIO_AF_USART3 );
	//GPIO_PinLockConfig(GPIOD,GPIO_Pin_8 | GPIO_Pin_9);
	
	//USART3配置
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART3,ENABLE ) ;
	
	USART_InitTypeDef   USART_InitStruct ;
	USART_InitStruct.USART_BaudRate=bond;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b ;
	USART_InitStruct.USART_StopBits=USART_StopBits_1 ;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init  ( DEBUG_UART,&USART_InitStruct) ;
	
	USART_ITConfig( DEBUG_UART,  USART_IT_RXNE,ENABLE) ;
	USART_ITConfig( DEBUG_UART,  USART_IT_IDLE,ENABLE) ;

	//配置NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=DEBUG_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( DEBUG_UART,ENABLE) ;
	
}

/*
****************************************************************************************
* Function: DEBUG_UART_IRQHandler
* Description: None
* Input: None
* Output: None
* Return: None
* Author: weihaoMo
* Others: None
* Date of completion: 2020-09-01
* Date of last modify: 2019-09-01
****************************************************************************************
*/
//#include "4G.h"
//#include "wifi.h"
//TYPE_DEBUG_U debug_rev; 
//void DEBUG_UART_IRQHandler(void)
//{
////	uint8_t data;
////	
////	data=DEBUG_UART->DR;//USART_ReceiveData(DEBUG_UART );  
////	
////	while(  RESET==USART_GetFlagStatus(WIFI_UART,USART_FLAG_TXE) );//等待发送缓冲区空
////	USART_SendData(WIFI_UART,  data );  


//	
//	uint8_t data;
//	if(DEBUG_UART->SR & (0X01<<5))  //接收中断
//	{
//		debug_rev.RevBuf[debug_rev.RevLen]=DEBUG_UART->DR;
//		debug_rev.RevLen++;

//	}
//	else if(DEBUG_UART->SR & (0X01<<4))//空闲中断
//	{
//		data=DEBUG_UART->SR;
//		data=DEBUG_UART->DR;
//	
//		debug_rev.BufLen=debug_rev.RevLen;
//		debug_rev.RevLen=0;
//		debug_rev.RevOver=1;
//		
//	}  
//}

/*
****************************************************************************************
* Function: debug_uart_tx_bytes
* Description: None
* Input: TxBuffer：待发送数据缓冲区首地址  
         Length：待发送数据长度（字节）
* Output: None
* Return: None
* Author: weihaoMo
* Others: None
* Date of completion: 2020-09-01
* Date of last modify: 2019-09-01
****************************************************************************************
*/
void debug_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( DEBUG_UART, USART_FLAG_TXE ));
		USART_SendData(DEBUG_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}


#pragma import(__use_no_semihosting_swi) //取消半主机状态

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;

int fputc(int ch, FILE *f) 
{
	while((DEBUG_UART->SR &(0X01<<7))==0);  //等待之前的数据发送完毕
		DEBUG_UART->DR=ch;
  return (ch);
}

int ferror(FILE *f) {
  /* Your implementation of ferror */
  return EOF;
}


void _ttywrch(int ch) {
  while((DEBUG_UART->SR &(0X01<<7))==0);
		DEBUG_UART->DR=ch;
}


void _sys_exit(int return_code) {
label:  goto label;  /* endless loop */
}

