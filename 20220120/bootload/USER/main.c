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

#define BOOTLOAD_ADDR 0x08000000  //所在扇区：01
#define APP_ADDR      0x08008000  //所在扇区：23456
//#define NEW_232_APP_FILE_NAME "232fw.bin"//短文件名
#define NEW_485_APP_FILE_NAME "485fw.bin"//短文件名"485fw.bin

#define DEV_FW_FILE "dev_fw.txt"//保存当前设备中运行的软件版本号的文件
#define DOWNLOAD_FW_FILE "fw.txt"//记录SD中保存的固件版本
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
		fresult= f_mount (&fs,"0:",1);//将SD卡挂载到文件系统中
		if(FR_OK!=fresult)
		{
			printf("boot:f_mout error\r\n");
			printf("running APP from 0x08008000\r\n");
			printf("--------------------------------------------------------\r\n\r\n");
			iap_load_app(APP_ADDR);				
			
		}
		//查询是否有更新标志文件
		//读取本地软件版本号"dev_fw.txt"
		//1.如果打开文件失败，则表示设备没有用过，直接加载固件更新APP，然后直接删除app固件文件
		//2.打开"dev_fw.txt"文件成功，再打开"fw.txt",如果BanBenID<fw485ID,则加载固件更新APP,更改"fw.txt"文件
		//3.打开文件成功，再打开"fw.txt",如果BanBenID>=fw485ID,直接跳转启动APP
		char ret= sd_read_dev_firmware_version(&BanBenID,&XBanBenID);
		if(ret==1)//打开失败
		{
			new_app_flat=1;		
		}
		else if(ret==0)//打开成功
		{
			if(0==sd_read_firmware_version(&fw485ID))//再打开"fw.txt"
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
			//直接删除app固件文件
				fresult=f_unlink(NEW_485_APP_FILE_NAME);
				if(FR_OK==fresult)
				{
						printf("remove new app \r\n");
				}				
			}
			else if(new_app_flat==2)//更改"fw.txt"文件
			{
				if(0==sd_mark_dev_firmware_version(fw485ID,XBanBenID))
				{
					printf("固件版本号更新成功\r\n");
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
	
	fresult=f_open (&fp, NEW_485_APP_FILE_NAME,FA_OPEN_EXISTING |FA_READ);//打开文件。如果文件不存在，则打开失败。 只读
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




//初始，尝试打开保存在SD卡中的保存当前设备中运行的软件版本号的文件
//如果打开成功，则读出记录的版本号信息
//注意：未启动操作系统时使用
char sd_read_dev_firmware_version(int *BanBenID,int *XBanBenID)
{
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DEV_FW_FILE,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		return  1;
	}
	else//打开成功，则读出记录的版本号信息
	{
		uint8_t revbuf[50]={0};
		fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//读取全部信息
		if(FR_OK!=fresult)
		{
			printf("f_read error\r\n");
			f_close(&fp );
			return 5;
		}			
		printf("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //将字符串解析成json结构体
		if (NULL == root_json)
		{
			printf("解析错误\r\n");
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



//读取保存在SD卡中的从服务器上面下载下来的最新固件版本号,没有保存到固件则返回-1
//注意：未启动操作系统时使用
char sd_read_firmware_version(int *fw485ID)
{
	FRESULT fresult;
	FIL fp;
	UINT bw;
	
	fresult=f_open (&fp, DOWNLOAD_FW_FILE,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		printf("f_open error 3\r\n");
		return 1;
	}	
	
	uint8_t revbuf[50]={0};
	fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//读取全部信息
	if(FR_OK!=fresult)
	{
		printf("f_read error\r\n");
		f_close(&fp );
		return 2;
	}	
	
		printf("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //将字符串解析成json结构体
		if (NULL == root_json)
		{
			printf("解析错误\r\n");
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


//记录当前设备中运行的软件版本号,本机软件版本、连接本机的232设备软件版本
char sd_mark_dev_firmware_version(int BanBenID,int XBanBenID)
{
	
	char ret;
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DEV_FW_FILE,FA_CREATE_ALWAYS|FA_READ|FA_WRITE);// 创建一个新文件。如果文件已存在，则它将被截断并覆盖
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
