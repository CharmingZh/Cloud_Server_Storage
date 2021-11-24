#include <func.h>
#include "poss.h"
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <sys/timerfd.h>
const int MAXNUM = 30;
#define MAXUSER 3

int main(int argc,char *argv[]){
    ARGS_CHECK(argc,4);
    int processNum=atoi(argv[3]);
    pProcess_data_t pData=(pProcess_data_t)calloc(processNum,sizeof(process_data_t));
    makeChild(processNum,pData);

    int sfd=0;
    tcpInit(argv[1],argv[2],&sfd);

    int epfd=epoll_create(1);
    epollAddFd(sfd,epfd);

    for(int i=0;i<processNum;++i){
        epollAddFd(pData[i].pipefd,epfd);
    }

    //监听时间描述符到监听集合
    struct itimerspec new_value;
    struct timespec now;
    uint64_t exp;
    //ssize_t s;

    int ret = clock_gettime(CLOCK_REALTIME, &now);
    assert(ret != -1);

    new_value.it_value.tv_sec = 1; 
    new_value.it_value.tv_nsec = now.tv_nsec; 

    new_value.it_interval.tv_sec = 1;      
    new_value.it_interval.tv_nsec = 0;

    int timefd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK); 
    assert(timefd != -1);

    ret = timerfd_settime(timefd, 0, &new_value, NULL); 
    epollAddFd(timefd,epfd);


    struct epoll_event evs[MAXNUM+5];

    int newfd=0;
    int readyNum=0;

    pTimeQue_t timeQue = (pTimeQue_t)calloc(1,sizeof(timeQue_t));

    timeQueInit(timeQue);
    int nowTime = 0;

    pTimeNode_t nowTimeNode;//轮盘指针

    nowTimeNode = timeQue->timeHead;//开始指向头节点
    
    
    //定义一个MAP结构体数组  用来存放每个进程的 进程号 和 状态 以及 最后相应的时间点   
    
     map_t userMap[4];
     
     memset(userMap,0,sizeof(map_t)*4);
    
    
    
    while(1){

        readyNum=epoll_wait(epfd,evs,MAXNUM+5,-1);
        for(int i=0;i<readyNum;++i){
            if(evs[i].data.fd==sfd){
                newfd=accept(sfd,NULL,NULL);
                //每次加入一个用户时，在相应的位置的数组0号位置记录个数，                   
                if(0 == nowTimeNode->userGroup){
                    nowTimeNode->userGroup = 1;
                }
                else{
                    nowTimeNode->userGroup += 1;
                    //printf("now user Numbers = %d\n",nowTimeNode->userGroup[0]);
                }                   
                
                for(int j=0;j<processNum;++j){
                    if(0==pData[j].flag){
                        sendFd(pData[j].pipefd,newfd);
                        pData[j].flag=1;
                            
                        userMap[j+1].flag = 1;
                        userMap[j+1].userID = j+1;
                        userMap[j+1].lastPackTime = nowTime;
                        
                        //printf("userMap[%d] = %d\n",j+1,userMap->lastPackTime);             

                              // j+1表示子进程的标号，存储在当前节点的数组中；
                        //printf("nowTimeuserGroup = %d\n",nowTimeNode->userGroup);
                        break;
                    }
                }
                close(newfd);
            }

            else if(evs[i].data.fd == timefd)
            {
                ++nowTime;
                 read(evs[i].data.fd, &exp, sizeof(uint64_t)); 
            
              //  printf("now time address = %d\n",nowTime);

                nowTimeNode = nowTimeNode->nextTimeNode;
                //printf("ok \n");
                //当当前位置的数组0号位的数大于0时，遍历数组的,表示这个点目前有多少个进程
                
                //int userNumber = nowTimeNode->userGroup[0];           
                //printf("now point user numbers = %d\n",userNumber);
                int closeNum = 0;//已经关闭的个数

                char closeBuf[6] = "close";
                
                        //遍历userMap,找到时间点相同的进程号，
                for(int i = 1; i <= MAXUSER; ++i){
                       //printf("userMap[%d] = %d\n",i,userMap[i].lastPackTime);
                        if(userMap[i].lastPackTime == nowTime){
                                printf("send the close msg\n");
                                write(pData[i-1].pipefd,closeBuf,sizeof(closeBuf));
                                memset(&userMap[i],0,sizeof(map_t));
                                
                                pData[i-1].flag = 0;//进程置为空闲i
                                ++closeNum;
                        }
                    }
                    //printf("closed user numbers = %d\n",closeNum);
                
                if(nowTime >= 30){
                    nowTime = 0;
                }
            }
            else
            {
                for(int j=0;j<processNum;++j){
                    if(pData[j].pipefd==evs[i].data.fd){
                        char buf[64]={0};
                        read(pData[j].pipefd,buf,sizeof(buf));
                        //printf("the father get the msg from chlid = %s\n",buf);
                        if(strcmp(buf,"a") == 0){
                            pData[j].flag=0;
                            memset(&userMap[j+1],0,sizeof(map_t));
                        }
                        else if(strcmp(buf,"updata the time") == 0){                 
                            //nowTimeNode->userGroup[j+1] = j+1;// j+1表示子进程的标号，存储在当前节点的数组中；
                            //printf("the father process updata the data time %d\n",nowTime);
                            
                            
                            userMap[j+1].lastPackTime = nowTime;
                        }
                        break;
                    }
                }
            }
        }
    }
    return 0;
}

