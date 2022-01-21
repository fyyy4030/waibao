#include "io.h"
#include "io_bit.h"
#include "my_print.h"

//PB9-1
void exti9_init(void)
{
	//�˿�ʱ��ʹ��PE
	RCC->AHB1ENR |=0X01<<1;
	//PB9����Ϊ��ͨ���ܸ�������
	GPIOB->MODER &=~(0X03<<18);
	GPIOB->PUPDR &=~(0X03<<18);
	//ʹ��ϵͳ���ÿ�����ʱ��
	RCC->APB2ENR |=0X01<<14;
	
	//������ӳ��---EXTI9ӳ�䵽PB9
	SYSCFG->EXTICR[2] &=~(0X0F<<4);
	SYSCFG->EXTICR[2] |=0X01<<4;
	//��Ϊ���˫��Ե
	EXTI->FTSR |=0X01<<9;
	EXTI->RTSR |=(0X01<<9);
	//����EXTI2�ϵ�����ж�
	EXTI->SWIER &=~(0X01<<9);
	//����exti2�ϵ��¼�
	EXTI->EMR &=~(0X01<<9); 
	
	//��NVIC�м��㡢����EXTI�жϵ����ȼ�
	 NVIC_SetPriority(EXTI9_5_IRQn,NVIC_EncodePriority(7-2,0,0));
	 //ʹ���ж�-----�������ж�û�д����ã�
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	//��EXTI2�ж�
	EXTI->IMR |=0X01<<9;
	
}


//PE0-2

void exti0_init(void)
{
	//�˿�ʱ��ʹ��PE
	RCC->AHB1ENR |=0X01<<4;
	//pe0����Ϊ��ͨ���ܸ�������
	GPIOE->MODER &=~(0X03<<0);
	GPIOE->PUPDR &=~(0X03<<0);
	//ʹ��ϵͳ���ÿ�����ʱ��
	RCC->APB2ENR |=0X01<<14;
	
	//������ӳ��---EXTI0ӳ�䵽PE0
	SYSCFG->EXTICR[0] &=~(0X0F<<0);
	SYSCFG->EXTICR[0] |=0X04<<0;
	//��Ϊ���˫��Ե
	EXTI->FTSR |=0X01<<0;
	EXTI->RTSR |=(0X01<<0);
	//����EXTI2�ϵ�����ж�
	EXTI->SWIER &=~(0X01<<0);
	//����exti2�ϵ��¼�
	EXTI->EMR &=~(0X01<<0); 
	
	//��NVIC�м��㡢����EXTI�жϵ����ȼ�
	 NVIC_SetPriority(EXTI0_IRQn,NVIC_EncodePriority(7-2,0,0));
	 //ʹ���ж�-----�������ж�û�д����ã�
	NVIC_EnableIRQ(EXTI0_IRQn);
	//��EXTI2�ж�
	EXTI->IMR |=0X01<<0;
	
}
uint8_t flagIOJieGuo=0;
uint8_t flagIO1=0;
uint8_t flagIO2=0;

void EXTI0_IRQHandler(void)
{
	//��������־
	EXTI->PR |=0X01<<4;
	while(EXTI->PR & (0X01<<4));  //�ȴ���־����ɹ�
	
	if(PEin(0))
	{
		MY_PRINT("io2 ��ʼ\r\n");
		flagIO2=1;
		flagIOJieGuo=1;
	}
	else
	{
		MY_PRINT("io2 ����\r\n");
	}
	
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI->PR &(0X01<<9))   //�ⲿ�ж���9����������ж�
	{
		EXTI->PR =0X01<<9;
		while(EXTI->PR &(0x01<<9));
		
		if(PBin(9))
		{
			MY_PRINT("io1 ��ʼ\r\n");
			flagIO1=1;
			flagIOJieGuo=1;
		}
		else
		{
			MY_PRINT("io1 ����\r\n");
		}	


		
	}
	
	
	
}




void IO_tongdao_init(void)
{
	exti9_init( );
	exti0_init( );
	
}

