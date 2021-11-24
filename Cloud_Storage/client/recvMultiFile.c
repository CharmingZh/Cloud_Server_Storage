#include <func.h>
#include "poss.h"

//pQue,&fd,&type,filename,md5,&begin,&size,&server,&order
int recvMultiFile(char *filename,char* md5,off_t begin,off_t size,servSock_t server,int order,char* token){
    printf("i an com\r\n");
    int sfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(sfd,-1,"socket");
    
    struct sockaddr_in serAddr;
    memset(&serAddr,0,sizeof(serAddr));
    serAddr.sin_family=AF_INET;
    serAddr.sin_addr.s_addr=inet_addr(server.ip);
    serAddr.sin_port=htons(atoi(server.port));

    int ret=0;
    ret=connect(sfd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    ERROR_CHECK(ret,-1,"connect");
    
    Train_t train;
    memset(&train,0,sizeof(train));
    train.len=strlen(token);
    strcpy(train.buf,token);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);

    printf("%s\n",token);
    memset(&train,0,sizeof(train));
    recv(sfd,&train.len,4,0);
    if(-1==train.len){
        printf("connect fail\n");
        return -1;
    }

    train.len=strlen(md5);
    strcpy(train.buf,md5);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);
    printf("transmd5=%s\n",md5);

    memset(&train,0,sizeof(train));
    train.len=sizeof(begin);
    sprintf(train.buf,"%ld",begin);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);
    
    memset(&train,0,sizeof(train));
    train.len=sizeof(size);
    sprintf(train.buf,"%ld",size);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);

    
    printf("%ld %ld %s %d\n",begin,size,md5,order);

    recvFile_mult(sfd,md5,begin,size,order);
    printf("connect success\n");
    return 0;
    
}
