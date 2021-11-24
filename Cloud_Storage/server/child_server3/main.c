#include <func.h>
#include "pthread_pool.h"
#include "database.h"
#define MAXNUM 5
int exitPipe[2];
void sigFunc(int signum){
    printf("sig is comming\n");
    write(exitPipe[1],&signum,4);
}
int main(int argc,char*argv[])
{
    ARGS_CHECK(argc,2);
    pipe(exitPipe);
    if(fork()){
        close(exitPipe[0]);
        signal(SIGUSR1,sigFunc);
        wait(NULL);
        printf("child exit\n");
        exit(0);
    }
    threadPool_t pool;
    memset(&pool,0,sizeof(pool));
    int threadNum=MAXNUM;
    threadPool_init(&pool,threadNum);
    threadPool_start(&pool);
    int sfd=0;

    /* int fd=open("f6a6fe33e0a49de86681fc8c4703b7fa",O_RDWR); */
    /* ERROR_CHECK(fd,-1,"open"); */

    MYSQL* conn;
    conn=mysqlRun();
    MYSQL_RES* res;
    MYSQL_ROW row;
    char command[256]={0};
    sprintf(command,"select * from server where id = %d",atoi(argv[1]));
    res=selectCommand(conn,command);
    row=mysql_fetch_row(res);

    tcpInit(row[1],row[2],&sfd); 

    int epfd=epoll_create(1);
    epollAddFd(sfd,epfd);
    close(exitPipe[1]);
    epollAddFd(exitPipe[0],epfd);
    struct epoll_event evs[MAXNUM];
    int readyNum=0;
    int newFd[MAXNUM]={0};
    while(1){
        readyNum = epoll_wait(epfd,evs,MAXNUM,-1);
        for(int i = 0;i < readyNum;++i){
            if(evs[i].data.fd==sfd){
                int j=0;
                for(j=0;j<MAXNUM;++j){
                    if(0==newFd[j]){
                        newFd[j]=accept(sfd,NULL,NULL);
                        break;
                    }
                }
                
                pthread_mutex_lock(&pool.que.que_mutex);
                queInsert(&pool.que,newFd[j],conn);
                pthread_cond_signal(&pool.que.cond);
                pthread_mutex_unlock(&pool.que.que_mutex);
            }
            if(evs[i].data.fd == exitPipe[0]){
                for(int j=0;j<threadNum;++j){
                    pthread_cancel(pool.pthid[j]);
                }
                for(int j=0;j<threadNum;++j){
                    pthread_join(pool.pthid[j],NULL);
                }
                exit(0);
            }
        }
    }
    return 0;
}

