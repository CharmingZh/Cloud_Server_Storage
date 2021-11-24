#include <func.h>
#include "database.h"
#include "pthread_pool.h"

//删除文件
int func_file(char* md5,int id,MYSQL* conn){
    MYSQL_RES* res=NULL;
    MYSQL_ROW row;
    char command[128]={0};
    memset(command,0,sizeof(command));
    //1.删除文件在数据库的对应条目
    sprintf(command,"delete from file where md5=\'%s\' and id=%d",md5,id);
    int ret=mysqlCommand1(conn,command);
    ERROR_CHECK(ret,-1,"mysqlCommand1_file");
    //2.查找待删除文件是否有其他关联
    sprintf(command,"select * from file where md5=\'%s\'",md5);
    mysql_free_result(res);//清空res
    res=selectCommand(conn,command);
    row=mysql_fetch_row(res);
    if(row==NULL){//没有，直接删除文件
        printf("no relation\n");
        char buf[64]={0};
        sprintf(buf,"rm file/%s",md5);
        system(buf);
    }
    printf("File deleted\n");
    return 0;
}
//删除目录
int func_dir(int workID,int uid,MYSQL* conn){
    MYSQL_RES* res=NULL;
    MYSQL_ROW row;
    char command[128]={0};
    //1.删除workID下的所有文件
    memset(command,0,sizeof(command));
    mysql_free_result(res);
    sprintf(command,"select * from file where parent_id=%d and type=1",workID);
    res=selectCommand(conn,command);
    while((row=mysql_fetch_row(res))!=NULL){
        func_file(row[4],atoi(row[0]),conn);
    }
    //2.删除workID下的所有目录
    memset(command,0,sizeof(command));
    mysql_free_result(res);
    sprintf(command,"select * from file where parent_id=%d and type=0",workID);
    res=selectCommand(conn,command);
    while((row=mysql_fetch_row(res))!=NULL){
        func_dir(atoi(row[0]),uid,conn);
        //删除空目录
        memset(command,0,sizeof(command));
        sprintf(command,"delete from file where id=%d",atoi(row[0]));
        int ret=mysqlCommand1(conn,command);
        ERROR_CHECK(ret,-1,"mysqlCommand1_dir");
    }
    return 0;
}
//clienFd，参数1，参数2，当前工作目录id，用户id,数据库
int rmFile(int clienFd,char* parameter1,char* parameter2,int workID,int uid,MYSQL* conn){
    //情形1：删除目录
    if(strcmp("-r",parameter1)==0){
        MYSQL_RES* res=NULL;
        MYSQL_ROW row;
        char command[128]={0};//存放数据库命令

        sprintf(command,"select * from file where parent_id=%d and filename=\'%s\' and type=0",workID,parameter2);
        res=selectCommand(conn,command);
        row=mysql_fetch_row(res);
        if(row==NULL){//目录不存在，报错返回
            char buf[30]={0};
            strcpy(buf,"Directory does not exist");
            wrongCom(clienFd,buf);
            return 0;
        }
        func_dir(atoi(row[0]),uid,conn);
        //删除空目录
        memset(command,0,sizeof(command));
        sprintf(command,"delete from file where id=%d",atoi(row[0]));
        int ret=mysqlCommand1(conn,command);
        ERROR_CHECK(ret,-1,"mysqlCommand1_dir");
        printf("Directory deleted\n");
    }
    //情形2：删除文件
    else{
        MYSQL_RES* res=NULL;
        MYSQL_ROW row;
        char command[128]={0};//存放数据库命令

        sprintf(command,"select * from file where parent_id=%d and filename=\'%s\' and type=1",workID,parameter1);
        res=selectCommand(conn,command);
        row=mysql_fetch_row(res);
        if(row==NULL){//文件不存在，报错返回
            char buf[30]={0};
            strcpy(buf,"Filename does not exist");
            wrongCom(clienFd,buf);
            return 0;
        }
        func_file(row[4],atoi(row[0]),conn);//文件存在，执行删除
        printf("File deleted\n");
    }
    Train_t train;
    memset(&train,0,sizeof(train));
    strcpy(train.buf,"delete success");
    train.len=strlen(train.buf);
    send(clienFd,&train.len,4,0);
    send(clienFd,train.buf,train.len,0);
    return 0;
}
