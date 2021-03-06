//#pragma once 微软提供的防止头文件包含的方???
#ifndef __PTHREAD_POOL__
#define __PTHREAD_POOL__
#include <func.h>
#include "linkList.h"
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

int threadPool_init(pthreadPool_t pPool,int threadNum);
int threadPool_start(pthreadPool_t pPool);
int transFile(int clienFd,MYSQL* conn);
int epollAddFd(int sfd,int epfd);
int tcpInit(char *ip,char *port,int *sockFd);
int transFile_mult(int clienFd,char* md5,off_t begin,off_t dataLen);
#endif

