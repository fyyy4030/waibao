/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "rs485.h"
#include "crc16.h"
#include "FreeRTOS.h"//���ͷ�ļ�������task.h֮ǰ���������������򱨴�
#include "task.h"
#include "my_print.h"
/*
****************************************************************************************
* CONSTANTS (��������)
****************************************************************************************
*/
uint8_t devID_master=0x00;//��������ID,�̶�0x00
uint8_t devID_salve=0x0A;//�ӻ�����ID 0x02~0x1E
/*
****************************************************************************************
* TYPEDEFS (���Ͷ���)
****************************************************************************************
*/
/*
   1   2               3               4             5             6               7 8                   N          8+N+1    8+N+2       8+N+3
֡ͷ��2Byte��		���ͷ�ID(1Byte)	���շ�ID(1Byte)	ָ����(1Byte)		���кţ�1Byte��		���ݳ��ȣ�2Byte��		����(nByte)		CRC16У����(2Byte)		֡β
0xA5	0x5A						                                                              	L	H			                           L	H	            0x88
*/

#define FRAME_HEAD1              0xA5                               //֡ͷ1���壬0xA5
#define FRAME_HEAD2              0x5A                               //֡ͷ2���壬0x5A

#define FRAME_MIN_LEN            11                                //��������������С��Ҫ11�ֽ�

#define INDEX_FRAME_HEAD1         (0)
#define LENG_FRAME_HEAD1          (1)                                 //֡ͷ1ռ���ֽ�����

#define INDEX_FRAME_HEAD2         (INDEX_FRAME_HEAD1+LENG_FRAME_HEAD1)
#define LENG_FRAME_HEAD2          (1)                                  //֡ͷ2ռ���ֽ�����

#define INDEX_SENDER_ID           (INDEX_FRAME_HEAD2+LENG_FRAME_HEAD2)  //֡�д�ŷ��ͷ�ID���±�����
#define LENG_SENDER_ID            (1)                                   //���ͷ�IDռ���ֽ�����

#define INDEX_RECEIVER_ID         (INDEX_SENDER_ID+LENG_SENDER_ID)        //֡�д�Ž��շ�ID���±�����
#define LENG_RECEIVER_ID          (1)                                     //���շ�IDռ���ֽ�����

#define INDEX_COMMAND             (INDEX_RECEIVER_ID+LENG_RECEIVER_ID)    //֡�д��ָ������±�����
#define LENG_COMMAND              (1)                                    //ָ����ռ���ֽ�����

#define INDEX_SERIAL_NUMBER       (INDEX_COMMAND+LENG_COMMAND)         //֡�д�����кŵ��±�����
#define LENG_SERIAL_NUMBER        (1)                                    //���к�ռ���ֽ�����


#define INDEX_DATA_LENG           (INDEX_SERIAL_NUMBER+LENG_SERIAL_NUMBER)     //֡�д��ָʾ���ݳ��ȵ��±�����
#define LENG_DATA_LENG            (2)                                         //���ݳ���ռ�õ��ֽ�����

#define INDEX_DATA                (INDEX_DATA_LENG+LENG_DATA_LENG)            //֡�д����������±�����

#define LENG_CHECK_LENG           (2)                               //У������ռ�õ��ֽ�����

#define FRAME_END                  0x88                             //֡β����,0x88
#define INDEX_FRAME_END                                        
#define LENG_FRAME_END            (1)                             //֡βռ���ֽ�����




uint8_t flagSensorDataCome=0;//����485�ӻ��Ĵ���������

uint8_t flagFirmwareStart=0;//�������ӻ�����Ĺ̼���ʼ�����־
uint8_t flagFirmwareData=0;//�������ӻ�����Ĺ̼����ݱ�־
uint8_t flagFirmwareEnd=0;//�������ӻ�����Ĺ̼�������ɱ�־
uint8_t flagInquiry=0;//������ӻ�ѯ�ʱ�־
uint8_t flagSlaveUpdateFW=0;//�ӻ�������������̼���־
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
	//GPIO����
	RCC_AHB1PeriphClockCmd  ( RCC_AHB1Periph_GPIOC, ENABLE ) ; 
	
	GPIO_InitTypeDef  GPIO_InitStruct;//���ò����ṹ��
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);	
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource6,  GPIO_AF_USART6 );
	GPIO_PinAFConfig (GPIOC,  GPIO_PinSource7,  GPIO_AF_USART6 );
	GPIO_PinLockConfig(GPIOC,GPIO_Pin_6 | GPIO_Pin_7);
	
	//USART6����
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

	//����NVIC
	NVIC_InitTypeDef   NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel=RS485_UART_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority=2;//��Ӧ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelSubPriority=2;//��ռ���ȼ�
	NVIC_InitStruct.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init  ( &NVIC_InitStruct ) ; 
	
	USART_Cmd( RS485_UART,ENABLE) ;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIOC->ODR &=~(1<<2);//����
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
	if(RS485_UART->SR & (0X01<<5))  //�����ж�
	{
		RS485rev.RevBuf[RS485rev.RevLen]=RS485_UART->DR;
		RS485rev.RevLen++;

	}
	else if(RS485_UART->SR & (0X01<<4))//�����ж�
	{
		data=RS485_UART->SR;
		data=RS485_UART->DR;
	
		RS485rev.BufLen=RS485rev.RevLen;
		RS485rev.RevLen=0;
		RS485rev.RevOver=1;
		//�ж��Ƿ�����������Ĵ��������ݰ�
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
					if(RS485rev.RevBuf[INDEX_RECEIVER_ID]==devID_salve)//���ͷ�ID�����Լ���ID��ʾѯ�ʵ����Լ�
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
void send_rs485_commond(uint8_t *cmd,uint16_t len)
{
	GPIOC->ODR |=(1<<2);//SEND_RS485_MODE;
	delay_microsecond(3000);
	rs485_uart_tx_bytes( (uint8_t*)cmd, len );
	delay_microsecond(3000);
	GPIOC->ODR &=~(1<<2);//����//REV_RS485_MODE;
}


//--------------------------------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------------------------------------------


//��������֡
//o_frame :�������,�������ɵ�����֡�ֽ�;
//o_frameLen:�������,��������֡����;
//i_frameBufSize:�������,��������֡�Ļ���������;
//i_data:�������,Ҫ�����ԭʼ����;
//����0Ϊ����ɹ�������ڲ���o_frame��o_frameLen�����
static char packetFrame( uint8_t *o_frame,  uint16_t *o_frameLen,const uint16_t i_frameBufSize, const struct DataFrame *i_data)
{
	int vtmp = 0;
	unsigned short int index = 0;
	unsigned short int frameLen = FRAME_MIN_LEN + i_data->dataLength;        //���㵱ǰ֡���ֽ�����

	//�жϻ����������Ƿ�������СҪ�����Ͻ�����־��
	if(i_frameBufSize < frameLen) {
			MY_PRINT("��������С������֡ʧ��!\r\n");
			return 1;
	}

	//��ջ�����
	memset(o_frame, 0, frameLen);
		
  //���������   ��0��1��֡ͷ��
	o_frame[index]=FRAME_HEAD1;
  index += LENG_FRAME_HEAD1;
	o_frame[index]=FRAME_HEAD2;
  index += LENG_FRAME_HEAD2;
	//���ͷ�ID
  memcpy(&o_frame[index], (const void *)&i_data->sender_id, LENG_SENDER_ID);  
  index += LENG_SENDER_ID;	
	//���շ�ID
	 memcpy(&o_frame[index], (const void *)&i_data->receiver_id, LENG_RECEIVER_ID);  
	index += LENG_RECEIVER_ID;	
	//������
	memcpy(&o_frame[index], (const void *)&i_data->commond, LENG_COMMAND);  
  index += LENG_COMMAND;
	//���к�
  memcpy(&o_frame[index], (const void *)&i_data->serial_number, LENG_SERIAL_NUMBER);  
	index += LENG_SERIAL_NUMBER;
	//���ݳ���
	memcpy(&o_frame[index], (const void *)&i_data->dataLength, LENG_DATA_LENG); 
  index += LENG_DATA_LENG;
	//����
	memcpy(&o_frame[index], (const void *)i_data->data, i_data->dataLength);   
  index += i_data->dataLength;		
	//У��ֵ
	uint16_t check=Get_Crc16(o_frame, index);
	memcpy(&o_frame[index], (const void *)&check, LENG_CHECK_LENG);  
  index += LENG_CHECK_LENG;

	//������
	o_frame[index]=FRAME_END;
  index += LENG_FRAME_END;

  *o_frameLen = index;
  return 0;
}

char check_rev_data(uint8_t *frame,  uint16_t frameLen)
{
	//����У��ֵ
	uint16_t check=Get_Crc16(frame, frameLen-3);
	
	uint16_t rev_check=*(uint16_t *)&frame[frameLen-3];

	if(check==rev_check)
		return 0;//У��ɹ�
	else 
		return 1;
	
}


/*��ʾ���շ���Ҫ���й̼����������ý���׼��
sender_id���ͷ�ID
receiver_id���շ�ID
firmware_type �̼����� (0x01��485�豸 ; 0x02:232�豸)
firmware_size �̼��ֽڴ�С
����0������ɲ����յ��Է���ȷ��Ӧ
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
	while(cnt<1000)//�ȴ���Ӧ
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}
	if(cnt>=1000)//û���յ���Ӧ���ط�
	{
		if(data.serial_number>=2)
		{
			return 1;//����3�ζ�û��ЧӦ
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<1000&&RS485rev.RevOver==1)//�յ�Ӧ��
	{
		RS485rev.RevOver=0;
		
		//У��Ӧ��
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//У��ɹ�
			return 0;
		}
		else{
			//У��ʧ�ܣ��ط�
			if(data.serial_number>=2)
			{
				return 2;//����3��У��ʧ��
			}	
			data.serial_number++;
			goto nx;
		}
			
	}
	
}
    #include "debug_uart.h"
/*�̼����䣬���δ������Я��1000�ֽ�����
sender_id���ͷ�ID
receiver_id���շ�ID
firmware_data ָ��̼����� 
length ���δ���Ĺ̼����ݳ���
����0������ɲ����յ��Է���ȷ��Ӧ
*/
char send485_firmware_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* firmware_data,uint16_t length)
{
	//���δ������Я��1000�ֽ�����
	if(length>MAX_DATA_LENGTH)
		return 3;//�������ݹ���
	
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
	while(cnt<1000)//�ȴ���Ӧ
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}	
	if(cnt>=1000)//û���յ���Ӧ���ط�
	{
		if(data.serial_number>=2)
		{
			return 1;//����3�ζ�û��ЧӦ
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<1000&&RS485rev.RevOver==1)//�յ�Ӧ��
	{
		RS485rev.RevOver=0;
		//У��Ӧ��
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//У��ɹ�
			return 0;
		}
		else{
			//У��ʧ�ܣ��ط�
			if(data.serial_number>=2)
			{
				return 2;//����3��У��ʧ��
			}	
			data.serial_number++;
			goto nx;
		}
			
	}	
}

/*��ʾ���շ��̼��������
sender_id���ͷ�ID
receiver_id���շ�ID
firmware_type �̼����� (0x01��232�豸 ; 0x02:485�豸)
trans_cnt �̼��ִ�����
����0������ɲ����յ��Է���ȷ��Ӧ
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
	while(cnt<1000)//�ȴ���Ӧ
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}
	if(cnt>=1000)//û���յ���Ӧ���ط�
	{
		if(data.serial_number>=2)
		{
			return 1;//����3�ζ�û��ЧӦ
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<1000&&RS485rev.RevOver==1)//�յ�Ӧ��
	{
		RS485rev.RevOver=0;
		//У��Ӧ��
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//У��ɹ�
			return 0;
		}
		else{
			//У��ʧ�ܣ��ط�
			if(data.serial_number>=2)
			{
				return 2;//����3��У��ʧ��
			}	
			data.serial_number++;
			goto nx;
		}
			
	}
	
}

/*���䴫�����ɼ�����
sender_id���ͷ�ID
receiver_id���շ�ID
sensor_data ָ�򴫸������� 
length ���������ݳ���
����0������ɲ����յ��Է���ȷ��Ӧ
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
	while(cnt<500)//�ȴ���Ӧ
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}	
	if(cnt>=500)//û���յ���Ӧ���ط�
	{
		if(data.serial_number>=2)
		{
			return 1;//����3�ζ�û��ЧӦ
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<500&&RS485rev.RevOver==1)//�յ�Ӧ��
	{
		RS485rev.RevOver=0;
		//У��Ӧ��
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//У��ɹ�
			return 0;
		}
		else{
			//У��ʧ�ܣ��ط�
			if(data.serial_number>=2)
			{
				return 2;//����3��У��ʧ��
			}	
			data.serial_number++;
			goto nx;
		}
			
	}		
}

//������ӻ���������ѯ�����ݰ�
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
	while(cnt<400)//�ȴ���Ӧ
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1 || flagSensorDataCome==1)//�յ���Ӧ������յ��˴���������
		{
			break;
		}
	}	
	if(cnt>=400)//û���յ���Ӧ
	{
		return 1;
	}
	else if(cnt<400)//�յ�Ӧ��
	{
		if(RS485rev.RevOver==1)
		{
			RS485rev.RevOver=0;
			//У��Ӧ��
			if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) )
				return 0;//У��ɹ�
			else
				return 2;
		}
		
	}			
	
}
//�ӻ�����������Ҫ���й̼�����
//fw_type��0x01��232 �̼�; 0x02:485�̼�
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
	while(cnt<500)//�ȴ���Ӧ
	{
		vTaskDelay(1);//delay_microsecond(1000);//1ms
		cnt++;
		if(RS485rev.RevOver==1)
		{
			break;
		}
	}	
	if(cnt>=500)//û���յ���Ӧ���ط�
	{
		if(data.serial_number>=2)
		{
			return 1;//����3�ζ�û��ЧӦ
		}
		
		data.serial_number++;
		goto nx;
	}
	else if(cnt<500&&RS485rev.RevOver==1)//�յ�Ӧ��
	{
		RS485rev.RevOver=0;
		//У��Ӧ��
		if( 0==check_rev_data(RS485rev.RevBuf,  RS485rev.BufLen) ){
			//У��ɹ�
			return 0;
		}
		else{
			//У��ʧ�ܣ��ط�
			if(data.serial_number>=2)
			{
				return 2;//����3��У��ʧ��
			}	
			data.serial_number++;
			goto nx;
		}
			
	}		
	
	
	
}


/*������Ӧ*/
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

/*���ʹ�����*/
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









