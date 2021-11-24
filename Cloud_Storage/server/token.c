#include <func.h>
#include "token.h"
#include "database.h"

#include "include/l8w8jwt/encode.h"
#define EFFECTIVE_TIME 1800
int encodeInit(pCode_t pCode){

    /* char* jwt; */
    /* size_t jwt_length; */
    l8w8jwt_encoding_params_init(pCode);
    pCode->alg = L8W8JWT_ALG_HS512;
    pCode->sub = "client";//jwt面向用户
    pCode->iss = "team 11";//jwt签发者
    pCode->secret_key = (unsigned char*)"YoUR sUpEr S3krEt 1337 HMAC kEy HeRE";//产生token的密钥
    pCode->secret_key_length = strlen((char*)pCode->secret_key);
    return 0;
}


char * tokenGenerate(code_t code ,char * username){
    char * jwt;
    size_t jwt_length;
    code.aud = username; 
    code.out = &jwt;
    code.out_length = &jwt_length;
    int r = l8w8jwt_encode(&code);
    if(r == L8W8JWT_SUCCESS){
        return jwt;
    }
    return NULL;
}

int tokenServerPut(MYSQL * conn,tokenServer_t ts){
    char buf[512] = {0};
    sprintf(buf,"insert into tokenServer (userId,token,lasttime) values (%d,'%s',%ld)",ts.userId,ts.token,ts.lasttime);
    mysqlCommand1(conn,buf); 
    return 0;
}

int tokenCompare(MYSQL * conn,tokenClient_t tc){
    char buf[512] = {0};
    sprintf(buf,"select count(token) from tokenServer where token = '%s' and userId = %d",tc.token,tc.userId);
    MYSQL_RES *res = selectCommand(conn,buf);
    MYSQL_ROW row = mysql_fetch_row(res);
    int ret = atoi(row[0]);
    if(0 == ret){
        return 2;//没有匹配项
    }else{
    memset(buf,0,sizeof(buf));
    sprintf(buf,"select lasttime from tokenServer where token = '%s'",tc.token);
    res = selectCommand(conn,buf);
    row = mysql_fetch_row(res);
    time_t now = time(NULL);
    if(now - atoi(row[0]) > EFFECTIVE_TIME){//超时
        memset (buf,0,sizeof(buf));
        sprintf(buf,"delete from tokenServer where token = '%s'",tc.token);
        mysqlCommand1(conn, buf);
        return 1;
    }else{//修改用户表的时间
        memset(buf,0,sizeof(buf));
        sprintf(buf,"update tokenServer set lasttime = %ld where token = '%s'",now,tc.token);
        mysqlCommand1(conn, buf);
        return 0;
    }
 }
}

//客户端内存里，有一个标志位，如果为0，代表此时没有token，1，代表token是存在的,这样才能确定存不存在，才可进行比较









