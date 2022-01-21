#include "rtc.h"
#include "stdio.h"





//进入初始化模式
void  Into_RTC_Init_Mode(void)
{
	//进入初始化模式(日历计数器将停止工作并且其值可更新)
	RTC->ISR |=0x01<<7;
	//等待进入初始化模式成功(允许更新日历寄存器)
	while( !(RTC->ISR&(0x01<<6)));
}

//退出初始化模式
void  Out_RTC_Init_Mode(void)
{
	//退出初始化模式
	RTC->ISR &=~(0x01<<7);	
	//等到退出初始化模式完毕
	while((RTC->ISR&(0x01<<6)));
}

//十进制转BCD
u8 DEC_2_BCD(u8 dec_data)
{
	return (((dec_data/10)<<4) | (dec_data%10));
}

//BCD转十进制
u8 BCD_2_DEC(u8 bcd_data)
{
	return (((bcd_data&0xf0)>>4)*10 + (bcd_data&0x0f));
}



//设置时间
void Set_RTC_Time(u8 hour,u8 min,u8 sec,u8 am_pm)
{
	u32 temp;
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	//进入初始化模式
	Into_RTC_Init_Mode( );
	//设置时间
	temp=(am_pm<<22|DEC_2_BCD(hour)<<16|DEC_2_BCD(min)<<8|DEC_2_BCD(sec)<<0);
	RTC->TR=temp;
	//退出初始化模式
	Out_RTC_Init_Mode( );
	//开启写保护
	RTC->WPR =0xFF;
}

//设置日期
void Set_RTC_Date( u16 year,u8 month,u8 day)
{
	u32 temp;
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	//进入初始化模式
	Into_RTC_Init_Mode( );
	//设置日期
	temp=(DEC_2_BCD(year-1990)<<16|DEC_2_BCD(month)<<8|DEC_2_BCD(day)<<0);
	RTC->DR=temp;
	//退出初始化模式
	Out_RTC_Init_Mode( );
	//开启写保护
	RTC->WPR =0xFF;
}


//获取时间
void Get_RTC_Time(Type_RTC_DATE* rtc_time)
{
	u32 temp;
	
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	RTC->ISR &= ~(1 << 5);//不同步
	while(!(RTC->ISR & (1 << 5)))
	{
		;//等待同步
	}
	//开启写保护
	RTC->WPR =0xFF;
	
	//读取时间
	temp=RTC->TR;
	rtc_time->hour=BCD_2_DEC((temp&(0x3f<<16))>>16);
	rtc_time->min=BCD_2_DEC((temp&(0x7f<<8))>>8);
	rtc_time->sec=BCD_2_DEC(temp&(0x7f));
}

//获取日期
void Get_RTC_DATE(Type_RTC_DATE* rtc_date)
{
	u32 temp;
	
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	RTC->ISR &= ~(1 << 5);//不同步
	while(!(RTC->ISR & (1 << 5)))
	{
		;//等待同步
	}
	//开启写保护
	RTC->WPR =0xFF;
	
	//读取时间
	temp=RTC->DR;
	rtc_date->year=BCD_2_DEC((temp&(0xff<<16))>>16)+1990;
	rtc_date->month=BCD_2_DEC((temp&(0x1f<<8))>>8);
	rtc_date->day=BCD_2_DEC(temp&(0x3f));
	rtc_date->week=BCD_2_DEC((temp&(0x1<<12))>>12);
}

void RTC__Init(void)
{
  RCC->APB1ENR|=0x01<<28;//使能电源接口时钟
	PWR->CR |=0x01<<8;//使能对 RTC、 RTC 备份寄存器和备份 SRAM 的访问
	RCC->BDCR |=0x01<<0;//打开LSE
	while(!(RCC->BDCR&(0x01<<1)));//等待LSE准备好
	RCC->BDCR &=~(0x03<<8);//选择LSE作为RTC时钟源
	RCC->BDCR |=0x01<<8;
	RCC->BDCR |=0x01<<15;//使能RTC时钟
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	//进入初始化模式(日历计数器将停止工作并且其值可更新)
	Into_RTC_Init_Mode( );
	//RTC相关设置 
	RTC->CR &=~(0x01<<6);//24小时制
	RTC->CR &=~(0x01<<5);//旁路影子寄存器
	RTC->PRER =0;
	RTC->PRER |=0xFF<<0;//同步预分频255+1
	RTC->PRER |=0x7F<<16;//异步预分频127+1
	//退出初始化模式
	Out_RTC_Init_Mode( );
	RTC->WPR =0xFF;//使能写保护（写入错误密钥值）	
	
	//设定时间，日期
	//Set_RTC_Time(15,39,40,0);
	//Set_RTC_Date(2021,12,27);
}

//自动唤醒功能初始化
//wut_num 多少个周期产生唤醒事件标志
void RTC_Wakeup_Init(u16 wut_num)
{
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	//禁止唤醒定时器，并等待允许更新唤醒定时器配置
	RTC->CR &=~(0x01<<10);
	while(!(RTC->ISR&(0x01<<2)));
	
	//唤醒时钟选择--ck_spre时钟（1HZ）
	RTC->CR &=~(0x07<<0);
	RTC->CR |=(0x04<<0);
	
	RTC->WUTR =wut_num;//wut个1hz周期产生一次WUTF标志
	
	RTC->ISR &=~(0x01<<10);//清一次WUTF标志，防止标志错误
	
	//边沿检测设置（上升沿检测）
	EXTI->RTSR |=(0x01<<22);
	EXTI->FTSR &=~(0x01<<22);
//屏蔽软件中断
	EXTI->SWIER &=~(0x01<<22);
//屏蔽事件
	EXTI->EMR &=~(0x01<<22);
//模块级中断使能
	EXTI->IMR |=(0x01<<22);
//系统级中断使能
	NVIC_SetPriority(RTC_WKUP_IRQn, NVIC_EncodePriority(7-2, 2, 2));//设置优先级
	NVIC_EnableIRQ(RTC_WKUP_IRQn);//使能中断
	
	RTC->CR |=(0x01<<14);//使能唤醒定时器中断	
	RTC->CR |=(0x01<<10);//使能唤醒定时器
	
	RTC->WPR =0xFF;//激活写保护

	
}

Type_RTC_DATE rtc_data;

void RTC_WKUP_IRQHandler(void)
{
	EXTI->PR |=(0x01<<22);//清除中断标志
	while(EXTI->PR & (0x01<<22));//等待清除中断标志完毕
	RTC->WPR = 0XCA;
	RTC->WPR = 0X53;//去除写保护
	
	RTC->ISR &= ~(1 << 10);
		
	RTC->WPR = 0XFF;//激活写保护
	
	Get_RTC_Time(&rtc_data);
	Get_RTC_DATE(&rtc_data);
	
}




//闹钟A
//每分钟的第5秒产生中断
void RTC_ALARM_A_Init(void)
{
	//取消写保护（密钥0xCA 0x53）
	RTC->WPR =0xCA;
	RTC->WPR =0x53;
	
	//关闭闹钟A，并等待允许更新闹钟A
	RTC->CR &=~(0x01<<8);
	while(!(RTC->ISR&(0x01<<0)));
	
	RTC->ALRMAR |=(0x01<<31);//日期无关
	RTC->ALRMAR |=(0x01<<30);//星期无关
	RTC->ALRMAR |=(0x01<<23);//小时无关
	RTC->ALRMAR |=(0x01<<15);//分钟无关
	//5秒
	RTC->ALRMAR &=~(0x01<<7);
	RTC->ALRMAR &=~(0x7f<<0);
	RTC->ALRMAR |=DEC_2_BCD(5);
	
	RTC->ISR &=~(0x01<<8);//清一次WUTF标志，防止标志错误
	
	//边沿检测设置（上升沿检测）
	EXTI->RTSR |=(0x01<<17);
	EXTI->FTSR &=~(0x01<<17);
//屏蔽软件中断
	EXTI->SWIER &=~(0x01<<17);
//屏蔽事件
	EXTI->EMR &=~(0x01<<17);
//模块级中断使能
	EXTI->IMR |=(0x01<<17);
//系统级中断使能
	NVIC_SetPriority(RTC_Alarm_IRQn, NVIC_EncodePriority(7-2, 2, 2));//设置优先级
	NVIC_EnableIRQ(RTC_Alarm_IRQn);//使能中断
	
	RTC->CR |=(0x01<<12);//使能闹钟A中断
	RTC->CR |=(0x01<<8);//使能闹钟A
	RTC->WPR =0xFF;//激活写保护
}

void RTC_Alarm_IRQHandler(void)
{
	EXTI->PR |=(0x01<<17);//清除中断标志
	while(EXTI->PR & (0x01<<17));//等待清除中断标志完毕
	RTC->WPR = 0XCA;
	RTC->WPR = 0X53;//去除写保护
	
	RTC->ISR &= ~(1 << 8);
		
	RTC->WPR = 0XFF;//激活写保护
	
	//LED1=!LED1;
}




