#include <func.h>
#include "poss.h"
#define MAXTIMEQUE 30 

int timeQueInit(pTimeQue_t timeQue)//初始化一个大小为三十的循环链表
{
       timeQue->timeHead = NULL;
       timeQue->timeTail = NULL;

       for(int i = 0; i < MAXTIMEQUE ; ++i){
            pTimeNode_t pTime = (pTimeNode_t)calloc(1,sizeof(timeNode_t));
            if(timeQue->timeHead == NULL ){
                timeQue->timeHead = pTime;
                timeQue->timeTail = pTime;
            }
            else{
                timeQue->timeTail->nextTimeNode = pTime;
                timeQue->timeTail = pTime;
            }
       } 
       timeQue->timeTail->nextTimeNode = timeQue->timeHead;
    return 0;
}

