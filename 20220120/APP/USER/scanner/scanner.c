
/*
****************************************************************************************




*********************************版权所有，盗版必究*********************************
****************************************************************************************
*/

/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "scanner.h"
#include "stdio.h"
#include "string.h"
#include "my_print.h"
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
static void delay_microsecond(uint32_t microsecond);

/*
****************************************************************************************
* LOCAL FUNCTIONS (静态函数)
****************************************************************************************
*/
static void delay_microsecond(uint32_t microsecond)
{
	microsecond *= 42;
	
	for(uint32_t i = 0; i < microsecond; i++)
	{
		;
	}
}



/*
****************************************************************************************
* PUBLIC FUNCTIONS (全局函数)
****************************************************************************************
*/

/*
****************************************************************************************
* Function: scanner_UARTConfig
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
void scanner_232UARTConfig(uint32_t bond)
{
	//GPIO配置
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOC, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//配置参数结构体
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource10,  GPIO_AF_UART4 );
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource11,  GPIO_AF_UART4 );
	GPIO_PinLockConfig(GPIOC,GPIO_Pin_10 | GPIO_Pin_11);
	
	//USART6配置
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_UART4,ENABLE ) ;
	
	USART_InitTypeDef   USART_InitStruct ;
	USART_InitStruct.USART_BaudRate=bond;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b ;
	USART_InitStruct.USART_StopBits=USART_StopBits_1 ;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init  ( SCANNER_UART,&USART_InitStruct) ;
	
	USART_ITConfig( SCANNER_UART,  USART_IT_RXNE,ENABLE) ;
	USART_ITConfig( SCANNER_UART,  USART_IT_IDLE,ENABLE) ;

	//配置NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=SCANNER_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( SCANNER_UART,ENABLE) ;

}


#include "led.h"
TYPE_SCANNER_U scannerrev={0};
char TDSaoMa[TDSaoMa_LEN_MAX];
/*
****************************************************************************************
* Function: SCANNER_UART_IRQHandler
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
void SCANNER_UART_IRQHandler(void)
{
	uint8_t data;
	if(SCANNER_UART->SR & (0X01<<5))  //接收中断
	{
		scannerrev.RevBuf[scannerrev.RevLen]=SCANNER_UART->DR;
		scannerrev.RevLen++;

	}
	else if(SCANNER_UART->SR & (0X01<<4))//空闲中断
	{
		data=SCANNER_UART->SR;
		data=SCANNER_UART->DR;
	
		scannerrev.BufLen=scannerrev.RevLen;
		scannerrev.RevLen=0;
		scannerrev.RevOver=1;
		memset(TDSaoMa,0,TDSaoMa_LEN_MAX);
		if(scannerrev.RevBuf[scannerrev.BufLen-2]=='\r'&&scannerrev.RevBuf[scannerrev.BufLen-1]=='\n')  
			memcpy(TDSaoMa,scannerrev.RevBuf,scannerrev.BufLen-2);//最后两个字节是\r\n
		else
			memcpy(TDSaoMa,scannerrev.RevBuf,scannerrev.BufLen);
		
		TDSaoMa[TDSaoMa_LEN_MAX-1]='\0';
		MY_PRINT("\r\n扫码：%s\r\n",TDSaoMa);
//		LED4_ON;delay_microsecond(150*1000);
//		LED4_OFF;delay_microsecond(150*1000);
	} 

	
}

/*
****************************************************************************************
* Function: scanner_uart_tx_bytes
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
void scanner_uart_tx_bytes( uint8_t* TxBuffer, uint8_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( SCANNER_UART, USART_FLAG_TXE ));
		USART_SendData(SCANNER_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}


/*
****************************************************************************************
* Function: scanner_uart_tx_bytes
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
void send_scanner_commond(uint8_t *cmd,uint8_t len)
{
	scanner_uart_tx_bytes( (uint8_t*)cmd, len );
}





