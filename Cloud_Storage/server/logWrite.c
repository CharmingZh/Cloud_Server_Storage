#include <func.h>
#include "database.h"
//将客户端的操作记录在服务器的日志系统中（log表）
//表结构：用户名 命令的具体的内容 命令完成的时间
#define LOG_MAXSIZE 200
// create table log (opId int auto_increment, username varchar(30),cmd varchar(50),excTime timestamp,primary key(opId));

int logWrite(MYSQL * conn,int id,char * op)//conn:已经和mysql连接上的结构体指针 op:键入命令的字符串 
{   
    char buf[256];
    char name[64];
    /* int ret = 0; */
    memset(buf,0,sizeof(buf));
    memset(name,0,sizeof(name));
    if(0 == id){
        strcpy(name,"\\\\");
    }else{
        sprintf(buf,"select username from user where id = %d",id);
        MYSQL_RES * res = selectCommand(conn, buf);
        MYSQL_ROW row = mysql_fetch_row(res);
        strcpy(name,(char *)row[0]);
        memset(buf,0,sizeof(buf));
    }
    /* strcpy(buf,"show create t log"); */
    /* ret = mysqlCommand1(conn,buf); */

    /* printf("ret = %d\n",ret); */
    /* if(-1 == ret){//如果不能查到logi */
    /*     memset(buf,0,sizeof(buf)); */
    /*     strcpy(buf,"create table log (opId int auto_increment, username varchar(30),cmd varchar(50),excTime timestamp,primary key(opId))"); */
    /*     mysqlCommand1(conn,buf); */
    /* } */
    //insert 函数，要提前拼接好命令，一次传入
    memset(buf,0,sizeof(buf));
    MYSQL_RES * res = selectCommand(conn,"select count(opId) from log");
    MYSQL_ROW row = mysql_fetch_row(res);
    int logsize = atoi(row[0]);
    if(logsize == LOG_MAXSIZE){//如果日志数量超过了最大限制，删除一项，再添加
        MYSQL_RES * resD = selectCommand(conn,"select opId from log");
        MYSQL_ROW rowD = mysql_fetch_row(resD);
        int opId = atoi(rowD[0]);
        sprintf(buf,"delete from log where opId = %d",opId);
        mysqlCommand1(conn,buf);
    }


    memset(buf,0,sizeof(buf));
    sprintf(buf,"insert into log (username, cmd, excTime) values ('%s' , '%s' ,now())",name,op);
    mysqlCommand1(conn,buf);
    return 0;
}




