#define _GNU_SOURCE
#include <func.h>
#include "pthread_pool.h"
#define MAXSIZE 100*1024*1024

//clienFd，MD5，开始，文件段长度
int transFile_mult(int clienFd,char* md5,off_t begin,off_t dataLen){
    //通知客户端接收文件
    int flag=-2;
    send(clienFd,&flag,4,0);
    //1.接收已发送文件段长度
    off_t transLen=0;
    recv(clienFd,&transLen,sizeof(off_t),0);
    //printf("transLen = %ld\n",transLen);
    if(transLen==dataLen){
        printf("File transfer complete\n");
        return 0;
    }
    char filename[64]={0};
    sprintf(filename,"%s",md5);
    int fd=open(filename,O_RDWR);
    ERROR_CHECK(fd,-1,"open");
    lseek(fd,begin+transLen,SEEK_SET);//偏移指针到指定位置

    //模式1：零拷贝
    if(MAXSIZE==0){
        int fd1[2];
        pipe(fd1);

        while(transLen<dataLen){
            int ret=splice(fd,0,fd1[1],0,dataLen,0);//从文件处写入管道写端
            if(ret==0){
                return 0;
            }
            ret=splice(fd1[0],0,clienFd,0,ret,0);//从管道读端读到socket处
            transLen+=ret;
            printf("newLen=%ld\n",transLen);
        }
        close(fd);
        return 0;
    }
    //模式2：非零拷贝
    Train_t train;
    while(1){
        memset(&train,0,sizeof(train));
        off_t newLen=0;
        if(dataLen-transLen-1000>=0){
            newLen=1000;
        }else{
            newLen=dataLen-transLen;
        }
        int ret=read(fd,train.buf,newLen);
        if(0==ret){
            break;
        }
        transLen+=ret;
        train.len=ret;
        ret=send(clienFd,&train,4+train.len,0);
        if(ret==-1){
            break;
        }
        if(transLen==dataLen){
            break;
        }
    }
    close(fd);
    return 0;
}
