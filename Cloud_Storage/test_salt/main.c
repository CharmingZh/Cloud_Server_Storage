#define _XOPEN_SOURCE 
#include<stdio.h> 
#include<stdlib.h> 
#include<string.h> 
#include<unistd.h> 
#include<shadow.h> 
#include<errno.h>

int main(int argc,char**argv){
    if(argc!=3){
        printf("wrong argc\n");
        return 0;
    }
    
    char *ptr=crypt(argv[1],argv[2]);
    printf("pass=%s salt=%s crypt=%s\n",argv[1],argv[2],ptr);
    return 0; 
}
