
#define _GNU_SOURCE
#include<func.h>

int mergeFile(char* md5,char* fileName){
    char name0[64]={0};
    char name1[64]={0};
    char name2[64]={0};
    sprintf(name0,"%s_0",md5);
    sprintf(name1,"%s_1",md5);
    sprintf(name2,"%s_2",md5);

    //printf("md5=%s\n",md5);
    //printf("filename=%s\n",fileName);
    struct stat fileInfo0;
    struct stat fileInfo1;
    struct stat fileInfo2;
    stat(name0,&fileInfo0); 
    stat(name1,&fileInfo1);
    stat(name2,&fileInfo2);
    //struct stat fileInfo;
    //bzero(&fileInfo,sizeof(fileInfo));

    /* while(1){ */
    /*     bzero(&fileInfo0,sizeof(fileInfo0)); */
    /*     bzero(&fileInfo1,sizeof(fileInfo1)); */
    /*     bzero(&fileInfo2,sizeof(fileInfo2)); */
    /*     off_t sumLen=0; */
    /*     stat(name0,&fileInfo0); */
    /*     stat(name1,&fileInfo1); */
    /*     stat(name2,&fileInfo2); */
    /*     sumLen=fileInfo1.st_size+fileInfo2.st_size+fileInfo0.st_size; */
    /*     if(sumLen==totalLen){ */
    /*         printf("Merging~~\n"); */
    /*         break; */
    /*     } */
    /* } */
    
    int fd=open(fileName,O_RDWR|O_CREAT,0666);
    //stat(filename,fileInfo);

    int pfd[2]={0};
    pipe(pfd);
    //存入0号文件段
    off_t transLen=0;    
    int fd0=open(name0,O_RDWR);
    while(transLen<fileInfo0.st_size){
        int ret=splice(fd0,0,pfd[1],0,fileInfo0.st_size,0);
        ret=splice(pfd[0],0,fd,0,ret,0);//存入合并文件
        transLen+=ret;
    }
    //存入1号文件段
    transLen=0;
    int fd1=open(name1,O_RDWR);
    while(transLen<fileInfo1.st_size){
        int ret=splice(fd1,0,pfd[1],0,fileInfo1.st_size,0);
        ret=splice(pfd[0],0,fd,0,ret,0);//存入合并文件
        transLen+=ret;
    }
    //存入2号文件段
    transLen=0;
    int fd2=open(name2,O_RDWR);
    while(transLen<fileInfo2.st_size){
        int ret=splice(fd2,0,pfd[1],0,fileInfo2.st_size,0);
        ret=splice(pfd[0],0,fd,0,ret,0);//存入合并文件
        transLen+=ret;
    }
    printf("merge success!\r\n\r\n");

    close(fd0);
    close(fd1);
    close(fd2);

    char command[64]={0};
    sprintf(command,"rm %s",name0);
    system(command);//删除文件0
    memset(command,0,sizeof(command));
    sprintf(command,"rm %s",name1);
    system(command);//删除文件1
    memset(command,0,sizeof(command));
    sprintf(command,"rm %s",name2);
    system(command);//删除文件2
    
    //printf("delete success\r\n");
    close(fd);
    return 0;
}
