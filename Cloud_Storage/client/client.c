#define _XOPEN_SOURCE
#include <func.h>
#include <shadow.h>
#include "poss.h"
#include <openssl/md5.h>
#include "pthread_pool.h"
#define READ_DATA_SIZE  1024
#define MD5_SIZE        16
#define MD5_STR_LEN     (MD5_SIZE * 2)

int logon(int fd,char *token){
    int ret=0;
    Train_t train;
    char salt[15]={0};
    do{
        memset(&train,0,sizeof(train));
        printf("please enter your user name: ");
        scanf("%16s",train.buf);
        train.len=strlen(train.buf);
        ret=send(fd,&train.len,4,0);
        ERROR_CHECK(ret,-1,"send");
        ret=send(fd,train.buf,train.len,0);
        ret=recv(fd,&train.len,4,0);
        if(train.len==-1){
            memset(&train,0,sizeof(train));
            ret=recv(fd,&train.len,4,0);
            ret=recv(fd,train.buf,train.len,MSG_WAITALL);
            printf("%s\n",train.buf);
        }else{
            memset(train.buf,0,sizeof(train.buf));
            ret=recv(fd,train.buf,train.len,MSG_WAITALL);
            strncpy(salt,train.buf,12);
            break;
        }
    }while(1);
    char *ptr=NULL;
    int flag=3;
    do{
        memset(&train,0,sizeof(train));
        ptr=getpass("please enter your password:");
        char *tcr=NULL;
        tcr=crypt(ptr,salt);
        strcpy(train.buf,tcr);
        train.len=strlen(train.buf);
        ret=send(fd,&train.len,4,0);
        ret=send(fd,train.buf,train.len,MSG_WAITALL);
        ret=recv(fd,&train.len,4,0);
        //free(ptr);
        if(train.len==-1){
            memset(&train,0,sizeof(train));
            ret=recv(fd,&train.len,4,0);
            ret=recv(fd,train.buf,train.len,MSG_WAITALL);
            printf("%s\n",train.buf);
            --flag;
            if(!flag){
                break;
            }
        }else{
            memset(train.buf,0,sizeof(train.buf));
            ret=recv(fd,train.buf,train.len,MSG_WAITALL);
            printf("%s\n",train.buf);

            memset(train.buf,0,sizeof(train.buf));
            recv(fd,&train.len,4,0);
            recv(fd,train.buf,train.len,0);
            strcpy(token,train.buf);
            break;
        }
    }while(1);
    if(flag){
        return 0;
    }else{
        return -1;
    }
}

int registration(int fd,char* token){
    int ret=0;
    Train_t train;
    char salt[15]={0};
    do{
        memset(&train,0,sizeof(train));
        printf("please enter your user name: ");
        scanf("%16s",train.buf);
        train.len=strlen(train.buf);
        ret=send(fd,&train.len,4,0);
        ERROR_CHECK(ret,-1,"send");
        ret=send(fd,train.buf,train.len,0);
        ret=recv(fd,&train.len,4,0);
        if(train.len==-1){
            memset(&train,0,sizeof(train));
            ret=recv(fd,&train.len,4,0);
            ret=recv(fd,train.buf,train.len,MSG_WAITALL);
            //printf("%s\n",train.buf);
        }else{
            memset(train.buf,0,sizeof(train.buf));
            ret=recv(fd,train.buf,train.len,MSG_WAITALL);
            //strncpy(salt,train.buf,14);
            strcpy(salt,train.buf);
            //printf("%s\n",salt);
            break;
        }
    }while(1);
    char *ptr=NULL;
    char ptr_d[20]={0};
    do{
        memset(&train,0,sizeof(train));
        ptr=getpass("please enter your password:");
        strcpy(ptr_d,ptr);
        //free(ptr);
        ptr=getpass("please enter again:");
        if(strcmp(ptr,ptr_d)){
            printf("password wrong!\n");
            ptr=NULL;
        }else{
            break;
        }
        //free(ptr);
        ptr=NULL;
    }while(1);
    char *tcr=NULL;
    //printf("%s\n",salt);
    tcr=crypt(ptr_d,salt);
    strcpy(train.buf,tcr);
    //printf("1\n");
    train.len=strlen(train.buf);
    ret=send(fd,&train.len,4,0);
    ret=send(fd,train.buf,train.len,MSG_WAITALL);
    ret=recv(fd,&train.len,4,0);
    memset(train.buf,0,sizeof(train.buf));
    ret=recv(fd,train.buf,train.len,MSG_WAITALL);
    printf("%s\n",train.buf);

    memset(train.buf,0,sizeof(train.buf));
    recv(fd,&train.len,4,0);
    recv(fd,train.buf,train.len,0);
    strcpy(token,train.buf);

    return 0;
}

int judgeOnIn(int fd,char* token){
    Train_t train;
    memset(&train,0,sizeof(train));
    char c;
    char ch;
    do{
        printf("logon or registration? l:logon  r:registration\n");
        scanf("%c",&c);
        while ((ch = getchar()) != EOF && ch != '\n');
    }while(c!='l' && c!='r');
    if(c=='l'){
        train.len=-1;
        send(fd,&train.len,4,0);
        return logon(fd,token);
    }else{
        return registration(fd,token);
    }
}

int add_tcp(char* ip,char* port,int sfd){
    Train_t train;
    int server_fd=0;
    tcpInit(ip,port,&server_fd);

    memset(&train,0,sizeof(train));
    train.len=strlen(ip);
    strcpy(train.buf,ip);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);

    memset(&train,0,sizeof(train));
    train.len=strlen(port);
    strcpy(train.buf,port);
    send(sfd,&train.len,4,0);
    send(sfd,train.buf,train.len,0);
        
    int newfd=accept(server_fd,NULL,NULL);
    //printf("%d %d\n",sfd,newfd);
    close(server_fd);
    return newfd;

}

int main(int argc, char** argv)
{

    ARGS_CHECK(argc,2);
    FILE* fconf;
    char filename[64]={0};
    sprintf(filename,"%s.conf",argv[1]);
    fconf=fopen(filename,"r+");

    char server_ip[24]={0};
    fgets(server_ip,23,fconf);
    server_ip[strlen(server_ip)-1]='\0';

    char server_port[10]={0};
    fgets(server_port,9,fconf);
    server_port[strlen(server_port)-1]='\0';

    char client_ip[24]={0};
    fgets(client_ip,23,fconf);
    client_ip[strlen(client_ip)-1]='\0';

    char client_port[10]={0};
    fgets(client_port,9,fconf);
    client_port[strlen(client_port)-1]='\0';
    fclose(fconf);
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(sfd, -1, "socket");

    struct sockaddr_in serAddr;
    memset(&serAddr, 0, sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(server_ip);
    serAddr.sin_port = htons(atoi(server_port));
    //int reuse=1;
    //setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

    int ret = 0;

    ret = connect(sfd, (struct sockaddr*)&serAddr, sizeof(serAddr));
    ERROR_CHECK(ret, -1, "connect");
    char token[256]={0};
    ret=judgeOnIn(sfd,token);
    //printf("%s\n",token);

    if(ret){
        printf("the program will exit\n");
        return 0;
    }else{
        printf("connect success\n");
    }
    threadPool_t pool;
    memset(&pool,0,sizeof(pool));
    int threadNum=4;
    threadPool_init(&pool,threadNum);
    threadPool_start(&pool);

    pthread_mutex_lock(&pool.que.que_mutex);
    servSock_t server1;
    queInsert(&pool.que,sfd,5,0,0,0,0,server1,0,0);//1表示上传,2表示下载
    pthread_cond_signal(&pool.que.cond);
    pthread_mutex_unlock(&pool.que.que_mutex);

    //printf("put the mission\n");
    int epfd=epoll_create(1);
    epollAddFd(sfd,epfd);
    //epollAddFd(STDIN_FILENO,epfd);
    struct epoll_event evs[2];
    int readyNum=0;
    Train_t train;
    memset(&train,0,sizeof(train));

    char buf[1000] = {0};
    while(1){
        memset(buf,0,sizeof(buf));
        memset(&train,0,sizeof(train));
        readyNum=epoll_wait(epfd,evs,2,-1);
        for(int i=0;i<readyNum;++i){
            if(evs[i].data.fd==sfd){
                ret=recv(sfd,&train.len,4,0);
                if(0==ret){
                    for(int j=0;j<threadNum;++j){
                        pthread_cancel(pool.pthid[j]);
                    }
                    for(int j=0;j<threadNum;++j){
                        pthread_join(pool.pthid[j],NULL);
                    }
                    printf("server closed!\n");
                    return 0;
                }
                if(train.len==-1){
                    recv(sfd,&train.len,4,0);
                    recv(sfd,train.buf,train.len,0);
                    printf("%s\r\n\r\n",train.buf);

                }else if(-2==train.len){
                    int newfd=add_tcp(client_ip,client_port,sfd);
                    pthread_mutex_lock(&pool.que.que_mutex);
                    servSock_t server;
                    queInsert(&pool.que,newfd,2,0,0,0,0,server,0,0);//1表示上传,2表示下载
                    pthread_cond_signal(&pool.que.cond);
                    pthread_mutex_unlock(&pool.que.que_mutex);

                    //recvFile(sfd);
                }else if(train.len == -3){

#if 1
                    int newfd=add_tcp(client_ip,client_port,sfd);
                    pthread_mutex_lock(&pool.que.que_mutex);
                    servSock_t server;
                    queInsert(&pool.que,newfd,1,0,0,0,0,server,0,0);//1表示上传
                    pthread_cond_signal(&pool.que.cond);
                    pthread_mutex_unlock(&pool.que.que_mutex);
#endif
                    //clientPutsFile(train,sfd);                    
                }else if(train.len==-4){//多点下载
                    char name[64]={0};
                    recv(sfd,&train.len,4,0);
                    recv(sfd,train.buf,train.len,0);
                    strcpy(name,train.buf);

                    char md5[64]={0};

                    memset(&train,0,sizeof(train));
                    recv(sfd,&train.len,4,0);
                    recv(sfd,train.buf,train.len,0);
                    strcpy(md5,train.buf);
                    
                    char fileSize[16]={0};
                    memset(&train,0,sizeof(train));
                    recv(sfd,&train.len,4,0);
                    recv(sfd,train.buf,train.len,0);
                    strcpy(fileSize,train.buf);
                    
                    off_t begin[3]={0};
                    off_t size[3]={0};
                    off_t all_size=0;

                    begin[0]=0;
                    size[0]=atoi(fileSize)/3;
                    all_size=size[0];

                    for(int j=1;j<2;++j){
                        begin[j]=begin[j-1]+size[j-1];
                        size[j]=size[0];
                        all_size+=size[j];
                    }

                    begin[2]=begin[1]+size[1];
                    size[2]=atoi(fileSize)-all_size;


                    servSock_t server[3];
                    memset(server,0,sizeof(server[1])*3);
                    for(int j=0;j<3;++j){
                        memset(&train,0,sizeof(train));
                        recv(sfd,&train.len,4,0);
                        recv(sfd,train.buf,train.len,0);
                        strcpy(server[j].ip,train.buf);

                        memset(&train,0,sizeof(train));
                        recv(sfd,&train.len,4,0);
                        recv(sfd,train.buf,train.len,0);
                        strcpy(server[j].port,train.buf);

                        pthread_mutex_lock(&pool.que.que_mutex);
                        queInsert(&pool.que,sfd,3,name,md5,begin[j],size[j],server[j],j,token);//1表示上传
                        pthread_cond_signal(&pool.que.cond);
                        pthread_mutex_unlock(&pool.que.que_mutex);
                    }
                }
                else if(train.len>0){
                    recv(sfd,train.buf,train.len,0);
                    printf("%s\r\n\r\n",train.buf);
                }
            }
            /* if(evs[i].data.fd==STDIN_FILENO){ */
            /*     read(STDIN_FILENO,buf,sizeof(buf)); */
            /*     train.len=strlen(buf)-1; */
            /*     send(sfd,&train.len,4,0); */
            /*     send(sfd,buf,train.len,0); */
            /* } */
        }
    }
    printf("1\n");
    

    int dataLen = 0;
    recv(sfd, &dataLen, 4, 0);
    recv(sfd, buf, dataLen, 0);

    int fd = open(buf, O_RDWR|O_CREAT, 0666);

    off_t fileSize = 0;
    off_t recvLen = 0;
    
    recv(sfd, &dataLen, 4, 0);
    recv(sfd, &fileSize, dataLen, 0);
    printf("fileSize = %ld\n", fileSize);

    float rate = 0;

    

    while(1){
        memset(buf, 0, sizeof(buf));
        ret = recv(sfd, &dataLen, 4, 0);

        if(0 == dataLen){
            break;
        }
        
        ret = recvCycle(sfd, buf, dataLen);
        recvLen += ret;

        rate = (float)recvLen / fileSize *100;
        printf("|");
        for(int i=0;i<rate/100*20;++i){
            printf("[]");
        }
        for(int i=rate/100*20+1;i<20;++i){
            printf(" ");
        }
        printf("|");
        printf("rate = %5.2f %%\r", rate);
        fflush(stdout);

        write(fd, buf, ret);
    }
    printf("\n");
    close(sfd);
    return 0;
}

