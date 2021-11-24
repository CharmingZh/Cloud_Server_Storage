/*************************************************************************
    > File Name: epollAddFd.c
    > Author: wanke
    > Mail: 937502923@qq.com 
    > Created Time: 2021年04月19日 星期一 21时06分30秒
 ************************************************************************/

#include <func.h>
int epollAddFd(int sfd,int epfd){
	struct epoll_event event;
    memset(&event,0,sizeof(event));

    event.events=EPOLLIN;
    event.data.fd=sfd;
    epoll_ctl(epfd,EPOLL_CTL_ADD,sfd,&event);
	return 0;
}

