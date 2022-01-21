#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stm32f4xx.h"
#include "sd_sdio.h"
#include "diskio.h"
#include "ff.h"
#include "debug_uart.h"
#include "cJSON.h"
#include "led.h"

static void delay_microsecond(uint32_t microsecond)
{
	microsecond *= 42;
	
	for(uint32_t i = 0; i < microsecond; i++)
	{
		;
	}
}


void iap_load_app(u32 appxaddr);

#define BOOTLOAD_ADDR 0x08000000  //����������01
#define APP_ADDR      0x08008000  //����������23456
//#define NEW_232_APP_FILE_NAME "232fw.bin"//���ļ���
#define NEW_485_APP_FILE_NAME "485fw.bin"//���ļ���"485fw.bin

#define DEV_FW_FILE "dev_fw.txt"//���浱ǰ�豸�����е�����汾�ŵ��ļ�
#define DOWNLOAD_FW_FILE "fw.txt"//��¼SD�б���Ĺ̼��汾
char sd_read_firmware_version(int *fw485ID);
char sd_read_dev_firmware_version(int *BanBenID,int *XBanBenID);
char sd_mark_dev_firmware_version(int BanBenID,int XBanBenID);
char updata_app_from_sd_card(void);

int main(void)
{
	uint8_t new_app_flat=0;
	int fw485ID=-1;
	int BanBenID=-1;
	int XBanBenID=-1;
	
	debug_uartConfig(115200);
	LED_Init( );

	FATFS       fs;
	FRESULT fresult;
	if(SD_OK!=SD_Init( ))
	{
		printf("sd_init err\r\n");
	}
	else
	{
		fresult= f_mount (&fs,"0:",1);//��SD�����ص��ļ�ϵͳ��
		if(FR_OK!=fresult)
		{
			printf("boot:f_mout error\r\n");
			printf("running APP from 0x08008000\r\n");
			printf("--------------------------------------------------------\r\n\r\n");
			iap_load_app(APP_ADDR);				
			
		}
		//��ѯ�Ƿ��и��±�־�ļ�
		//��ȡ��������汾��"dev_fw.txt"
		//1.������ļ�ʧ�ܣ����ʾ�豸û���ù���ֱ�Ӽ��ع̼�����APP��Ȼ��ֱ��ɾ��app�̼��ļ�
		//2.��"dev_fw.txt"�ļ��ɹ����ٴ�"fw.txt",���BanBenID<fw485ID,����ع̼�����APP,����"fw.txt"�ļ�
		//3.���ļ��ɹ����ٴ�"fw.txt",���BanBenID>=fw485ID,ֱ����ת����APP
		char ret= sd_read_dev_firmware_version(&BanBenID,&XBanBenID);
		if(ret==1)//��ʧ��
		{
			new_app_flat=1;		
		}
		else if(ret==0)//�򿪳ɹ�
		{
			if(0==sd_read_firmware_version(&fw485ID))//�ٴ�"fw.txt"
			{
				if(BanBenID<fw485ID)
				{
					new_app_flat=2;		
				}
			}
		}
	}	
	
	if(new_app_flat>0)
	{
		if(0==updata_app_from_sd_card( ))
		{
			printf("update app ok\r\n");
			LED1_ON;LED2_ON;LED3_ON;LED4_ON;
			delay_microsecond(1000*300);
			LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;
			delay_microsecond(1000*300);			
			LED1_ON;LED2_ON;LED3_ON;LED4_ON;
			delay_microsecond(1000*300);
			LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;
			delay_microsecond(1000*300);
			LED1_ON;LED2_ON;LED3_ON;LED4_ON;
			delay_microsecond(1000*300);
			LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;
			delay_microsecond(1000*300);
			LED1_ON;LED2_ON;LED3_ON;LED4_ON;
			delay_microsecond(1000*300);
			LED1_OFF;LED2_OFF;LED3_OFF;LED4_OFF;
			delay_microsecond(1000*300);			
			if(new_app_flat==1)
			{
			//ֱ��ɾ��app�̼��ļ�
				fresult=f_unlink(NEW_485_APP_FILE_NAME);
				if(FR_OK==fresult)
				{
						printf("remove new app \r\n");
				}				
			}
			else if(new_app_flat==2)//����"fw.txt"�ļ�
			{
				if(0==sd_mark_dev_firmware_version(fw485ID,XBanBenID))
				{
					printf("�̼��汾�Ÿ��³ɹ�\r\n");
				}
			}
		}
		else
		{
			printf("update app err,please restart device\r\n");
			f_mount (&fs,"0:",0);
			while(1){;}
		}
	}
	
	f_mount (&fs,"0:",0);
	printf("running APP from 0x08008000\r\n");
	printf("--------------------------------------------------------\r\n\r\n");
	iap_load_app(APP_ADDR);	
	
	

	while(1)
	{
		;
	}
}




char updata_app_from_sd_card(void)
{
	uint32_t updata_addr=APP_ADDR;
	
	FLASH_Unlock( );
	FLASH_DataCacheCmd(DISABLE);
	FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGAERR|FLASH_FLAG_WRPERR);
	FLASH_EraseSector(FLASH_Sector_2, VoltageRange_3);printf("FLASH_EraseSector(FLASH_Sector_2)\r\n");
	FLASH_EraseSector(FLASH_Sector_3, VoltageRange_3);printf("FLASH_EraseSector(FLASH_Sector_3)\r\n");
	FLASH_EraseSector(FLASH_Sector_4, VoltageRange_3);printf("FLASH_EraseSector(FLASH_Sector_4)\r\n");
	FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);printf("FLASH_EraseSector(FLASH_Sector_5)\r\n");
	FLASH_EraseSector(FLASH_Sector_6, VoltageRange_3);printf("FLASH_EraseSector(FLASH_Sector_6)\r\n");
	FLASH_DataCacheCmd(ENABLE);
		

	FRESULT fresult;
	FIL fp;
	UINT bw;
	uint8_t app_buf[1024]={0};
	uint32_t size=0;
	
	fresult=f_open (&fp, NEW_485_APP_FILE_NAME,FA_OPEN_EXISTING |FA_READ);//���ļ�������ļ������ڣ����ʧ�ܡ� ֻ��
	if(FR_OK!=fresult){
		FLASH_Lock( );
		printf("f_open error\r\n");
		return 2;
	}
	
	while(1)
	{
		fresult= f_read(&fp,app_buf,1024,&bw);
		if(FR_OK!=fresult)
		{
			f_close(&fp );	
			FLASH_Lock( );
			printf("f_read error\r\n");
			return 3;
		}	
		if(bw>0)
		{
			for(int i=0;i<bw;i++)
			{
				FLASH_ProgramByte(updata_addr+i, app_buf[i]);
			}
			printf("from 0x%x,writen %d bytes......\r\n",updata_addr,bw);
			updata_addr+=bw;
			size+=bw;
		}
		if(bw<1024)
		{
			printf("update over!!!!!  %dbytes\r\n",size);
			break;
		}
		
	}
	f_close(&fp );
	FLASH_Lock( );
	return 0;	
	
}




//��ʼ�����Դ򿪱�����SD���еı��浱ǰ�豸�����е�����汾�ŵ��ļ�
//����򿪳ɹ����������¼�İ汾����Ϣ
//ע�⣺δ��������ϵͳʱʹ��
char sd_read_dev_firmware_version(int *BanBenID,int *XBanBenID)
{
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DEV_FW_FILE,FA_OPEN_EXISTING|FA_READ);//����ļ������ڣ����ʧ�ܡ�(Ĭ��)
	if(FR_OK!=fresult)
	{
		return  1;
	}
	else//�򿪳ɹ����������¼�İ汾����Ϣ
	{
		uint8_t revbuf[50]={0};
		fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//��ȡȫ����Ϣ
		if(FR_OK!=fresult)
		{
			printf("f_read error\r\n");
			f_close(&fp );
			return 5;
		}			
		printf("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //���ַ���������json�ṹ��
		if (NULL == root_json)
		{
			printf("��������\r\n");
			cJSON_Delete(root_json);
			f_close(&fp );
			return 6;
		}		
		cJSON *Item= cJSON_GetObjectItem(root_json, "BanBenID");
		if(Item!=NULL)
			*BanBenID=Item->valueint;
		Item= cJSON_GetObjectItem(root_json, "XBanBenID");
		if(Item!=NULL)
			*XBanBenID=Item->valueint;			
		
		cJSON_Delete(root_json);	
		f_close(&fp );
		return 0;	
	}

		
}



//��ȡ������SD���еĴӷ����������������������¹̼��汾��,û�б��浽�̼��򷵻�-1
//ע�⣺δ��������ϵͳʱʹ��
char sd_read_firmware_version(int *fw485ID)
{
	FRESULT fresult;
	FIL fp;
	UINT bw;
	
	fresult=f_open (&fp, DOWNLOAD_FW_FILE,FA_OPEN_EXISTING|FA_READ);//����ļ������ڣ����ʧ�ܡ�(Ĭ��)
	if(FR_OK!=fresult)
	{
		printf("f_open error 3\r\n");
		return 1;
	}	
	
	uint8_t revbuf[50]={0};
	fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//��ȡȫ����Ϣ
	if(FR_OK!=fresult)
	{
		printf("f_read error\r\n");
		f_close(&fp );
		return 2;
	}	
	
		printf("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //���ַ���������json�ṹ��
		if (NULL == root_json)
		{
			printf("��������\r\n");
			cJSON_Delete(root_json);
			f_close(&fp );
			return 3;
		}		
		cJSON *Item= cJSON_GetObjectItem(root_json, "485_fw");
		if(Item!=NULL)
			*fw485ID=Item->valueint;
		
		cJSON_Delete(root_json);		
		f_close(&fp );	
		return 0;
}


//��¼��ǰ�豸�����е�����汾��,��������汾�����ӱ�����232�豸����汾
char sd_mark_dev_firmware_version(int BanBenID,int XBanBenID)
{
	
	char ret;
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DEV_FW_FILE,FA_CREATE_ALWAYS|FA_READ|FA_WRITE);// ����һ�����ļ�������ļ��Ѵ��ڣ����������ضϲ�����
	if(FR_OK!=fresult)
	{
		printf("f_open error 6\r\n");
		ret=2;
		goto EXIT1;
	}	
	char string[50]={0};
	sprintf(string,"{\"BanBenID\":%d,\"XBanBenID\":%d}",BanBenID,XBanBenID);
	
	fresult= f_write(&fp,string,strlen((const char *)string),&bw);
	if(FR_OK!=fresult)
	{
		printf("f_write error\r\n");
		free(string);
		ret=4;
		goto EXIT2;
	}	
	free(string);
	
	ret=0;
	EXIT2:
	f_close(&fp );	
	EXIT1:
	return ret;			
}
