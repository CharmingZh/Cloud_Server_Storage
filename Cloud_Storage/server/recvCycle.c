#include <func.h>

int recvCycle(int sockFd, void *buf, int totalLen)
{
    int ret = 0;
    int recvLen = 0;
    while(recvLen < totalLen){
        ret = recv(sockFd, (char*)buf + recvLen, totalLen - recvLen, 0);
        recvLen += ret;
        if(0 == ret){
            return -1;
        }
    }
    return recvLen;
}
