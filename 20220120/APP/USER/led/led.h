#ifndef _LED_H_   //_XXX_H_
#define _LED_H_

//���м�����������궨�塢����������ȫ�ֱ������ⲿ����
#include "stm32f4xx.h"

#define LED4_OFF GPIOA->ODR &=~(0x01<<4)
#define LED3_OFF GPIOA->ODR &=~(0x01<<3)
#define LED2_OFF GPIOA->ODR &=~(0x01<<5)
#define LED1_OFF GPIOA->ODR &=~(0x01<<6)
 
#define LED4_ON	GPIOA->ODR |=(0x01<<4) 
#define LED3_ON	GPIOA->ODR |=(0x01<<3) 
#define LED2_ON	GPIOA->ODR |=(0x01<<5)
#define LED1_ON	GPIOA->ODR |=(0x01<<6) 


void LED_Init(void);

#endif



