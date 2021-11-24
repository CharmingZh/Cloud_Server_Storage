#define _GNU_SOURCE
#include <func.h>
#include "pthread_pool.h"
#include "poss.h"
#define MAXFIZE 100*1024*1024
int putsFile(int fd,int arg,char comBuf[3][64],int *workDir,int uid,MYSQL* conn){
    {       
        char wrongMsg[64] = {0};
        char command[128] = {0};
        MYSQL_RES* res = NULL;
        MYSQL_ROW row;
        Train_t train;
        memset(&train,0,sizeof(train));

        char fileNameBuf[64] = {0};
        memset(fileNameBuf,0,sizeof(fileNameBuf));
        /* sprintf(command,"select id from file where parent_id = \'%d\' and filename = \'%s\'",*workDir,comBuf[1]);//查找是否存在文件名 */

        /* res = selectCommand(conn,command);//查找失败 res = NULL */

        /* if(res == NULL){//数据库不存在， */
        /*     strcpy(wrongMsg,"mysql is not exist!"); */
        /*     wrongCom(fd,wrongMsg); */
        /*     return -1; */
        /* } */
        /* else if( (row = mysql_fetch_row(res))!= NULL ){ */
        /*     strcpy(wrongMsg,"filename is exist!"); */
        /*     wrongCom(fd,wrongMsg); */
        /*     return -1; */
        /* } */
        /* else if((row = mysql_fetch_row(res)) == NULL){//文件名不存在 */
        /*     //strcpy(recvBuf,"please send the md5");//让客户端发送md5值进行检查,文件名发送回去 */
        /* train.len = -3;//-3 标志位 为上传 */
        /* send(fd,&train.len,4,0); */
        memset(&train,0,sizeof(train));
        strcpy(train.buf,comBuf[1]);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);

        memset(&train,0,sizeof(train));
        recv(fd,&train.len,4,0);
        if(train.len == -1){

            return -1;
        }
        recv(fd,train.buf,train.len,0);

        char md5Buf[64] = {0};
        memset(md5Buf,0,sizeof(md5Buf));
        strcpy(md5Buf,train.buf);
        sprintf(command,"select id from file where md5 = \'%s\'",md5Buf);
        //mysql_free_result(res);
        //res =NULL;
        res = selectCommand(conn,command);
        if(res == NULL){
            strcpy(wrongMsg,"there is no database");
            wrongCom(fd,wrongMsg);
            return -1;
        }

        else if( (row = mysql_fetch_row(res)) != NULL){
            //在数据库加上新文件名
            mysql_free_result(res);
            res = NULL;
            memset(command,0,sizeof(command));
            sprintf(command,"select filesize from file where md5 = '%s'",md5Buf);
            res = selectCommand(conn,command);
            row = mysql_fetch_row(res);
            long long filesize =atoi(row[0]);

            memset(command,0,sizeof(command));
            sprintf(command,"select * from file where parent_id =%d and filename ='%s'",*workDir,comBuf[1]);
            res=selectCommand(conn,command);
            row=mysql_fetch_row(res);
            if(row!=NULL){
                strcpy(wrongMsg,"the file is exist");
                wrongCom(fd,wrongMsg);
                return -1;   
            }

            memset(command,0,sizeof(command));
            sprintf(command,"insert into file (parent_id,filename,owner_id,md5,filesize,type)values(%d,'%s',%d,'%s',%lld,1)",*workDir,comBuf[1],uid,md5Buf,filesize);
            mysqlCommand1(conn,command);         

            printf("add the filename success\n");
            printf("the filename is different , but the file is exist\n");                

            strcpy(wrongMsg,"add the filename success,but the file is exist");
            wrongCom(fd,wrongMsg);
            return -1;
        }

        else{
            memset(&train,0,sizeof(train));
            strcpy(train.buf,"please send the file");
            train.len = strlen(train.buf);
            send(fd,&train.len,4,0);
            send(fd,train.buf,train.len,0);

            //接受文件
            int dataLen = 0;
            char buf1[1000] = {0};
            recv(fd,&dataLen,4,0);
            recv(fd,buf1,dataLen,0);

            char dirFile[64] = {0};
            strcpy(dirFile,"file/");//创建同名文件,用MD5值保存
            strcat(dirFile,md5Buf);
            int fp = open(dirFile,O_RDWR|O_CREAT,0600);
            printf("get the md5 filename success\n");
            off_t fileSize = 0;
            //off_t recvLen = 0;
            dataLen = 0;
            recv(fd,&dataLen,4,0);
            //printf("datalen =  %d\n",dataLen);
            recv(fd,&fileSize,dataLen,0);
            //printf("filesize = %ld\n",fileSize);   
            memset(command,0,sizeof(command));

            sprintf(command,"insert into file (parent_id,filename,owner_id,md5,filesize,type)values(%d,'%s',%d,'%s',%ld,1)",*workDir,comBuf[1],uid,md5Buf,fileSize);
            mysqlCommand1(conn,command);
            if(fileSize > MAXFIZE){
                int fd1[2];
                pipe(fd1);
                off_t recvLen = 0;
                
                while(recvLen < fileSize){
                    int ret = splice(fd,0,fd1[1],0,fileSize,0);

                    if(0 == ret){
                        if(recvLen >= fileSize){
                           break;
                        }
                        else{
                            printf("the client halt,trans defeated,delete incomplete file\n");
                            close(fp);
                            char buf[64] = {0};
                            sprintf(buf,"rm %s",dirFile);
                            system(buf);

                            memset(command,0,sizeof(command));
                            sprintf(command,"delete from file where md5='%s'",md5Buf);
                            mysqlCommand1(conn,command);

                            return -1;
                        }
                    }

                    ret = splice(fd1[0],0,fp,0,ret,0);
                    recvLen += ret;    
                }

            }

            else{
                while(1){
                    memset(buf1,0,sizeof(buf1));
                    int ret = recv(fd,&dataLen,4,0);

                    if(0 == dataLen){
                        break;
                    }
                    ret = recvCycle(fd,buf1,dataLen);
                    if(-1 == ret){
                        printf("the client halt,trans defeated,delete incomplete file\n");
                        close(fp);
                        char buf[64] = {0};
                        sprintf(buf,"rm %s",dirFile);
                        system(buf);

                        memset(command,0,sizeof(command));
                        sprintf(command,"delete from file where md5='%s'",md5Buf);
                        mysqlCommand1(conn,command);

                        return -1;
                    }
                    //recvLen +=ret;
                    write(fp,buf1,ret);
                }
            }
            printf("recv file success\n"); 
            /* printf("copy the file to the child sever\n"); */
            char buf[64] = {0};
            //sprintf(buf,"",dirFile);
            memset(buf,0,sizeof(buf));
            sprintf(buf,"cp %s child_server1",dirFile);
            system(buf);
            memset(buf,0,sizeof(buf));
            sprintf(buf,"cp %s child_server2",dirFile);
            system(buf);
            memset(buf,0,sizeof(buf));
            sprintf(buf,"cp %s child_server3",dirFile);
            system(buf);
            //printf("cp over\n");

                     
            close(fp);
        }

        //检测当前工作目录下是否有同名文件
        //检测文件的MD5值，看看有没有相匹配的文件，如果有则只需要添加数据库的数据即可
        //如果没有匹配，就开始传输文件，将文件名保存为MD5值
        return 0;
    }
    }
