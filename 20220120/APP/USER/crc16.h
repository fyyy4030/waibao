#ifndef __CRC16_H__
#define __CRC16_H__

#ifdef __cplusplus
 extern "C" {
#endif


#include "stm32f4xx.h"

uint16_t Get_Crc16(uint8_t *puchMsg, uint16_t usDataLen);
uint16_t Get_Crc16_Persistent(uint8_t uchCRCHi,uint8_t uchCRCLo,uint8_t *puchMsg, uint16_t usDataLen);

unsigned int calc_crc32(unsigned int crc, const void *buf, unsigned int size);


#ifdef __cplusplus
}
#endif

#endif
