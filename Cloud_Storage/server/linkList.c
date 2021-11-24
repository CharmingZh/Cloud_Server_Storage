
#include <func.h>
#include "linkList.h"

int que_init(pQue_t pq){
    pq->size=0;
    pq->phead=NULL;
    pq->ptail=NULL;
    pthread_cond_init(&pq->cond,NULL);
    pthread_mutex_init(&pq->que_mutex,NULL);
    return 0;
}

int queInsert(pQue_t pq,int fd,char comBuf[][64],int arg,int workdir,int uid,MYSQL* conn){
    pNode_t pNew=(pNode_t)calloc(1,sizeof(Node_t));
    
    pNew->fd=fd;
    for(int i=0;i<3;++i){
        for(int j=0;j<64;++j){
            pNew->comBuf[i][j]=comBuf[i][j];
        }
    }
    pNew->arg=arg;
    pNew->workdir=workdir;
    pNew->uid=uid;
    pNew->conn=conn;
    if(pq->phead){
        pq->ptail->next=pNew;
        pq->ptail=pNew;
    }else{
        pq->phead=pNew;
        pq->ptail=pNew;
    }
    ++pq->size;
    return 0;
}

int queGet(pQue_t pq,int *fd,char comBuf[][64],int* arg,int *workdir,int *uid,MYSQL** conn){
    if(!pq->size){
        return -1;
    }
    pNode_t temp=NULL;
    *fd=pq->phead->fd;
    for(int i=0;i<3;++i){
        for(int j=0;j<64;++j){
            comBuf[i][j]=pq->phead->comBuf[i][j];
        }
    }
    *arg=pq->phead->arg;
    *workdir=pq->phead->workdir;
    *uid=pq->phead->uid;
    *conn=pq->phead->conn;

    temp=pq->phead;
    pq->phead=pq->phead->next;
    --pq->size;
    if(!pq->size){
        pq->ptail=NULL;
    }
    free(temp);
    temp=NULL;
    return 0;
}

int queClear(pQue_t pq){
    while(pq->size!=0){
       break; 
    }
    return 0;
}
