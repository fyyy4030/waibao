#include "makedata.h"
#include "my_tasks.h"
#include "scanner.h"
/**********************************
[
	{
	"TDName":"PDO1空载电压(mv)",
	"TDFanWei":"4750~5250",
	"TDValue":5104.0,
	"TDJieGuo":1,
	"TDHanSu":100,
	"TDXuHao":60,
	"TDJiLuTime":"2021-07-14 14:13:07"
	},
	{
	"TDName":"PDO2空载电压(mv)",
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
				data->TDJiLuTime=malloc(20);//用完要释放
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

	/*将JSON对象写入string对象中*/
	string = cJSON_PrintUnformatted(resolutions);
	if(string == NULL){
		MY_PRINT("Failed to print resolutions.\n");
		cJSON_Delete(resolutions);
		return NULL;
	}

	/*释放cJSON对象占用的资源*/
	cJSON_Delete(resolutions);
	
	return string;

}
#include "string.h"
#include "wifi.h"
//i_frame 232测试结果数据的原始数据
//i_frameLen 原始数据长度
//data输出参数
char make_data_and_report(uint8_t *i_frame,uint16_t i_frameLen,char *TDSaoMa)
{
	char ret;
	struct RS232DataFrame data={0};
	//解析232传递过来的数据包
	if(0!=rs232_data_prase( (uint8_t *)i_frame, i_frameLen,&data))
	{
		return 100;
	}
	//变成JSON格式
	char*string= MyCreatJson(&data,TDSaoMa);
	if(string==NULL)
	{
		return 101;
	}
	//上报服务器
	flagReport_ing=1;//开始上报
	if(	flagNetWorkDev==DEV_4G)
		ret=air724_4g_http_data_report(data.TongDaoNum,(uint8_t *)string,data.TDCiShu,data.TDJZh,data.TDZongHeGe);
	else if(flagNetWorkDev==DEV_WIFI)
		ret=wifi_http_data_report(data.TongDaoNum,(uint8_t *)string,data.TDCiShu,data.TDJZh,data.TDZongHeGe);
	flagReport_ing=0;//上报结束
	free(string);string=NULL;
	//myprintf(&data);
	free_rs232_data(&data);	
	//myprintf(&data);
	memset(TDSaoMa,0,strlen(TDSaoMa));

	return ret;

}

//扫码上报
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

	/*将JSON对象写入string对象中*/
	string = cJSON_PrintUnformatted(resolutions);
	if(string == NULL){
		MY_PRINT("Failed to print resolutions.\n");
		cJSON_Delete(resolutions);
		return 10;
	}
	/*释放cJSON对象占用的资源*/
	cJSON_Delete(resolutions);
	
	
	//变成JSON格式
	//char*string= MyCreatJson(&data,TDSaoMa);
	//上报服务器
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






//上传IO　　io_n－1,2  value1,2
char make_data_and_report_io(char io_n,char value)
{
	char ret;
	u8 i=0;
	char *string=NULL;

	cJSON *resolutions = cJSON_CreateArray();
	cJSON *resolution=NULL;
	
	resolution = cJSON_CreateObject();
	if(io_n==1)
		cJSON_AddStringToObject(resolution, "TDName", (char *)"io通道(1)");
	else if(io_n==2)
		cJSON_AddStringToObject(resolution, "TDName", (char *)"io通道(2)");

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

	/*将JSON对象写入string对象中*/
	string = cJSON_PrintUnformatted(resolutions);
	if(string == NULL){
		MY_PRINT("Failed to print resolutions.\n");
		cJSON_Delete(resolutions);
		return 10;
	}
	/*释放cJSON对象占用的资源*/
	cJSON_Delete(resolutions);
	
	
	char TDNZongHeGe[2]={0};
	if(value==2)
		TDNZongHeGe[0]='2';
	else if(value==1)
		TDNZongHeGe[0]='1';
	
	
	//上报服务器
	if(	flagNetWorkDev==DEV_4G)
		ret=air724_4g_http_data_report(1,(uint8_t *)string,1,NULL,(uint8_t *)TDNZongHeGe);
	else if(flagNetWorkDev==DEV_WIFI)
		ret=wifi_http_data_report(1,(uint8_t *)string,1,NULL,(uint8_t *)TDNZongHeGe);
	free(string);string=NULL;
	//myprintf(&data);
	//myprintf(&data);

	return ret;

}




