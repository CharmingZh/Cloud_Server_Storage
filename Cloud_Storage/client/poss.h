/*************************************************************************
    > File Name: poss.h
    > Author: wanke
    > Mail: 937502923@qq.com 
    > Created Time: 2021å¹?4æœ?9æ—?æ˜ŸæœŸä¸€ 20æ—?1åˆ?0ç§?
 ************************************************************************/
#ifndef __POSS_
#define __POSS_
#include <func.h>
int recvCycle(int sockFd, void *buf, int totalLen);
int epollAddFd(int sfd,int epfd);
typedef struct{
    int len;
    char buf[1000];
}Train_t;
int transFile(int clienFd,char *fileNameBuf);
int Compute_file_md5(const char *file_path, char *value);
int clientPutsFile(int sfd);
int tcpInit(char *ip,char *port,int *sockFd);
int inComplete(int rows,char* filename,char* md5);
int recvFile(int sfd);
typedef struct{
    char filename[64];
    char md5[64];
}HalfFile_t;
typedef struct{
    char ip[16];
    char port[8];
}servSock_t,*pServSock_t;
int recvFile_mult(int sfd,char* md5,off_t begin,off_t segLen,int serverNum);

#endif
