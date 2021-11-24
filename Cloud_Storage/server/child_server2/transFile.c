
#include <func.h>
#include "pthread_pool.h"
#include "token.h"
#include "database.h"
void sigfunc(int signum)
{
    printf("sig %d is comming\n",signum);
    
}

int transFile(int clienFd,MYSQL* conn)
{
    

    signal(SIGPIPE, sigfunc);
    Train_t train;
    memset(&train, 0, sizeof(train));

    recv(clienFd,&train.len,4,0);
    recv(clienFd,train.buf,train.len,0);

    tokenClient_t tc;
    strcpy(tc.token,train.buf);
    int ret=tokenCompare(conn,tc);
    //printf("%s\n",tc.token);
    memset(&train,0,sizeof(train));
    if(ret){
        train.len=-1;
        send(clienFd,&train.len,4,0);
        return -1;
    }
    train.len=1;
    send(clienFd,&train.len,4,0);
    char md5[64]={0};
    off_t begin=0;
    off_t datalen=0;
    recv(clienFd,&train.len,4,0);
    recv(clienFd,train.buf,train.len,0);
    strcpy(md5,train.buf);

    memset(&train,0,sizeof(train));
    recv(clienFd,&train.len,4,0);
    recv(clienFd,train.buf,train.len,0);
    begin=atoi(train.buf);
    
    memset(&train,0,sizeof(train));
    recv(clienFd,&train.len,4,0);
    recv(clienFd,train.buf,train.len,0);
    datalen=atoi(train.buf);

    //printf("%ld %ld %s\n",begin,datalen,md5);
    transFile_mult(clienFd,md5,begin,datalen);
    printf("connect success\n");


    
    return 0;

}
