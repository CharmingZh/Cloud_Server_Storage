#define _GNU_SOURCE
#include<func.h>
#include "poss.h"
#define MAXSIZE 100*1024*1024

int recvCycle(int sockFd,void* buf,int totalLen){
    int ret=0;
    int recvLen=0;
    while(recvLen<totalLen){
        ret=recv(sockFd,(char*)buf+recvLen,totalLen-recvLen,0);
        recvLen+=ret;
        if(0==ret){
            return -1;
        }
    }
    return recvLen;
}
//sfd，MD5，接收文件段的开始，接收文件段总长，文件段号
int recvFile_mult(int sfd,char* md5,off_t begin,off_t segLen,int serverNum){
    char res[64]={0};//存放要保存的文件名
    sprintf(res,"%s_%d",md5,serverNum);
    //printf("res=%s\n",res);
    int fd=open(res,O_RDWR|O_CREAT,0666);

    struct stat fileInfo;
    bzero(&fileInfo,sizeof(fileInfo));
    stat(res,&fileInfo);
    //发送已接收文件段长
    off_t recvLen=fileInfo.st_size;
    send(sfd,&recvLen,sizeof(off_t),0);
    if(recvLen==segLen){
        printf("%d：please merge files\r\n",serverNum);
        return 0;
    }

    lseek(fd,recvLen,SEEK_SET);//偏移指针到指定位???

    char buf[1000]={0};
    int dataLen=0;
    //模式1：零拷贝
    if(0==MAXSIZE){
        int fd1[2];
        pipe(fd1);

        while(recvLen<segLen){
            int ret=splice(sfd,0,fd1[1],0,4096,SPLICE_F_MORE);
            //if(0==ret){
            //    printf("Download failed:server shutdown\n");
            //    return 0;
            //}
            ret=splice(fd1[0],0,fd,0,ret,SPLICE_F_MORE);
            recvLen+=ret;
            printf("newLen = %ld\n",recvLen);

        }
        printf("Download success!\r\n");
        return 0;
    }
    //模式2：非零拷贝
    while(1){
        memset(buf,0,sizeof(buf));
        recv(sfd,&dataLen,4,0);
        if(0==dataLen){//服务端断开后退???
            break;
        }
        int ret=recvCycle(sfd,buf,dataLen); 
        if(-1==ret){//服务端断开后退???
            break;
        }
        recvLen+=ret;

        write(fd,buf,ret);
        if(recvLen==segLen){
            printf("Download success!\r\n");
            return 0;
        }
    }

    return 0;
}
