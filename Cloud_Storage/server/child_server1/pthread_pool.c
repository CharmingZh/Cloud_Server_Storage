#include <func.h>
#include "pthread_pool.h"
#include "linkList.h"
int threadPool_init(pthreadPool_t pPool, int threadNum){
    pPool->startFlag=0;
    pPool->threadNum=threadNum;
    pPool->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
    que_init(&pPool->que);
    return 0;
}
void cleanFunc(void *p){
    pQue_t pQue=(pQue_t)p;
    pthread_mutex_unlock(&pQue->que_mutex);
    perror("unlock");
}

void* threadFunc(void *p){
    pQue_t pQue=(pQue_t)p;
    int fd=0;
    MYSQL* conn;
    int ret=-1;
    while(1){
        pthread_mutex_lock(&pQue->que_mutex);
        pthread_cleanup_push(cleanFunc,pQue);
        if(!pQue->size){
            pthread_cond_wait(&pQue->cond,&pQue->que_mutex);
        }
        ret=queGet(pQue,&fd,&conn);
        pthread_mutex_unlock(&pQue->que_mutex);
        if(!ret){
            int oldstate=0;
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldstate);
            transFile(fd,conn); 
            pthread_setcancelstate(oldstate,NULL);
        }
        pthread_cleanup_pop(1);
    }
    pthread_exit(NULL);
}

int threadPool_start(pthreadPool_t pPool){
    for(int i=0;i<pPool->threadNum;++i){
        pthread_create(&pPool->pthid[i],NULL,threadFunc,&pPool->que);
    }
    return 0;
}
