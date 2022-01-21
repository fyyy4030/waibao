#ifndef _RTC_H_
#define _RTC_H_

#include "stm32f4xx.h"

typedef struct
{
	u8 hour;
	u8 min;
	u8 sec;
	u16 year;
	u8 month;
	u8 day;
	u8 week;
}Type_RTC_DATE;

extern Type_RTC_DATE rtc_data;

void RTC__Init(void);
void Get_RTC_Time(Type_RTC_DATE* rtc_time);
void Get_RTC_DATE(Type_RTC_DATE* rtc_date);
void RTC_Wakeup_Init(u16 wut_num);
void RTC_ALARM_A_Init(void);
#endif
