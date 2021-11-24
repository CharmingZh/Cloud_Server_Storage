#include <func.h>
#include "poss.h"
#include "pthread_pool.h"
#include "linkList.h"
#include <termio.h>
int Sum = 0;
struct termios tm;
int threadPool_init(pthreadPool_t pPool, int threadNum){
    pPool->startFlag=0;
    pPool->threadNum=threadNum;
    pPool->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
    que_init(&pPool->que);
    int in_fd = 0;
    if (tcgetattr(in_fd, &tm) < 0) {//保存现在的终端设置
        return -1;
    }
    return 0;
}
void cleanFunc(void *p){
    pQue_t pQue=(pQue_t)p;
    pthread_mutex_unlock(&pQue->que_mutex);
    printf("thread leave\r\n");
    if (tcsetattr(0, TCSANOW, &tm) < 0) {//更改设置为最初的样子
        return;
    }
}
/* int fd; */
/*     int type; */
/*     char filename[64]; */
/*     char md5[32]; */
/*     off_t begin; */
/*     off_t size; */
/*     servSock_t server; */
/*     int order; */


void* threadFunc(void *p){
   
    pQue_t pQue=(pQue_t)p;
    int fd=0;
    int ret=-1;
    int type=0;
    char filename[64]={0};
    char md5[64]={0};
    off_t begin=0;
    off_t size=0;
    servSock_t server;
    memset(&server,0,sizeof(servSock_t));
    int order=0;
    char token[256]={0};
    pthread_cleanup_push(cleanFunc,pQue);
    while(1){
        pthread_mutex_lock(&pQue->que_mutex);
        if(!pQue->size){
            pthread_cond_wait(&pQue->cond,&pQue->que_mutex);
        }
        ret=queGet(pQue,&fd,&type,filename,md5,&begin,&size,&server,&order,token);
        pthread_mutex_unlock(&pQue->que_mutex);
        if(!ret){
            //int oldstate=0;
            //pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldstate);
            if(1==type){
                clientPutsFile(fd); 
            }else if(2==type){
                recvFile(fd);               
            }else if(3==type){
                //  recvMultiFile(filename,md5,begin,size,server,order,token);
#if 1
                int sfd=socket(AF_INET,SOCK_STREAM,0);

                struct sockaddr_in serAddr;
                memset(&serAddr,0,sizeof(serAddr));
                serAddr.sin_family=AF_INET;
                serAddr.sin_addr.s_addr=inet_addr(server.ip);
                serAddr.sin_port=htons(atoi(server.port));

                ret=0;
                ret=connect(sfd,(struct sockaddr*)&serAddr,sizeof(serAddr));

                Train_t train;
                memset(&train,0,sizeof(train));
                train.len=strlen(token);
                strcpy(train.buf,token);
                send(sfd,&train.len,4,0);
                send(sfd,train.buf,train.len,0);

                //printf("%s\n",token);
                memset(&train,0,sizeof(train));
                recv(sfd,&train.len,4,0);
                if(-1==train.len){
                    printf("connect fail\r\n");
                    continue;
                }

                train.len=strlen(md5);
                strcpy(train.buf,md5);
                send(sfd,&train.len,4,0);
                send(sfd,train.buf,train.len,0);
                //printf("transmd5=%s\n",md5);

                memset(&train,0,sizeof(train));
                sprintf(train.buf,"%ld",begin);
                train.len=strlen(train.buf);
                send(sfd,&train.len,4,0);
                send(sfd,train.buf,train.len,0);

                memset(&train,0,sizeof(train));
                sprintf(train.buf,"%ld",size);
                train.len=strlen(train.buf);
                send(sfd,&train.len,4,0);
                send(sfd,train.buf,train.len,0);


                //printf("%ld %ld %s %d\n",begin,size,md5,order);

                recvFile_mult(sfd,md5,begin,size,order);
                Sum +=1;
                printf("connect success,Sum = %d\r\n",Sum);
                
                if(3 == Sum){
//me,char *md5,off_t begin,off_t size,servSock_t server,int order,char* token)
                    pthread_mutex_lock(&pQue->que_mutex);
                    queInsert(pQue,sfd,4,filename,md5,begin,size,server,order,token);//1表示上传
                    pthread_cond_signal(&pQue->cond);
                    pthread_mutex_unlock(&pQue->que_mutex);
                    Sum=0;
                }
#endif 
            }else if(4==type){
                mergeFile(md5,filename); 
            }else if(5==type){
                //pthread_setcancelstate(oldstate,NULL);
                get_input(fd);
            }
            //pthread_setcancelstate(oldstate,NULL);
        }
    }
    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}

int threadPool_start(pthreadPool_t pPool){
    for(int i=0;i<pPool->threadNum;++i){
        pthread_create(&pPool->pthid[i],NULL,threadFunc,&pPool->que);
    }
    return 0;
}
