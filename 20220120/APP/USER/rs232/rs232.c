/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rs232.h"
#include "crc16.h"
//#include "FreeRTOS.h"//这个头文件必须在task.h之前被包含进来，否则报错，
//#include "task.h"
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
   1   2               3               4 5             N             8+N+1    8+N+2       8+N+3
帧头（2Byte）			指令码(1Byte)		数据长度（2Byte）		数据(nByte)		CRC16校验码(2Byte)		帧尾（2Byte）	
0xA5	0x5A				 L	H			         L	H	                               L	H	               0x0D 0x0A                   
*/

#define RS232_FRAME_HEAD1              0xA5                               //帧头1定义，0xA5
#define RS232_FRAME_HEAD2              0x5A                               //帧头2定义，0x5A

#define RS232_INDEX_FRAME_HEAD1         (0)
#define RS232_LENG_FRAME_HEAD1          (1)                                 //帧头1占用字节数量

#define RS232_INDEX_FRAME_HEAD2         (1)
#define RS232_LENG_FRAME_HEAD2          (1)                                  //帧头2占用字节数量

#define RS232_INDEX_COMMAND             (2)                                 //帧中存放指令码的下标索引
#define RS232_LENG_COMMAND              (1)                                 //指令码占用字节数量

#define RS232_INDEX_DATA_LENG           (3)                   //帧中存放指示数据长度的下标索引
#define RS232_LENG_DATA_LENG            (2)                                         //数据长度占用的字节数量

#define RS232_INDEX_DATA                (5)            //帧中存放数据域的下标索引

#define RS232_LENG_CHECK_LENG           (2)                               //校验数据占用的字节数量

#define RS232_FRAME_END1                0x0D                             //帧尾定义,0x0D
#define RS232_FRAME_END2                0x0A                             //帧尾定义,0x0A                                    






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
static char rs232_check_rev_data(uint8_t *frame,  uint16_t frameLen);
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
* Function: RS232_UARTConfig
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
void rs232_uart_config(uint32_t bond)
{
//	//GPIO配置
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

	//USART3配置
	RCC_APB1PeriphClockCmd ( RCC_APB1Periph_USART3,ENABLE ) ;
	
	USART_InitTypeDef   USART_InitStruct ;
	USART_InitStruct.USART_BaudRate=bond;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b ;
	USART_InitStruct.USART_StopBits=USART_StopBits_1 ;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init  ( RS232_UART,&USART_InitStruct) ;
	
	USART_ITConfig( RS232_UART,  USART_IT_RXNE,ENABLE) ;
	USART_ITConfig( RS232_UART,  USART_IT_IDLE,ENABLE) ;

	//配置NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=RS232_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( RS232_UART,ENABLE) ;
}

uint8_t flag232CeShiJieGuo=0;//来自232的某通道测试结果数据
uint8_t flag232GetVer=0;//来自232的版本信息
uint8_t flag232UpDate=0;//来自232的更新固件请求
uint8_t flag232_3ack=0;//来自232的连续3次应答

TYPE_RS232_U RS232rev={0};
/*
****************************************************************************************
* Function: RS232_UART_IRQHandler
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
void RS232_UART_IRQHandler(void)
{
	uint8_t data;
	if(RS232_UART->SR & (0X01<<5))  //接收中断
	{
		RS232rev.RevBuf[RS232rev.RevLen]=RS232_UART->DR;
		RS232rev.RevLen++;

	}
	else if(RS232_UART->SR & (0X01<<4))//空闲中断
	{
		data=RS232_UART->SR;
		data=RS232_UART->DR;
	
		RS232rev.BufLen=RS232rev.RevLen;
		RS232rev.RevLen=0;
		RS232rev.RevOver=1;
		//判断是否是主动传输的传感器数据包
		if(RS232rev.RevBuf[RS232_INDEX_FRAME_HEAD1]==RS232_FRAME_HEAD1 && RS232rev.RevBuf[RS232_INDEX_FRAME_HEAD2]==RS232_FRAME_HEAD2)
		{
			if(RS232rev.RevBuf[RS232rev.BufLen-2]==RS232_FRAME_END1 && RS232rev.RevBuf[RS232rev.BufLen-1]==RS232_FRAME_END2)
			{
				//命令01 ~ 08
				if(RS232rev.RevBuf[RS232_INDEX_COMMAND]>=0x01 && RS232rev.RevBuf[RS232_INDEX_COMMAND]<=0x08 && *(uint16_t*)&RS232rev.RevBuf[RS232_INDEX_DATA_LENG]>0)
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//校验成功
					{
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
						flag232CeShiJieGuo=1;
					}
					else
					{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					
					
				}	
				/*命令09*/
				else if(RS232rev.RevBuf[RS232_INDEX_COMMAND]==0x09 && *(uint16_t*)&RS232rev.RevBuf[RS232_INDEX_DATA_LENG]>0)
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//校验成功
					{
						flag232GetVer=1;
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					else{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
				}
				/*命令0a*/
				else if(RS232rev.RevBuf[RS232_INDEX_COMMAND]==0x0a )
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//校验成功
					{
						flag232UpDate=1;//更新固件给232板子
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					else{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
				}
				/*命令0C*/
				else if(RS232rev.RevBuf[RS232_INDEX_COMMAND]==0x0C && *(uint16_t*)&RS232rev.RevBuf[RS232_INDEX_DATA_LENG]>0)
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//校验成功
					{
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					else{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
				}		
				else if(RS232rev.BufLen==0x21)
				{
					
					if(RS232rev.RevBuf[2]==0x0B && RS232rev.RevBuf[13]==0x0B &&  RS232rev.RevBuf[27]=='o'&&  RS232rev.RevBuf[28]=='k')
					{
						RS232rev.RevOver=0;
						flag232_3ack=1;
					}
				}
			}


		}



	} 

	
}

/*
****************************************************************************************
* Function: RS232_uart_tx_bytes
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
void rs232_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( RS232_UART, USART_FLAG_TXE ));
		USART_SendData(RS232_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}

//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------
void send_rs232_ok_ack(uint8_t cmd)
{
	uint8_t buf[11]={0xA5,0x5A,0x00,0x02,0x00,'o','k',0x00,0x00,0x0d,0x0a};
	buf[2]=cmd;
	uint16_t check=Get_Crc16(buf, 7);
	memcpy(&buf[7], (const void *)&check, 2);
	rs232_uart_tx_bytes( (uint8_t*)buf, 11 );
}

void send_rs232_err_ack(uint8_t cmd)
{
	uint8_t buf[12]={0xA5,0x5A,0x00,0x03,0x00,'e','r','r',0x00,0x00,0x0d,0x0a};
	buf[2]=cmd;
	uint16_t check=Get_Crc16(buf, 8);
	memcpy(&buf[8], (const void *)&check, 2);
	rs232_uart_tx_bytes( (uint8_t*)buf, 12 );
}

//A5 5A 09 00 00 AA C3 0D 0A 
char send_rs232_09cmd(int* XBanBenID)
{
	uint8_t buf[9]={0xA5,0x5A,0x09,0x00,0x00,0x00,0x00,0x0d,0x0a};
	uint16_t check=Get_Crc16(buf, 5);
	memcpy(&buf[5], (const void *)&check, 2);
	rs232_uart_tx_bytes( (uint8_t*)buf, 9 );	
	int cnt=0;
	while(0==flag232GetVer)
	{
		delay_microsecond(1000);
		if(++cnt>=500)
			return 1;
	}
	flag232GetVer=0;
	
	*XBanBenID=(RS232rev.RevBuf[RS232_INDEX_DATA+13]-'0')*10+RS232rev.RevBuf[RS232_INDEX_DATA+14]-'0';
	
	return 0;
}
void send_rs232_0Ccmd(void)
{
	uint8_t buf[9]={0xA5,0x5A,0x0C,0x00,0x00,0x00,0x00,0x0d,0x0a};
	uint16_t check=Get_Crc16(buf, 5);
	memcpy(&buf[5], (const void *)&check, 2);
	rs232_uart_tx_bytes( (uint8_t*)buf, 9 );	
	
	
}



//发送更新包数据下的数据格式
//发送状态0x00（数据发送中）0x01 发送完毕 0x02发送 校验码
static char send_rs2332_updata_firmware(uint8_t *buf,uint16_t length)
{
//	uint8_t buf[15+1024]={0};
//	buf[0]=0xA5;buf[1]=0x5A;
//	buf[2]=0x0b;//命令
//	*(uint16_t*)&buf[3]=length;
//	buf[5]=status;//发送状态
//	memcpy(&buf[6], (const void *)data, length);  
//	
//	uint16_t check=Get_Crc16(buf, 6+length);
//	buf[6+length]=check&0x0F;
//	buf[6+length+1]=(check&0xF0)>>8;

//	buf[6+length+2]=0x0d;
//	buf[6+length+3]=0x0a;
	
	int timeout=0;
	int cnt=0;
nx:	
	rs232_uart_tx_bytes( (uint8_t*)buf, length );	
	while(timeout<800)//等待响应
	{
		delay_microsecond(1000);//vTaskDelay(1);1ms
		timeout++;
		if(RS232rev.RevOver==1 || flag232_3ack==1)
		{
			break;
		}
	}	
	if(timeout>=800)//没有收到回应，重发
	{
		if(cnt>=9)
		{
			return 1;//发了10次都没有效应
		}
		cnt++;
		goto nx;
	}
	else if(timeout<800&&RS232rev.RevOver==1)//收到应答
	{
		RS232rev.RevOver=0;
		//校验应答
		if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) 
			    && RS232rev.RevBuf[5]=='o' && RS232rev.RevBuf[6]=='k')
		{
			//校验成功
			return 0;
		}
		else{
			//校验失败
			if(cnt>=9)
			{
				return 2;//发了10次还是校验失败
			}	
			cnt++;
			goto nx;
		}
			
	}	
	else if(timeout<800&&flag232_3ack==1)
	{
		//flag232_3ack=0;//在函数rs232_wait_updata_ok(void)清楚标志
		//收到连续3次应答
		return 0;
	}
	
}

//发送中
char send_rs2332_updata_firmware_data_ing(uint8_t *data,uint16_t length)
{
	if(length!=1024)
		return 8;
	
	uint8_t buf[10+1024]={0};
	buf[0]=0xA5;buf[1]=0x5A;
	buf[2]=0x0b;//命令
	*(uint16_t*)&buf[3]=length;
	buf[5]=0x00;//发送状态
	memcpy(&buf[6], (const void *)data, length);  
	
	uint16_t check=Get_Crc16(buf, 6+length);
	memcpy(&buf[6+length], (const void *)&check, 2);

	buf[6+length+2]=0x0d;
	buf[6+length+3]=0x0a;	
	
	return send_rs2332_updata_firmware(buf,10+length);
}
//发送最后一笔
char send_rs2332_updata_firmware_data_end(uint8_t *data,uint16_t length)
{
	int num=1024-length;//要补num个0xFF
	
	uint8_t buf[10+1024]={0};
	buf[0]=0xA5;buf[1]=0x5A;
	buf[2]=0x0b;//命令
	*(uint16_t*)&buf[3]=length;
	buf[5]=0x01;//发送状态
	memcpy(&buf[6], (const void *)data, length);  
	memset(&buf[6+length], 0xFF, num);  
	
	uint16_t check=Get_Crc16(buf, 6+length+num);
	memcpy(&buf[6+length+num], (const void *)&check, 2);

	buf[6+length+num+2]=0x0d;
	buf[6+length+num+3]=0x0a;	
	
	return send_rs2332_updata_firmware(buf,10+length+num);
}
//发送固件大小
char send_rs2332_updata_firmware_size(int firmware_size)
{
	uint8_t buf[10+4]={0};
	buf[0]=0xA5;buf[1]=0x5A;
	buf[2]=0x0b;//命令
	*(uint16_t*)&buf[3]=4;
	buf[5]=0x02;//发送状态
	*(int *)&buf[6]=firmware_size;
	uint16_t check=Get_Crc16(buf, 6+4);
	memcpy(&buf[6+4], (const void *)&check, 2);

	buf[6+4+2]=0x0d;
	buf[6+4+3]=0x0a;	
	
	
	return send_rs2332_updata_firmware(buf,10+4);
}

char rs232_wait_updata_ok(void)
{
	if(flag232_3ack==1)
	{
		flag232_3ack=0;
		return 0;
	}
	
	int timeout=0;
	int cnt=0;	
	while(1)
	{
		delay_microsecond(1000);//vTaskDelay(1);1ms
		if(++timeout>=1000*10)//等待10ms
		{
			return 1;//超时
		}
		if(flag232_3ack==1)
		{
			return 0;
		}
		
	}

	

}





static char rs232_check_rev_data(uint8_t *frame,  uint16_t frameLen)
{
	//计算校验值
	uint16_t check=Get_Crc16(frame, frameLen-4);
	
	uint16_t rev_check=*(uint16_t *)&frame[frameLen-4];

	if(check==rev_check)
		return 0;//校验成功
	else 
		return 1;
	
}


//o_data：传递项目头节点进来
//返回本项目的末尾指针
static char* rs232_data_xiangmu_prase(char *Pstart,struct ceshixiangmu **o_data)
{
	struct ceshixiangmu *tail=NULL;//指向最后一个节点(当前需要新增的项目节点)

	if(*o_data==NULL)//空链表第一个项目
	{
		*o_data=malloc(sizeof(struct ceshixiangmu));//  用完要释放
		memset(*o_data,0,sizeof(struct ceshixiangmu));
		tail=*o_data;//头节点就是尾巴节点
	}
	else//非空链表o_data是头节点，找到尾节点
	{
		struct ceshixiangmu *temp=malloc(sizeof(struct ceshixiangmu));//创建一个新节点  用完要释放
		memset(temp,0,sizeof(struct ceshixiangmu));
		//寻找最后一个节点
		tail=*o_data;//先把头定为尾巴
		while(1)
		{
			if(tail->next==NULL)//找到了尾节点
			{
				tail->next=temp;
				tail=tail->next;
				break;
			}
			else
			{
				tail=tail->next;
			}
		}
	}
	//--------------把解析到的项目数据传递到tail节点中--------------------------------------/
	char *p=NULL;
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return NULL;
	}	
	tail->TDNmae=malloc(p-Pstart-1+1);//用完要释放
	if(tail->TDNmae!=NULL)
	{
		memset(tail->TDNmae,0,p-Pstart-1+1);
		memcpy(tail->TDNmae,Pstart+1,p-Pstart-1);		
	}
	Pstart=p+1;	
	
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return NULL;
	}	
	tail->TDFanWei=malloc(p-Pstart-1+1);//用完要释放
	if(tail->TDFanWei!=NULL)
	{
		memset(tail->TDFanWei,0,p-Pstart-1+1);
		memcpy(tail->TDFanWei,Pstart+1,p-Pstart-1);		
	}
	Pstart=p+1;		
	
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return NULL;
	}		
	tail->TDValue=0;
	float xiaoshu=1;
	for(char i=0;i<p-Pstart-1;i++) 
	{
		if(Pstart[1+i]=='.')
		{
			xiaoshu=pow(10, p-Pstart-1  - i -1);
		}
		else
		{
			tail->TDValue =tail->TDValue*10+(Pstart[1+i]-'0');
		}
		
	}	
	tail->TDValue = tail->TDValue/xiaoshu;
	Pstart=p+1;	
	
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return NULL;
	}		
	tail->TDJieGuo=Pstart[1]-'0';
	Pstart=p+1;	
	
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return NULL;
	}		
	tail->TDXuHao=Pstart[1]-'0';
	Pstart=p+1;	
	
	tail->next=NULL;
	
	return Pstart;
}


//232通道测试结果数据解析
//返回0为解析成功，结果在参数RS232DataFrame中输出
char rs232_data_prase( uint8_t *i_frame,uint16_t i_frameLen,struct RS232DataFrame *o_data)
{
	
	//校验
	if( 0!=rs232_check_rev_data(i_frame,  i_frameLen) )
	{
		//校验失败
		return 1;
	}
	//解析出是哪一个通道的解释结果
	o_data->TongDaoNum=i_frame[RS232_INDEX_COMMAND];

	//数据内容起始地址
	char * Pstart=(char *)&i_frame[RS232_INDEX_DATA];
	//数据内容的字节长度
	uint16_t data_length=*(uint16_t *)&i_frame[RS232_INDEX_DATA_LENG];
	
	//char * Pstart=(char *)i_frame;//***
	
	uint8_t xuhao=0;
	char *p=NULL;
	//跳过序号1的数据内容
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
	xuhao++; //1
	Pstart=p+1;
	//---------------------------
	//跳过序号2的数据内容
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
//	if(p-Pstart-1>0){//p-Pstart-1为‘{’ ‘}’中间的数据长度
//		o_data->SheBeiBiaoShi=malloc(p-Pstart-1+1);//用完要释放
//		memset(o_data->SheBeiBiaoShi,0,p-Pstart-1+1);
//		memcpy(o_data->SheBeiBiaoShi,Pstart+1,p-Pstart-1);
//	}
//	else{
//		o_data->SheBeiBiaoShi=NULL;
//	}
	xuhao++;	//2
	Pstart=p+1;
	//---------------------------
	//跳过序号3的数据内容
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
//	if(p-Pstart-1>0){//p-Pstart-1为‘{’ ‘}’中间的数据长度
//		o_data->SBName=malloc(p-Pstart-1+1);//用完要释放
//		memset(o_data->SBName,0,p-Pstart-1+1);
//		memcpy(o_data->SBName,Pstart+1,p-Pstart-1);
//	}
//	else{
//		o_data->SBName=NULL;
//	}	
	xuhao++;	//3
	Pstart=p+1;
	//---------------------------
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
	if(p-Pstart-1>0){//p-Pstart-1为‘{’ ‘}’中间的数据长度
		o_data->TDJiLuTime=malloc(p-Pstart-1+1);//用完要释放
		memset(o_data->TDJiLuTime,0,p-Pstart-1+1);
		memcpy(o_data->TDJiLuTime,Pstart+1,p-Pstart-1);
	}
	else{
		o_data->TDJiLuTime=NULL;
	}
	xuhao++;	//4
	Pstart=p+1;	
	//----------------------------
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
	if(p-Pstart-1>0){//p-Pstart-1为‘{’ ‘}’中间的数据长度
		o_data->TDJZh=malloc(p-Pstart-1+1);//用完要释放
		memset(o_data->TDJZh,0,p-Pstart-1+1);
		memcpy(o_data->TDJZh,Pstart+1,p-Pstart-1);
	}	
	else{
		o_data->TDJZh=NULL;
	}	
	xuhao++;	//5
	Pstart=p+1;		
	//----------------------------
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
	if(p-Pstart-1>0){//p-Pstart-1为‘{’ ‘}’中间的数据长度
		o_data->TDZongHeGe=malloc(p-Pstart-1+1);//用完要释放
		memset(o_data->TDZongHeGe,0,p-Pstart-1+1);
		memcpy(o_data->TDZongHeGe,Pstart+1,p-Pstart-1);
	}	
	else{
		o_data->TDZongHeGe=NULL;
	}	
	xuhao++;	//6
	Pstart=p+1;	
	//----------------------------
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}		
	o_data->TDCiShu=0;
	for(char i=0;i<p-Pstart-1;i++)
	{
		o_data->TDCiShu =o_data->TDCiShu*10+(Pstart[1+i]-'0');
	}
	xuhao++;	//7
	Pstart=p+1;	
	//----------------------------
	//项目
	uint8_t num=0;
	while(1)
	{
		if(*Pstart=='{')
		{
			Pstart=rs232_data_xiangmu_prase(Pstart,&o_data->item);
		}
		else
			break;
	}
  return 0;
}


void myprintf(struct RS232DataFrame *data)
{
	MY_PRINT("--------------------------------------------------\r\n");
	MY_PRINT("TongDaoNum:%d\r\n",data->TongDaoNum);
	
	MY_PRINT("TDJiLuTime:%s\r\n",data->TDJiLuTime);
	
	MY_PRINT("TDJZh:%s\r\n",data->TDJZh);
	
	MY_PRINT("TDZongHeGe:%s\r\n",data->TDZongHeGe);
	
	MY_PRINT("TDCiShu:%d\r\n",data->TDCiShu);
	MY_PRINT("--------------------------------------------------\r\n");
	struct ceshixiangmu *temp=data->item;
	while(1)
	{
		if(temp==NULL)
		{
			break;
		}
		else
		{
			MY_PRINT("TDXuHao:%d\r\n",temp->TDXuHao);
			MY_PRINT("TDNmae:%s\r\n",temp->TDNmae);
			MY_PRINT("TDFanWei:%s\r\n",temp->TDFanWei);
			MY_PRINT("TDValue:%.1f\r\n",temp->TDValue);
			MY_PRINT("TDJieGuo:%d\r\n",temp->TDJieGuo);
			MY_PRINT("--------------------------------------------------\r\n");
			temp=temp->next;
		}
	}
	
}

void free_rs232_data(struct RS232DataFrame *data)
{
	free(data->TDJiLuTime);data->TDJiLuTime=NULL;
	free(data->TDJZh);data->TDJZh=NULL;
	free(data->TDZongHeGe);data->TDZongHeGe=NULL;
	
	struct ceshixiangmu *head=data->item;
	struct ceshixiangmu *temp=NULL;
	while(1)
	{
		if(head==NULL)
		{
			break;
		}
		else
		{
			temp=head;//表头存储
			head=temp->next; //指向下一个节点
			//释放表头
			free(temp->TDNmae);temp->TDNmae=NULL;
			free(temp->TDFanWei);temp->TDFanWei=NULL;
			free(temp);temp=NULL;
			
		}
	}
	
}

//void free_rs232_data(struct RS232DataFrame *data)
//{
//	free(data->TDJiLuTime);data->TDJiLuTime=NULL;
//	free(data->TDJZh);data->TDJZh=NULL;
//	free(data->TDZongHeGe);data->TDZongHeGe=NULL;
//	
//	struct ceshixiangmu **head=&data->item;
//	struct ceshixiangmu **temp=NULL;
//	while(1)
//	{
//		if(*head==NULL)
//		{
//			break;
//		}
//		else
//		{
//			temp=head;
//			free((*temp)->TDNmae);
//			(*temp)->TDNmae=NULL;
//			free((*temp)->TDFanWei);(*temp)->TDFanWei=NULL;
//			*head=(*temp)->next;
//			free(*temp);*temp=NULL;
//			
//		}
//	}
//	
//}




