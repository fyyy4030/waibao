#ifndef _BASE64_H_
#define _BASE64_H_
#include <stdio.h>
#include <string.h>
int base64_encode(const unsigned char *sourceData, char *outputData);
int base64_decode(const char *base64, unsigned char * deData);
int charIndex(const char* str, char c);

#endif


