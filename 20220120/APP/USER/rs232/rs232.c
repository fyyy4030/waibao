/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "math.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rs232.h"
#include "crc16.h"
//#include "FreeRTOS.h"//���ͷ�ļ�������task.h֮ǰ���������������򱨴�
//#include "task.h"
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
   1   2               3               4 5             N             8+N+1    8+N+2       8+N+3
֡ͷ��2Byte��			ָ����(1Byte)		���ݳ��ȣ�2Byte��		����(nByte)		CRC16У����(2Byte)		֡β��2Byte��	
0xA5	0x5A				 L	H			         L	H	                               L	H	               0x0D 0x0A                   
*/

#define RS232_FRAME_HEAD1              0xA5                               //֡ͷ1���壬0xA5
#define RS232_FRAME_HEAD2              0x5A                               //֡ͷ2���壬0x5A

#define RS232_INDEX_FRAME_HEAD1         (0)
#define RS232_LENG_FRAME_HEAD1          (1)                                 //֡ͷ1ռ���ֽ�����

#define RS232_INDEX_FRAME_HEAD2         (1)
#define RS232_LENG_FRAME_HEAD2          (1)                                  //֡ͷ2ռ���ֽ�����

#define RS232_INDEX_COMMAND             (2)                                 //֡�д��ָ������±�����
#define RS232_LENG_COMMAND              (1)                                 //ָ����ռ���ֽ�����

#define RS232_INDEX_DATA_LENG           (3)                   //֡�д��ָʾ���ݳ��ȵ��±�����
#define RS232_LENG_DATA_LENG            (2)                                         //���ݳ���ռ�õ��ֽ�����

#define RS232_INDEX_DATA                (5)            //֡�д����������±�����

#define RS232_LENG_CHECK_LENG           (2)                               //У������ռ�õ��ֽ�����

#define RS232_FRAME_END1                0x0D                             //֡β����,0x0D
#define RS232_FRAME_END2                0x0A                             //֡β����,0x0A                                    






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
static char rs232_check_rev_data(uint8_t *frame,  uint16_t frameLen);
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
//	//GPIO����
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOD, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//���ò����ṹ��
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOD, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOD,  GPIO_PinSource8,  GPIO_AF_USART3 );
	GPIO_PinAFConfig (GPIOD,  GPIO_PinSource9,  GPIO_AF_USART3 );

	//USART3����
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

	//����NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=RS232_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//��ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( RS232_UART,ENABLE) ;
}

uint8_t flag232CeShiJieGuo=0;//����232��ĳͨ�����Խ������
uint8_t flag232GetVer=0;//����232�İ汾��Ϣ
uint8_t flag232UpDate=0;//����232�ĸ��¹̼�����
uint8_t flag232_3ack=0;//����232������3��Ӧ��

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
	if(RS232_UART->SR & (0X01<<5))  //�����ж�
	{
		RS232rev.RevBuf[RS232rev.RevLen]=RS232_UART->DR;
		RS232rev.RevLen++;

	}
	else if(RS232_UART->SR & (0X01<<4))//�����ж�
	{
		data=RS232_UART->SR;
		data=RS232_UART->DR;
	
		RS232rev.BufLen=RS232rev.RevLen;
		RS232rev.RevLen=0;
		RS232rev.RevOver=1;
		//�ж��Ƿ�����������Ĵ��������ݰ�
		if(RS232rev.RevBuf[RS232_INDEX_FRAME_HEAD1]==RS232_FRAME_HEAD1 && RS232rev.RevBuf[RS232_INDEX_FRAME_HEAD2]==RS232_FRAME_HEAD2)
		{
			if(RS232rev.RevBuf[RS232rev.BufLen-2]==RS232_FRAME_END1 && RS232rev.RevBuf[RS232rev.BufLen-1]==RS232_FRAME_END2)
			{
				//����01 ~ 08
				if(RS232rev.RevBuf[RS232_INDEX_COMMAND]>=0x01 && RS232rev.RevBuf[RS232_INDEX_COMMAND]<=0x08 && *(uint16_t*)&RS232rev.RevBuf[RS232_INDEX_DATA_LENG]>0)
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//У��ɹ�
					{
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
						flag232CeShiJieGuo=1;
					}
					else
					{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					
					
				}	
				/*����09*/
				else if(RS232rev.RevBuf[RS232_INDEX_COMMAND]==0x09 && *(uint16_t*)&RS232rev.RevBuf[RS232_INDEX_DATA_LENG]>0)
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//У��ɹ�
					{
						flag232GetVer=1;
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					else{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
				}
				/*����0a*/
				else if(RS232rev.RevBuf[RS232_INDEX_COMMAND]==0x0a )
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//У��ɹ�
					{
						flag232UpDate=1;//���¹̼���232����
						send_rs232_ok_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
					else{
						send_rs232_err_ack(RS232rev.RevBuf[RS232_INDEX_COMMAND]);
					}
				}
				/*����0C*/
				else if(RS232rev.RevBuf[RS232_INDEX_COMMAND]==0x0C && *(uint16_t*)&RS232rev.RevBuf[RS232_INDEX_DATA_LENG]>0)
				{
					RS232rev.RevOver=0;
					if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) )//У��ɹ�
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



//���͸��°������µ����ݸ�ʽ
//����״̬0x00�����ݷ����У�0x01 ������� 0x02���� У����
static char send_rs2332_updata_firmware(uint8_t *buf,uint16_t length)
{
//	uint8_t buf[15+1024]={0};
//	buf[0]=0xA5;buf[1]=0x5A;
//	buf[2]=0x0b;//����
//	*(uint16_t*)&buf[3]=length;
//	buf[5]=status;//����״̬
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
	while(timeout<800)//�ȴ���Ӧ
	{
		delay_microsecond(1000);//vTaskDelay(1);1ms
		timeout++;
		if(RS232rev.RevOver==1 || flag232_3ack==1)
		{
			break;
		}
	}	
	if(timeout>=800)//û���յ���Ӧ���ط�
	{
		if(cnt>=9)
		{
			return 1;//����10�ζ�û��ЧӦ
		}
		cnt++;
		goto nx;
	}
	else if(timeout<800&&RS232rev.RevOver==1)//�յ�Ӧ��
	{
		RS232rev.RevOver=0;
		//У��Ӧ��
		if( 0==rs232_check_rev_data(RS232rev.RevBuf,  RS232rev.BufLen) 
			    && RS232rev.RevBuf[5]=='o' && RS232rev.RevBuf[6]=='k')
		{
			//У��ɹ�
			return 0;
		}
		else{
			//У��ʧ��
			if(cnt>=9)
			{
				return 2;//����10�λ���У��ʧ��
			}	
			cnt++;
			goto nx;
		}
			
	}	
	else if(timeout<800&&flag232_3ack==1)
	{
		//flag232_3ack=0;//�ں���rs232_wait_updata_ok(void)�����־
		//�յ�����3��Ӧ��
		return 0;
	}
	
}

//������
char send_rs2332_updata_firmware_data_ing(uint8_t *data,uint16_t length)
{
	if(length!=1024)
		return 8;
	
	uint8_t buf[10+1024]={0};
	buf[0]=0xA5;buf[1]=0x5A;
	buf[2]=0x0b;//����
	*(uint16_t*)&buf[3]=length;
	buf[5]=0x00;//����״̬
	memcpy(&buf[6], (const void *)data, length);  
	
	uint16_t check=Get_Crc16(buf, 6+length);
	memcpy(&buf[6+length], (const void *)&check, 2);

	buf[6+length+2]=0x0d;
	buf[6+length+3]=0x0a;	
	
	return send_rs2332_updata_firmware(buf,10+length);
}
//�������һ��
char send_rs2332_updata_firmware_data_end(uint8_t *data,uint16_t length)
{
	int num=1024-length;//Ҫ��num��0xFF
	
	uint8_t buf[10+1024]={0};
	buf[0]=0xA5;buf[1]=0x5A;
	buf[2]=0x0b;//����
	*(uint16_t*)&buf[3]=length;
	buf[5]=0x01;//����״̬
	memcpy(&buf[6], (const void *)data, length);  
	memset(&buf[6+length], 0xFF, num);  
	
	uint16_t check=Get_Crc16(buf, 6+length+num);
	memcpy(&buf[6+length+num], (const void *)&check, 2);

	buf[6+length+num+2]=0x0d;
	buf[6+length+num+3]=0x0a;	
	
	return send_rs2332_updata_firmware(buf,10+length+num);
}
//���͹̼���С
char send_rs2332_updata_firmware_size(int firmware_size)
{
	uint8_t buf[10+4]={0};
	buf[0]=0xA5;buf[1]=0x5A;
	buf[2]=0x0b;//����
	*(uint16_t*)&buf[3]=4;
	buf[5]=0x02;//����״̬
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
		if(++timeout>=1000*10)//�ȴ�10ms
		{
			return 1;//��ʱ
		}
		if(flag232_3ack==1)
		{
			return 0;
		}
		
	}

	

}





static char rs232_check_rev_data(uint8_t *frame,  uint16_t frameLen)
{
	//����У��ֵ
	uint16_t check=Get_Crc16(frame, frameLen-4);
	
	uint16_t rev_check=*(uint16_t *)&frame[frameLen-4];

	if(check==rev_check)
		return 0;//У��ɹ�
	else 
		return 1;
	
}


//o_data��������Ŀͷ�ڵ����
//���ر���Ŀ��ĩβָ��
static char* rs232_data_xiangmu_prase(char *Pstart,struct ceshixiangmu **o_data)
{
	struct ceshixiangmu *tail=NULL;//ָ�����һ���ڵ�(��ǰ��Ҫ��������Ŀ�ڵ�)

	if(*o_data==NULL)//�������һ����Ŀ
	{
		*o_data=malloc(sizeof(struct ceshixiangmu));//  ����Ҫ�ͷ�
		memset(*o_data,0,sizeof(struct ceshixiangmu));
		tail=*o_data;//ͷ�ڵ����β�ͽڵ�
	}
	else//�ǿ�����o_data��ͷ�ڵ㣬�ҵ�β�ڵ�
	{
		struct ceshixiangmu *temp=malloc(sizeof(struct ceshixiangmu));//����һ���½ڵ�  ����Ҫ�ͷ�
		memset(temp,0,sizeof(struct ceshixiangmu));
		//Ѱ�����һ���ڵ�
		tail=*o_data;//�Ȱ�ͷ��Ϊβ��
		while(1)
		{
			if(tail->next==NULL)//�ҵ���β�ڵ�
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
	//--------------�ѽ���������Ŀ���ݴ��ݵ�tail�ڵ���--------------------------------------/
	char *p=NULL;
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return NULL;
	}	
	tail->TDNmae=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
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
	tail->TDFanWei=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
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


//232ͨ�����Խ�����ݽ���
//����0Ϊ�����ɹ�������ڲ���RS232DataFrame�����
char rs232_data_prase( uint8_t *i_frame,uint16_t i_frameLen,struct RS232DataFrame *o_data)
{
	
	//У��
	if( 0!=rs232_check_rev_data(i_frame,  i_frameLen) )
	{
		//У��ʧ��
		return 1;
	}
	//����������һ��ͨ���Ľ��ͽ��
	o_data->TongDaoNum=i_frame[RS232_INDEX_COMMAND];

	//����������ʼ��ַ
	char * Pstart=(char *)&i_frame[RS232_INDEX_DATA];
	//�������ݵ��ֽڳ���
	uint16_t data_length=*(uint16_t *)&i_frame[RS232_INDEX_DATA_LENG];
	
	//char * Pstart=(char *)i_frame;//***
	
	uint8_t xuhao=0;
	char *p=NULL;
	//�������1����������
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
	xuhao++; //1
	Pstart=p+1;
	//---------------------------
	//�������2����������
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
//	if(p-Pstart-1>0){//p-Pstart-1Ϊ��{�� ��}���м�����ݳ���
//		o_data->SheBeiBiaoShi=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
//		memset(o_data->SheBeiBiaoShi,0,p-Pstart-1+1);
//		memcpy(o_data->SheBeiBiaoShi,Pstart+1,p-Pstart-1);
//	}
//	else{
//		o_data->SheBeiBiaoShi=NULL;
//	}
	xuhao++;	//2
	Pstart=p+1;
	//---------------------------
	//�������3����������
	p=strstr((const char *)Pstart,"}");
	if(p==NULL){
		return 2;
	}	
//	if(p-Pstart-1>0){//p-Pstart-1Ϊ��{�� ��}���м�����ݳ���
//		o_data->SBName=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
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
	if(p-Pstart-1>0){//p-Pstart-1Ϊ��{�� ��}���м�����ݳ���
		o_data->TDJiLuTime=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
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
	if(p-Pstart-1>0){//p-Pstart-1Ϊ��{�� ��}���м�����ݳ���
		o_data->TDJZh=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
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
	if(p-Pstart-1>0){//p-Pstart-1Ϊ��{�� ��}���м�����ݳ���
		o_data->TDZongHeGe=malloc(p-Pstart-1+1);//����Ҫ�ͷ�
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
	//��Ŀ
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
			temp=head;//��ͷ�洢
			head=temp->next; //ָ����һ���ڵ�
			//�ͷű�ͷ
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




