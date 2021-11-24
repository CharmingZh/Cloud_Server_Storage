#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include <func.h>
#include "poss.h"

typedef struct node{
    int fd;
    int type;
    char filename[64];
    char md5[64];
    off_t begin;
    off_t size;
    servSock_t server;
    int order;
    char token[256];
    struct node *next;
}Node_t,*pNode_t;

typedef struct tag_que{
    int size;
    pNode_t phead,ptail;
    pthread_cond_t cond;
    pthread_mutex_t que_mutex;
}Que_t,*pQue_t;

int que_init(pQue_t pq);
int queInsert(pQue_t pq,int fd,int type,char *filename,char *md5,off_t begin,off_t size,servSock_t server,int order,char *token);
int queGet(pQue_t pq,int *fd,int *type,char* filename,char *md5,off_t * begin,off_t * size,pServSock_t pserver,int *order,char *token);
#endif
