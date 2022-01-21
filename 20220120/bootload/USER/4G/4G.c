

/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "4G.h"
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
* Function: air724_4g_uartConfig
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
void air724_4g_uartConfig(uint32_t bond)
{
	//GPIO配置
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOA, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//配置参数结构体
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_9 | GPIO_Pin_10;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOA,  GPIO_PinSource9,  GPIO_AF_USART1 );
	GPIO_PinAFConfig (GPIOA,  GPIO_PinSource10,  GPIO_AF_USART1 );
	GPIO_PinLockConfig(GPIOA,GPIO_Pin_9 | GPIO_Pin_10);
	
	//USART1配置
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_USART1,ENABLE ) ; ;
	
	USART_InitTypeDef   USART_InitStruct ;
	USART_InitStruct.USART_BaudRate=bond;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b ;
	USART_InitStruct.USART_StopBits=USART_StopBits_1 ;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init  ( AIR724_4G_UART,&USART_InitStruct) ;
	
	USART_ITConfig( AIR724_4G_UART,  USART_IT_RXNE,ENABLE) ;

	//配置NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=AIR724_4G_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( AIR724_4G_UART,ENABLE) ;
	
}
 TYPE_AIR724_4G_U air724_4g_rev={0}; 
/*
****************************************************************************************
* Function: AIR724_4G_UART_IRQHandler
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
void AIR724_4G_UART_IRQHandler(void)
{
	uint8_t data;
	if(AIR724_4G_UART->SR & (0X01<<5))  //接收中断
	{
		air724_4g_rev.RevBuf[air724_4g_rev.RevLen]=AIR724_4G_UART->DR;
		air724_4g_rev.RevLen++;

	}
	else if(AIR724_4G_UART->SR & (0X01<<4))//空闲中断
	{
		data=AIR724_4G_UART->SR;
		data=AIR724_4G_UART->DR;
	
		air724_4g_rev.BufLen=air724_4g_rev.RevLen;
		air724_4g_rev.RevLen=0;
		air724_4g_rev.RevOver=1;

	}  

}

/*
****************************************************************************************
* Function: air724_4g_tx_bytes
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
void air724_4g_tx_bytes( uint8_t* TxBuffer, uint8_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( AIR724_4G_UART, USART_FLAG_TXE ));
		USART_SendData(AIR724_4G_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}





