#include "io.h"
#include "io_bit.h"
#include "my_print.h"

//PB9-1
void exti9_init(void)
{
	//端口时钟使能PE
	RCC->AHB1ENR |=0X01<<1;
	//PB9设置为普通功能浮空输入
	GPIOB->MODER &=~(0X03<<18);
	GPIOB->PUPDR &=~(0X03<<18);
	//使能系统配置控制器时钟
	RCC->APB2ENR |=0X01<<14;
	
	//设置重映射---EXTI9映射到PB9
	SYSCFG->EXTICR[2] &=~(0X0F<<4);
	SYSCFG->EXTICR[2] |=0X01<<4;
	//设为检测双边缘
	EXTI->FTSR |=0X01<<9;
	EXTI->RTSR |=(0X01<<9);
	//禁用EXTI2上的软件中断
	EXTI->SWIER &=~(0X01<<9);
	//禁用exti2上的事件
	EXTI->EMR &=~(0X01<<9); 
	
	//在NVIC中计算、设置EXTI中断的优先级
	 NVIC_SetPriority(EXTI9_5_IRQn,NVIC_EncodePriority(7-2,0,0));
	 //使能中断-----（核心中断没有此设置）
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	//开EXTI2中断
	EXTI->IMR |=0X01<<9;
	
}


//PE0-2

void exti0_init(void)
{
	//端口时钟使能PE
	RCC->AHB1ENR |=0X01<<4;
	//pe0设置为普通功能浮空输入
	GPIOE->MODER &=~(0X03<<0);
	GPIOE->PUPDR &=~(0X03<<0);
	//使能系统配置控制器时钟
	RCC->APB2ENR |=0X01<<14;
	
	//设置重映射---EXTI0映射到PE0
	SYSCFG->EXTICR[0] &=~(0X0F<<0);
	SYSCFG->EXTICR[0] |=0X04<<0;
	//设为检测双边缘
	EXTI->FTSR |=0X01<<0;
	EXTI->RTSR |=(0X01<<0);
	//禁用EXTI2上的软件中断
	EXTI->SWIER &=~(0X01<<0);
	//禁用exti2上的事件
	EXTI->EMR &=~(0X01<<0); 
	
	//在NVIC中计算、设置EXTI中断的优先级
	 NVIC_SetPriority(EXTI0_IRQn,NVIC_EncodePriority(7-2,0,0));
	 //使能中断-----（核心中断没有此设置）
	NVIC_EnableIRQ(EXTI0_IRQn);
	//开EXTI2中断
	EXTI->IMR |=0X01<<0;
	
}
uint8_t flagIOJieGuo=0;
uint8_t flagIO1=0;
uint8_t flagIO2=0;

void EXTI0_IRQHandler(void)
{
	//清除挂起标志
	EXTI->PR |=0X01<<4;
	while(EXTI->PR & (0X01<<4));  //等待标志清除成功
	
	if(PEin(0))
	{
		MY_PRINT("io2 开始\r\n");
		flagIO2=1;
		flagIOJieGuo=1;
	}
	else
	{
		MY_PRINT("io2 结束\r\n");
	}
	
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI->PR &(0X01<<9))   //外部中断线9上面产生了中断
	{
		EXTI->PR =0X01<<9;
		while(EXTI->PR &(0x01<<9));
		
		if(PBin(9))
		{
			MY_PRINT("io1 开始\r\n");
			flagIO1=1;
			flagIOJieGuo=1;
		}
		else
		{
			MY_PRINT("io1 结束\r\n");
		}	


		
	}
	
	
	
}




void IO_tongdao_init(void)
{
	exti9_init( );
	exti0_init( );
	
}

