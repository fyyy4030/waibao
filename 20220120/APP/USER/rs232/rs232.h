#ifndef __RS232_H__
#define __RS232_H__

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



//������Ŀ
struct ceshixiangmu{
	uint8_t TDXuHao;//�������
	uint8_t* TDNmae;
	uint8_t* TDFanWei;
	float TDValue;
	uint8_t TDJieGuo;//��Ŀ���Խ��
	struct ceshixiangmu *next;
};

struct RS232DataFrame{
//	uint8_t *SheBeiBiaoShi;//�豸ID--����Ҫ�ͷ�
//	uint8_t *SBName;       //�豸����--����Ҫ�ͷ�
	uint8_t TongDaoNum;
	uint8_t* TDJiLuTime;//����ʱ��--����Ҫ�ͷ�
	uint8_t* TDJZh;//���Ի��ֺ�  ����Ҫ�ͷ�
	uint8_t* TDZongHeGe;//�ܽ�� ����Ҫ�ͷ�
	int TDCiShu;//���Դ���
	struct ceshixiangmu *item;
};




typedef struct
{
	uint8_t RevBuf[1024*3];
	uint16_t RevLen;
	uint8_t RevOver;
	uint16_t BufLen;
  uint8_t  type;
}TYPE_RS232_U;

extern TYPE_RS232_U RS232rev; 



extern uint8_t devID_master;
extern uint8_t devID_salve;
/*
****************************************************************************************
* EXTERNAL VARIABLES (�ⲿ����)
****************************************************************************************
*/
extern uint8_t flag232CeShiJieGuo;
extern uint8_t flag232GetVer;
extern uint8_t flag232UpDate;
extern uint8_t flag232_3ack;
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
#define RS232_UART USART3

#define RS232_UART_IRQn USART3_IRQn
#define RS232_UART_IRQHandler USART3_IRQHandler


#define RS232_GPIO_CLOCK_BUS      AHB1ENR
#define RS232_GPIO_CLOCK_BUS_BIT  2
//#define RS232_GPIO GPIOC
//#define RS232_GPIO_PIN 8

//#define SEND_RS232_MODE   GPIO_SetBits( RS232_GPIO,  GPIO_Pin_8 )//��������-��
//#define REV_RS232_MODE    GPIO_ResetBits( RS232_GPIO,GPIO_Pin_8 )  //��������-��


/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (����ȫ�ֺ���)
****************************************************************************************
*/

void rs232_uart_config(uint32_t bond);
void rs232_uart_tx_bytes( uint8_t* TxBuffer, uint16_t Length );
//void send_rs232_commond(uint8_t *cmd,uint16_t len);

void send_rs232_ok_ack(uint8_t cmd);
void send_rs232_err_ack(uint8_t cmd);

char send_rs232_09cmd(int* XBanBenID);

char rs232_data_prase( uint8_t *i_frame,uint16_t i_frameLen,struct RS232DataFrame *o_data);
void myprintf(struct RS232DataFrame *data);
void free_rs232_data(struct RS232DataFrame *data);


//�̼������232���ӣ�������
char send_rs2332_updata_firmware_data_ing(uint8_t *data,uint16_t length);
//�̼������232���ӣ��������һ��
char send_rs2332_updata_firmware_data_end(uint8_t *data,uint16_t length);
//�̼������232���ӣ�����У��
char send_rs2332_updata_firmware_size(int firmware_size);
char rs232_wait_updata_ok(void);
#ifdef __cplusplus
}
#endif

#endif
