/*************************************************************************
    > File Name: sendFd.c
    > Author: wanke
    > Mail: 937502923@qq.com 
    > Created Time: 2021年04月19日 星期一 21时34分26秒
 ************************************************************************/

#include <func.h>
#include "poss.h"
int sendFd(int pipeFd,int fd){
	struct msghdr msg;
    memset(&msg,0,sizeof(msg));

    struct iovec iov;
    memset(&iov,0,sizeof(iov));
    char buf[8]={0};
    strcpy(buf,"hi");

    iov.iov_base=buf;
    iov.iov_len=strlen(buf);

    msg.msg_iov=&iov;
    msg.msg_iovlen=1;

    int len=CMSG_LEN(sizeof(int));
    struct cmsghdr *cmsg=(struct cmsghdr*)calloc(1,len);

    cmsg->cmsg_len =len;
    cmsg->cmsg_level=SOL_SOCKET;
    cmsg->cmsg_type=SCM_RIGHTS;

    *(int *)CMSG_DATA(cmsg)=fd;

    msg.msg_control=cmsg;
    msg.msg_controllen=len;
    sendmsg(pipeFd,&msg,0);

	return 0;
}

