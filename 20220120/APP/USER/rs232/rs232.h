#ifndef __RS232_H__
#define __RS232_H__

#ifdef __cplusplus
 extern "C" {
#endif


/*
****************************************************************************************
* INCLUDES (头文件包含)
****************************************************************************************
*/
#include "stm32f4xx.h"

/*
****************************************************************************************
* TYPEDEFS (数据类型重定义)
****************************************************************************************
*/



//测试项目
struct ceshixiangmu{
	uint8_t TDXuHao;//测试序号
	uint8_t* TDNmae;
	uint8_t* TDFanWei;
	float TDValue;
	uint8_t TDJieGuo;//项目测试结果
	struct ceshixiangmu *next;
};

struct RS232DataFrame{
//	uint8_t *SheBeiBiaoShi;//设备ID--用完要释放
//	uint8_t *SBName;       //设备名称--用完要释放
	uint8_t TongDaoNum;
	uint8_t* TDJiLuTime;//测试时间--用完要释放
	uint8_t* TDJZh;//测试机种号  用完要释放
	uint8_t* TDZongHeGe;//总结果 用完要释放
	int TDCiShu;//测试次数
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
* EXTERNAL VARIABLES (外部变量)
****************************************************************************************
*/
extern uint8_t flag232CeShiJieGuo;
extern uint8_t flag232GetVer;
extern uint8_t flag232UpDate;
extern uint8_t flag232_3ack;
/*
****************************************************************************************
* CONSTANTS (常量)
****************************************************************************************
*/


/*
****************************************************************************************
* MACROS (宏定义)
****************************************************************************************
*/
#define RS232_UART USART3

#define RS232_UART_IRQn USART3_IRQn
#define RS232_UART_IRQHandler USART3_IRQHandler


#define RS232_GPIO_CLOCK_BUS      AHB1ENR
#define RS232_GPIO_CLOCK_BUS_BIT  2
//#define RS232_GPIO GPIOC
//#define RS232_GPIO_PIN 8

//#define SEND_RS232_MODE   GPIO_SetBits( RS232_GPIO,  GPIO_Pin_8 )//主机发送-高
//#define REV_RS232_MODE    GPIO_ResetBits( RS232_GPIO,GPIO_Pin_8 )  //主机接收-低


/*
****************************************************************************************
* PUBLIC FUNCTIONS DECLARE (声明全局函数)
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


//固件传输给232板子，发送中
char send_rs2332_updata_firmware_data_ing(uint8_t *data,uint16_t length);
//固件传输给232板子，发送最后一笔
char send_rs2332_updata_firmware_data_end(uint8_t *data,uint16_t length);
//固件传输给232板子，发送校验
char send_rs2332_updata_firmware_size(int firmware_size);
char rs232_wait_updata_ok(void);
#ifdef __cplusplus
}
#endif

#endif
