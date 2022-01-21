#include "stm32f4xx.h"

typedef void(*iapfun)(void);
iapfun jump2app;


__asm void MSR_MSP(uint32_t addr)
{
    MSR MSP, r0
    BX r14;
}
//��ת��Ӧ�ó����
//appxaddr:�û�������ʼ��ַ.
void iap_load_app(u32 appxaddr)
{
	if(((*(uint32_t *)appxaddr)&0x1FFE0000)==0x10000000)	//���ջ����ַ�Ƿ�Ϸ�.
	{ 
		jump2app=(iapfun)*(uint32_t*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
		MSR_MSP(*(uint32_t*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
		for(int i = 0; i < 8; i++)
		{			
			NVIC->ICER[i] = 0xFFFFFFFF;	/* �ر��ж�*/
			NVIC->ICPR[i] = 0xFFFFFFFF;	/* ����жϱ�־λ */
		}
		jump2app();									//��ת��APP.
	}
	else
	{
		printf("no app\r\n");
	}
}



////��ת��Ӧ�ó����
////appxaddr:�û�������ʼ��ַ.
//void iap_load_app(u32 appxaddr)
//{
//	void (*jump2app)();//����һ������ָ��
//	
//	
//	if(((*(uint32_t *)appxaddr)&0x2FFE0000)==0x20000000)	//���ջ����ַ�Ƿ�Ϸ�.
//	{ 
//		jump2app=(void(*)())*(uint32_t*)(appxaddr+4);		//�û��������ڶ�����Ϊ����ʼ��ַ(��λ��ַ)		
//		MSR_MSP(*(uint32_t*)appxaddr);					//��ʼ��APP��ջָ��(�û��������ĵ�һ�������ڴ��ջ����ַ)
//		for(int i = 0; i < 8; i++)
//		{			
//			NVIC->ICER[i] = 0xFFFFFFFF;	/* �ر��ж�*/
//			NVIC->ICPR[i] = 0xFFFFFFFF;	/* ����жϱ�־λ */
//		}
//		jump2app();									//��ת��APP.
//	}
//	else
//	{
//		printf("no app\r\n");
//	}
//}