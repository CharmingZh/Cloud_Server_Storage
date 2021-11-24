#define _XOPEN_SOURCE
#define _GNU_SOURCE
#include <func.h>
#include <shadow.h>
#include "poss.h"
#define MAXSIZE 100*1024*1024
/* int recvCycle(int sockFd,void *buf,int totalLen){ */
/*     int ret=0; */
/*     int recvLen=0; */
/*     while(recvLen<totalLen){ */
/*         ret=recv(sockFd,(char*)buf+recvLen,totalLen-recvLen,0); */
/*         recvLen+=ret; */
/*         if(0==ret){ */
/*             return -1; */
/*         } */
/*     } */
/*     return recvLen; */
/* } */
int inComplete(int rows,char* filename,char* md5){
    HalfFile_t msg[100];
    memset(msg,0,sizeof(HalfFile_t));
    FILE* stream;
    stream=fopen("incomplete","r+");
    fread(msg,sizeof(HalfFile_t),rows,stream);
    FILE* fp;
    fp=fopen("res","a+");//移入信息
    for(int i=0;i<rows;++i){
        int a=strcmp(msg[i].filename,filename);
        int b=strcmp(msg[i].md5,md5);
        if(a==0&&b==0){
            continue;
        }else{
            fwrite(&msg[i],sizeof(HalfFile_t),1,fp);
        }
    }
    fclose(stream);
    fclose(fp);
    system("rm incomplete");
    system("mv res incomplete");
    return 0;
}
int recvFile(int sfd){
    int dataLen=0;
    char filename[64]={0};
    //1.接收文件名
    recv(sfd,&dataLen,4,0);
    recv(sfd,&filename,dataLen,0);
    //printf("filename = %s\n",filename);
    int fd=0;
    //2.接收md5值
    char md5[64]={0};
    recv(sfd,&md5,32,0);
    //printf("md5 = %s\n",md5);
    //3.接收文件总长度
    off_t fileSize=0;
    off_t recvLen=0;
    recv(sfd,&dataLen,4,0);
    recv(sfd,&fileSize,dataLen,0);
    //printf("filesize = %ld\n",fileSize);
    //4.断点续传
    HalfFile_t msg[100];//结构体数组
    struct stat fileInfo;
    bzero(&fileInfo,sizeof(fileInfo));
    int rows=0;//文件的行数

    stat("incomplete",&fileInfo);
    rows=fileInfo.st_size/sizeof(HalfFile_t);

    FILE* stream;
    stream=fopen("incomplete","a+");
    fread(msg,sizeof(HalfFile_t),rows,stream);
    int flag=0;
    for(int i=0;i<rows;++i){
        int a=strcmp(msg[i].filename,filename);
        int b=strcmp(msg[i].md5,md5);
        if(a==0&&b==0){
            flag=1;
            printf("'%s' already exists:download from the interrupt position\r\n\r\n",filename);
            fd=open(filename,O_RDWR,0666);
            break;
        }
    }
    if(flag==0){
        fd=open(filename,O_RDWR|O_CREAT,0666);
        HalfFile_t newMsg;
        memset(&newMsg,0,sizeof(newMsg));
        strcpy(newMsg.filename,filename);
        strcpy(newMsg.md5,md5);
        int ret=fwrite(&newMsg,sizeof(HalfFile_t),1,stream);
        ERROR_CHECK(ret,-1,"fwirte");
        ++rows;
        fclose(stream);
    }
    bzero(&fileInfo,sizeof(fileInfo));
    stat(filename,&fileInfo);
    recvLen=fileInfo.st_size;
    send(sfd,&recvLen,sizeof(off_t),0);//发送已接收长度
    //printf("recvLen = %ld\n",recvLen);
    if(flag==0&&recvLen){
        inComplete(rows,filename,md5);
        printf("Download failed:file name already exist\r\n\r\n");
        return 0;
    }

    lseek(fd,recvLen,SEEK_SET);//偏移指针到指定位置

    //int rate=0;
    char buf[1000]={0};
    //struct timeval begin,end;
    //gettimeofday(&begin,NULL);

    if(fileSize>MAXSIZE){//零拷贝
        int fd1[2];
        pipe(fd1);
        while(recvLen<fileSize){
            int ret=splice(sfd,0,fd1[1],0,fileSize,0);
            if(0==ret){//服务器断开后退出
                break;
            }
            ret=splice(fd1[0],0,fd,0,ret,0);
            recvLen+=ret;
            //进度条打印
            /* rate=recvLen*100/fileSize; */
            /* char arr[52]={'\0'}; */
            /* int i=0; */
            /* while(i<=rate/2){ */
            /*     arr[i]='='; */
            /*     ++i; */
            /* } */
            /* printf("[%-50s][%d%%]\r",arr,rate); */
            /* fflush(stdout); */
        }
        if(recvLen==fileSize){
            //gettimeofday(&end,NULL);
            /* printf("\n"); */
            /* printf("cost time = %ld\n",(end.tv_sec-begin.tv_sec)*1000000+(end.tv_usec-begin.tv_usec)); */
            printf("Download success!\r\n\r\n");
            inComplete(rows,filename,md5);
            return 0;
        }
    }
    else{
        while(1){
            memset(buf,0,sizeof(buf));
            recv(sfd,&dataLen,4,0);
            if(0==dataLen){//服务端断开后退出
                break;
            }
            int ret=recvCycle(sfd,buf,dataLen); 
            if(-1==ret){//服务端断开后退出
                break;
            }
            recvLen+=ret;
            //进度条打印
            /* rate=recvLen*100/fileSize; */
            /* char arr[52]={'\0'}; */
            /* int i=0; */
            /* while(i<=rate/2){ */
            /*     arr[i]='='; */
            /*     ++i; */
            /* } */
            /* printf("[%-50s][%d%%]\r",arr,rate); */
            /* fflush(stdout); */

            write(fd,buf,ret);
            if(recvLen==fileSize){
                /* gettimeofday(&end,NULL); */
                /* printf("\n"); */
                /* printf("cost time = %ld\n",(end.tv_sec-begin.tv_sec)*1000000+(end.tv_usec-begin.tv_usec)); */
                printf("Download success!\r\n\r\n");
                inComplete(rows,filename,md5);
                return 0;
            }
        }
    }

    //printf("\n");
    printf("Download fail:server disconnected\r\n\r\n");
    return 0;
}
