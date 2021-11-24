
#define _GNU_SOURCE
#include <func.h>
#include "poss.h"
#define MAXFIZE  100*1024*1024
void sigfunc(int signum)
{
    printf("sig is comming\n");
}

int transFile(int clienFd,char *fileNameBuf)
{

    signal(SIGPIPE, sigfunc);
    Train_t train;
    memset(&train, 0, sizeof(train));
    strcpy(train.buf,fileNameBuf);//文件名传输给火车头

    int fd = open(train.buf, O_RDWR);
    //ERROR_CHECK(fd,-1,"open");
    if(fd==-1){
        printf("filename is not exist\r\n\r\n");
        return -1;
    }
    train.len = strlen(fileNameBuf);
    send(clienFd, &train.len, 4 , 0);
    send(clienFd,fileNameBuf,train.len,0);

    struct stat fileInfo;
    bzero(&fileInfo, sizeof(fileInfo));

    fstat(fd, &fileInfo);
    memset(&train,0,sizeof(train));

    train.len = sizeof(fileInfo.st_size);
    memcpy(train.buf, &fileInfo.st_size, train.len);
    
    //printf("filesize = %ld\r\n",fileInfo.st_size);
    send(clienFd, &train.len, 4 , 0);
    send(clienFd,train.buf,train.len,0);

    if(fileInfo.st_size > MAXFIZE ){
        int sfd[2];
        pipe(sfd);
        off_t recvLen = 0;

        while(recvLen < fileInfo.st_size){
            int ret = splice(fd,0,sfd[1],0,fileInfo.st_size,0);
            //int ret = splice(fd,0,sfd[1],0,32,0);
            ret = splice(sfd[0],0,clienFd,0,ret,0);
            recvLen += ret;
        }
        printf("splice send success\r\n\r\n");
        return 0;
    }


    while(1){
        int ret=read(fd,train.buf,sizeof(train.buf));
        train.len=ret;
        int ret1=send(clienFd,&train,4+train.len,0);
        if(0==ret){
            break;
        }
        if(-1==ret1){
            printf("server exit\r\n\r\n");
            break;
        }
    }
    printf("the file send success!\r\n\r\n");
    return 0;

}
