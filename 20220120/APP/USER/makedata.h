#ifndef _CJSONMAKE_H_
#define _CJSONMAKE_H_
#include "stm32f4xx.h"
#include "cjson.h"
#include "stdlib.h"
#include "stdio.h"
#include "4G.h"
#include "wifi.h"
#include "rs232.h"
#include "rs485.h"


typedef struct{
	unsigned int CJID;
//	char SheBeiBiaoShi[25]; 
//  char SBName[10];
	char SBBH[20];
	char ZaXiang[40];
	char TDJZh[20];
	int BanBenID;
	int XBanBenID;
}TYPE_HTTP_KEY;
extern TYPE_HTTP_KEY http_key;


char make_data_and_report(uint8_t *i_frame,uint16_t i_frameLen,char *TDSaoMa);
char make_data_and_report_saoma(char *TDSaoMa);//µ•∂¿…œ±®…®¬Î
char make_data_and_report_io(char io_n,char value);
#endif