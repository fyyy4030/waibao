#include "rtc.h"
#include "stdio.h"





//�����ʼ��ģʽ
void  Into_RTC_Init_Mode(void)
{
	//�����ʼ��ģʽ(������������ֹͣ����������ֵ�ɸ���)
	RTC->ISR |=0x01<<7;
	//�ȴ������ʼ��ģʽ�ɹ�(������������Ĵ���)
	while( !(RTC->ISR&(0x01<<6)));
}

//�˳���ʼ��ģʽ
void  Out_RTC_Init_Mode(void)
{
	//�˳���ʼ��ģʽ
	RTC->ISR &=~(0x01<<7);	
	//�ȵ��˳���ʼ��ģʽ���
	while((RTC->ISR&(0x01<<6)));
}

//ʮ����תBCD
u8 DEC_2_BCD(u8 dec_data)
{
	return (((dec_data/10)<<4) | (dec_data%10));
}

//BCDתʮ����
u8 BCD_2_DEC(u8 bcd_data)
{
	return (((bcd_data&0xf0)>>4)*10 + (bcd_data&0x0f));
}



//����ʱ��
void Set_RTC_Time(u8 hour,u8 min,u8 sec,u8 am_pm)
{
	u32 temp;
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	//�����ʼ��ģʽ
	Into_RTC_Init_Mode( );
	//����ʱ��
	temp=(am_pm<<22|DEC_2_BCD(hour)<<16|DEC_2_BCD(min)<<8|DEC_2_BCD(sec)<<0);
	RTC->TR=temp;
	//�˳���ʼ��ģʽ
	Out_RTC_Init_Mode( );
	//����д����
	RTC->WPR =0xFF;
}

//��������
void Set_RTC_Date( u16 year,u8 month,u8 day)
{
	u32 temp;
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	//�����ʼ��ģʽ
	Into_RTC_Init_Mode( );
	//��������
	temp=(DEC_2_BCD(year-1990)<<16|DEC_2_BCD(month)<<8|DEC_2_BCD(day)<<0);
	RTC->DR=temp;
	//�˳���ʼ��ģʽ
	Out_RTC_Init_Mode( );
	//����д����
	RTC->WPR =0xFF;
}


//��ȡʱ��
void Get_RTC_Time(Type_RTC_DATE* rtc_time)
{
	u32 temp;
	
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	RTC->ISR &= ~(1 << 5);//��ͬ��
	while(!(RTC->ISR & (1 << 5)))
	{
		;//�ȴ�ͬ��
	}
	//����д����
	RTC->WPR =0xFF;
	
	//��ȡʱ��
	temp=RTC->TR;
	rtc_time->hour=BCD_2_DEC((temp&(0x3f<<16))>>16);
	rtc_time->min=BCD_2_DEC((temp&(0x7f<<8))>>8);
	rtc_time->sec=BCD_2_DEC(temp&(0x7f));
}

//��ȡ����
void Get_RTC_DATE(Type_RTC_DATE* rtc_date)
{
	u32 temp;
	
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	RTC->ISR &= ~(1 << 5);//��ͬ��
	while(!(RTC->ISR & (1 << 5)))
	{
		;//�ȴ�ͬ��
	}
	//����д����
	RTC->WPR =0xFF;
	
	//��ȡʱ��
	temp=RTC->DR;
	rtc_date->year=BCD_2_DEC((temp&(0xff<<16))>>16)+1990;
	rtc_date->month=BCD_2_DEC((temp&(0x1f<<8))>>8);
	rtc_date->day=BCD_2_DEC(temp&(0x3f));
	rtc_date->week=BCD_2_DEC((temp&(0x1<<12))>>12);
}

void RTC__Init(void)
{
  RCC->APB1ENR|=0x01<<28;//ʹ�ܵ�Դ�ӿ�ʱ��
	PWR->CR |=0x01<<8;//ʹ�ܶ� RTC�� RTC ���ݼĴ����ͱ��� SRAM �ķ���
	RCC->BDCR |=0x01<<0;//��LSE
	while(!(RCC->BDCR&(0x01<<1)));//�ȴ�LSE׼����
	RCC->BDCR &=~(0x03<<8);//ѡ��LSE��ΪRTCʱ��Դ
	RCC->BDCR |=0x01<<8;
	RCC->BDCR |=0x01<<15;//ʹ��RTCʱ��
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	//�����ʼ��ģʽ(������������ֹͣ����������ֵ�ɸ���)
	Into_RTC_Init_Mode( );
	//RTC������� 
	RTC->CR &=~(0x01<<6);//24Сʱ��
	RTC->CR &=~(0x01<<5);//��·Ӱ�ӼĴ���
	RTC->PRER =0;
	RTC->PRER |=0xFF<<0;//ͬ��Ԥ��Ƶ255+1
	RTC->PRER |=0x7F<<16;//�첽Ԥ��Ƶ127+1
	//�˳���ʼ��ģʽ
	Out_RTC_Init_Mode( );
	RTC->WPR =0xFF;//ʹ��д������д�������Կֵ��	
	
	//�趨ʱ�䣬����
	//Set_RTC_Time(15,39,40,0);
	//Set_RTC_Date(2021,12,27);
}

//�Զ����ѹ��ܳ�ʼ��
//wut_num ���ٸ����ڲ��������¼���־
void RTC_Wakeup_Init(u16 wut_num)
{
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	//��ֹ���Ѷ�ʱ�������ȴ�������»��Ѷ�ʱ������
	RTC->CR &=~(0x01<<10);
	while(!(RTC->ISR&(0x01<<2)));
	
	//����ʱ��ѡ��--ck_spreʱ�ӣ�1HZ��
	RTC->CR &=~(0x07<<0);
	RTC->CR |=(0x04<<0);
	
	RTC->WUTR =wut_num;//wut��1hz���ڲ���һ��WUTF��־
	
	RTC->ISR &=~(0x01<<10);//��һ��WUTF��־����ֹ��־����
	
	//���ؼ�����ã������ؼ�⣩
	EXTI->RTSR |=(0x01<<22);
	EXTI->FTSR &=~(0x01<<22);
//��������ж�
	EXTI->SWIER &=~(0x01<<22);
//�����¼�
	EXTI->EMR &=~(0x01<<22);
//ģ�鼶�ж�ʹ��
	EXTI->IMR |=(0x01<<22);
//ϵͳ���ж�ʹ��
	NVIC_SetPriority(RTC_WKUP_IRQn, NVIC_EncodePriority(7-2, 2, 2));//�������ȼ�
	NVIC_EnableIRQ(RTC_WKUP_IRQn);//ʹ���ж�
	
	RTC->CR |=(0x01<<14);//ʹ�ܻ��Ѷ�ʱ���ж�	
	RTC->CR |=(0x01<<10);//ʹ�ܻ��Ѷ�ʱ��
	
	RTC->WPR =0xFF;//����д����

	
}

Type_RTC_DATE rtc_data;

void RTC_WKUP_IRQHandler(void)
{
	EXTI->PR |=(0x01<<22);//����жϱ�־
	while(EXTI->PR & (0x01<<22));//�ȴ�����жϱ�־���
	RTC->WPR = 0XCA;
	RTC->WPR = 0X53;//ȥ��д����
	
	RTC->ISR &= ~(1 << 10);
		
	RTC->WPR = 0XFF;//����д����
	
	Get_RTC_Time(&rtc_data);
	Get_RTC_DATE(&rtc_data);
	
}




//����A
//ÿ���ӵĵ�5������ж�
void RTC_ALARM_A_Init(void)
{
	//ȡ��д��������Կ0xCA 0x53��
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	//�ر�����A�����ȴ������������A
	RTC->CR &=~(0x01<<8);
	while(!(RTC->ISR&(0x01<<0)));
	
	RTC->ALRMAR |=(0x01<<31);//�����޹�
	RTC->ALRMAR |=(0x01<<30);//�����޹�
	RTC->ALRMAR |=(0x01<<23);//Сʱ�޹�
	RTC->ALRMAR |=(0x01<<15);//�����޹�
	//5��
	RTC->ALRMAR &=~(0x01<<7);
	RTC->ALRMAR &=~(0x7f<<0);
	RTC->ALRMAR |=DEC_2_BCD(5);
	
	RTC->ISR &=~(0x01<<8);//��һ��WUTF��־����ֹ��־����
	
	//���ؼ�����ã������ؼ�⣩
	EXTI->RTSR |=(0x01<<17);
	EXTI->FTSR &=~(0x01<<17);
//��������ж�
	EXTI->SWIER &=~(0x01<<17);
//�����¼�
	EXTI->EMR &=~(0x01<<17);
//ģ�鼶�ж�ʹ��
	EXTI->IMR |=(0x01<<17);
//ϵͳ���ж�ʹ��
	NVIC_SetPriority(RTC_Alarm_IRQn, NVIC_EncodePriority(7-2, 2, 2));//�������ȼ�
	NVIC_EnableIRQ(RTC_Alarm_IRQn);//ʹ���ж�
	
	RTC->CR |=(0x01<<12);//ʹ������A�ж�
	RTC->CR |=(0x01<<8);//ʹ������A
	RTC->WPR =0xFF;//����д����
}

void RTC_Alarm_IRQHandler(void)
{
	EXTI->PR |=(0x01<<17);//����жϱ�־
	while(EXTI->PR & (0x01<<17));//�ȴ�����жϱ�־���
	RTC->WPR = 0XCA;
	RTC->WPR = 0X53;//ȥ��д����
	
	RTC->ISR &= ~(1 << 8);
		
	RTC->WPR = 0XFF;//����д����
	
	//LED1=!LED1;
}




