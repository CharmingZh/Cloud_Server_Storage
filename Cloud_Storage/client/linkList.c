
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
int queInsert(pQue_t pq,int fd,int type,char *filename,char *md5,off_t begin,off_t size,servSock_t server,int order,char* token){
    pNode_t pNew=(pNode_t)calloc(1,sizeof(Node_t));
    pNew->fd=fd;
    pNew->type=type;

    if(type>2 && type<5){
        strcpy(pNew->filename,filename);
        strcpy(pNew->md5,md5);
        pNew->begin=begin;
        pNew->size=size;
        memcpy(&pNew->server,&server,sizeof(servSock_t));
        pNew->order=order;
        strcpy(pNew->token,token);
    }

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
/* int fd; */
/*     int type; */
/*     char filename[64]; */
/*     char md5[32]; */
/*     off_t begin; */
/*     off_t size; */
/*     servSock_t server; */
/*     int order; */

int queGet(pQue_t pq,int *fd,int *type,char* filename,char *md5,off_t * begin,off_t * size,pServSock_t pserver,int *order,char* token){
    if(!pq->size){
        return -1;
    }
    pNode_t temp=NULL;
    *fd=pq->phead->fd;
    *type=pq->phead->type;

    if(*type>2 && *type<5){
        strcpy(filename,pq->phead->filename);
        strcpy(md5,pq->phead->md5);
        *begin=pq->phead->begin;
        *size=pq->phead->size;
        memcpy(pserver,&pq->phead->server,sizeof(servSock_t));
        *order=pq->phead->order;
        strcpy(token,pq->phead->token);
    }

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
