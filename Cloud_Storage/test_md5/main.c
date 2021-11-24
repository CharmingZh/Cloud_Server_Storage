#include<stdio.h> 
#include<string.h> 
#include<stdlib.h> 
#include<openssl/md5.h> 
int main(int argc,char* argv[]) { 
    char  buf[16]="helloworld"; 
    unsigned char md[16]; 
    int i; 
    MD5((unsigned char*)buf,strlen(buf),md); 
    printf("%s\n",md); 
    for(i=0;i<strlen(md);i++) 
        printf("%x",md[i]); 
    printf("\n"); 
    return 0;  
} 

