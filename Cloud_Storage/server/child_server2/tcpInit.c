/*************************************************************************
    > File Name: tcpInit.c
    > Author: wanke
    > Mail: 937502923@qq.com 
    > Created Time: 2021年04月19日 星期一 20时58分00秒
 ************************************************************************/

#include <func.h>
int tcpInit(char *ip,char *port,int *sockFd){
	int sfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(sfd,-1,"socket");

    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=inet_addr(ip);
    addr.sin_port=htons(atoi(port));

    int reuse=1;
    setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse));

    int ret=0;
    ret=bind(sfd,(struct sockaddr*)&addr,sizeof(addr));
    ERROR_CHECK(ret,-1,"bind");

    ret=listen(sfd,10);
    ERROR_CHECK(ret,-1,"listen");

    *sockFd=sfd;

	return 0;
}

