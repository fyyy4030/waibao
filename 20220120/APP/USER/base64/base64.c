#include "base64.h"

// 全局常量定义
const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


/* Base64加密 */
int base64_encode(const unsigned char *sourceData, char *outputData)
{
	unsigned char index = 0;
	int i=0, j=0;
	const int len = strlen((const char *)sourceData);
 
	//每三组一个进行编码
	for(; i < len; i += 3)
	{
		//第一个字节，根据源字节的第一个字节处理。 规则：源第一字节右移两位，去掉低2位，高2位补零。
		index = ((sourceData[i] >> 2) & 0x3f);
		outputData[j++] = base64char[index];
 
		//第二个字节，根据源字节的第一个字节和第二个字节联合处理。规则:第一个字节高6位去掉然后左移四位，第二个字节右移四位
		//即：源第一字节低2位 + 源第2字节高4位
		index = ((sourceData[i] << 4) & 0x30);
		if(i+1 < len)
		{
			index |= ((sourceData[i + 1] >> 4) & 0x0f);
			outputData[j++] = base64char[index];
		}
		else
		{
			outputData[j++] = base64char[index];
			outputData[j++] = '=';
			outputData[j++] = '=';
 
			break;//超出总长可以直接break
		}
 
		//第三个字节，根据源字节的第二个字节和第三个字节联合处理，
		//规则:第二个字节去掉高4位并左移两位（得高6位），第三个字节右移6位并去掉高6位（得低2位），相加即可
		index = ((sourceData[i + 1] << 2) & 0x3c);
		if((i + 2) < len)
		{
			index |= ((sourceData[i + 2] >> 6) & 0x03);
			outputData[j++] = base64char[index];
 
			//第四个字节，规则:源第三字节去掉高2位即可
			index = sourceData[i + 2] & 0x3f;
			outputData[j++] = base64char[index];
		}
		else
		{
			outputData[j++] = base64char[index];
			outputData[j++] = '=';
 
			break;
		}
	}
 
	outputData[j] = '\0';
 
	return 0;
}

/* Base64解密 */
int base64_decode(const char *base64, unsigned char * deData)
{
	int i = 0, j = 0;
	int trans[4] = {0};
	for (; base64[i] != '\0'; i += 4)
	{
		//获取第一与第二个字符，在base表中的索引值
		trans[0] = charIndex(base64char, base64[i]);
		trans[1] = charIndex(base64char, base64[i+1]);
 
		// 1/3
		deData[j++] = ((trans[0] << 2) & 0xfc) | ((trans[1] >> 4) & 0x03);
		if (base64[i + 2] == '=')
			continue;
		else
			trans[2] = charIndex(base64char, base64[i + 2]);
 
		// 2/3
		deData[j++] = ((trans[1] << 4) & 0xf0) | ((trans[2] >> 2) & 0x0f);
		if (base64[i + 3] == '=')
			continue;
		else
			trans[3] = charIndex(base64char, base64[i + 3]);
 
		// 3/3
		deData[j++] = ((trans[2] << 6) & 0xc0) | (trans[3] & 0x3f);
	}
 
	deData[j] = '\0';
	return j;
}


/* 在字符串中查询特定字符的索引 */
int charIndex(const char* str, char c)
{
	const char *pIndex = strchr(str, c);
	if(pIndex == NULL)
		return -1;
	else
		return (pIndex - str);
}





