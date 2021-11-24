#define _XOPEN_SOURCE
#include <func.h>
#include <shadow.h>
#include "poss.h"
#include <openssl/md5.h>
#define READ_DATA_SIZE  1024
#define MD5_SIZE        16
#define MD5_STR_LEN     (MD5_SIZE * 2)

int clientPutsFile(int sfd)
{
    int ret = 0;
    Train_t train;
    char fileNameBuf[64] = {0};
    memset(&train,0,sizeof(train));
    recv(sfd,&train.len,4,0);
    recv(sfd,train.buf,train.len,0);
    strcpy(fileNameBuf,train.buf);//存取文件名
    const char *file_path = fileNameBuf;
    char md5_str[MD5_STR_LEN+1];

    ret = Compute_file_md5(file_path,md5_str);
    if(0 != ret){
        //printf("the file is not exist\n");
        memset(&train,0,sizeof(train));
        train.len = -1;
        send(sfd,&train.len,4,0);
        
        return -1;
    }
    //传输MD5值
    memset(&train,0,sizeof(train));
    strcpy(train.buf,md5_str);
    train.len = strlen(train.buf);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);

    memset(&train,0,sizeof(train));
    recv(sfd,&train.len,4,0);

    if(train.len == -1){
        recv(sfd,&train.len,4,0);
        recv(sfd,train.buf,train.len,0);
        printf("%s\r\n",train.buf);
        printf("please choose other operation\r\n");
        return 0;
    }
    recv(sfd,train.buf,train.len,0);
    if(strcmp(train.buf,"please send the file") == 0){
        transFile(sfd,fileNameBuf); 
    }
    return 0;
}
