/*************************************************************************
    > File Name: poss.h
    > Author: wanke
    > Mail: 937502923@qq.com 
    > Created Time: 2021�?4�?9�?星期一 20�?1�?0�?
 ************************************************************************/

#include <func.h>
#include "database.h"
typedef struct{
    short flag;
    int pipefd;
    pid_t pid;
}process_data_t,*pProcess_data_t;
typedef struct Map{
    int userID;
    int flag;
    int lastPackTime;
}map_t,*pMap_t;

typedef struct TimeNode{//ʱ���ṹ��
    int userGroup;//
    struct TimeNode *nextTimeNode;
}timeNode_t,*pTimeNode_t;

typedef  struct TimeQue{//ʱ�����
    int timeQueSize;
    pTimeNode_t timeHead,timeTail;
}timeQue_t,*pTimeQue_t;


int timeQueInit(pTimeQue_t timeQue);//��ʼ��һ����СΪ��ʮ��ѭ������
int childFunc(int pipefd,MYSQL* conn);
int makeChild(int processNum,pProcess_data_t pData);
int recvFd(int pipeFd,int *fd);
int tcpInit(char *ip,char *port,int *sockFd);
int epollAddFd(int sfd,int epfd);
int sendFd(int pipeFd,int fd);
int transFile(int clienFd);
char* GenRandomString(int length); 
int recvCycle(int sockFd, void *buf, int totalLen);
