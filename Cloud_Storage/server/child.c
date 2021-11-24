#include <func.h>
#include "poss.h"
#include "pthread_pool.h"
#include "token.h"
int makeChild(int processNum,pProcess_data_t pData){//循环创建子进???
    int sfd[2];
    pid_t childPid=0;
    MYSQL* conn;
    conn=mysqlRun();
    for(int i=0;i<processNum;++i){
        socketpair(AF_LOCAL,SOCK_STREAM,0,sfd);
        childPid=fork();
        if(0==childPid){
            close(sfd[1]);
            childFunc(sfd[0],conn);
            exit(0);
        }
        close(sfd[0]);
        pData[i].flag=0;
        pData[i].pid=childPid;
        pData[i].pipefd=sfd[1];
    }
    return 0;
}

int exitPipe[2];
void sigFunc_thread(int signum){//收到关闭命令时的处理函数
    printf("the process is closing\n");
    write(exitPipe[1],&signum,4);
}
char* GenRandomString(int length) { 
    int flag,i; 
    char* string; 
    srand((unsigned)time(NULL)); 
    if((string=(char*)malloc(length))==NULL) { 
        printf("malloc failed! flag:14\n"); 
        return NULL;  
    } 
    for(i=0;i<length+1;i++) { 
        flag=rand()%3; 
        switch(flag) { 
        case 0: string[i]='A'+rand()%26; 
                break; 
        case 1: string[i]='a'+rand()%26; 
                break; 
        case 2: string[i]='0'+rand()%10; 
                break;  
        default: string[i]='x'; 
                 break;  
        }  
    } 
    string[length]='\0';
    return string; 
}

int add_tcp(int clienFd){
    Train_t train;
    memset(&train,0,sizeof(train));
    recv(clienFd,&train.len,4,0);
    recv(clienFd,train.buf,train.len,0);
    char ip[24]={0};
    strcpy(ip,train.buf);

    memset(&train,0,sizeof(train));
    recv(clienFd,&train.len,4,0);
    recv(clienFd,train.buf,train.len,0);
    char port[10]={0};
    strcpy(port,train.buf);

    int newfd=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serAddr;
    memset(&serAddr, 0, sizeof(serAddr));
    serAddr.sin_family = AF_INET;
    serAddr.sin_addr.s_addr = inet_addr(ip);
    serAddr.sin_port = htons(atoi(port));
    int ret=connect(newfd,(struct sockaddr*)&serAddr,sizeof(serAddr));
    if(ret==-1){
        return -1;
    }else{
        return newfd;
    }

}

int childFunc(int pipefd,MYSQL* conn){//子进程的工作函数
    int clienFd=0;//与客户端连接用的文件描述???
    pipe(exitPipe);
    if(fork()){//主进程负责监听信???
        close(exitPipe[0]);
        signal(SIGUSR1,sigFunc_thread);
        wait(NULL);
        printf("child exit\n");
        exit(0);
    }
    close(exitPipe[1]);
    threadPool_t pool;//线程池第四期时要???
    memset(&pool,0,sizeof(pool));
    int threadNum=3;
    threadPool_init(&pool,threadNum);
    threadPool_start(&pool);
    int log_condition=3;//服务端的接受状态，0表示服务端要接收客户端的命令???表示服务端要接收客户端的密码???表示服务端要接收客户端的用户???3表示注册
    int user_id=0;//用户ID
    while(1){//每一次循环代表一个新的客户端
        recvFd(pipefd,&clienFd);//接收父进程传来的与客户端连接的fd
        int epfd=epoll_create(1);//创建epoll监听
        epollAddFd(clienFd,epfd);
        epollAddFd(exitPipe[0],epfd);
        epollAddFd(pipefd,epfd);
        struct epoll_event evs[2];
        int readyNum=0;
        /* char rootDir[128]="file/user1"; */
        /* char workDir[128]={0}; */
        int rootDir=0;//用户根目录ID
        int workDir=0;//用户当前工作目录ID
        char wDir[128]={0};//当前工作目录字符???
        //DIR* dirp=opendir(rootDir);
        Train_t train;
        memset(&train,0,sizeof(train));
        int flag=1;//1表示当前正与客户端连接，0表示连接断开
        while(flag){
            readyNum = epoll_wait(epfd,evs,2,-1);
            for(int i = 0;i < readyNum;++i){
                memset(&train,0,sizeof(train));
                if(evs[i].data.fd==clienFd){//客户端发来消息时响应
                    int ret=recv(clienFd,&train.len,4,0);
                    if(0==ret){//???户端退???
                        printf("a client exit\n");
                        pthread_mutex_lock(&pool.que.que_mutex);
                        queClear(&pool.que);
                        pthread_mutex_unlock(&pool.que.que_mutex);
                        user_id=0;
                        log_condition=3;
                        flag=0;
                        break;
                    }
                    if(train.len>-1){
                        recv(clienFd,train.buf,train.len,MSG_WAITALL);
                    }
                    if(!log_condition){//???时表示接收客户端命令

                        ret=recvCommand(clienFd,train.buf,&workDir,user_id,wDir,conn,&pool);
                        char buf[64] = {0};
                        memset(buf,0,sizeof(buf));
                        strcpy(buf,"updata the time");

                        write(pipefd,buf,sizeof(buf));
                        printf("have a command,please updata the time\n");

                    }else if(log_condition==2){//???时表示接收客户端的用户名
                        char logBuf[64]={0};
                        strcpy(logBuf,"a user try to logon");
                        logWrite(conn,user_id,logBuf);

                        char command[128]={0};
                        sprintf(command,"select id,salt from user where username = \'%.15s\'",train.buf);
                        MYSQL_RES* res;
                        MYSQL_ROW row;
                        res=selectCommand(conn,command);
                        memset(&train,0,sizeof(train));
                        if(res==NULL || (row=mysql_fetch_row(res))==NULL){
                            train.len=-1;
                            send(clienFd,&train.len,4,0);
                            strcpy(train.buf,"Cannot find this username!");
                            train.len=strlen(train.buf);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                        }else{
                            user_id=atoi(row[0]);
                            train.len=strlen(row[1]);
                            strcpy(train.buf,row[1]);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                            --log_condition;
                        }
                        mysql_free_result(res);
                        //给主进程发送更新当前用户使用时间的消息
                        char buf[64] = {0};
                        memset(buf,0,sizeof(buf));
                        strcpy(buf,"updata the time");
                        write(pipefd,buf,sizeof(buf));
                        printf("into the username,please updata the time\n");

                    }else if(log_condition==1){//???时表示要接收密码
                        char logBuf[64]={0};
                        strcpy(logBuf,"enter password");
                        logWrite(conn,user_id,logBuf);

                        char command[128]={0};
                        sprintf(command,"select cryptpasswd,pwd from user where id = \'%d\'",user_id);
                        MYSQL_RES* res;
                        res=selectCommand(conn,command);
                        MYSQL_ROW row=mysql_fetch_row(res);
                        if(strcmp(row[0],train.buf)==0){
                            memset(&train,0,sizeof(train));
                            strcpy(train.buf,"logon success!");
                            train.len=strlen(train.buf);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                            --log_condition;
                            rootDir=atoi(row[1]);
                            workDir=rootDir;
                            mysql_free_result(res);
                            memset(command,0,sizeof(command));
                            sprintf(command,"select filename from file where id = \'%d\'",rootDir);
                            res=selectCommand(conn,command);
                            row=mysql_fetch_row(res);
                            sprintf(wDir,"%s/",row[0]);
                            code_t code;
                            encodeInit(&code);
                            char *token=NULL;
                            token=tokenGenerate(code,row[0]);
                            tokenServer_t ts;
                            ts.userId=user_id;
                            strcpy(ts.token,token);
                            ts.lasttime=time(NULL);
                            tokenServerPut(conn,ts);
                            memset(&train,0,sizeof(train));
                            train.len=strlen(token);
                            strcpy(train.buf,token);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                            free(token);
                            token=NULL;
                        }else{
                            train.len=-1;
                            send(clienFd,&train.len,4,0);
                            memset(&train,0,sizeof(train));
                            strcpy(train.buf,"logon fail!");
                            train.len=strlen(train.buf);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                        }
                        mysql_free_result(res);
                        char buf[64] = {0};
                        memset(buf,0,sizeof(buf));
                        strcpy(buf,"updata the time");
                        write(pipefd,buf,sizeof(buf));
                        printf("into the password, please updata the time\n");

                    }else if(log_condition==3){
                        if(train.len==-1){
                            log_condition=2;
                            break;
                        }
                        char logBuf[64]={0};
                        strcpy(logBuf,"a user try to register");
                        logWrite(conn,user_id,logBuf);

                        char command[256]={0};
                        sprintf(command,"select id from user where username = \'%.15s\'",train.buf);

                        MYSQL_RES* res;
                        MYSQL_ROW row;
                        char name[20]={0};
                        char *str=NULL;
                        res=selectCommand(conn,command);
                        char buf[64] = {0};
                        memset(buf,0,sizeof(buf));
                        strcpy(buf,"updata the time");
                        write(pipefd,buf,sizeof(buf));
                        printf("register into the name, please updata the time\n");


                        if((row=mysql_fetch_row(res))!=NULL){
                            memset(&train,0,sizeof(train));
                            train.len=-1;
                            send(clienFd,&train.len,4,0);
                            strcpy(train.buf,"this username is already exist!");
                            train.len=strlen(train.buf);

                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                        }else{
                            strcpy(name,train.buf);
                            memset(&train,0,sizeof(train));
                            str=GenRandomString(12);
                            train.len=13;
                            strcpy(train.buf,str);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                            mysql_free_result(res);

                            recv(clienFd,&train.len,4,0);
                            memset(train.buf,0,sizeof(train.buf));
                            recv(clienFd,train.buf,train.len,0);
                            printf("%s\n",name);
                            int pwd=0;

                            sprintf(command,"insert into user (username,salt,cryptpasswd,pwd) values (\'%s\',\'%s\',\'%.100s\',0)",name,str,train.buf);
                            mysqlCommand1(conn,command);

                            memset(command,0 ,sizeof(command));
                            sprintf(command,"select id from user where username=\'%s\'",name);
                            res=selectCommand(conn,command);
                            row=mysql_fetch_row(res);
                            user_id=atoi(row[0]);
                            mysql_free_result(res);

                            memset(command,0,sizeof(command));
                            sprintf(command,"insert into file (parent_id,filename,owner_id,type) values (0,\'%s\',\'%d\',0)",name,user_id);
                            mysqlCommand1(conn,command);

                            memset(command,0,sizeof(command));
                            sprintf(command,"select id from file where owner_id=\'%d\'",user_id);
                            res=selectCommand(conn,command);
                            row=mysql_fetch_row(res);
                            pwd=atoi(row[0]);

                            memset(command,0,sizeof(command));
                            sprintf(command,"update user set pwd = %d where id = %d",pwd,user_id);
                            mysqlCommand1(conn,command);

                            memset(&train,0,sizeof(train));
                            strcpy(train.buf,"logon success!");
                            train.len=strlen(train.buf);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                            rootDir=pwd;
                            workDir=rootDir;
                            row=mysql_fetch_row(res);
                            sprintf(wDir,"%s/",name);

                            code_t code;
                            encodeInit(&code);
                            char *token=NULL;
                            token=tokenGenerate(code,name);
                            tokenServer_t ts;
                            ts.userId=user_id;
                            strcpy(ts.token,token);
                            ts.lasttime=time(NULL);
                            tokenServerPut(conn,ts);
                            memset(&train,0,sizeof(train));
                            train.len=strlen(token);
                            strcpy(train.buf,token);
                            send(clienFd,&train.len,4,0);
                            send(clienFd,train.buf,train.len,0);
                            free(token);
                            token=NULL;

                            log_condition=0;
                            char logBuf[64]={0};
                            strcpy(logBuf,"register success");
                            logWrite(conn,user_id,logBuf);

                        }

                        mysql_free_result(res);
                        memset(buf,0,sizeof(buf));
                        strcpy(buf,"updata the time");
                        write(pipefd,buf,sizeof(buf));
                        printf("register over, please updata the time\n");

                    }

                }
                if(evs[i].data.fd == exitPipe[0]){//收到父进程传来的关闭信号
                    for(int j=0;j<threadNum;++j){
                        pthread_cancel(pool.pthid[j]);
                    }
                    for(int j=0;j<threadNum;++j){
                        pthread_join(pool.pthid[j],NULL);
                    }
                    exit(0);
                }
                if(evs[i].data.fd == pipefd){
                    char closeBuf[6]={0};
                    read(pipefd,closeBuf,sizeof(closeBuf));
                    //printf("chlid recv success\n");
                    if(strcmp(closeBuf,"close") == 0){
                        close(clienFd);
                        printf("a client exit\n");
                        pthread_mutex_lock(&pool.que.que_mutex);
                        queClear(&pool.que);
                        pthread_mutex_unlock(&pool.que.que_mutex);
                        //printf("prepare to leave\n");
                        user_id=0;
                        log_condition=3;
                        flag=0;
                        break;
                    }
                }

            }
        }
        printf("relax\n");
        write(pipefd,"a",1);
    }
    return 0;
}
