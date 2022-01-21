
#include "led.h"


//******************************************
//函数功能:LED初始化
//参数：无
//返回值：无
//注意事项：无
void LED_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	GPIO_StructInit(&GPIO_InitStruct);
	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_4|GPIO_Pin_3|GPIO_Pin_5|GPIO_Pin_6;
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	//GPIO_InitStruct.GPIO_Speed=GPIO_Low_Speed;
	//GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	LED1_OFF;
	LED2_OFF;
	LED3_OFF;
	LED4_OFF;
	
}



