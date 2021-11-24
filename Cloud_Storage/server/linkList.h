#ifndef __LINKLIST_H__
#define __LINKLIST_H__

#include <func.h>
#include "database.h"
typedef struct node{
    int fd;
    char comBuf[3][64];
    int arg;
    int workdir;
    int uid;
    MYSQL* conn;

    struct node *next;
}Node_t,*pNode_t;

typedef struct tag_que{
    int size;
    pNode_t phead,ptail;
    pthread_cond_t cond;
    pthread_mutex_t que_mutex;
}Que_t,*pQue_t;

int que_init(pQue_t pq);
int queInsert(pQue_t pq,int fd,char comBuf[][64],int arg,int workdir,int uid,MYSQL* conn);
int queGet(pQue_t pq,int *fd,char comBuf[][64],int* arg,int *workdir,int* uid,MYSQL** conn);
int queClear(pQue_t pq);
#endif
