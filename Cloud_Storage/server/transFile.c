#define _GNU_SOURCE
#include <func.h>
#include "database.h"
#include "linkList.h"
#include "pthread_pool.h"
#define MAXSIZE 100*1024*1024

//clienFd，文件名，用户id，当前工作目录id，已接收文件长度，数据库
int getsFile(int clienFd,char* fileName,int uid,int workID,int recvLen,MYSQL* conn){
    MYSQL_RES* res=NULL;
    MYSQL_ROW row;
    char command[128]={0};//存放数据库命令
    char md5[32]={0};


    sprintf(command,"select * from file where parent_id=%d and owner_id=%d and filename=\'%s\'",workID,uid,fileName);
    res=selectCommand(conn,command);
    //若文件不存在,报错返回
    row=mysql_fetch_row(res);
    if(row==NULL){
        char buf[30]={0};
        strcpy(buf,"filename is not exist");
        wrongCom(clienFd,buf);
        return 0;
    }
    strcpy(md5,row[4]);//md5存放实际文件名

    Train_t train;
    memset(&train,0,sizeof(train));
    
        //1.传输文件名
    train.len=strlen(fileName);
    strcpy(train.buf,fileName);
    send(clienFd,&train,4+train.len,0);
    //2.传输md5值
    memset(&train,0,sizeof(train));
    send(clienFd,md5,sizeof(md5),0);

    char tepFileName[128]={0};
    sprintf(tepFileName,"file/%s",md5);

    int fd=open(tepFileName,O_RDWR);
    ERROR_CHECK(fd,-1,"open");
    //3.传输文件总长度
    struct stat fileInfo;
    bzero(&fileInfo,sizeof(fileInfo));
    fstat(fd,&fileInfo);
    train.len=sizeof(fileInfo.st_size);
    memcpy(train.buf,&fileInfo.st_size,train.len);
    send(clienFd,&train,4+train.len,0);
    //4.断点续传
    off_t transLen=0;
    recv(clienFd,&transLen,sizeof(off_t),0);
    if(transLen==fileInfo.st_size){
        close(fd);
        return 0;
    }
    lseek(fd,transLen,SEEK_SET);//偏移指针到指定位置

    if(fileInfo.st_size>MAXSIZE){
        int fd1[2];
        pipe(fd1);

        while(transLen<fileInfo.st_size){
            int ret=splice(fd,0,fd1[1],0,fileInfo.st_size,0);//从文件处写入管道写端
            //int ret=splice(fd,0,fd1[1],0,32,0);
            ret=splice(fd1[0],0,clienFd,0,ret,0);//从管道读端读到socket处
            transLen+=ret;
        }
        //printf("splice send success\n");
        close(fd);
        return 0;
    }

    while(1){
        int ret=read(fd,train.buf,sizeof(train.buf));
        if(0==ret){
            break;
        }
        train.len=ret;
        ret=send(clienFd,&train,4+train.len,0);
        //ERROR_CHECK(ret,-1,"send");
    }
    close(fd);
    return 0;
}
