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

#define NEW_232_APP_FILE_NAME "232app.bin"//���ļ���
#define NEW_485_APP_FILE_NAME "485app.bin"//���ļ���
#define NEW_APP_FLAG_FILE_NAME "new.txt"//���ļ���
#define NEW_APP_FLAG_CODE "meiyoumima"

#define CONFIG_DIR "config"
#define CONFIG_DEFAULT_FILE "config/defaultconfig.txt"//����Ĭ�����õ��ļ� CJID SBBH
#define CONFIG_NETWORK_FILE "config/network.txt"//������·�����Ϣ���ļ�

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

//��¼��ǰ�豸�����е�����汾��,��������汾�����ӱ�����232�豸����汾
char sd_mark_dev_firmware_version(int BanBenID,int XBanBenID,bool isOS);
//��¼������SD���е����¹̼��汾��,-1Ϊû�иù̼�
char sd_mark_firmware_version(int fw485ID,int fw232ID,bool isOS);


//��ʼ�����Դ򿪱�����SD���еı��浱ǰ�豸�����е�����汾�ŵ��ļ��������ʧ�ܣ������ڣ������½�һ����д��Ĭ�ϵİ汾��ֵ
//����򿪳ɹ����������¼�İ汾����Ϣ
//ע�⣺δ��������ϵͳʱʹ��
char sd_read_dev_firmware_version(int *BanBenID,int *XBanBenID);
//��ȡ������SD���еĴӷ����������������������¹̼��汾��,û�б��浽�̼��򷵻�-1
//ע�⣺δ��������ϵͳʱʹ��
char sd_read_firmware_version(int *fw485ID,int *fw232ID);


//��ȡ�̼����ļ���С������������485���ݸ��ӻ�ǰʹ��
char sd_read_firmware_size(const char * filename,int *firmware_size,bool isOS);

//��ȡ�̼����ݣ�����������485���ݸ��ӻ�ǰʹ��
//��start_addr��ʼ��length���ֽ�
char sd_read_firmware(const char * filename,uint8_t *rev_buf,int start_addr,int length,UINT* br,bool isOS);
#ifdef __cplusplus
}
#endif

#endif
