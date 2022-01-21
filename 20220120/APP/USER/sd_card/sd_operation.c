#include "sd_operation.h"
#include "cjson.h"
#include "rtc.h"
#include "4G.h"
#include "crc16.h"

#include "my_print.h"

#define READY 1   //就绪
#define NOT_READY 0 //未检测到卡

extern uint8_t flagSDReady;//SD卡就绪标志

/*
 *  功 能: 扫描并打印文件夹中所有文件的名字，包括短文件名和长文件名
 *         并记录文件夹中文件的个数(注：文件夹和隐藏文件不处理) 
 *	参数1: 文件夹的路径 例如 "0:/setting"
 *  参数2: 记录文件个数的指针
 */
static void cal_file_count_of_dir(uint8_t *dir_path, uint16_t *file_count)
{
    uint16_t count = 0;
    FRESULT ret;    // 文件操作结果
    FATFS fs;       // FatFs文件系统对象
    DIR dp;
    FIL fp;
    FILINFO Fileinfo;
      
       
    ret = f_opendir(&dp, (const TCHAR*)dir_path);
    if (ret == FR_NO_PATH)  // 没有该文件夹,创建一个
    {
        MY_PRINT("没有 %s\n", dir_path);
				f_mkdir ((const TCHAR*)dir_path);	
				if(file_count!=NULL)
					*file_count=0;
				return;
    }                                               
    else if (ret != FR_OK)
    {
        MY_PRINT("打开 %s 失败\n", dir_path);
				if(file_count!=NULL)
					*file_count=0;
				return;
    }
    else	// 打开成功
    {
        while(1)
        {
            ret = f_readdir(&dp, &Fileinfo);
            if(ret != FR_OK || Fileinfo.fname[0]==0 )
            {
                break;		// 读取失败或者读取完所有条目
            }
            else if (Fileinfo.fname[0] == '.')	// 隐藏文件
            {
                continue;
            }
            else if(Fileinfo.fattrib & AM_DIR)	// 是文件夹
            {
                MY_PRINT("Directory\n");
                continue;	// 文件夹不处理，继续读下一个
            }
            else
            {   
							MY_PRINT("Fileinfo.fname:%s\r\n", Fileinfo.fname);	// 打印文件名
							if(strstr(Fileinfo.fname,"record"))
							{
								count++;	// 记录文件的个数
							}
							
//                if(Fileinfo.lfname[0] == 0)		// 先判断长文件名，要是先判断短文件名会出错，不知道为啥
//                {
//                    if (Fileinfo.fname[0] != 0)
//                    {
//                        count++;
//                        MY_PRINT("%s\n", Fileinfo.fname);	// 打印短文件名
//                    }
//                }
//                else
//                {
//                    count++;	// 记录文件的个数
//                    MY_PRINT("%s\n", Fileinfo.lfname);	// 打印长文件名
//                }
            }  
        }
    }
    if(file_count!=NULL)
			*file_count = count;
    f_closedir(&dp);	// 打开之后要关闭文件夹

}


void sd_read_path(void)
{
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		return ;
	}
	cal_file_count_of_dir((uint8_t *)CONFIG_DIR, NULL);
	cal_file_count_of_dir((uint8_t *)OFFLINE_DIR, NULL);
	return;	
}




//读取设备默认配置
char sd_read_defaultconfig(char *onlySaoma,char* SBBH,uint32_t* CJID,int*SaoMaBaud,char *TDJZh,char *ZaXiang)
{
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		return 1;
	}
	FRESULT fresult;
	FIL fp;
	UINT bw;
	
	fresult=f_open (&fp, CONFIG_DEFAULT_FILE,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("打开失败（文件不存在）\r\n");
		//新建一个，写入默认的值
		FIL fp1;
		fresult=f_open (&fp1, CONFIG_DEFAULT_FILE,FA_OPEN_ALWAYS|FA_READ|FA_WRITE);// 如果文件存在，则打开；否则，创建一个新文件。
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_open error 2\r\n");
			return 2;
		}		
		char default_onlySaoma=0;
		char *default_sbbh="BFEBFBFF00030679";
		uint32_t default_cjid=5;
		int default_saoma_baud=115200;
		char *default_jzh="1131";
		char *default_ZaXiang="屏保$线别$TDXianBie$TDLX";
		
		cJSON *pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, "onlySaoMa", default_onlySaoma);
		cJSON_AddStringToObject(pRoot, "SBBH",default_sbbh);
		cJSON_AddNumberToObject(pRoot, "CJID",default_cjid);
		cJSON_AddNumberToObject(pRoot, "SaoMaBaud", default_saoma_baud);
		cJSON_AddStringToObject(pRoot, "TDJZh",default_jzh);
		cJSON_AddStringToObject(pRoot, "ZaXiang",default_ZaXiang);
		char *string=cJSON_PrintUnformatted(pRoot);
		if(string == NULL){
			MY_PRINT("Failed to print pRoot.\n");
			cJSON_Delete(pRoot);
			return 3;
		}
		cJSON_Delete(pRoot);	

		fresult= f_write(&fp1,string,strlen((const char *)string),&bw);
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_write error\r\n");
			free(string);
			return 4;
		}			
		free(string);
		f_close(&fp1 );	
		
		*onlySaoma=false;
		strcpy((char *)SBBH,default_sbbh);
		*CJID=default_cjid;
		*SaoMaBaud=115200;
		strcpy((char *)TDJZh,default_jzh);
		strcpy((char *)ZaXiang,default_ZaXiang);
		return 0;	
	}		
	else//打开成功，则读出厂家信息
	{
		uint8_t revbuf[200]={0};
		fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//读取全部信息
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_read error\r\n");
			f_close(&fp );
			return 5;
		}			
		MY_PRINT("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //将字符串解析成json结构体
		if (NULL == root_json)
		{
			MY_PRINT("解析错误\r\n");
			cJSON_Delete(root_json);
			f_close(&fp );
			return 6;
		}		
		cJSON *Item= cJSON_GetObjectItem(root_json, "onlySaoMa");
		if(Item!=NULL)
			*onlySaoma=Item->valueint;
		
		Item= cJSON_GetObjectItem(root_json, "SBBH");
		if(Item!=NULL)
			strcpy((char *)SBBH,Item->valuestring);
		
		Item= cJSON_GetObjectItem(root_json, "CJID");
		if(Item!=NULL)
			*CJID=Item->valueint;
		
		Item= cJSON_GetObjectItem(root_json, "SaoMaBaud");
		if(Item!=NULL)
			*SaoMaBaud=Item->valueint;	
		
		Item= cJSON_GetObjectItem(root_json, "TDJZh");
		if(Item!=NULL)
			strcpy((char *)TDJZh,Item->valuestring);		
		
		Item= cJSON_GetObjectItem(root_json, "ZaXiang");
		if(Item!=NULL)
			strcpy((char *)ZaXiang,Item->valuestring);			
		
		cJSON_Delete(root_json);	
		f_close(&fp );
		return 0;			
	}		
}

//#define   WIFISSID      "Moweihao-iPhone"
//#define   WIFIPWD       "123123123"
//#define   SERVER_IP     "8.134.112.231"   
//#define   SERVER_PORT   8099 
//#define   URL           "/ShuJu/QingQiu"

char sd_read_network_config(char* wifissid,char* wifipwd,char *server_ip,int *server_port,char *url)
{
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		return 1;
	}
	FRESULT fresult;
	FIL fp;
	UINT bw;
	
	fresult=f_open (&fp, CONFIG_NETWORK_FILE,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("打开失败（文件不存在）\r\n");
		//新建一个，写入默认的值
		FIL fp1;
		fresult=f_open (&fp1, CONFIG_NETWORK_FILE,FA_OPEN_ALWAYS|FA_READ|FA_WRITE);// 如果文件存在，则打开；否则，创建一个新文件。
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_open error 2\r\n");
			return 2;
		}		
		char *default_wifissid="wifissid";
		char *default_wifipwd="wifipwd";
		char *default_server_ip="8.134.112.231";
		int default_server_port=8099;
		char *default_url="/ShuJu/QingQiu";
		
		cJSON *pRoot = cJSON_CreateObject();
		cJSON_AddStringToObject(pRoot, "WIFISSID",default_wifissid);
		cJSON_AddStringToObject(pRoot, "WIFIPWD",default_wifipwd);
		cJSON_AddStringToObject(pRoot, "SERVER_IP",default_server_ip);
		cJSON_AddNumberToObject(pRoot, "SERVER_PORT", default_server_port);
		cJSON_AddStringToObject(pRoot, "URL",default_url);
		char *string=cJSON_PrintUnformatted(pRoot);
		if(string == NULL){
			MY_PRINT("Failed to print pRoot.\n");
			cJSON_Delete(pRoot);
			return 3;
		}
		cJSON_Delete(pRoot);	

		fresult= f_write(&fp1,string,strlen((const char *)string),&bw);
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_write error\r\n");
			free(string);
			return 4;
		}			
		free(string);
		f_close(&fp1 );	
		
		strcpy((char *)wifissid,default_wifissid);
		strcpy((char *)wifipwd,default_wifipwd);
		strcpy((char *)server_ip,default_server_ip);
		*server_port=default_server_port;
		strcpy((char *)url,default_url);
		
		return 0;	
	}		
	else//打开成功，则读出
	{
		uint8_t revbuf[200]={0};
		fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//读取全部信息
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_read error\r\n");
			f_close(&fp );
			return 5;
		}			
		MY_PRINT("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //将字符串解析成json结构体
		if (NULL == root_json)
		{
			MY_PRINT("解析错误\r\n");
			cJSON_Delete(root_json);
			f_close(&fp );
			return 6;
		}		
		cJSON *Item= cJSON_GetObjectItem(root_json, "WIFISSID");
		if(Item!=NULL&&wifissid!=NULL)
			strcpy((char *)wifissid,Item->valuestring);
		
		Item= cJSON_GetObjectItem(root_json, "WIFIPWD");
		if(Item!=NULL&&wifipwd!=NULL)
			strcpy((char *)wifipwd,Item->valuestring);
		
		Item= cJSON_GetObjectItem(root_json, "SERVER_IP");
		if(Item!=NULL&&server_ip!=NULL)
			strcpy((char *)server_ip,Item->valuestring);	

		Item= cJSON_GetObjectItem(root_json, "SERVER_PORT");
		if(Item!=NULL&&server_port!=NULL)
			*server_port=Item->valueint;

		Item= cJSON_GetObjectItem(root_json, "URL");
		if(Item!=NULL&&url!=NULL)
			strcpy((char *)url,Item->valuestring);			
		
		cJSON_Delete(root_json);	
		f_close(&fp );
		return 0;			
	}	
}
	



#define DEV_FW_FILE "dev_fw.txt"//保存当前设备中运行的软件版本号的文件
//初始，尝试打开保存在SD卡中的保存当前设备中运行的软件版本号的文件，如果打开失败（不存在），则新建一个，写入默认的版本号值
//如果打开成功，则读出记录的版本号信息
//注意：未启动操作系统时使用
char sd_read_dev_firmware_version(int *BanBenID,int *XBanBenID)
{
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		return 1;
	}
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DEV_FW_FILE,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("打开失败（文件不存在）\r\n");
		//新建一个，写入默认的版本号值
		FIL fp1;
		fresult=f_open (&fp1, DEV_FW_FILE,FA_OPEN_ALWAYS|FA_READ|FA_WRITE);// 如果文件存在，则打开；否则，创建一个新文件。
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_open error 2\r\n");
			return 2;
		}		
		
		cJSON *pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, "BanBenID", 1);
		cJSON_AddNumberToObject(pRoot, "XBanBenID", 0);
		char *string=cJSON_PrintUnformatted(pRoot);
		if(string == NULL){
			MY_PRINT("Failed to print pRoot.\n");
			cJSON_Delete(pRoot);
			return 3;
		}
		cJSON_Delete(pRoot);	

		fresult= f_write(&fp1,string,strlen((const char *)string),&bw);
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_write error\r\n");
			free(string);
			return 4;
		}			
		free(string);
		f_close(&fp1 );	
		
		*BanBenID=1;
		*XBanBenID=0;
		return 0;	
	}		
	else//打开成功，则读出记录的版本号信息
	{
		uint8_t revbuf[100]={0};
		fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//读取全部信息
		if(FR_OK!=fresult)
		{
			MY_PRINT("f_read error\r\n");
			f_close(&fp );
			return 5;
		}			
		MY_PRINT("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //将字符串解析成json结构体
		if (NULL == root_json)
		{
			MY_PRINT("解析错误\r\n");
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

#define DOWNLOAD_FW_FILE "fw.txt"
//读取保存在SD卡中的从服务器上面下载下来的最新固件版本号,没有保存到固件则返回-1
//注意：未启动操作系统时使用
char sd_read_firmware_version(int *fw485ID,int *fw232ID)
{
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		return 1;
	}
	FRESULT fresult;
	FIL fp;
	UINT bw;
	
	fresult=f_open (&fp, DOWNLOAD_FW_FILE,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 3\r\n");
		return 1;
	}	
	
	uint8_t revbuf[50]={0};
	fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);//读取全部信息
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_read error\r\n");
		f_close(&fp );
		return 2;
	}	
	
		MY_PRINT("%s\r\n",revbuf);
		cJSON *root_json = cJSON_Parse((const char *)revbuf);    //将字符串解析成json结构体
		if (NULL == root_json)
		{
			MY_PRINT("解析错误\r\n");
			cJSON_Delete(root_json);
			f_close(&fp );
			return 3;
		}		
		cJSON *Item= cJSON_GetObjectItem(root_json, "485_fw");
		if(Item!=NULL)
			*fw485ID=Item->valueint;
		
		Item= cJSON_GetObjectItem(root_json, "232_fw");
		if(Item!=NULL)
			*fw232ID=Item->valueint;	
		
		cJSON_Delete(root_json);		
		f_close(&fp );	
		return 0;
}
		

extern SemaphoreHandle_t Semaphore_SDCARD;//二值信号量

//记录当前设备中运行的软件版本号,本机软件版本、连接本机的232设备软件版本
char sd_mark_dev_firmware_version(int BanBenID,int XBanBenID,bool isOS)
{
	if(isOS==true){
		xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	}
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT1;

	}
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DEV_FW_FILE,FA_CREATE_ALWAYS|FA_READ|FA_WRITE);// 创建一个新文件。如果文件已存在，则它将被截断并覆盖
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 6\r\n");
		ret=2;
		goto EXIT1;
	}	
	
		cJSON *pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, "BanBenID", BanBenID);
		cJSON_AddNumberToObject(pRoot, "XBanBenID", XBanBenID);
		char *string=cJSON_PrintUnformatted(pRoot);
		if(string == NULL){
			MY_PRINT("Failed to print pRoot.\n");
			cJSON_Delete(pRoot);
			ret=3;
			goto EXIT2;
		}
		cJSON_Delete(pRoot);		
	
	fresult= f_write(&fp,string,strlen((const char *)string),&bw);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_write error\r\n");
		free(string);
		ret=4;
		goto EXIT2;
	}	
	free(string);
	
	ret=0;
	EXIT2:
	f_close(&fp );	
	EXIT1:
	if(isOS==true){
		xSemaphoreGive( Semaphore_SDCARD );
	}
	return ret;			
}

//记录保存在SD卡中的最新固件版本号,-1为没有该固件
char sd_mark_firmware_version(int fw485ID,int fw232ID,bool isOS)
{
	if(isOS==true){
		xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	}
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT1;

	}
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, DOWNLOAD_FW_FILE,FA_CREATE_ALWAYS|FA_READ|FA_WRITE);// 创建一个新文件。如果文件已存在，则它将被截断并覆盖
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 6\r\n");
		ret=2;
		goto EXIT1;
	}	
	
		cJSON *pRoot = cJSON_CreateObject();
		cJSON_AddNumberToObject(pRoot, "485_fw", fw485ID);
		cJSON_AddNumberToObject(pRoot, "232_fw", fw232ID);
		char *string=cJSON_PrintUnformatted(pRoot);
		if(string == NULL){
			MY_PRINT("Failed to print pRoot.\n");
			cJSON_Delete(pRoot);
			ret=3;
			goto EXIT2;
		}
		cJSON_Delete(pRoot);		
	
	fresult= f_write(&fp,string,strlen((const char *)string),&bw);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_write error\r\n");
		free(string);
		ret=4;
		goto EXIT2;
	}	
	free(string);
	
	ret=0;
	EXIT2:
	f_close(&fp );	
	EXIT1:
	if(isOS==true){
		xSemaphoreGive( Semaphore_SDCARD );
	}
	return ret;		
}
//读取固件的文件大小，在主机发送485数据给从机前使用
char sd_read_firmware_size(const char * filename,int *firmware_size,bool isOS)
{
	if(isOS==true){
		xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	}
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT1;

	}
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, filename,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 4\r\n");
		ret=2;
		goto EXIT1;
	}		
	
	*firmware_size=fp.obj.objsize;

	ret=0;
	EXIT2:
	f_close(&fp );	
	EXIT1:
	if(isOS==true){
		xSemaphoreGive( Semaphore_SDCARD );
	}
	return ret;			
}

//读取固件内容，在主机发送485数据给从机前使用
//从start_addr开始读length个字节
char sd_read_firmware(const char * filename,uint8_t *rev_buf,int start_addr,int length,UINT* br,bool isOS)
{
	if(isOS==true){
		xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	}
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT1;

	}
	FRESULT fresult;
	FIL fp;
	
	
	fresult=f_open (&fp, filename,FA_OPEN_EXISTING|FA_READ);//如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 5\r\n");
		ret=2;
		goto EXIT1;
	}		
	
	f_lseek(&fp,start_addr);//从fp+start_addr开始读取
	
	fresult= f_read(&fp,rev_buf,length,br);
	if(FR_OK!=fresult)
	{
		MY_PRINT("sd_change_base64_file f_read1 error\r\n");
		ret=3;
		goto EXIT2;
	}		
	
	ret=0;
	EXIT2:
	f_close(&fp );	
	EXIT1:
	if(isOS==true){
		xSemaphoreGive( Semaphore_SDCARD );
	}
	return ret;	
}





//记录断网时间点到本地SD卡 
char broken_network_record(void)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT;

	}
	FRESULT fresult;
	FIL fp;
	UINT bw;
	f_mkdir (OFFLINE_DIR);	
	fresult=f_open (&fp, "broken.txt",FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 7\r\n");
		ret=2;
		goto EXIT;
	}
	Get_RTC_Time(&rtc_data);
	Get_RTC_DATE(&rtc_data);
	
	u8 f_buf[100]={0};
	
	sprintf((char *)f_buf,"borken netork time:%04d/%02d/%02d  %02d:%02d:%02d\r\n\r\n",
				rtc_data.year,rtc_data.month,rtc_data.day,rtc_data.hour,rtc_data.min,rtc_data.sec);
	

	f_lseek(&fp,fp.obj.objsize);//写在文件末尾
	
	fresult= f_write(&fp,f_buf,strlen((const char *)f_buf),&bw);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_write error\r\n");
		f_close(&fp );
		ret=3;
		goto EXIT;
	}
	f_close(&fp );	
	
	ret=0;
	EXIT:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;	
}


//离线数据记录
char offline_data_record(uint8_t *pbuf,uint16_t length)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT;
	}
	
	uint16_t file_count=0;
	cal_file_count_of_dir((uint8_t *)OFFLINE_DIR, &file_count);
	MY_PRINT("record file count:%d\r\n",file_count);
	
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	uint8_t fn_buf[25]={0};
	sprintf((char *)fn_buf,"%s/record%d.txt",OFFLINE_DIR,file_count+1);

	fresult=f_open (&fp, (const char *)fn_buf , FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 7\r\n");
		ret=2;
		goto EXIT;
	}

	fresult= f_write(&fp,pbuf,length,&bw);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_write error\r\n");
		f_close(&fp );	
		ret=3;
		goto EXIT;
	}		
	
	f_close(&fp );
	ret=0;	
	EXIT:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;		
	
	
	
}


//检测本地SD卡是否有离线数据存储  函数需要改
char read_offline_data(uint8_t *buf,uint16_t* length,uint8_t *r_file)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT;
	}
	uint16_t file_count;
	cal_file_count_of_dir((uint8_t *)OFFLINE_DIR, &file_count);
	MY_PRINT("record file count:%d\r\n",file_count);	
	
	if(file_count==0)
	{
		MY_PRINT("record file NULL\r\n");	
		ret=2;
		goto EXIT;
	}
	
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	uint8_t fn_buf[25]={0};
	sprintf((char *)fn_buf,"%s/record%d.txt",OFFLINE_DIR,file_count);
	
	fresult=f_open (&fp, (char *)fn_buf,FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_open error 8\r\n");
		ret=3;
		goto EXIT;
	}
	
	uint8_t *revbuf=malloc(fp.obj.objsize+1);
	fresult= f_read(&fp,revbuf,fp.obj.objsize,&bw);
	if(FR_OK!=fresult)
	{
		MY_PRINT("f_read error\r\n");
		f_close(&fp );
		ret=4;
		goto EXIT;
	}	
	
	memcpy(buf,revbuf,bw);
	*length=bw;
	free(revbuf);
	
	f_close(&fp );
	
	memcpy(r_file,fn_buf,strlen((const char *)fn_buf));
//	//用完，删除文件
//	fresult=f_unlink((const TCHAR*)fn_buf);//remove_file((uint8_t *)OFFLINE_DIR,(uint8_t *)fn_buf);
//	if(FR_OK!=fresult)
//	{
//		f_mount (&this_fs,"0:",0);//解挂
//		f_mount (&this_fs,"0:",1);//重新挂载
//		fresult=f_unlink((const TCHAR*)fn_buf);
//	}		
//	if(FR_OK==fresult)
//	{
//		MY_PRINT("remove %s\r\n",fn_buf);
//	}
	
	ret=0;
	
	EXIT:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;
}
char offline_data_file_remove(uint8_t *r_file)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT;
	}	
	FRESULT fresult;

	fresult=f_unlink((const TCHAR*)r_file);//remove_file((uint8_t *)OFFLINE_DIR,(uint8_t *)fn_buf);
	if(FR_OK!=fresult)
	{
		f_mount (&this_fs,"0:",0);//解挂
		f_mount (&this_fs,"0:",1);//重新挂载
		fresult=f_unlink((const TCHAR*)r_file);
	}		
	if(FR_OK==fresult)
	{
		MY_PRINT("remove %s\r\n",r_file);
	}	
	
	ret=0;
	EXIT:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;	
}




//创建base64固件文件
//char sd_create_base64_firmware(const char * filename)
char sd_create_firmware_file(const char * filename)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT;
	}

	FRESULT fresult;
	FIL fp;

	fresult=f_open (&fp, (const char *)filename , FA_CREATE_ALWAYS|FA_READ|FA_WRITE);// 创建一个新文件。如果文件已存在，则它将被截断并覆盖
	if(FR_OK!=fresult)
	{
		MY_PRINT("sd_create_base64_firmware f_open error 9\r\n");
		ret=2;
		goto EXIT;
	}
	f_close(&fp );
	ret=0;	
	EXIT:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;	
}


//写base64固件
//char sd_write_base64_firmware(const char * filename,uint8_t* data,uint16_t length)
char sd_write_firmware_file(const char * filename,uint8_t* data,uint16_t length)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT;
	}
	
	FRESULT fresult;
	FIL fp;
	UINT bw;	
	
	fresult=f_open (&fp, (const char *)filename , FA_OPEN_EXISTING|FA_READ|FA_WRITE);//打开文件。如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("sd_write_base64_firmware f_open error 10\r\n");
		ret=2;
		goto EXIT;
	}

	f_lseek(&fp,fp.obj.objsize);//写在文件末尾，追加写入
	
	fresult= f_write(&fp,data,length,&bw);
	if(FR_OK!=fresult)
	{
		MY_PRINT(" sd_write_base64_firmware f_write error\r\n");
		f_close(&fp );	
		ret=3;
		goto EXIT;
	}		
	
	f_close(&fp );
	ret=0;	
	EXIT:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;		
}
#include "debug_uart.h"
#include "base64.h"
//把在Sd卡中的base64固件进行解码
char sd_change_base64_file(const char * base64_filename,char * filename)
{
	xSemaphoreTake( Semaphore_SDCARD, portMAX_DELAY );
	
	char ret;
	if(flagSDReady==NOT_READY)
	{
		MY_PRINT("sd card error ,check sd card\r\n");
		ret=1;
		goto EXIT3;
	}
	
	FRESULT fresult;
	FIL fp1;
	UINT bw;

	fresult=f_open (&fp1, (const char *)base64_filename , FA_OPEN_EXISTING|FA_READ);//打开文件。如果文件不存在，则打开失败。(默认)
	if(FR_OK!=fresult)
	{
		MY_PRINT("sd_change_base64_file f_open1 error\r\n");
		ret=2;
		goto EXIT3;
	}
	char file[30]={0};
	strcpy((char *)file,base64_filename+4);
	FIL fp2;
	fresult=f_open (&fp2, (const char *)file , FA_CREATE_ALWAYS|FA_READ|FA_WRITE);// 创建一个新文件。如果文件已存在，则它将被截断并覆盖
	if(FR_OK!=fresult)
	{
		MY_PRINT("sd_change_base64_file f_open2 error\r\n");
		ret=2;
		goto EXIT2;
	}	
	
	uint8_t base64_buf[1001]={0};
	uint8_t deDate_buf[1001]={0};
	int num=0;
	int firmware_size=0;
	UINT bw_write;
	int CRC32=0;
	while(1)
	{
		fresult= f_read(&fp1,base64_buf,1000,&bw);
		if(FR_OK!=fresult)
		{
			MY_PRINT("sd_change_base64_file f_read1 error\r\n");
			ret=3;
			goto EXIT1;
		}		
		//转码
		num=base64_decode((const char *)base64_buf, deDate_buf);
		
		MY_DEBUG_TX( (unsigned char *)deDate_buf, num );		
		
		if(bw<1000)//最后一次读取，减去最后4个字节的CRC32校验值
			fresult= f_write(&fp2,deDate_buf,num-4,&bw_write);
		else
			fresult= f_write(&fp2,deDate_buf,num,&bw_write);
		if(FR_OK!=fresult)
		{
			MY_PRINT(" sd_change_base64_file f_write2 error\r\n");
			ret=3;
			goto EXIT1;
		}			
		CRC32 = calc_crc32(CRC32, deDate_buf, bw_write);//校验固件
		firmware_size+=bw_write;
		if(bw<1000)//最后一次读取
		{
			if(CRC32!=*(int *)&deDate_buf[num-4])
			{
				MY_PRINT(" firmware crc32 err\r\n");
				ret=4;
				goto EXIT1;
			}
			MY_PRINT("\r\nCRC32 value:%X\r\n",CRC32);
			MY_PRINT("\r\n%s size:%d\r\n",file,firmware_size);
			break;
		}		
		memset(base64_buf,0,sizeof(base64_buf));	
		memset(deDate_buf,0,sizeof(deDate_buf));	
		vTaskDelay(200);	
	}
	if(filename!=NULL)
		strcpy((char *)filename,file);
	
	
	ret=0;	
	EXIT1:
	f_close(&fp2 );
	EXIT2:
	f_close(&fp1 );
	//用完，删除文件
	fresult=f_unlink((const TCHAR*)base64_filename);
	if(FR_OK==fresult)
	{
		MY_PRINT("remove %s\r\n",base64_filename);
	}
	EXIT3:
	xSemaphoreGive( Semaphore_SDCARD );
	return ret;			
}






////在SD卡记录有新固件的标志文件
//char mark_new_app_txt(void)
//{
//	if(flagSDReady==NOT_READY)
//	{
//		MY_PRINT("sd card error ,check sd card\r\n");
//		return 3;
//	}
//	FRESULT fresult;
//	FIL fp;
//	UINT bw;
//	fresult=f_open (&fp, NEW_APP_FLAG_FILE_NAME ,FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
//	if(FR_OK!=fresult)
//	{
//		MY_PRINT("f_open error 11\r\n");
//		return 1;
//	}	
//	
//	uint8_t buf[20]={0};
//	strcpy((char *)buf,NEW_APP_FLAG_CODE);

//	fresult= f_write(&fp,buf,strlen((const char *)buf)+1,&bw);
//	if(FR_OK!=fresult)
//	{
//		MY_PRINT("f_write error\r\n");
//		f_close(&fp );
//		return 2;
//	}
//	f_close(&fp );	
//	return 0;
//}