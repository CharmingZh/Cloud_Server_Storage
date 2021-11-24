
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

int queInsert(pQue_t pq,int fd,MYSQL* conn){
    pNode_t pNew=(pNode_t)calloc(1,sizeof(Node_t));
    pNew->fd=fd;
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

int queGet(pQue_t pq,int *fd,MYSQL** conn){
    if(!pq->size){
        return -1;
    }
    pNode_t temp=NULL;
    *fd=pq->phead->fd;
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
