

/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "4G.h"
#include "stdio.h"
#include "string.h"
#include "sd_operation.h"
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
static char flagGetFirmware=0;//接收固件标志


static char flag4gRST=0;
static char flag4gRDY=0;
static char flag4gAT=0;
static char flag4gREADY=0;
/*
****************************************************************************************
* LOCAL FUNCTIONS DECLARE (静态函数声明)
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
	//GPIO_PinLockConfig(GPIOA,GPIO_Pin_9 | GPIO_Pin_10);
	
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
	USART_ITConfig( AIR724_4G_UART,  USART_IT_IDLE,ENABLE) ;

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
 #include "debug_uart.h"
void AIR724_4G_UART_IRQHandler(void)
{
	uint8_t data;
	if(AIR724_4G_UART->SR & (0X01<<5))  //接收中断
	{
		
			air724_4g_rev.RevBuf[air724_4g_rev.RevLen]=AIR724_4G_UART->DR;
			air724_4g_rev.RevLen++;		
			if(air724_4g_rev.RevLen>_4G_BUF_MAX)
				air724_4g_rev.RevLen=_4G_BUF_MAX;	

	}
	else if(AIR724_4G_UART->SR & (0X01<<4))//空闲中断
	{
		data=AIR724_4G_UART->SR;
		data=AIR724_4G_UART->DR;

		air724_4g_rev.BufLen=air724_4g_rev.RevLen;
		air724_4g_rev.RevLen=0;
		air724_4g_rev.RevOver=1;
		MY_DEBUG_TX( air724_4g_rev.RevBuf, air724_4g_rev.BufLen );
		
		if(0==flag4gRST)
		{
			//if(NULL!=strstr((const char *)air724_4g_rev.RevBuf,"AT+RSTSET\r\n\r\nOK"))
			if(NULL!=strstr((const char *)air724_4g_rev.RevBuf,"RDY")||NULL!=strstr((const char *)air724_4g_rev.RevBuf,"^STN: "))
			{
				air724_4g_rev.RevOver=0;
				flag4gRST=1;
				MY_PRINT("\r\n\r\n\r\n4G RDY OK\r\n\r\n\r\n");
			}
		}
//		if(0==flag4gRDY)
//		{
//			if(NULL!=strstr((const char *)air724_4g_rev.RevBuf,"RDY"))
//			{
//				air724_4g_rev.RevOver=0;
//				flag4gRDY=1;
//				MY_PRINT("\r\n\r\n\r\n4G RDY OK\r\n\r\n\r\n");
//			}			
//		}
		
		if(0==flag4gAT)
		{
			if(NULL!=strstr((const char *)air724_4g_rev.RevBuf,"AT\r\n\r\nOK"))
			{
				air724_4g_rev.RevOver=0;
				flag4gAT=1;
				MY_PRINT("\r\n\r\n\r\n4G AT OK\r\n\r\n\r\n");
			}
		}		
		
		if(0==flag4gREADY)
		{
			if(NULL!=strstr((const char *)air724_4g_rev.RevBuf,"SMS READY")||NULL!=strstr((const char *)air724_4g_rev.RevBuf,"+NITZ:"))
			{
				air724_4g_rev.RevOver=0;
				flag4gREADY=1;
				MY_PRINT("\r\n\r\n\r\n4G SMS READY\r\n\r\n\r\n");
			}

		}
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
void air724_4g_tx_bytes( uint8_t* TxBuffer, uint16_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( AIR724_4G_UART, USART_FLAG_TXE ));
		USART_SendData(AIR724_4G_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}





//-------------------------------------------------------------------------------------------------
#define CMD_REV_TIMEOUT 5000
#define AT_OK 0
#define AT_TIMEOUT 1
#define AT_ERR 2


extern SemaphoreHandle_t Semaphore_4g;

//发送AT指令给4G ，响应信息at_resp中包含期待回复的内容at_resp_check则返回AT_OK
static char at_4g_send_at(const char* at_cmd,const char* at_resp_check)
{
	int cnt=0;

	memset(air724_4g_rev.RevBuf ,0,air724_4g_rev.BufLen);
	
	air724_4g_tx_bytes( (uint8_t *)at_cmd, strlen((const char *)at_cmd) );

	nx:	
	while(cnt<CMD_REV_TIMEOUT)
	{
		vTaskDelay(1);//delay_microsecond(500*2);//500us
		if(air724_4g_rev.RevOver==1)
		{
			break;
		}
		cnt++;
	}
	
	if(cnt>=CMD_REV_TIMEOUT)
	{
		
		return AT_TIMEOUT;
	}

	
	if(air724_4g_rev.RevOver==1)
	{
		air724_4g_rev.RevOver=0;
		
		if(strstr((const char *)air724_4g_rev.RevBuf,(const char *)at_resp_check))
		{
			//收到的响应中含有期待的响应信息
			//memset(air724_4g_rev.RevBuf ,0,sizeof(air724_4g_rev.RevBuf));
			
			return AT_OK;
			
		}
		if(strstr((const char *)air724_4g_rev.RevBuf,(const char *)"+CME ERROR"))//指令错误响应
		{
			return AT_ERR;
		}
		else
		{
			
			//收到的响应中没有期待的响应信息，继续等待接收，直到超时
			//memset(air724_4g_rev.RevBuf ,0,sizeof(air724_4g_rev.RevBuf));
			cnt=0;
			goto nx;
		}
	}
	
}
#include "rtc.h"
#define BUSY 0xFF
#define FREE 0
static uint8_t flagbusy=FREE;


char at_4g_send_rstset(void)
{
	int cnt=0;
	flag4gRST=0;//flag4gRST=0;
	memset(air724_4g_rev.RevBuf ,0,air724_4g_rev.BufLen);
	
	air724_4g_tx_bytes( (uint8_t *)"AT+RSTSET\r\n", strlen((const char *)"AT+RSTSET\r\n") );	
	
	while(cnt<3000)
	{
		vTaskDelay(1);//delay_microsecond(500*2);//500us
		if(flag4gRST==1)
		{
			return 0;
		}
		cnt++;
	}
	
	return 1;
	
}

char at_4g_send_AT(void)
{
	int cnt=0;
	flag4gAT=0;
	memset(air724_4g_rev.RevBuf ,0,air724_4g_rev.BufLen);
	
	air724_4g_tx_bytes( (uint8_t *)"AT\r\n", strlen((const char *)"AT\r\n") );	
	
	while(cnt<1500)
	{
		vTaskDelay(1);//delay_microsecond(500*2);//500us
		if(flag4gAT==1)
		{
			return 0;
		}
		cnt++;
	}
	
	return 1;
	
}


//0正常
char network_4g(bool isOS)
{
	char ret=0;
	int cnt=0;
	//模块上电
	//Pwoerkey拉低1~2秒开机
	
	again:	
	vTaskDelay(1000*3);
	cnt=0;
	while(1)
	{
		
		ret=at_4g_send_rstset();//ret=at_4g_send_at("AT+RSTSET\r\n","OK");
		if(0==ret)
		{
			break;
		}	
		else
		{
			delay_microsecond(100);//vTaskDelay(20);
			if(++cnt>=15)
			{
				cnt=0;
				goto again;
			}		
		}		
	}	
//	for(int i=0;i<15*100;i++)
//	{
//		vTaskDelay(10);
//		if(flag4gRDY==1)
//		{
//			goto again;
//		}
//	}	

	
	
	while(1)
	{	
		ret=at_4g_send_AT( );
		if(0==ret)
		{
			break;
		}	
		else
		{
			delay_microsecond(100);//vTaskDelay(20);
			if(++cnt>=15)
			{
				cnt=0;
				goto again;
			}		
		}

	}		
	
	
	for(int i=0;i<60*100;i++)
	{
		vTaskDelay(10);
		if(flag4gREADY==1)
		{
			break;
		}
		if(i>=60*100-1)
			goto again;
	}
	

	//delay_microsecond(5000);
	if(AT_OK==at_4g_send_at("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n","OK"))
	{
		MY_PRINT("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\" -- OK\r\n");
	}
	else
	{
		flag4gREADY=0;
		flag4gRST=0;
		flag4gRDY=0;
		flag4gAT=0;
		goto again;
	}
		
	//delay_microsecond(5000);
	if(AT_OK==at_4g_send_at("AT+SAPBR=3,1,\"APN\",\"\"\r\n","OK"))
	{
		MY_PRINT("AT+SAPBR=3,1,\"APN\",\"\" -- OK\r\n");
	}
	else
	{
		flag4gREADY=0;
		flag4gRST=0;
		flag4gRDY=0;
		flag4gAT=0;
		goto again;
	}
	//delay_microsecond(5000);
	if(AT_OK==at_4g_send_at("AT+SAPBR=1,1\r\n","OK"))
	{
		MY_PRINT("AT+SAPBR=1,1 -- OK\r\n");
	}
	else
	{
		flag4gREADY=0;
		flag4gRST=0;
		flag4gRDY=0;
		flag4gAT=0;
		goto again;
	}	
	//delay_microsecond(5000);
	if(AT_OK==at_4g_send_at("AT+SAPBR=2,1\r\n","OK"))
	{
		MY_PRINT("AT+SAPBR=2,1 -- OK\r\n");
	}
	else
	{
		flag4gREADY=0;
		flag4gRST=0;
		flag4gRDY=0;
		flag4gAT=0;
		goto again;
	}
	//delay_microsecond(5000);
//	if(AT_OK==at_4g_send_at("AT+HTTPTERM\r\n","OK"))
//	{
//		MY_PRINT("AT+HTTPTERM -- OK\r\n");
//	}
	if(AT_OK==at_4g_send_at("AT+CCLK?\r\n","OK"))
	{
		MY_PRINT("AT+CCLK -- OK\r\n");
		char *p=strstr((const char *)air724_4g_rev.RevBuf,"+CCLK: \"");
		p=p+(strlen("+CCLK: \""));
		rtc_data.year=2000+(p[0]-'0')*10+(p[1]-'0');
		p+=3;
		rtc_data.month=(p[0]-'0')*10+(p[1]-'0');
		p+=3;
		rtc_data.day=(p[0]-'0')*10+(p[1]-'0');
		p+=3;
		rtc_data.hour=(p[0]-'0')*10+(p[1]-'0');
		p+=3;
		rtc_data.min=(p[0]-'0')*10+(p[1]-'0');
		p+=3;
		rtc_data.sec=(p[0]-'0')*10+(p[1]-'0');
		Set_RTC_Time(rtc_data.hour,rtc_data.min,rtc_data.sec,0);
		Set_RTC_Date(rtc_data.year,rtc_data.month,rtc_data.day);	
				char buf[20]={0};
				sprintf((char *)buf,"%d-%d-%d %02d:%02d:%02d",rtc_data.year,rtc_data.month,rtc_data.day,rtc_data.hour,rtc_data.min,rtc_data.sec);		
				MY_PRINT("%s\r\n",buf);
	}		
	MY_PRINT("\r\n**********************************network_4g init ok******************\r\n");
}

//#define   WIFISSID      "Moweihao-iPhone"
//#define   WIFIPWD       "123123123"
//#define   SERVER_IP     "8.134.112.231"   
//#define   SERVER_PORT   8099 
//#define   URL           "/ShuJu/QingQiu"

static char air724_4g_http_post(const char * body)
{	
	char ret;
	
	at_4g_send_at("AT+HTTPTERM\r\n","OK");

	ret=at_4g_send_at("AT+HTTPINIT\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT3;
	}
	
	ret=at_4g_send_at("AT+HTTPPARA=\"CID\",1\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=2;
		goto EXIT3;
	}	

	//ret=at_4g_send_at("AT+HTTPPARA=\"URL\",\"http://8.134.112.231:8099/ShuJu/QingQiu\"\r\n","OK");
	char BUFF[300]={0};
	sprintf(BUFF,"AT+HTTPPARA=\"URL\",\"http://%s:%d%s\"\r\n",SERVER_IP,SERVER_PORT,URL);
	ret=at_4g_send_at(BUFF,"OK");
	if(ret!=AT_OK)
	{
		ret=3;
		goto EXIT3;
	}		

	uint8_t buf_len[10]={0};
	sprintf((char *)buf_len,"%d",strlen((const char *)body));
	uint8_t buf[30]={"AT+HTTPDATA="};
	strcat((char *)buf,(const char *)buf_len);
	strcat((char *)buf,(const char *)",5000\r\n");
	//ret=at_4g_send_at("AT+HTTPDATA=100,5000\r\n","DOWNLOAD");
	ret=at_4g_send_at((const char *)buf,"DOWNLOAD");
	if(ret!=AT_OK)
	{
		ret=4;
		goto EXIT3;
	}	
	//MY_PRINT("%s\r\n\r\n\r\n",buf);
	
	//ret=at_4g_send_at("JKName=2034&ID=0&SheBeiBiaoShi=888&CaoZuoType=1&CJID=5&IsZhuCe=true&SBMiaoSu=test&SBName=mo&SBType=1","OK");
	ret=at_4g_send_at((const char *)body,"OK");
	if(ret!=AT_OK)
	{
		ret=5;
		goto EXIT3;
	}	
	
	ret=at_4g_send_at("AT+HTTPACTION=1\r\n","+HTTPACTION: 1,200,");
	if(ret!=AT_OK)
	{
		ret=6;
		goto EXIT3;
	}		
	
	ret=at_4g_send_at("AT+HTTPREAD\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=7;
		goto EXIT3;
	}	
	if(NULL==strstr((const char *)air724_4g_rev.RevBuf,"\"ChengGong\":true"))
	{
		ret=8;
		goto EXIT3;
	}		
	ret=0;
	MY_PRINT("\r\n#########################4G####################################\r\n");
	MY_DEBUG_TX( air724_4g_rev.RevBuf, air724_4g_rev.BufLen );
	
	EXIT3:
	at_4g_send_at("AT+HTTPTERM\r\n","OK");
	return ret;	

}

//检测4G网络情况
//0~31网络正常 99网络不正常 100-检测不出（指令错误）
char air724_4g_network_check(void)
{
	xSemaphoreTake( Semaphore_4g, portMAX_DELAY );


	char ret=at_4g_send_at("AT+CSQ\r\n","OK");
	if(AT_OK!=ret)
	{
		ret=100;
		goto EXIT1;
	}
	uint8_t csq=99;
	char *p1=strstr((const char *)air724_4g_rev.RevBuf,"+CSQ: ");
	if(p1!=NULL)
	{
		p1=p1+6;
	}
	char *p2=strstr((const char *)air724_4g_rev.RevBuf,",");
	if(p2-p1==1)
	{
		csq=*p1-'0';
	}
	else if(p2-p1==2)
	{
		csq=(*p1-'0')*10 + (*(p1+1)-'0');
	}
	
	if(csq!=99){
		ret=csq;
	}
	else {
		ret=99;
	}
	
	EXIT1:
	xSemaphoreGive( Semaphore_4g );
	return ret;
	
}






//上报数据给服务器（仅主机）
char air724_4g_http_data_report(char TongDao_num,uint8_t *TongDaon,int TDCiShu,uint8_t *TDJZh,uint8_t *TDNZongHeGe)
{
	xSemaphoreTake( Semaphore_4g, portMAX_DELAY );		
	//调用2050接口
	//uint8_t body[15*1024]={"JKName=2050&ID=0"};
	uint8_t *body=malloc(15*1024);
	memset(body,0,15*1024);
	strcpy((char *)body,"JKName=2050&ID=0");
	
	
	uint8_t buf[3]={0};
 	for(char i=1;i<=8;i++)
	{
		strcat((char *)body,(const char *)"&TongDao");
		sprintf((char *)buf,"%d=",i);
		strcat((char *)body,(const char *)buf);
		if(TongDao_num==i)
		{
			strcat((char *)body,(const char *)TongDaon);
		}
		else
		{	
			//strcat((char *)body,(const char *)"\"\"");
			strcat((char *)body,(const char *)"");
		}
	}
	strcat((char *)body,(const char *)"&SBBH=");
	strcat((char *)body,(const char *)http_key.SBBH);
	uint8_t Buff[30]={0};
	sprintf((char *)Buff,"&CJID=%d",http_key.CJID);
	strcat((char *)body,(const char *)Buff);
	
	memset(Buff,0,sizeof(Buff));
	sprintf((char *)Buff,"&TDCiShu=%d",TDCiShu);
	strcat((char *)body,(const char *)Buff);	
	
	strcat((char *)body,(const char *)"&TDJZh=");
	if(NULL==TDJZh)
		strcat((char *)body,(const char *)http_key.TDJZh);
	else
		strcat((char *)body,(const char *)TDJZh);

	strcat((char *)body,(const char *)"&TDNZongHeGe=");
	if(NULL==TDNZongHeGe)
		strcat((char *)body,(const char *)" ");
	else
		strcat((char *)body,(const char *)TDNZongHeGe);	

	strcat((char *)body,(const char *)"&ZaXiang=\"");
	strcat((char *)body,(const char *)http_key.ZaXiang);
	strcat((char *)body,(const char *)"\"\r\n");
	
	int len=strlen((const char *)body);
	MY_DEBUG_TX(body,len);
	
	char ret=air724_4g_http_post((const char *)body);
	if(0==ret) 
	{
		MY_PRINT("4G:数据更新succeed\r\n");
	}
	else
	{
			MY_PRINT("4G:数据更新fail\r\n");
	}
	free(body);
	xSemaphoreGive( Semaphore_4g );
	return ret;
	
	
}

//检测固件版本是否需要更新（仅主机）
//BanBenID当前上位机版本号
//XBanBenID当前下位机版本号
//o_BanBenID 检测到服务器上的上位机版本号，不需要更新返回-1
//o_XBanBenID检测到服务器上的下位机版本号，不需要更新返回-1
char air724_4g_http_check_firmware(int BanBenID,int XBanBenID,int *o_BanBenID,int *o_XBanBenID)
{
	xSemaphoreTake( Semaphore_4g, portMAX_DELAY );		

	char ret;
	at_4g_send_at("AT+HTTPTERM\r\n","OK");

	ret=at_4g_send_at("AT+HTTPINIT\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT;
	}
	
	ret=at_4g_send_at("AT+HTTPPARA=\"CID\",1\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT;
	}	
	char buf[200]={0};                                                    
	sprintf(buf,"AT+HTTPPARA=\"URL\",\"http://%s:%d%s?JKName=2051&ID=0&SBBH=%s&SWJType=1&BanBenID=%d&XBanBenID=%d\"\r\n",
																							SERVER_IP,SERVER_PORT,URL,http_key.SBBH,BanBenID,XBanBenID);
	ret=at_4g_send_at(buf,"OK");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT;
	}	
	
	ret=at_4g_send_at("AT+HTTPACTION=0\r\n","+HTTPACTION: 0,200,");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT;
	}
	
	ret=at_4g_send_at("AT+HTTPREAD\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT;
	}
	
	MY_DEBUG_TX( air724_4g_rev.RevBuf, air724_4g_rev.BufLen );
	
	*o_BanBenID=*o_XBanBenID=-1;
	if(strstr((const char *)air724_4g_rev.RevBuf,"\"ChengGong\":true"))
	{
		MY_PRINT("2051 数据成功\r\n");
		char *p=strstr((const char *)air724_4g_rev.RevBuf,"\"ShuJu\":");
		if(p==NULL)
		{
			ret=0;
			goto EXIT;
		}
		p=strstr((const char *)p,":")+2;
		char *p1=strstr((const char *)p,"\"");
		char str[20]={0};
		memcpy(str,p,p1-p);
		MY_PRINT("\r\n%s",str);
		
		char *token = strtok(str, ",");
		if(0==strcmp(token,"false"))
			*o_BanBenID=-1;
		else if(0==strcmp(token,"true"))
			*o_BanBenID=0;
		
		token = strtok(NULL, ",");
		if(0==strcmp(token,"false"))
			*o_XBanBenID=-1;
		else if(0==strcmp(token,"true"))
			*o_XBanBenID=0;
			
		token = strtok(NULL, ",");	
		if(*o_BanBenID==0)//上位机需要更新
			*o_BanBenID=atoi(token);
		token = strtok(NULL, ",");			
		if(*o_XBanBenID==0)//下位机需要更新
			*o_XBanBenID=atoi(token);		
	}
	else
	{
		MY_PRINT("2051 数据失败\r\n");
	}
	
	ret=0;
	
	EXIT:
	at_4g_send_at("AT+HTTPTERM\r\n","OK");
	xSemaphoreGive( Semaphore_4g );
	return ret;		
	
	
}

//访问2052接口，专门给air724_4g_http_get_firmware调用
//返回0表示访问固件成功
static char air724_4g_http_get_2052(int BanBenHaoId,char * IsSWJ,int CiShu)
{
	char ret;
//	at_4g_send_at("AT+HTTPTERM\r\n","OK");

//	ret=at_4g_send_at("AT+HTTPINIT\r\n","OK");
//	if(ret!=AT_OK)
//	{
//		ret=1;
//		goto EXIT;
//	}	
//	ret=at_4g_send_at("AT+HTTPPARA=\"CID\",1\r\n","OK");
//	if(ret!=AT_OK)
//	{
//		ret=2;
//		goto EXIT;
//	}	
	
	char URL_BUF[300]={0};              //http://8.134.112.231:8099/ShuJu/QingQiu?JKName=2052&ID=0&SBBH=BFEBFBFF00030679&BanBenHaoId=0&IsSWJ=false&CiShu=2       
	sprintf(URL_BUF,"AT+HTTPPARA=\"URL\",\"http://%s:%d%s?JKName=2052&ID=0&SBBH=%s&BanBenHaoId=%d&IsSWJ=%s&CiShu=%d\"\r\n",SERVER_IP,SERVER_PORT,URL,http_key.SBBH,BanBenHaoId,IsSWJ,CiShu);
	ret=at_4g_send_at(URL_BUF,"OK");
	if(ret!=AT_OK)
	{
		ret=3;
		goto EXIT;
	}	
	ret=at_4g_send_at("AT+HTTPACTION=0\r\n","+HTTPACTION: 0,200,");
	if(ret!=AT_OK)
	{
		ret=4;
		goto EXIT;
	}
	ret=at_4g_send_at("AT+HTTPREAD\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=5;
		goto EXIT;
	}		
	
	MY_DEBUG_TX( air724_4g_rev.RevBuf, air724_4g_rev.BufLen );//输出响应信息
	
	if(NULL==strstr((const char *)air724_4g_rev.RevBuf,"\"ChengGong\":true"))
	{
		MY_PRINT("不需要更新/\r\n");
		ret=6;
		goto EXIT;		
	}

	ret=0;
	EXIT:
	//at_4g_send_at("AT+HTTPTERM\r\n","OK");
	return ret;		
}


//获取固件-2052
//BanBenHaoId传当前的上/下位机版本ID
//升级上位机传"true"，下位机传"false"
//filename 得到的固件的文件名
char air724_4g_http_get_firmware(int BanBenHaoId,char * IsSWJ,char *filename)
{
	xSemaphoreTake( Semaphore_4g, portMAX_DELAY );	
	char ret;
	at_4g_send_at("AT+HTTPTERM\r\n","OK");

	ret=at_4g_send_at("AT+HTTPINIT\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=1;
		goto EXIT;
	}	
	ret=at_4g_send_at("AT+HTTPPARA=\"CID\",1\r\n","OK");
	if(ret!=AT_OK)
	{
		ret=2;
		goto EXIT;
	}		
	
	
		char file[30]={"b64_"};
		if(0==strcmp(IsSWJ,"true")){
			strcat(file,"485fw.bin");
		}
		else if(0==strcmp(IsSWJ,"false")){
			strcat(file,"232fw.bin");
		}
		if(NULL!=filename){
			strcpy(filename,file);
		}

	
	int CiShu=1;
	int firmware_size=0;
	while(1)
	{
		if(0==air724_4g_http_get_2052(BanBenHaoId,IsSWJ,CiShu))
		{
			//访问成功，解析"NeiRomg",,没有了是这样{"ChengGong":true,"ShuJu":"[{\"FileName\":\"app_CRC.bin\",\"NeiRomg\":\"\"}]"}
			if(CiShu==1)//第一笔时候创建文件
			{
				MY_PRINT("\r\nFileName：%s\r\n",file);
				//SD卡创建该文件
				if(0!=sd_create_firmware_file((const char *)file))
				{
					ret=2;
					goto EXIT;		
				}			
			}

			char*p1=strstr((const char *)air724_4g_rev.RevBuf,"NeiRomg")+strlen("NeiRomg")+5;
			char*p2=strstr((const char *)p1,"\\\"}]\"}");
			if(p2>p1)
			{
				firmware_size+=p2-p1;
				MY_DEBUG_TX( (unsigned char *)p1, p2-p1 );		
				//写入sd卡
				if(0!=sd_write_firmware_file((const char *)file,(uint8_t*)p1,p2-p1))
				{
					ret=3;
					goto EXIT;			
				}				
			}
			else
			{
				//读取完毕
				MY_PRINT("\r\n读取次数：%d\r\n",CiShu);
				MY_PRINT("\r\n固件总大小(base64)：%d\r\n",firmware_size);
				ret=0;//返回成功
				goto EXIT;	
			}
			CiShu++;//读取下一笔
			vTaskDelay(200);			
		}		
		else
		{
			MY_PRINT("不需要更新\r\n");
			ret=1;//访问false
			goto EXIT;		
		}
		
	}
	ret=0;
	EXIT:
	at_4g_send_at("AT+HTTPTERM\r\n","OK");
	xSemaphoreGive( Semaphore_4g );
	return ret;		
}



#include "my_tasks.h"
TYPE_4G_STA _4g_Sta={0};
void network_status_check_4g_Task(void *pvParameters)
{

	MY_PRINT("network_status_check_4g_Task coming!!\r\n");
	
	network_4g(true);
	
	uint8_t ret=0;
	
	int report_BanBenID;
	int report_XBanBenID;
	int o_BanBenID=-1;//服务器上的固件版本号 -1表示不需要更新
	int o_XBanBenID=-1;//服务器上的固件版本号
	//vTaskSuspend(NULL);//挂起自己
	int cnt=0;
	char Flag=0;
	_4g_Sta.sta=0;
	while(1)
	{
		if(99<=air724_4g_network_check())
		{
			_4g_Sta.sta=0;
			Flag=0;
			MY_PRINT("\r\n4g: 4g network fail\r\n");
		}
		else
		{//设备有网
			_4g_Sta.sta=3;
			MY_PRINT("\r\n4g: 4g network ok\r\n");
			
			if(flagNetWorkDev==DEV_4G)
			{
				cnt=0;//目前测试，关闭固件检测
				if(++cnt>=10*15)//4S*15检测一次服务器版本
				{	
					//again:					
					cnt=0;
					if(FW485ID==-1)
						report_BanBenID=http_key.BanBenID;
					else
						report_BanBenID=FW485ID;
					if(FW232ID==-1)
						report_XBanBenID=http_key.XBanBenID;
					else
						report_XBanBenID=FW232ID;
					ret=air724_4g_http_check_firmware(report_BanBenID,report_XBanBenID,&o_BanBenID,&o_XBanBenID);		
					if(ret==0)
					{
						//_4g_Sta.sta=3;//访问服务器正常
						if(o_BanBenID!=-1)//上位机需要更新
						{
							MY_PRINT("\r\n4G 上位机需要更新\r\n");
							xEventGroupSetBits(firmware_event_group, GET_NEW_485FIRMWARE_FROM_SERVER);
							xTaskNotify( server_data_process_TaskHandle, o_BanBenID,eSetValueWithOverwrite); //覆盖写入
							
						}
						else if(o_XBanBenID!=-1)//下位机需要更新
						{			
							MY_PRINT("\r\n4G 下位机需要更新\r\n");
							xEventGroupSetBits(firmware_event_group, GET_NEW_232FIRMWARE_FROM_SERVER);
							xTaskNotify( server_data_process_TaskHandle, o_XBanBenID,eSetValueWithOverwrite); //覆盖写入				

						}						
					}		
					else
					{
						//_4g_Sta.sta=2;//4G网络正常，访问服务器失败
						MY_PRINT("\r\n4G网络正常，访问服务器失败\r\n");
						//vTaskDelay(4000);//4秒
						//goto again;
						Flag=0;
					}
				}				
			}
			
		}		
		vTaskDelay(6000);//4秒检测一次		
		
	}
}

//void network_status_check_4g_Task(void *pvParameters)
//{

//	MY_PRINT("network_status_check_4g_Task coming!!\r\n");
//	uint8_t ret=0;
//	
//	int report_BanBenID;
//	int report_XBanBenID;
//		int o_BanBenID=-1;//服务器上的固件版本号 -1表示不需要更新
//		int o_XBanBenID=-1;//服务器上的固件版本号
//	//vTaskSuspend(NULL);//挂起自己
//	char cnt=0;
//	while(1)
//	{
//				
//		//如果使用4G,不断检测物联网卡的网络情况
//			//ret=air724_4g_connect_to_http_server();
//			if(FW485ID==-1)
//				report_BanBenID=http_key.BanBenID;
//			else
//				report_BanBenID=FW485ID;
//			if(FW232ID==-1)
//				report_XBanBenID=http_key.XBanBenID;
//			else
//				report_XBanBenID=FW232ID;
//			ret=air724_4g_http_check_firmware(report_BanBenID,report_XBanBenID,&o_BanBenID,&o_XBanBenID);
//			if(ret==0)
//			{
//				_4g_Sta.sta=1;
//				//LED1_ON;LED2_ON;
//				MY_PRINT("\r\n4g: online\r\n");
//				if(flagNetWorkDev==DEV_4G)
//				{
//					if(++cnt>=4)
//					{
//						cnt=0;
//						if(o_BanBenID!=-1)//上位机需要更新
//						{
//							MY_PRINT("\r\n4G 上位机需要更新\r\n");
//							xEventGroupSetBits(firmware_event_group, GET_NEW_485FIRMWARE_FROM_SERVER);
//							xTaskNotify( server_data_process_TaskHandle, o_BanBenID,eSetValueWithOverwrite); //覆盖写入
//							
//						}
//						if(o_XBanBenID!=-1)//下位机需要更新
//						{			
//							MY_PRINT("\r\n4G 下位机需要更新\r\n");
//							xEventGroupSetBits(firmware_event_group, GET_NEW_232FIRMWARE_FROM_SERVER);
//							xTaskNotify( server_data_process_TaskHandle, o_XBanBenID,eSetValueWithOverwrite); //覆盖写入				

//						}					
//					}					
//				}


//			}
//			else
//			{
//				_4g_Sta.sta=0;
//				//LED2_OFF;
//				if(99<=air724_4g_network_check())
//				{
//					//LED1_OFF;
//					MY_PRINT("\r\n4g: 4g offline\r\n");
//				}
//				else{//设备有网，服务器失联
//					//LED1_ON;
//					MY_PRINT("\r\n4g: server disconnected\r\n");
//				}
//					
//			}
//		vTaskDelay(7000);//7秒检测一次		
//	}
//}



