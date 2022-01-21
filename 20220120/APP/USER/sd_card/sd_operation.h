#ifndef __SD_OPERATION_H__
#define __SD_OPERATION_H__

#ifdef __cplusplus
 extern "C" {
#endif


#include "stm32f4xx.h"
#include "sd_sdio.h"
#include "diskio.h"
#include "stdio.h"
#include "string.h"
#include "ff.h"
#include "stdlib.h"
#include <stdbool.h>




extern FATFS       this_fs;

#define NEW_232_APP_FILE_NAME "232app.bin"//短文件名
#define NEW_485_APP_FILE_NAME "485app.bin"//短文件名
#define NEW_APP_FLAG_FILE_NAME "new.txt"//短文件名
#define NEW_APP_FLAG_CODE "meiyoumima"

#define CONFIG_DIR "config"
#define CONFIG_DEFAULT_FILE "config/defaultconfig.txt"//保存默认配置的文件 CJID SBBH
#define CONFIG_NETWORK_FILE "config/network.txt"//保存网路相关信息的文件

#define OFFLINE_DIR "offline"
#define OFFLINE_RECORD_FILE_NAME "offline/record.txt"


void sd_read_path(void);
char mark_new_app_txt(void);

char sd_write_log(char flag);

char sd_read_log(uint8_t* revbuf);


char broken_network_record(void);
char offline_data_record(uint8_t *pbuf,uint16_t length);

char sd_read_defaultconfig(char *onlySaoma,char* SBBH,uint32_t* CJID,int*SaoMaBaud,char *TDJZh,char *ZaXiang);

char sd_read_network_config(char* wifissid,char* wifipwd,char *server_ip,int *server_port,char *url);

char read_offline_data(uint8_t *buf,uint16_t* length,uint8_t *r_file);
char offline_data_file_remove(uint8_t *r_file);

char sd_create_firmware_file(const char * filename);//char sd_create_base64_firmware(const char * filename);
char sd_write_firmware_file(const char * filename,uint8_t* data,uint16_t length);//char sd_write_base64_firmware(const char * filename,uint8_t* data,uint16_t length);
char sd_change_base64_file(const char * base64_filename,char * filename);

//记录当前设备中运行的软件版本号,本机软件版本、连接本机的232设备软件版本
char sd_mark_dev_firmware_version(int BanBenID,int XBanBenID,bool isOS);
//记录保存在SD卡中的最新固件版本号,-1为没有该固件
char sd_mark_firmware_version(int fw485ID,int fw232ID,bool isOS);


//初始，尝试打开保存在SD卡中的保存当前设备中运行的软件版本号的文件，如果打开失败（不存在），则新建一个，写入默认的版本号值
//如果打开成功，则读出记录的版本号信息
//注意：未启动操作系统时使用
char sd_read_dev_firmware_version(int *BanBenID,int *XBanBenID);
//读取保存在SD卡中的从服务器上面下载下来的最新固件版本号,没有保存到固件则返回-1
//注意：未启动操作系统时使用
char sd_read_firmware_version(int *fw485ID,int *fw232ID);


//读取固件的文件大小，在主机发送485数据给从机前使用
char sd_read_firmware_size(const char * filename,int *firmware_size,bool isOS);

//读取固件内容，在主机发送485数据给从机前使用
//从start_addr开始读length个字节
char sd_read_firmware(const char * filename,uint8_t *rev_buf,int start_addr,int length,UINT* br,bool isOS);
#ifdef __cplusplus
}
#endif

#endif
