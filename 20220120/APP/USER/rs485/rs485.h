#ifndef __RS485_H__
#define __RS485_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (ͷ�ļ�����)
****************************************************************************************
*/
#include "stm32f4xx.h"

/*
****************************************************************************************
* TYPEDEFS (���������ض���)
****************************************************************************************
*/

#define  MAX_DATA_LENGTH     1000          //����ֽ�����  

//����֡�ṹ
struct DataFrame{
	                                                          //��0��1��֡ͷ��
	unsigned char        sender_id;                           // 2  -���ͷ�ID
	unsigned char        receiver_id;                         // 3  -���շ���ID
  unsigned char        commond;                           // 4-������
	unsigned char        serial_number;                     // 5  -���к�
  unsigned short int   dataLength;                        //6,7 -���ݳ���         
  unsigned char       data[MAX_DATA_LENGTH];             //ԭʼ�������ݻ����� 
  //unsigned short int  check;                             //8+N+1  8+N+2 CRC16У����
                                                         //8+N+3  ֡β��
};




typedef struct
{
	uint8_t RevBuf[1024];
	uint16_t RevLen;
	uint8_t RevOver;
	uint16_t BufLen;
  uint8_t  type;
}TYPE_RS485_U;

extern TYPE_RS485_U RS485rev; 



extern uint8_t devID_master;
extern uint8_t devID_salve;
/*
****************************************************************************************
* EXTERNAL VARIABLES (�ⲿ����)
****************************************************************************************
*/
extern uint8_t flagSensorDataCome;
extern uint8_t flagFirmwareStart;//�������ӻ�����Ĺ̼���ʼ�����־
extern uint8_t flagFirmwareData;//�������ӻ�����Ĺ̼����ݱ�־
extern uint8_t flagFirmwareEnd;//�������ӻ�����Ĺ̼�������ɱ�־
extern uint8_t flagInquiry;//������ӻ�ѯ�ʱ�־
extern uint8_t flagSlaveUpdateFW;//�ӻ�������������̼���־
/*ȫ�ֱ�־������״̬************************************************************
            ����������״̬
            SD������
						�豸���ͣ�������/�����磩
*/
extern uint8_t flagNetWork;//ʹ��WIFI,����·��Ϊ1///ʹ��4G,������Ϊ1
#define NETWORK_ON  1 //������
#define NETWORK_OFF 0 //������
extern uint8_t flagServerConn;
#define CONNECTED 1 //����
#define DISCONNECT 0 //�Ͽ�
extern uint8_t flagSDReady;//SD��������־
#define READY 1   //����
#define NOT_READY 0 //δ��⵽��
extern uint8_t flagDevType;
#define MASTER 1 //�����������繦��
#define SALVER 0 //�ӻ��������繦��
//******************************************************************************
/*
****************************************************************************************
* CONSTANTS (����)
****************************************************************************************
*/


/*
****************************************************************************************
* MACROS (�궨��)
****************************************************************************************
*/
#define RS485_UART USART6

#define RS485_UART_IRQn USART6_IRQn
#define RS485_UART_IRQHandler USART6_IRQHandler


#define RS485_GPIO_CLOCK_BUS      AHB1ENR
#define RS485_GPIO_CLOCK_BUS_BIT  2
//#define RS485_GPIO GPIOC
//#define RS485_GPIO_PIN 8

//#define SEND_RS485_MODE   GPIO_SetBits( RS485_GPIO,  GPIO_Pin_8 )//��������-��
//#define REV_RS485_MODE    GPIO_ResetBits( RS485_GPIO,GPIO_Pin_8 )  //��������-��


/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (����ȫ�ֺ���)
****************************************************************************************
*/

void rs485_uart_config(uint32_t bond);
void rs485_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length );
void send_rs485_commond(uint8_t *cmd,uint16_t len);


char check_rev_data(uint8_t *frame,  uint16_t frameLen);

/*��ʾ���շ���Ҫ���й̼����������ý���׼��
sender_id���ͷ�ID
receiver_id���շ�ID
firmware_type �̼����� (0x01��232�豸 ; 0x02:485�豸)
firmware_size �̼��ֽڴ�С
����0������ɲ����յ��Է���ȷ��Ӧ
*/
char send485_firmware_update_start(uint8_t sender_id,uint8_t receiver_id,uint8_t firmware_type,uint16_t firmware_size);

/*�̼����䣬���δ������Я��1000�ֽ�����
sender_id���ͷ�ID
receiver_id���շ�ID
firmware_data ָ��̼����� 
length ���δ���Ĺ̼����ݳ���
����0������ɲ����յ��Է���ȷ��Ӧ
*/
char send485_firmware_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* firmware_data,uint16_t length);

/*��ʾ���շ��̼��������
sender_id���ͷ�ID
receiver_id���շ�ID
firmware_type �̼����� (0x01��232�豸 ; 0x02:485�豸)
trans_cnt �̼��ִ�����
����0������ɲ����յ��Է���ȷ��Ӧ
*/
char send485_firmware_update_end(uint8_t sender_id,uint8_t receiver_id,uint16_t trans_cnt,int version);

/*���䴫�����ɼ�����
sender_id���ͷ�ID
receiver_id���շ�ID
sensor_data ָ�򴫸������� 
length ���������ݳ���
����0������ɲ����յ��Է���ȷ��Ӧ
*/
char send485_sensor_data(uint8_t sender_id,uint8_t receiver_id,uint8_t* sensor_data,uint16_t length);

//������ӻ���������ѯ�����ݰ�
char send485_master_inquiry(uint8_t salve_id,int fw485ID,int fw232ID);

char send485_frimware_update_notify(uint8_t sender_id,uint8_t receiver_id,uint8_t fw_type);

/*������Ӧ*/
char send485_ack(uint8_t sender_id,uint8_t receiver_id,uint8_t cmd,uint8_t serial_number);

/*���ʹ�����*/
char send485_ack_err(uint8_t sender_id,uint8_t receiver_id,uint8_t err_code);
#ifdef __cplusplus
}
#endif

#endif
