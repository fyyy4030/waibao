#include "stm32f4xx.h"

typedef void(*iapfun)(void);
iapfun jump2app;


__asm void MSR_MSP(uint32_t addr)
{
    MSR MSP, r0
    BX r14;
}
//跳转到应用程序段
//appxaddr:用户代码起始地址.
void iap_load_app(u32 appxaddr)
{
	if(((*(uint32_t *)appxaddr)&0x1FFE0000)==0x10000000)	//检查栈顶地址是否合法.
	{ 
		jump2app=(iapfun)*(uint32_t*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
		MSR_MSP(*(uint32_t*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		for(int i = 0; i < 8; i++)
		{			
			NVIC->ICER[i] = 0xFFFFFFFF;	/* 关闭中断*/
			NVIC->ICPR[i] = 0xFFFFFFFF;	/* 清除中断标志位 */
		}
		jump2app();									//跳转到APP.
	}
	else
	{
		printf("no app\r\n");
	}
}



////跳转到应用程序段
////appxaddr:用户代码起始地址.
//void iap_load_app(u32 appxaddr)
//{
//	void (*jump2app)();//定义一个函数指针
//	
//	
//	if(((*(uint32_t *)appxaddr)&0x2FFE0000)==0x20000000)	//检查栈顶地址是否合法.
//	{ 
//		jump2app=(void(*)())*(uint32_t*)(appxaddr+4);		//用户代码区第二个字为程序开始地址(复位地址)		
//		MSR_MSP(*(uint32_t*)appxaddr);					//初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
//		for(int i = 0; i < 8; i++)
//		{			
//			NVIC->ICER[i] = 0xFFFFFFFF;	/* 关闭中断*/
//			NVIC->ICPR[i] = 0xFFFFFFFF;	/* 清除中断标志位 */
//		}
//		jump2app();									//跳转到APP.
//	}
//	else
//	{
//		printf("no app\r\n");
//	}
//}