/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rs485.h"
#include "crc16.h"
#include "FreeRTOS.h"//这个头文件必须在task.h之前被包含进来，否则报错，
#include "task.h"
#include "my_print.h"
/*
****************************************************************************************
* CONSTANTS (常量定义)
****************************************************************************************
*/
uint8_t devID_master=0x00;//主机本机ID,固定0x00
uint8_t devID_salve=0x0A;//从机本机ID 0x02~0x1E
/*
****************************************************************************************
* TYPEDEFS (类型定义)
****************************************************************************************
*/
/*
   1   2               3               4             5             6               7 8                   N          8+N+1    8+N+2       8+N+3
帧头（2Byte）		发送方ID(1Byte)	接收方ID(1Byte)	指令码(1Byte)		序列号（1Byte）		数据长度（2Byte）		数据(nByte)		CRC16校验码(2Byte)		帧尾
0xA5	0x5A						                                                              	L	H			                           L	H	            0x88
*/

#define FRAME_HEAD1              0xA5                               //帧头1定义，0xA5
#define FRAME_HEAD2              0x5A                               //帧头2定义，0x5A

#define FRAME_MIN_LEN            11                                //不包含数据域最小需要11字节

#define INDEX_FRAME_HEAD1         (0)
#define LENG_FRAME_HEAD1          (1)                                 //帧头1占用字节数量

#define INDEX_FRAME_HEAD2         (INDEX_FRAME_HEAD1+LENG_FRAME_HEAD1)
#define LENG_FRAME_HEAD2          (1)                                  //帧头2占用字节数量

#define INDEX_SENDER_ID           (INDEX_FRAME_HEAD2+LENG_FRAME_HEAD2)  //帧中存放发送方ID的下标索引
#define LENG_SENDER_ID            (1)                                   //发送方ID占用字节数量

#define INDEX_RECEIVER_ID         (INDEX_SENDER_ID+LENG_SENDER_ID)        //帧中存放接收方ID的下标索引
#define LENG_RECEIVER_ID          (1)                                     //接收方ID占用字节数量

#define INDEX_COMMAND             (INDEX_RECEIVER_ID+LENG_RECEIVER_ID)    //帧中存放指令码的下标索引
#define LENG_COMMAND              (1)                                    //指令码占用字节数量

#define INDEX_SERIAL_NUMBER       (INDEX_COMMAND+LENG_COMMAND)         //帧中存放序列号的下标索引
#define LENG_SERIAL_NUMBER        (1)                                    //序列号占用字节数量


#define INDEX_DATA_LENG           (INDEX_SERIAL_NUMBER+LENG_SERIAL_NUMBER)     //帧中存放指示数据长度的下标索引
#define LENG_DATA_LENG            (2)                                         //数据长度占用的字节数量

#define INDEX_DATA                (INDEX_DATA_LENG+LENG_DATA_LENG)            //帧中存放数据域的下标索引

#define LENG_CHECK_LENG           (2)                               //校验数据占用的字节数量

#define FRAME_END                  0x88                             //帧尾定义,0x88
#define INDEX_FRAME_END                                        
#define LENG_FRAME_END            (1)                             //帧尾占用字节数量




uint8_t flagSensorDataCome=0;//来自485从机的传感器数据

uint8_t flagFirmwareStart=0;//主机给从机传输的固件开始传输标志
uint8_t flagFirmwareData=0;//主机给从机传输的固件内容标志
uint8_t flagFirmwareEnd=0;//主机给从机传输的固件传输完成标志
uint8_t flagInquiry=0;//主机向从机询问标志
uint8_t flagSlaveUpdateFW=0;//从机请求主机传输固件标志
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
* Function: RS485_UARTConfig
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
void rs485_uart_config(uint32_t bond)
{
	//GPIO配置
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOC, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//配置参数结构体
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource6,  GPIO_AF_USART6 );
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource7,  GPIO_AF_USART6 );
	GPIO_PinLockConfig(GPIOC,GPIO_Pin_6 | GPIO_Pin_7);
	
	//USART6配置
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_USART6,ENABLE ) ;
	
	USART_InitTypeDef   USART_InitStruct ;
	USART_InitStruct.USART_BaudRate=bond;
	USART_InitStruct.USART_WordLength=USART_WordLength_8b ;
	USART_InitStruct.USART_StopBits=USART_StopBits_1 ;
	USART_InitStruct.USART_Parity=USART_Parity_No;
	USART_InitStruct.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;
	USART_InitStruct.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_Init  ( RS485_UART,&USART_InitStruct) ;
	
	USART_ITConfig( RS485_UART,  USART_IT_RXNE,ENABLE) ;
	USART_ITConfig( RS485_UART,  USART_IT_IDLE,ENABLE) ;

	//配置NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=RS485_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//响应优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( RS485_UART,ENABLE) ;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIOC->ODR &=~(1<<2);//接收
}



TYPE_RS485_U RS485rev={0};
/*
****************************************************************************************
* Function: RS485_UART_IRQHandler
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
void RS485_UART_IRQHandler(void)
{
	uint8_t data;
	if(RS485_UART->SR & (0X01<<5))  //接收中断
	{
		RS485rev.RevBuf[RS485rev.RevLen]=RS485_UART->DR;
		RS485rev.RevLen++;

	}
	else if(RS485_UART->SR & (0X01<<4))//空闲中断
	{
		data=RS485_UART->SR;
		data=RS485_UART->DR;
	
		RS485rev.BufLen=RS485rev.RevLen;
		RS485rev.RevLen=0;
		RS485rev.RevOver=1;
		//判断是否是主动传输的传感器数据包
		if(flagDevType==MASTER)
		{
			if(RS485rev.RevBuf[INDEX_FRAME_HEAD1]==FRAME_HEAD1 && RS485rev.RevBuf[INDEX_FRAME_HEAD2]==FRAME_HEAD2)
			{
				if(RS485rev.RevBuf[INDEX_COMMAND]==0x04 && *(uint16_t*)&RS485rev.RevBuf[INDEX_DATA_LENG]>0)
				{
					RS485rev.RevOver=0;
					flagSensorDataCome=1;
				}
				else if(RS485rev.RevBuf[INDEX_COMMAND]==0x06 && *(uint16_t*)&RS485rev.RevBuf[INDEX_DATA_LENG]==1)
				{
					RS485rev.RevOver=0;
					flagSlaveUpdateFW=1;
				}
			}
		}
		else if(flagDevType==SALVER)
		{
			if(RS485rev.RevBuf[INDEX_FRAME_HEAD1]==FRAME_HEAD1 && RS485rev.RevBuf[INDEX_FRAME_HEAD2]==FRAME_HEAD2)
			{
				if(RS485rev.RevBuf[INDEX_COMMAND]==0x01 && *(uint16_t*)&RS485rev.RevBuf[INDEX_DATA_LENG]>0)
				{
					RS485rev.RevOver=0;
					flagFirmwareStart=1;
				}
				else if(RS485rev.RevBuf[INDEX_COMMAND]==0x02 && *(uint16_t*)&RS485rev.RevBuf[INDEX_DATA_LENG]>0)
				{
					RS485rev.RevOver=0;
					flagFirmwareData=1;				
				}
				else if(RS485rev.RevBuf[INDEX_COMMAND]==0x03 && *(uint16_t*)&RS485rev.RevBuf[INDEX_DATA_LENG]>0)
				{
					RS485rev.RevOver=0;
					flagFirmwareEnd=1;				
				}
				else if(RS485rev.RevBuf[INDEX_COMMAND]==0x05 && *(uint16_t*)&RS485rev.RevBuf[INDEX_DATA_LENG]==8)
				{
					RS485rev.RevOver=0;
					if(RS485rev.RevBuf[INDEX_RECEIVER_ID]==devID_salve)//接送方ID等于自己的ID表示询问的是自己
					{
						flagInquiry=1;	
					}
								
				}
			}
		}


	} 

	
}

/*
****************************************************************************************
* Function: RS485_uart_tx_bytes
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
void rs485_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length )
{
	while( Length-- )
	{
		while( RESET == USART_GetFlagStatus( RS485_UART, USART_FLAG_TXE ));
		USART_SendData(RS485_UART,  *TxBuffer ); 
		TxBuffer++;
	}
}


/*
****************************************************************************************
* Function: RS485_uart_tx_bytes
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
void send_rs485_commond(uint8_t *cmd,uint16_t len)
{
	GPIOC->ODR |=(1<<2);//SEND_RS485_MODE;
	delay_microsecond(3000);
	rs485_uart_tx_bytes( (uint8_t*)cmd, len );
	delay_microsecond(3000);
	GPIOC->ODR &=~(1<<2);//接收//REV_RS485_MODE;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------


//生成数据帧
//o_frame :输出参数,保存生成的数据帧字节;
//o_frameLen:输出参数,保存数据帧长度;
//i_frameBufSize:输入参数,保存数据帧的缓冲区长度;
//i_data:输入参数,要编码的原始数据;
//返回0为编码成功，结果在参数o_frame，o_frameLen中输出
static char packetFrame( uint8_t *o_frame,  uint16_t *o_frameLen,const uint16_t i_frameBufSize, const struct DataFrame *i_data)
{
	int vtmp = 0;
	unsigned short int index = 0;
	unsigned short int frameLen = FRAME_MIN_LEN + i_data->dataLength;        //计算当前帧总字节数量

	//判断缓冲区长度是否满足最小要求（算上结束标志）
	if(i_frameBufSize < frameLen) {
			MY_PRINT("缓冲区过小，生成帧失败!\r\n");
			return 1;
	}

	//清空缓冲区
	memset(o_frame, 0, frameLen);
		
  //填充数据域   （0、1是帧头）
	o_frame[index]=FRAME_HEAD1;
  index += LENG_FRAME_HEAD1;
	o_frame[index]=FRAME_HEAD2;
  index += LENG_FRAME_HEAD2;
	//发送方ID
  memcpy(&o_frame[index], (const void *)&i_data->sender_id, LENG_SENDER_ID);  
  index += LENG_SENDER_ID;	
	//接收方ID
	 memcpy(&o_frame[index], (const void *)&i_data->receiver_id, LENG_RECEIVER_ID);  
	index += LENG_RECEIVER_ID;	
	//命令字
	memcpy(&o_frame[index], (const void *)&i_data->commond, LENG_COMMAND);  
  index += LENG_COMMAND;
	//序列号
  memcpy(&o_frame[index], (const void *)&i_data->serial_number, LENG_SERIAL_NUMBER);  
	index += LENG_SERIAL_NUMBER;
	//数据长度
	memcpy(&o_frame[index], (const void *)&i_data->dataLength, LENG_DATA_LENG); 
  index += LENG_DATA_LENG;
	//数据
	memcpy(&o_frame[index], (const void *)i_data->data, i_data->dataLength);   
  index += i_data->dataLength;		
	//校验值
	uint16_t check=Get_Crc16(o_frame, index);
	memcpy(&o_frame[index], (const void *)&check, LENG_CHECK_LENG);  
  index += LENG_CHECK_LENG;

	//结束符
	o_frame[index]=FRAME_END;
  index += LENG_FRAME_END;

  *o_frameLen = index;
  return 0;
}

char check_rev_data(uint8_t *frame,  uint16_t frameLen)
{
	//计算校验值
	uint16_t check=Get_Crc16(frame, frameLen-3);
	
	uint16_t rev_check=*(uint16_t *)&frame[frameLen-3];

	if(check==rev_check)
		return 0;//校验成功
	else 
		return 1;
	
}


/*提示接收方将要进行固件升级，做好接收准备
sender_id发送方ID
receiver_id接收方ID
firmware_type 固件类型 (0x01：485设备 ; 0x02:232设备)
firmware_size 固件字节大小
返回0发送完成并且收到对方正确回应
*/
char send485_firmware_update_start(uint8_t sender_id,uint8_t receiver_id,uint8_t firmware_type,uint16_t firmware_size)
{
	int cnt=0;
	struct DataFrame data={0};
	
	data.sender_id=sender_id;
	data.receiver_id=receiver_id;
	data.commond=0x01;
	data.serial_number=0x00;
	data.dataLength=3;
	data.data[0]=firmware_type;
	memcpy(&data.data[1], (const void *)&firmware_size, 2);  

	uint8_t o_frame[1024]={0};
	uint16_t o_frameLen=0;	

	nx:	
	packetFrame( o_frame,  &o_frameLen,1024, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);
	while(cnt<1000)//等待响应
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}
	if(cnt>=1000)//没有收到回应，重发
	{
		if(data.serial_number>=2)
		{
			return 1;//发了3次都没有效应
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<1000&&RS485rev.RevOver==1)//收到应答
	{
		RS485rev.RevOver=0;
		
		//校验应答
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//校验成功
			return 0;
		}
		else{
			//校验失败，重发
			if(data.serial_number>=2)
			{
				return 2;//发了3次校验失败
			}	
			data.serial_number++;
			goto nx;
		}
			
	}
	
}
    #include "debug_uart.h"
/*固件传输，单次传输最多携带1000字节数据
sender_id发送方ID
receiver_id接收方ID
firmware_data 指向固件数据 
length 本次传输的固件数据长度
返回0发送完成并且收到对方正确回应
*/
char send485_firmware_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* firmware_data,uint16_t length)
{
	//单次传输最多携带1000字节数据
	if(length>MAX_DATA_LENGTH)
		return 3;//错误，数据过长
	
	int cnt=0;
	struct DataFrame data={0};
	data.sender_id=sender_id;
	data.receiver_id=receiver_id;
	data.commond=0x02;
	data.serial_number=0x00;
	data.dataLength=length;
	memcpy(&data.data[0], (const void *)firmware_data, length);  

	uint8_t o_frame[1024]={0};
	uint16_t o_frameLen=0;	
	nx:	
	packetFrame( o_frame,  &o_frameLen,1024, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);	
	while(cnt<1000)//等待响应
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}	
	if(cnt>=1000)//没有收到回应，重发
	{
		if(data.serial_number>=2)
		{
			return 1;//发了3次都没有效应
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<1000&&RS485rev.RevOver==1)//收到应答
	{
		RS485rev.RevOver=0;
		//校验应答
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//校验成功
			return 0;
		}
		else{
			//校验失败，重发
			if(data.serial_number>=2)
			{
				return 2;//发了3次校验失败
			}	
			data.serial_number++;
			goto nx;
		}
			
	}	
}

/*提示接收方固件传输完成
sender_id发送方ID
receiver_id接收方ID
firmware_type 固件类型 (0x01：232设备 ; 0x02:485设备)
trans_cnt 固件分传次数
返回0发送完成并且收到对方正确回应
*/
char send485_firmware_update_end(uint8_t sender_id,uint8_t receiver_id,uint16_t trans_cnt,int version)
{
	int cnt=0;
	struct DataFrame data={0};
	
	data.sender_id=sender_id;
	data.receiver_id=receiver_id;
	data.commond=0x03;
	data.serial_number=0x00;
	data.dataLength=2+4;
	memcpy(&data.data[0], (const void *)&trans_cnt, 2); 
  memcpy(&data.data[2], (const void *)&version, 4);  	

	uint8_t o_frame[1024]={0};
	uint16_t o_frameLen=0;	

	nx:	
	packetFrame( o_frame,  &o_frameLen,1024, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);
	while(cnt<1000)//等待响应
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}
	if(cnt>=1000)//没有收到回应，重发
	{
		if(data.serial_number>=2)
		{
			return 1;//发了3次都没有效应
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<1000&&RS485rev.RevOver==1)//收到应答
	{
		RS485rev.RevOver=0;
		//校验应答
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//校验成功
			return 0;
		}
		else{
			//校验失败，重发
			if(data.serial_number>=2)
			{
				return 2;//发了3次校验失败
			}	
			data.serial_number++;
			goto nx;
		}
			
	}
	
}

/*传输传感器采集数据
sender_id发送方ID
receiver_id接收方ID
sensor_data 指向传感器数据 
length 传感器数据长度
返回0发送完成并且收到对方正确回应
*/
char send485_sensor_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* sensor_data,uint16_t length)
{
	int cnt=0;
	struct DataFrame data={0};
	data.sender_id=sender_id;
	data.receiver_id=receiver_id;
	data.commond=0x04;
	data.serial_number=0x00;
	data.dataLength=length;
	memcpy(&data.data[0], (const void *)sensor_data, length);  

	uint8_t o_frame[1024]={0};
	uint16_t o_frameLen=0;	
	nx:	
	packetFrame( o_frame,  &o_frameLen,1024, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);	
	while(cnt<500)//等待响应
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}	
	if(cnt>=500)//没有收到回应，重发
	{
		if(data.serial_number>=2)
		{
			return 1;//发了3次都没有效应
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<500&&RS485rev.RevOver==1)//收到应答
	{
		RS485rev.RevOver=0;
		//校验应答
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//校验成功
			return 0;
		}
		else{
			//校验失败，重发
			if(data.serial_number>=2)
			{
				return 2;//发了3次校验失败
			}	
			data.serial_number++;
			goto nx;
		}
			
	}		
}

//主机向从机发送在线询问数据包
char send485_master_inquiry(uint8_t salve_id,int fw485ID,int fw232ID)
{
	int cnt=0;
	struct DataFrame data={0};
	data.sender_id=devID_master;
	data.receiver_id=salve_id;
	data.commond=0x05;
	data.serial_number=0x00;
	data.dataLength=8;
	*(int *)&data.data[0]=fw485ID;
	*(int *)&data.data[4]=fw232ID;
	
	uint8_t o_frame[25]={0};
	uint16_t o_frameLen=0;	
	//nx:	
	packetFrame( o_frame,  &o_frameLen,25, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);	
	while(cnt<400)//等待响应
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1 || flagSensorDataCome==1)//收到了应答或者收到了传感器数据
		{
			break;
		}
	}	
	if(cnt>=400)//没有收到回应
	{
		return 1;
	}
	else if(cnt<400)//收到应答
	{
		if(RS485rev.RevOver==1)
		{
			RS485rev.RevOver=0;
			//校验应答
			if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) )
				return 0;//校验成功
			else
				return 2;
		}
		
	}			
	
}
//从机告诉主机需要进行固件更新
//fw_type：0x01：232 固件; 0x02:485固件
char send485_frimware_update_notify(uint8_t sender_id,uint8_t receiver_id,uint8_t fw_type)
{
	int cnt=0;
	struct DataFrame data={0};
	data.sender_id=sender_id;
	data.receiver_id=receiver_id;
	data.commond=0x06;
	data.serial_number=0x00;
	data.dataLength=1;
	data.data[0]=fw_type;

	uint8_t o_frame[15]={0};
	uint16_t o_frameLen=0;	
	nx:	
	packetFrame( o_frame,  &o_frameLen,15, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);	
	while(cnt<500)//等待响应
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}	
	if(cnt>=500)//没有收到回应，重发
	{
		if(data.serial_number>=2)
		{
			return 1;//发了3次都没有效应
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<500&&RS485rev.RevOver==1)//收到应答
	{
		RS485rev.RevOver=0;
		//校验应答
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//校验成功
			return 0;
		}
		else{
			//校验失败，重发
			if(data.serial_number>=2)
			{
				return 2;//发了3次校验失败
			}	
			data.serial_number++;
			goto nx;
		}
			
	}		
	
	
	
}


/*发送响应*/
char send485_ack(uint8_t sender_id,uint8_t receiver_id,uint8_t cmd,uint8_t serial_number)
{
	struct DataFrame data={0};
	data.sender_id=sender_id;
	data.receiver_id=receiver_id;
	data.commond=cmd;
	data.serial_number=serial_number;
	data.dataLength=0;
	
	uint8_t o_frame[1024]={0};
	uint16_t o_frameLen=0;	

	packetFrame( o_frame,  &o_frameLen,1024, &data);
	send_rs485_commond((uint8_t *)o_frame,o_frameLen);		
	
	return 0;
}

/*发送错误反馈*/
char send485_ack_err(uint8_t sender_id,uint8_t receiver_id,uint8_t err_code)
{
	uint8_t frame[5]={0};
	frame[0]=0x87;
	frame[1]=0x78;
	frame[2]=sender_id;
	frame[3]=receiver_id;
	frame[4]=err_code;
	send_rs485_commond((uint8_t *)frame,5);		
	
	return 0;
}

/*
char send485_firmware(uint8_t sender_id,uint8_t receiver_id,uint8_t firmware_type)
{
	char ret;
	ret=send485_firmware_update_start(sender_id,receiver_id,firmware_type,firmware_size);
	if(ret!=0)
		return 1;
	while(1)
	{
		if(firmware_size/1000 > 0)
		{
			ret=send485_firmware_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* firmware_data,1000)
		}
	}
	
	
	ret=send485_firmware_update_end(uint8_t sender_id,uint8_t receiver_id,uint16_t trans_cnt)
}
*/









