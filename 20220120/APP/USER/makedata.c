#include "makedata.h"
#include "my_tasks.h"
#include "scanner.h"
/**********************************
[
	{
	"TDName":"PDO1���ص�ѹ(mv)",
	"TDFanWei":"4750~5250",
	"TDValue":5104.0,
	"TDJieGuo":1,
	"TDHanSu":100,
	"TDXuHao":60,
	"TDJiLuTime":"2021-07-14 14:13:07"
	},
	{
	"TDName":"PDO2���ص�ѹ(mv)",
	"TDFanWei":"8550~9450",
	"TDValue":9080.0,
	"TDJieGuo":1,
	"TDHanSu":100,
	"TDXuHao":61,
	"TDJiLuTime":"2021-07-14 14:13:07"
	}
]


**********************************/


#include "string.h"
#include "rtc.h"

TYPE_HTTP_KEY http_key={0};


static char* MyCreatJson(struct RS232DataFrame *data,char *TDSaoMa)
{
	u8 i=0;
	char *string=NULL;

	cJSON *resolutions = cJSON_CreateArray();
	cJSON *resolution=NULL;
	
	struct ceshixiangmu *temp=data->item;
	while(1)
	{
		if(temp==NULL)
		{
			break;
		}
		else
		{
			resolution = cJSON_CreateObject();
			if(temp->TDNmae!=NULL)
				cJSON_AddStringToObject(resolution, "TDName", (char *)temp->TDNmae);
			else
				cJSON_AddStringToObject(resolution, "TDName", "");
			if(temp->TDFanWei!=NULL)
				cJSON_AddStringToObject(resolution, "TDFanWei", (char *)temp->TDFanWei);
			else
				cJSON_AddStringToObject(resolution, "TDFanWei", "");
			
			cJSON_AddNumberToObject(resolution, "TDValue", temp->TDValue);
			cJSON_AddNumberToObject(resolution, "TDJieGuo", temp->TDJieGuo);
			cJSON_AddNumberToObject(resolution, "TDHanSu", 100);
			cJSON_AddNumberToObject(resolution, "TDXuHao", temp->TDXuHao);
			if(NULL==data->TDJiLuTime){
				data->TDJiLuTime=malloc(20);//����Ҫ�ͷ�
				memset(data->TDJiLuTime,0,20);
				Get_RTC_Time(&rtc_data);
				Get_RTC_DATE(&rtc_data);
				sprintf((char *)data->TDJiLuTime,"%d-%d-%d %02d:%02d:%02d",rtc_data.year,rtc_data.month,rtc_data.day,rtc_data.hour,rtc_data.min,rtc_data.sec);
			}			
			cJSON_AddStringToObject(resolution, "TDJiLuTime", (char *)data->TDJiLuTime);
			TDSaoMa[TDSaoMa_LEN_MAX-1]='\0';
			cJSON_AddStringToObject(resolution, "TDSaoMa", (char *)TDSaoMa);//add
			cJSON_AddItemToArray(resolutions, resolution);			
			temp=temp->next;
		}
	}

	/*��JSON����д��string������*/
	string = cJSON_PrintUnformatted(resolutions);
	if(string == NULL){
		MY_PRINT("Failed to print resolutions.\n");
		cJSON_Delete(resolutions);
		return NULL;
	}

	/*�ͷ�cJSON����ռ�õ���Դ*/
	cJSON_Delete(resolutions);
	
	return string;

}
#include "string.h"
#include "wifi.h"
//i_frame 232���Խ�����ݵ�ԭʼ����
//i_frameLen ԭʼ���ݳ���
//data�������
char make_data_and_report(uint8_t *i_frame,uint16_t i_frameLen,char *TDSaoMa)
{
	char ret;
	struct RS232DataFrame data={0};
	//����232���ݹ��������ݰ�
	if(0!=rs232_data_prase( (uint8_t *)i_frame, i_frameLen,&data))
	{
		return 100;
	}
	//���JSON��ʽ
	char*string= MyCreatJson(&data,TDSaoMa);
	if(string==NULL)
	{
		return 101;
	}
	//�ϱ�������
	flagReport_ing=1;//��ʼ�ϱ�
	if(	flagNetWorkDev==DEV_4G)
		ret=air724_4g_http_data_report(data.TongDaoNum,(uint8_t *)string,data.TDCiShu,data.TDJZh,data.TDZongHeGe);
	else if(flagNetWorkDev==DEV_WIFI)
		ret=wifi_http_data_report(data.TongDaoNum,(uint8_t *)string,data.TDCiShu,data.TDJZh,data.TDZongHeGe);
	flagReport_ing=0;//�ϱ�����
	free(string);string=NULL;
	//myprintf(&data);
	free_rs232_data(&data);	
	//myprintf(&data);
	memset(TDSaoMa,0,strlen(TDSaoMa));

	return ret;

}

//ɨ���ϱ�
char make_data_and_report_saoma(char *TDSaoMa)
{
	char ret;
	u8 i=0;
	char *string=NULL;

	cJSON *resolutions = cJSON_CreateArray();
	cJSON *resolution=NULL;
	
	resolution = cJSON_CreateObject();
	cJSON_AddNullToObject(resolution, "TDName");
	cJSON_AddNullToObject(resolution, "TDFanWei");
	cJSON_AddNullToObject(resolution, "TDValue");
	cJSON_AddNullToObject(resolution, "TDJieGuo");
	cJSON_AddNullToObject(resolution, "TDHanSu");
	cJSON_AddNullToObject(resolution, "TDXuHao");
	
	char timeBUF[20]={0};
	Get_RTC_Time(&rtc_data);
	Get_RTC_DATE(&rtc_data);
	sprintf((char *)timeBUF,"%d-%d-%d %02d:%02d:%02d",rtc_data.year,rtc_data.month,rtc_data.day,rtc_data.hour,rtc_data.min,rtc_data.sec);
	cJSON_AddStringToObject(resolution, "TDJiLuTime", (char *)timeBUF);
	TDSaoMa[TDSaoMa_LEN_MAX-1]='\0';
	cJSON_AddStringToObject(resolution, "TDSaoMa", (char *)TDSaoMa);//add
	cJSON_AddItemToArray(resolutions, resolution);			

	/*��JSON����д��string������*/
	string = cJSON_PrintUnformatted(resolutions);
	if(string == NULL){
		MY_PRINT("Failed to print resolutions.\n");
		cJSON_Delete(resolutions);
		return 10;
	}
	/*�ͷ�cJSON����ռ�õ���Դ*/
	cJSON_Delete(resolutions);
	
	
	//���JSON��ʽ
	//char*string= MyCreatJson(&data,TDSaoMa);
	//�ϱ�������
	if(	flagNetWorkDev==DEV_4G)
		ret=air724_4g_http_data_report(1,(uint8_t *)string,1,NULL,NULL);
	else if(flagNetWorkDev==DEV_WIFI)
		ret=wifi_http_data_report(1,(uint8_t *)string,1,NULL,NULL);
	free(string);string=NULL;
	//myprintf(&data);
	//myprintf(&data);
	memset(TDSaoMa,0,strlen(TDSaoMa));

	return ret;

}






//�ϴ�IO����io_n��1,2  value1,2
char make_data_and_report_io(char io_n,char value)
{
	char ret;
	u8 i=0;
	char *string=NULL;

	cJSON *resolutions = cJSON_CreateArray();
	cJSON *resolution=NULL;
	
	resolution = cJSON_CreateObject();
	if(io_n==1)
		cJSON_AddStringToObject(resolution, "TDName", (char *)"ioͨ��(1)");
	else if(io_n==2)
		cJSON_AddStringToObject(resolution, "TDName", (char *)"ioͨ��(2)");

	cJSON_AddStringToObject(resolution, "TDFanWei","1~1");//cJSON_AddNullToObject(resolution, "TDFanWei");
	cJSON_AddNumberToObject(resolution, "TDValue",value);//cJSON_AddNullToObject(resolution, "TDValue");
	cJSON_AddNumberToObject(resolution, "TDJieGuo",value);//cJSON_AddNullToObject(resolution, "TDJieGuo");
	cJSON_AddNullToObject(resolution, "TDHanSu");
	cJSON_AddNullToObject(resolution, "TDXuHao");	

	char timeBUF[20]={0};
	Get_RTC_Time(&rtc_data);
	Get_RTC_DATE(&rtc_data);
	sprintf((char *)timeBUF,"%d-%d-%d %02d:%02d:%02d",rtc_data.year,rtc_data.month,rtc_data.day,rtc_data.hour,rtc_data.min,rtc_data.sec);
	cJSON_AddStringToObject(resolution, "TDJiLuTime", (char *)timeBUF);
	cJSON_AddItemToArray(resolutions, resolution);			

	/*��JSON����д��string������*/
	string = cJSON_PrintUnformatted(resolutions);
	if(string == NULL){
		MY_PRINT("Failed to print resolutions.\n");
		cJSON_Delete(resolutions);
		return 10;
	}
	/*�ͷ�cJSON����ռ�õ���Դ*/
	cJSON_Delete(resolutions);
	
	
	char TDNZongHeGe[2]={0};
	if(value==2)
		TDNZongHeGe[0]='2';
	else if(value==1)
		TDNZongHeGe[0]='1';
	
	
	//�ϱ�������
	if(	flagNetWorkDev==DEV_4G)
		ret=air724_4g_http_data_report(1,(uint8_t *)string,1,NULL,(uint8_t *)TDNZongHeGe);
	else if(flagNetWorkDev==DEV_WIFI)
		ret=wifi_http_data_report(1,(uint8_t *)string,1,NULL,(uint8_t *)TDNZongHeGe);
	free(string);string=NULL;
	//myprintf(&data);
	//myprintf(&data);

	return ret;

}




