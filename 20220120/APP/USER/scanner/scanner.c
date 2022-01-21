
/*
****************************************************************************************




*********************************��Ȩ���У�����ؾ�*********************************
****************************************************************************************
*/

/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "scanner.h"
#include "stdio.h"
#include "string.h"
#include "my_print.h"
/*
****************************************************************************************
* CONSTANTS (��������)
****************************************************************************************
*/



/*
****************************************************************************************
* TYPEDEFS (���Ͷ���)
****************************************************************************************
*/


/*
****************************************************************************************
* LOCAL VARIABLES (��̬����)
****************************************************************************************
*/


/*
****************************************************************************************
* LOCAL FUNCTIONS DECLARE (��̬��������)
****************************************************************************************
*/
static void delay_microsecond(uint32_t microsecond);

/*
****************************************************************************************
* LOCAL FUNCTIONS (��̬����)
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
* PUBLIC FUNCTIONS (ȫ�ֺ���)
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
	//GPIO����
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOC, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//���ò����ṹ��
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource10,  GPIO_AF_UART4 );
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource11,  GPIO_AF_UART4 );
	GPIO_PinLockConfig(GPIOC,GPIO_Pin_10 | GPIO_Pin_11);
	
	//USART6����
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

	//����NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=SCANNER_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//��ռ���ȼ�
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
	if(SCANNER_UART->SR & (0X01<<5))  //�����ж�
	{
		scannerrev.RevBuf[scannerrev.RevLen]=SCANNER_UART->DR;
		scannerrev.RevLen++;

	}
	else if(SCANNER_UART->SR & (0X01<<4))//�����ж�
	{
		data=SCANNER_UART->SR;
		data=SCANNER_UART->DR;
	
		scannerrev.BufLen=scannerrev.RevLen;
		scannerrev.RevLen=0;
		scannerrev.RevOver=1;
		memset(TDSaoMa,0,TDSaoMa_LEN_MAX);
		if(scannerrev.RevBuf[scannerrev.BufLen-2]=='\r'&&scannerrev.RevBuf[scannerrev.BufLen-1]=='\n')  
			memcpy(TDSaoMa,scannerrev.RevBuf,scannerrev.BufLen-2);//��������ֽ���\r\n
		else
			memcpy(TDSaoMa,scannerrev.RevBuf,scannerrev.BufLen);
		
		TDSaoMa[TDSaoMa_LEN_MAX-1]='\0';
		MY_PRINT("\r\nɨ�룺%s\r\n",TDSaoMa);
//		LED4_ON;delay_microsecond(150*1000);
//		LED4_OFF;delay_microsecond(150*1000);
	} 

	
}

/*
****************************************************************************************
* Function: scanner_uart_tx_bytes
* Description: None
* Input: TxBuffer�����������ݻ������׵�ַ  
         Length�����������ݳ��ȣ��ֽڣ�
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
* Input: TxBuffer�����������ݻ������׵�ַ  
         Length�����������ݳ��ȣ��ֽڣ�
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





