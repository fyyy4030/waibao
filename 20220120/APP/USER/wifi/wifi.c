

/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "wifi.h"
#include "stdio.h"
#include "string.h"
#include "debug_uart.h"
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
char wifi_get_sta_cnt=100;

#define MODE_NORMAL 0
#define MODE_TOUCHUAN 1
char flagWifiMode=MODE_NORMAL;  
/*
****************************************************************************************
* Function: wifi_uartConfig
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
void wifi_uartConfig(uint32_t bond)
{
	//GPIO配置
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOD, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//配置参数结构体
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_5 | GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOD,  GPIO_PinSource5,  GPIO_AF_USART2 );
	GPIO_PinAFConfig (GPIOD,  GPIO_PinSource6,  GPIO_AF_USART2 );
	GPIO_PinLockConfig(GPIOD,GPIO_Pin_5 | GPIO_Pin_6);
	
	//USART2配置
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART2,ENABLE ) ; ;
	
	USART_InitTypeDef   USART_InitStruct ;
	USART_InitStruct.USART_BaudRate=bond;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b ;
	USART_InitStruct.USART_StopBits=USART_StopBits_1 ;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init  ( WIFI_UART,&USART_InitStruct) ;
	
	USART_ITConfig( WIFI_UART,  USART_IT_RXNE,ENABLE) ;
	USART_ITConfig( WIFI_UART,  USART_IT_IDLE,ENABLE) ;

	//配置NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=WIFI_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( WIFI_UART,ENABLE) ;
	
}
 TYPE_WIFI_U wifi_rev={0}; 
/*
****************************************************************************************
* Function: WIFI_UART_IRQHandler
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
void WIFI_UART_IRQHandler(void)
{
	uint8_t data;
	if(WIFI_UART->SR & (0X01<<5))  //接收中断
	{
		wifi_rev.RevBuf[wifi_rev.RevLen]=WIFI_UART->DR;
		wifi_rev.RevLen++;
		if(wifi_rev.RevLen>WIFI_BUF_MAX)
				wifi_rev.RevLen=WIFI_BUF_MAX;	
	}
	else if(WIFI_UART->SR & (0X01<<4))//空闲中断
	{
		data=WIFI_UART->SR;
		data=WIFI_UART->DR;
	
		wifi_rev.BufLen=wifi_rev.RevLen;
		wifi_rev.RevLen=0;
		wifi_rev.RevOver=1;
		//MY_DEBUG_TX( wifi_rev.RevBuf, wifi_rev.BufLen );
	}  

}

/*
****************************************************************************************
* Function: wifi_tx_bytes
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
void wifi_tx_bytes( uint8_t* TxBuffer, uint16_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( WIFI_UART, USART_FLAG_TXE ));
		USART_SendData(WIFI_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}

static void wifi_tx_string( uint8_t* TxBuffer)
{
	while(*TxBuffer!='\0')
	{
		while( RESET == USART_GetFlagStatus( WIFI_UART, USART_FLAG_TXE ));
		USART_SendData(WIFI_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}




//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
//#########################################################################################################
#include "debug_uart.h"
bool wifi_Flag_TcpIp_Connected=false;
bool wifi_Flag_Ap_Connected=false;

extern SemaphoreHandle_t Semaphore_wifi;

static char wifi_send_data(uint8_t *send_buf,uint16_t Length)
{
	memset(wifi_rev.RevBuf,0,sizeof(wifi_rev.RevBuf));//清空接受缓冲区
	wifi_tx_bytes( send_buf,Length );
}

//函数功能：发送指定AT指令给ESP8266
//参数说明：send_buf指向待发送的AT指令  
//返回值：0 收到的字符串中有期望接受到的字符串，  1没有  
//注意事项：1.指令必须以/r/n结束
//时间：2018/11/11
//作者：weihao Mo
static char wifi_send_cmd(uint8_t *send_buf)
{
	memset(wifi_rev.RevBuf,0,sizeof(wifi_rev.RevBuf));//清空接受缓冲区
	wifi_tx_string(send_buf);	
}






//函数功能：超时等待期望接收的信息
//参数说明： respond_buf指向期望接收的信息  timeout等待超时时间(ms)
//返回值：0 收到的字符串中有期望接受到的字符串，  1没有  
//注意事项：
//时间：2018/11/11
//作者：weihao Mo
static char wifi_wait_for_respoend(uint16_t timeout,uint8_t* num,uint8_t CountOfParameter, ...)
{
  if(CountOfParameter == 0)
     return 1;	
	
	va_list tag;
	va_start (tag, CountOfParameter);
	char *arg[CountOfParameter];
	for(uint8_t i = 0; i < CountOfParameter ; i++)
			arg[i] = va_arg (tag, char *);
	va_end (tag);
	
	
	uint16_t time=0;
	while(1)
	{
		while(0==wifi_rev.RevOver)//等待传输完成标志
		{
			vTaskDelay(1);//delay_microsecond(1000);//
			if(++time>timeout) //超时1S
			{
				return 1;
			}
		}
		wifi_rev.RevOver=0;//清标志
		//MY_DEBUG_TX( wifi_rev.RevBuf, wifi_rev.BufLen );
		for(char i=0;i<CountOfParameter;i++)
		{
			if(NULL!=strstr((const char *)wifi_rev.RevBuf,(const char *)arg[i]))
			{	
				if(NULL!=num)
					*num=i+1;
				
				return 0;
			}
		}

	}
}



//函数功能：退出透传模式
//参数说明：无
//返回值：无
//注意事项：无
//时间：2018/11/11
//作者：weihao Mo
void exit_pass_throughMode(void)
{
	while(1)
	{
		wifi_tx_string((uint8_t *)"+++");
		if(0==wifi_wait_for_respoend(500,NULL,1,(uint8_t *)"+++")){
			break;
		}		
	}
	vTaskDelay(100);
	//wifi_tx_string((uint8_t *)"+++");
	//wifi_tx_string((uint8_t *)"+++");
}

//char wifi_soft_reset(void)
//{
//	wifi_send_cmd((uint8_t *)"AT+RESTORE\r\n");
//		uint16_t time=0;
//	
//	while(1)
//	{
//		while(0==wifi_rev.RevOver)//等待传输完成标志
//		{
//			delay_microsecond(1000);//
//			if(++time>3000) //超时1S
//			{
//				return 1;
//			}
//		}
//		wifi_rev.RevOver=0;//清标志
//		//MY_DEBUG_TX( wifi_rev.RevBuf, wifi_rev.BufLen );
//		if(NULL!=strstr((const char *)wifi_rev.RevBuf,(const char *)"ready"))
//		{	
//			return 0;
//		}

//	}

//}

static char wifi_send_data_tcp(uint16_t dataLen, uint8_t *data)
{
	if(false==wifi_Flag_TcpIp_Connected)
		return 1;

	wifi_send_cmd((uint8_t *)"AT+CIPMODE=0\r\n");//普通传输模式
	if(0!=wifi_wait_for_respoend(5000,NULL,1,(uint8_t *)"OK")){
	//	wifi_init( );
		return 1;
	}	

	uint8_t buf[100]={0};
	sprintf((char *)buf, "AT+CIPSEND=%d\r\n", dataLen);
	
	wifi_send_cmd((uint8_t *)buf);
	if(0!=wifi_wait_for_respoend(5000,NULL,1,(uint8_t *)">")){
		wifi_init( );
		return 2;
	}
	
	wifi_send_data( data, dataLen );
	if(0!=wifi_wait_for_respoend(10000,NULL,1,(uint8_t *)"SEND OK")){
		return 3;
	}	
	
	return 0;
}


static char wifi_send_data_tcp_touchuan(uint16_t dataLen, uint8_t *data)
{
	if(false==wifi_Flag_TcpIp_Connected)
		return 1;
	
	wifi_send_cmd((uint8_t *)"AT+CIPMODE=1\r\n");//透传模式，发送完要收到推出透传
	if(0!=wifi_wait_for_respoend(5000,NULL,1,(uint8_t *)"OK")){
		//wifi_init( );
		return 1;
	}
	
	uint8_t buf[100]={0};
	sprintf((char *)buf, "AT+CIPSEND\r\n");//sprintf((char *)buf, "AT+CIPSEND=%d\r\n", dataLen);
	
	wifi_send_cmd((uint8_t *)buf);
	if(0!=wifi_wait_for_respoend(5000,NULL,1,(uint8_t *)">")){
		wifi_init( );
		return 2;
	}
	
	wifi_send_data( data, dataLen );
//	if(0!=wifi_wait_for_respoend(10000,NULL,1,(uint8_t *)"SEND OK")){
//		return 3;
//	}	
	
	return 0;
}



//已经是透传模式，直接发送
static char wifi_report_data_tcp(uint16_t dataLen, uint8_t *data)
{
	if(false==wifi_Flag_TcpIp_Connected)
		return 1;

	while(flagWifiMode!=MODE_TOUCHUAN)//等待标志位变成透传模式
	{
		vTaskDelay(1);
	}
	wifi_send_data( data, dataLen );
//	if(0!=wifi_wait_for_respoend(10000,NULL,1,(uint8_t *)"SEND OK")){
//		return 3;
//	}	
	
	return 0;
}


static char wifi_mode_to_normal(void)
{
	if(false==wifi_Flag_TcpIp_Connected)
		return 1;
	if(flagWifiMode==MODE_TOUCHUAN)
	{
			exit_pass_throughMode( );
			wifi_send_cmd((uint8_t *)"AT+CIPMODE=0\r\n");//普通传输模式
			if(0!=wifi_wait_for_respoend(5000,NULL,2,(uint8_t *)"OK",(uint8_t *)"ERROR")){
				return 2;
			}			
		
	}
	flagWifiMode=MODE_NORMAL;
	return 0;	
}


static char wifi_mode_to_touchuan(void)
{
	if(false==wifi_Flag_TcpIp_Connected)
		return 1;
	
	if(flagWifiMode==MODE_NORMAL)
	{
		wifi_send_cmd((uint8_t *)"AT+CIPMODE=1\r\n");//透传模式，发送完要收到推出透传
		if(0!=wifi_wait_for_respoend(5000,NULL,1,(uint8_t *)"OK")){
			//wifi_init( );
			return 2;
		}
		
		uint8_t buf[100]={0};
		sprintf((char *)buf, "AT+CIPSEND\r\n");//sprintf((char *)buf, "AT+CIPSEND=%d\r\n", dataLen);
		
		wifi_send_cmd((uint8_t *)buf);
		if(0!=wifi_wait_for_respoend(5000,NULL,1,(uint8_t *)">")){
			//wifi_init( );
			return 3;
		}		
	}

	flagWifiMode=MODE_TOUCHUAN;
	return 0;
}

char wifi_init(void)
{
	exit_pass_throughMode( );
	again:	
	//if(0!=wifi_soft_reset( )) {return 1;}
	wifi_send_cmd((uint8_t *)"AT+RESTORE\r\n");
	if(0!=wifi_wait_for_respoend(3000,NULL,1,(uint8_t *)"ready"))
	{
		goto again;
	}
	
	wifi_send_cmd((uint8_t *)"AT+CWMODE=1\r\n");
	if(0!=wifi_wait_for_respoend(1000,NULL,1,(uint8_t *)"OK"))
		goto again;
	
	wifi_send_cmd((uint8_t *)"AT1\r\n");
	wifi_wait_for_respoend(1000,NULL,1,(uint8_t *)"OK","ERROR");
  
	flagWifiMode=MODE_NORMAL; //CIPMODE=0 
	return 0;
}


//函数功能：ESP8266连接网络
//参数说明：ssid：指向目标AP的SSID（WIFI名）   pwd：指向WIFI密码
//返回值：0：成功连接
//注意事项：1.WiFi模组工作模式为单STA模式
//时间：2018/11/11
//作者：weihao Mo
char wifi_connet_ap(uint8_t *ssid,uint8_t *pwd)
{
	xSemaphoreTake( Semaphore_wifi, portMAX_DELAY );
	
	wifi_mode_to_normal( );
	
	uint8_t buf[100]={"AT+CWJAP="};
	strcat((char *)buf,"\"");
	strcat((char *)buf,(const char *)ssid);
	strcat((char *)buf,"\",\"");
	strcat((char *)buf,(const char *)pwd);
	strcat((char *)buf,"\"\r\n");
	
	wifi_send_cmd((uint8_t *)buf);	
	char ret= wifi_wait_for_respoend(1000*15,NULL,1,(uint8_t *)"OK");
	if(ret==0)
	{
		wifi_Flag_Ap_Connected=true;
		//LED1_ON;
	}
	xSemaphoreGive( Semaphore_wifi );
	return ret;
}


//函数功能：ESP8266连接服务器
//参数说明：type:连接类型（TCP/UDP） remote_ip:服务器IP   remote_port：服务器端口
//返回值：0：成功连接
//注意事项：1.WiFi模组工作模式为单STA模式
//时间：2018/11/11
//作者：weihao Mo
 char wifi_connet_server(uint8_t* tppe,uint8_t *remote_ip,int remote_port)
{
	xSemaphoreTake( Semaphore_wifi, portMAX_DELAY );
	
	wifi_mode_to_normal( );

	char buf[100]={0};
	sprintf(buf,"AT+CIPSTART=\"%s\",\"%s\",%d\r\n",tppe,remote_ip,remote_port);
	
	wifi_send_cmd((uint8_t *)buf);
	char ret = wifi_wait_for_respoend(5000,NULL,3,(uint8_t *)"OK",(uint8_t *)"ALREADY CONNECTED",(uint8_t *)"ERROR");
	if(ret==0)
	{
		wifi_Flag_TcpIp_Connected=true;
	}
	xSemaphoreGive( Semaphore_wifi );
	return ret;
}

//上报数据给服务器（仅主机）
char wifi_http_data_report(char TongDao_num,uint8_t *TongDaon,int TDCiShu,uint8_t *TDJZh,uint8_t *TDNZongHeGe)
{
	xSemaphoreTake( Semaphore_wifi, portMAX_DELAY );
	//调用2050接口
	//uint8_t body[1024]={"POST /ShuJu/QingQiu HTTP/1.1\r\nHost: 8.134.112.231\r\nContent-Length: 635\r\nConnection: keep-alive\r\n\r\n"};
	
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
	if(TDNZongHeGe==NULL)
		strcat((char *)body,(const char *)" ");
	else
		strcat((char *)body,(const char *)TDNZongHeGe);	

	strcat((char *)body,(const char *)"&ZaXiang=\"");
	strcat((char *)body,(const char *)http_key.ZaXiang);
	strcat((char *)body,(const char *)"\"");
	
//	//char R_BUF[WIFI_BUF_MAX+50]={0};
//  char* R_BUF=malloc(WIFI_BUF_MAX+50);
//	memset(R_BUF,0,WIFI_BUF_MAX+50);
//	//sprintf(R_BUF,"POST /ShuJu/QingQiu HTTP/1.1\r\nHost: 8.134.112.231\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n%s",strlen((char *)body),body);
//	sprintf(R_BUF,"POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n%s",URL,SERVER_IP,strlen((char *)body),body);
	
	char H_BUF[200]={0};
	sprintf(H_BUF,"POST %s HTTP/1.1\r\nHost: %s\r\nContent-Length: %d\r\nConnection: keep-alive\r\n\r\n",URL,SERVER_IP,strlen((char *)body));
	int h_len=strlen(H_BUF);
	int body_len=strlen((char *)body);    
	for(int i=0;i<body_len;i++)
	{
		body[body_len+h_len-1-i]=body[body_len-1-i];
	}
	memcpy(body,H_BUF,h_len);
	
	
	MY_DEBUG_TX(body,strlen((char *)body));

	//char ret= wifi_send_data_tcp(strlen((char *)R_BUF), (uint8_t *)R_BUF);
	char ret= wifi_report_data_tcp(strlen((char *)body), (uint8_t *)body);//已经在透出模式下，直接发送
	if(ret==0)
	{
		MY_PRINT("\r\n#########################wifi####################################\r\n");
		if(0==wifi_wait_for_respoend(20000,NULL,1,(uint8_t *)"HTTP/1.1 200 OK"))//if(0==wifi_wait_for_respoend(20000,NULL,1,(uint8_t *)"+IPD"))
		{
			MY_DEBUG_TX( wifi_rev.RevBuf, wifi_rev.BufLen );
			if(strstr((const char *)wifi_rev.RevBuf,(const char *)"\"ChengGong\":true"))
			{
				MY_PRINT("\r\nwifi:数据更新succeed\r\n");
				wifi_get_sta_cnt=0;//检测WIFI状态标志清零，从头再计数
			}
			else
			{
				MY_PRINT("\r\nwifi:数据更新fail\r\n");
			}			
		}
		else 
		{
			ret=5;
		}
			
	}
	//exit_pass_throughMode( );
	//free(R_BUF);
	free(body);
	xSemaphoreGive( Semaphore_wifi );
	vTaskDelay(100);
	return ret;	
	
	
}
	



//检测固件版本是否需要更新（仅主机）
//BanBenID当前上位机版本号
//XBanBenID当前下位机版本号
//o_BanBenID 检测到服务器上的上位机版本号，不需要更新返回-1
//o_XBanBenID检测到服务器上的下位机版本号，不需要更新返回-1
char wifi_http_check_firmware(int BanBenID,int XBanBenID,int *o_BanBenID,int *o_XBanBenID)
{
	xSemaphoreTake( Semaphore_wifi, portMAX_DELAY );
	
	*o_BanBenID=*o_XBanBenID=-1;
	
	char R_BUF[1024]={0};
	sprintf(R_BUF,"GET %s?JKName=2051&ID=0&SBBH=%s&SWJType=1&BanBenID=%d&XBanBenID=%d HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n",
										                                                        URL,http_key.SBBH,BanBenID,XBanBenID,SERVER_IP);
	char ret= wifi_report_data_tcp(strlen((char *)R_BUF), (uint8_t *)R_BUF);
	if(ret==0)
	{
		MY_PRINT("\r\n#########################wifi####################################\r\n");
		if(0==wifi_wait_for_respoend(20000,NULL,1,(uint8_t *)"HTTP/1.1 200 OK"))//if(0==wifi_wait_for_respoend(20000,NULL,1,(uint8_t *)"+IPD"))
		{
			if(strstr((const char *)wifi_rev.RevBuf,(const char *)"\"ChengGong\":true"))
			{
				MY_PRINT("\r\n2051 获得服务器固件版本成功\r\n");
				MY_DEBUG_TX( wifi_rev.RevBuf, wifi_rev.BufLen );
				char *p=strstr((const char *)wifi_rev.RevBuf,"\"ShuJu\":");
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
		}		
	}	
	
	xSemaphoreGive( Semaphore_wifi );
	return ret;	
}

static char wifi_http_get_2052(int BanBenHaoId,char * IsSWJ,int CiShu)
{
	char R_BUF[1024]={0};	
	sprintf(R_BUF,"GET %s?JKName=2052&ID=0&SBBH=%s&BanBenHaoId=%d&IsSWJ=%s&CiShu=%d HTTP/1.1\r\nHost: %s\r\nConnection: keep-alive\r\n\r\n",
	                                                                       URL,http_key.SBBH,BanBenHaoId,IsSWJ,CiShu,SERVER_IP);
	char ret= wifi_report_data_tcp(strlen((char *)R_BUF), (uint8_t *)R_BUF);
	if(ret!=0)
	{
		return ret;
	}
	else
	{
		if(0==wifi_wait_for_respoend(20000,NULL,1,(uint8_t *)"HTTP/1.1 200 OK"))//if(0==wifi_wait_for_respoend(20000,NULL,1,(uint8_t *)"+IPD"))
		{
			MY_DEBUG_TX( (unsigned char *)wifi_rev.RevBuf, wifi_rev.BufLen );		
			if(NULL==strstr((const char *)wifi_rev.RevBuf,"\"ChengGong\":true"))
			{
				MY_PRINT("不需要更新/\r\n");
				return 8;
			}			
		}
		else
			return 7;
		
		
	}
	
	return 0;
}

//获取固件-2052
//BanBenHaoId传当前的上/下位机版本ID
//升级上位机传"true"，下位机传"false"
//filename 得到的固件的文件名
char wifi_http_get_firmware(int BanBenHaoId,char * IsSWJ,char *filename)
{
	xSemaphoreTake( Semaphore_wifi, portMAX_DELAY );

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
		
	char ret=0;
	int CiShu=1;
	int firmware_size=0;
	while(1)
	{
		if(0==wifi_http_get_2052(BanBenHaoId,IsSWJ,CiShu))
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
			char*p1=strstr((const char *)wifi_rev.RevBuf,"NeiRomg")+strlen("NeiRomg")+5;
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
	
	EXIT:
	xSemaphoreGive( Semaphore_wifi );
	return ret;	
}




/*
查询网络连接信息
<stat>： ESP8266 Station 接口的状态
2： ESP8266 Station 已连接 AP，获得 IP 地址
3： ESP8266 Station 已建?立 TCP 或 UDP 传输
4： ESP8266 Station 断开?网络连接
5： ESP8266 Station 未连接 AP
*/
bool  wifi_tcpip_get_connection_status(uint8_t *sta )
{
    uint8_t result;
    xSemaphoreTake( Semaphore_wifi, portMAX_DELAY );
	
	  wifi_mode_to_normal( );

		wifi_send_cmd((uint8_t *)"AT+CIPSTATUS\r\n");
		if(0!=wifi_wait_for_respoend(1000,NULL,2,(uint8_t *)"\r\nSTATUS:",(uint8_t *)"+CIPSTATUS:")) 
		{
			xSemaphoreGive( Semaphore_wifi );
			return false;
		}
		char *str=NULL;
		if(strstr((char *)wifi_rev.RevBuf, "+CIPSTATUS:"))
		{
			*sta=3;
		}
		else if(strstr((char *)wifi_rev.RevBuf, "STATUS:0"))
		{
			*sta=0;
		}			
		else if(strstr((char *)wifi_rev.RevBuf, "STATUS:1"))
		{
			*sta=1;
		}		
		else if(strstr((char *)wifi_rev.RevBuf, "STATUS:2"))
		{
			*sta=2;
		}
		else if(strstr((char *)wifi_rev.RevBuf, "STATUS:4"))
		{
			*sta=4;
		}
		else if(strstr((char *)wifi_rev.RevBuf, "STATUS:5"))
		{
			*sta=5;
		}		
		else
		{
			xSemaphoreGive( Semaphore_wifi );
			return false;
		}
		
		xSemaphoreGive( Semaphore_wifi );
    return true;
}





////#define   WIFISSID      "Moweihao-iPhone"
////#define   WIFIPWD       "123123123"
////#define   SERVER_IP     "8.134.112.231"   
////#define   SERVER_PORT   8099 
////#define   URL           "/ShuJu/QingQiu"
TYPE_WIFI_STA _Wifi_Sta={0};
void network_status_check_wifi_Task(void *pvParameters)
{
	char Flag=0;
	
	MY_PRINT("network_status_check_wifi_Task coming!!\r\n");
	wifi_init( );

	//vTaskResume(master_rev_485dev_data_handle_TaskHandle);//唤醒master_rev_485dev_data_handle_Task
	int report_BanBenID;
	int report_XBanBenID;
	int o_BanBenID=-1;//服务器上的固件版本号 -1表示不需要更新
	int o_XBanBenID=-1;//服务器上的固件版本号
	int cnt=0;
	while(1)
	{
		
			 //2： ESP8266 Station 已连接 AP，获得 IP 地址
       //3： ESP8266 Station 已建立 TCP 或 UDP 传输
       //4： ESP8266 Station 断开网络连接
       //5： ESP8266 Station 未连接 AP
		  if(wifi_get_sta_cnt==100||++wifi_get_sta_cnt>=30)
			{
				Check_NetSate:
				wifi_get_sta_cnt=0;
				
				wifi_tcpip_get_connection_status(&_Wifi_Sta.sta);
			
			

			if(_Wifi_Sta.sta == 2||_Wifi_Sta.sta == 4 ) // 断开与服务器连接，需要重新连接服务器
			{
				vTaskResume(network_status_check_4g_TaskHandle);//add 唤醒4G网络任务
				while(wifi_connet_server((uint8_t *)"TCP",(uint8_t *)SERVER_IP,SERVER_PORT))
				{
					MY_PRINT("\r\nwifi:wifi broken server\r\n");
					vTaskDelay(1000);
				}
				//_Wifi_Sta.sta = 3;
				goto Check_NetSate;               //重新跳转到网络检测部分，检测是否连接了有效AP
			}	
			else if(_Wifi_Sta.sta == 5 || _Wifi_Sta.sta == 0 ||_Wifi_Sta.sta == 1) // 断开AP连接，需要重新连接AP
			{
				vTaskResume(network_status_check_4g_TaskHandle);//add 唤醒4G网络任务
        while(wifi_connet_ap((uint8_t *)WIFISSID,(uint8_t *)WIFIPWD))
        {
          vTaskDelay(300);
					MY_PRINT("\r\nwifi:wifi broken ap\r\n");
        }		
				Flag=0;
				goto Check_NetSate; 
				//vTaskSuspend(network_status_check_4g_TaskHandle);//add 挂起4G网络任务
			}
			else if(_Wifi_Sta.sta == 3)
			{
				wifi_mode_to_touchuan( );//进入透传模式
				
				vTaskSuspend(network_status_check_4g_TaskHandle);//add 挂起4G网络任务
				MY_PRINT("\r\nwifi:wifi online\r\n");
				if(flagNetWorkDev==DEV_WIFI)
				{
					cnt=0;//目前测试，关闭固件检测
					if(++cnt>=60*15)//1S 60*15检测一次
					{
						cnt=0;
						//定期询问服务器的固件版本
						if(FW485ID==-1)
							report_BanBenID=http_key.BanBenID;
						else
							report_BanBenID=FW485ID;
						if(FW232ID==-1)
							report_XBanBenID=http_key.XBanBenID;
						else
							report_XBanBenID=FW232ID;
						if(0==wifi_http_check_firmware(report_BanBenID,report_XBanBenID,&o_BanBenID,&o_XBanBenID))
						{
							if(o_BanBenID!=-1)//上位机需要更新
							{
								MY_PRINT("\r\nWIFI 上位机需要更新\r\n");
								xEventGroupSetBits(firmware_event_group, GET_NEW_485FIRMWARE_FROM_SERVER);
								xTaskNotify( server_data_process_TaskHandle, o_BanBenID,eSetValueWithOverwrite); //覆盖写入
								
							}
							else if(o_XBanBenID!=-1)//下位机需要更新
							{			
								MY_PRINT("\r\nWIFI 下位机需要更新\r\n");
								xEventGroupSetBits(firmware_event_group, GET_NEW_232FIRMWARE_FROM_SERVER);
								xTaskNotify( server_data_process_TaskHandle, o_XBanBenID,eSetValueWithOverwrite); //覆盖写入				
							}						
						}
						cnt=0;
					}					
				}
			}
		}
		vTaskDelay(1000*1);//不能空循环
	}
}


