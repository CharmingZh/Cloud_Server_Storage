#ifndef __TOKEN_
#define __TOKEN_
#include <func.h>
#include "../include/l8w8jwt/encode.h"
#include "database.h"
//#include <mysql/mysql.h>
typedef struct l8w8jwt_encoding_params code_t,*pCode_t;//需要用这个结构体进行编码和解码



typedef struct{
    int userId;
    char token[256];
}tokenClient_t,pTokenClient_t;

typedef struct{
    int userId;
    char token[256];
    time_t lasttime;
}tokenServer_t,pTokenServer_t;


int encodeInit(pCode_t pCode);//初始化编码用的结构体，如指定加密方式等，成功返回0;

//int decodeInit(pCode_t pCode);//初始化解码用的结构体

char * tokenGenerate(code_t code, char *username);//根据用户相关信息产生一个token,字符串

int tokenServerPut(MYSQL * conn,tokenServer_t ts);//把客户name和token值,和token产生的时间存入



int tokenCompare(MYSQL * conn,tokenClient_t tc);//服务端对发送来的token进行check，如果信息对上了且没有超时，修改lastVerified
//为当前时间，返回0，如果超时删掉这个tokenpair，并返回1.没找到返回2，需要用户重新输入用户名和密码。
//
#endif
