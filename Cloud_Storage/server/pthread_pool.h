//#pragma once 微软提供的防止头文件包含的方�?
#ifndef __PTHREAD_POOL__
#define __PTHREAD_POOL__
#include <func.h>
#include "linkList.h"
#include "database.h"
typedef struct{
    short startFlag;
    int threadNum;
    pthread_t *pthid;
    Que_t que;
}threadPool_t,*pthreadPool_t;


typedef struct{
    int len;
    char buf[1000];
}Train_t;
int rmFile(int fd,char* parameter1,char* parameter2,int workID,int uid,MYSQL* conn);
int getsFile(int clienFd,char* fileName,int uid,int workID,int recvLen,MYSQL* conn);
int threadPool_init(pthreadPool_t pPool,int threadNum);
int threadPool_start(pthreadPool_t pPool);
int transFile(int clienFd);
int epollAddFd(int sfd,int epfd);
int tcpInit(char *ip,char *port,int *sockFd);
int recvCommand(int fd,char *buf,int *workDir,int uid,char* wDir,MYSQL* conn,pthreadPool_t pPool);
int wrongCom(int fd,char * buf);
int putsFile(int fd,int arg,char comBuf[3][64],int *workDir,int uid,MYSQL* conn);
int add_tcp(int clienFd);
int cp(int fd,int *workDir,char *buf,int uid,char *wDir,MYSQL *conn);
int mv(int fd,int *workDir,char *buf,int uid,char *wDir,MYSQL *conn);
int help(int fd,int args);
int cp_r(int fd,int *workDir,char *buf,int uid,char *wDir,MYSQL *conn);
int mv_r(int fd,int *workDir,char *buf,int uid,char *wDir,MYSQL *conn);
int man(int fd,int arg,char *buf);
#endif

