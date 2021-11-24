#include <func.h>
#include "pthread_pool.h"
#include "linkList.h"
#include "database.h"
int wrongCom(int fd,char * buf);
int mv(int fd,int *workDir,char *buf,int uid,char *wDir,MYSQL *conn){
    int j=0;
    char comBuf[3][64];//存储命令以及命令参数
    for(int i=0;i<3;++i){
        memset(comBuf[i],0,sizeof(comBuf[i]));//初始化数组
    }
    int arg=0;//命令参数数量，包括命令
    for(int i=0;i<3;++i){//根据空格将收到的字符串分解成命令及命令参数
        int k=0;
        while(buf[j]==' '){
            ++j;
        }
        while(buf[j]!='\0'){
            if(buf[j]!=' '){
                comBuf[i][k]=buf[j];
            }else{
                break;
            }
            ++j;
            ++k;
        }
        if(k>0){
            ++arg;
        }
    }
    MYSQL_RES* res=NULL;
    MYSQL_ROW row;
    Train_t train;
    memset(&train,0,sizeof(train));
    char wrongMsg[64]={0};
    char command[256]={0};//要用到的数据库操作函数
    if(arg != 3){
        strcpy(wrongMsg,"wrong args!");
        wrongCom(fd,wrongMsg);
        return -1;
    }
    
    //记录传入的工作目录，mv操作不会改变当前工作目录
    int curDir = *workDir;
    //记录此用户的根目录 
    int root = 0;
    sprintf(command,"select pwd from user where id = %d",uid);
    res = selectCommand(conn,command);
    row = mysql_fetch_row(res);//atoi(row[0])是根目录
    root = atoi(row[0]);

    int ret = 0;
    //实现mv
    //
    //判断当前工作目录是否存在要拷贝的文件
    sprintf(command,"select filename,md5,filesize,id from file where parent_id = %d \
            AND filename = '%s' AND type = 1",curDir,comBuf[1]);
    res = selectCommand(conn,command);
    row = mysql_fetch_row(res);
    if(row == NULL){
        strcpy(wrongMsg,"file not exist\n");
        wrongCom(fd,wrongMsg);
        return -1;
    }
    //如果存在就存储文件信息
    char filename[64] = {0};
    strcpy(filename,row[0]);
    char md5[100] = {0};
    strcpy(md5,row[1]);
    int filesize = atoi(row[2]);
    int fileID = atoi(row[3]);
    //继续，判断目标目录是一级还是二级目录
    int layer = 0;//记录目标目录的层数
    char dir[128] = {0};
    char dir_list[5][32] = {0};
    strcpy(dir,comBuf[2]);
    int loc = 0;
    int n = 0;
    int ptr = 0;
    for(int i = 0;i<strlen(dir)+1;i++){
        if(dir[i] == '/'){
            for(int j = 0;j<i-loc;j++){//开始复制
                dir_list[n][j] = dir[ptr];
                ptr++;
            }
            n++;
            if(n>4){
                strcpy(wrongMsg,"cannot copy to more than 5 layers dir\n");
                wrongCom(fd,wrongMsg);
                return -1;
            }
            layer++;
            loc = i+1;
            ptr = i+1;

        }
        if(dir[i] == '\0'){
            for(int j = 0;j<strlen(dir)-loc;j++){
                dir_list[n][j] = dir[ptr];
                ptr++; 
            }
            layer++;
        }
    }
    //单级目录
    if(layer == 1){
        //先判断是否为特殊目录
        if(strcmp(dir_list[0],".") == 0){//如果是根目录，先判断根目录下是否已有此文件
            sprintf(command,"select id from file where parent_id = %d AND filename = '%s'",root,filename);
            res = selectCommand(conn,command);
            row = mysql_fetch_row(res);
            if(row){//如果文件存在
                strcpy(wrongMsg,"file has existed!\n");
                wrongCom(fd,wrongMsg);
                return -1;
            }
            //开始拷贝
            sprintf(command,"insert into file (parent_id,filename,owner_id,md5,filesize,type) \
                    values (%d,'%s',%d,'%s',%d,%d)",root,filename,uid,md5,filesize,1);
            ret = mysqlCommand1(conn,command);
            if(ret == 0){
                sprintf(command,"delete from file where id = %d",fileID);
                mysqlCommand1(conn,command);
                return 0;
            }
            else{
                strcpy(wrongMsg,"move failed!\n");
                wrongCom(fd,wrongMsg);
                return -1; 
            }
        }
        if(strcmp(dir_list[0],"..") == 0){//如果目标目录是上一级目录，先找上一级目录的id
            sprintf(command,"select parent_id from file where id = %d",curDir);
            res = selectCommand(conn,command);
            row = mysql_fetch_row(res);
            if(atoi(row[0]) == 0){//如果上一级目录的id是0，代表已在根目录下，不能拷贝 
                strcpy(wrongMsg,"error dir!\n");
                wrongCom(fd,wrongMsg);
                return -1; 
            }
            int preDirID = atoi(row[0]);
            //再判断在上一级目录中，文件是否已存在
            sprintf(command,"select id from file where parent_id = %d AND filename = '%s'",preDirID,filename);
            res = selectCommand(conn,command);
            row = mysql_fetch_row(res);
            if(row){//如果文件存在
                strcpy(wrongMsg,"file has existed!\n");
                wrongCom(fd,wrongMsg);
                return -1; 
            }   
            //拷贝
            sprintf(command,"insert into file (parent_id,filename,owner_id,md5,filesize,type) \
                    values (%d,'%s',%d,'%s',%d,%d)",preDirID,filename,uid,md5,filesize,1);
            ret = mysqlCommand1(conn,command);
            if(ret){
                strcpy(wrongMsg,"move failed!\n");
                wrongCom(fd,wrongMsg);
                return -1; 
            }    
            else{
                sprintf(command,"delete from file where id = %d",fileID);
                mysqlCommand1(conn,command);
                return 0;
            }
        }
        //普通目录的情况
        int toDir = 0;
        //先判断普通目标目录在当前目录下是否存在
        sprintf(command,"select id from file where parent_id = %d AND filename = '%s'",curDir,comBuf[2]);
        res = selectCommand(conn,command);
        row = mysql_fetch_row(res);
        if(row == NULL){//atoi(row[0])是目标目录的id
            strcpy(wrongMsg,"dir not exist!\n");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        toDir = atoi(row[0]);
        //判断目标目录下是否已存在此文件
        sprintf(command,"select id from file where parent_id = %d AND filename = '%s'",toDir,filename);
        res = selectCommand(conn,command);
        row = mysql_fetch_row(res);
        if(row){//如果文件已存在
            strcpy(wrongMsg,"file has existed!\n");
            wrongCom(fd,wrongMsg);
            return -1;  
        }
        //拷贝
        sprintf(command,"insert into file (parent_id,filename,owner_id,md5,filesize,type) \
               values (%d,'%s',%d,'%s',%d,%d)",toDir,filename,uid,md5,filesize,1);
        ret = mysqlCommand1(conn,command);
        if(ret){
            strcpy(wrongMsg,"move failed!\n");
            wrongCom(fd,wrongMsg);
            return -1; 
        }    
        else{
            sprintf(command,"delete from file where id = %d",fileID);
            mysqlCommand1(conn,command);
            return 0;
        }
    }
    //多级目录
    else{
        //记录当前所在层数
        int curLv = 1;
        //先判断一级目录是否特殊，如果特殊则先切换当前工作目录
        if(strcmp(dir_list[0],".") == 0){//如果一级目录是根目录，切换当前工作目录到根目录
            curDir = root;
            curLv++;//切换了之后，就认为第一级目录存在
        }
        if(strcmp(dir_list[0],"..") == 0){//如果一级目录是上一级目录，切换当前工作目录到上一级目录
            //先得到上一级目录的id
            sprintf(command,"select parent_id from file where id = %d",curDir);
            res = selectCommand(conn,command);
            row = mysql_fetch_row(res);
            if(atoi(row[0]) == 0){//如果上一级目录的id是0，代表已在根目录下，不能拷贝 
                strcpy(wrongMsg,"error dir!\n");
                wrongCom(fd,wrongMsg);
                return -1; 
            }
            curDir = atoi(row[0]);
            curLv++;//切换了之后，就认为第一级目录一定存在
        }
        //特殊目录切换后或是普通目录，循环判断下级目录是否存在且不为 .或..
        for(int i = curLv-1;i<layer;i++){
            if(strcmp(dir_list[i],"..") == 0 || strcmp(dir_list[i],".") == 0){
                strcpy(wrongMsg,"error dir!\n");
                wrongCom(fd,wrongMsg);
                return -1; 
            }
            sprintf(command,"select id from file where parent_id = %d AND filename = '%s'",curDir,dir_list[i]);
            res = selectCommand(conn,command);
            row = mysql_fetch_row(res);
            if(row == NULL){//atoi(row[0])是目标目录的id
                strcpy(wrongMsg,"dir not exist!\n");
                wrongCom(fd,wrongMsg);
                return -1;
            }
            curDir = atoi(row[0]);
        }
        //如果所有目录都存在，判断最后一级目录中文件是否已存在
        sprintf(command,"select id from file where parent_id = %d AND filename = '%s'",curDir,filename);
        res = selectCommand(conn,command);
        row = mysql_fetch_row(res);
        if(row){//如果文件已存在
            strcpy(wrongMsg,"file has existed!\n");
            wrongCom(fd,wrongMsg);
            return -1;  
        }
        //拷贝
        sprintf(command,"insert into file (parent_id,filename,owner_id,md5,filesize,type) \
                values (%d,'%s',%d,'%s',%d,%d)",curDir,filename,uid,md5,filesize,1);
        ret = mysqlCommand1(conn,command);
        if(ret){
            strcpy(wrongMsg,"move failed!\n");
            wrongCom(fd,wrongMsg);
            return -1; 
        }    
        else{
            sprintf(command,"delete from file where id = %d",fileID);
            mysqlCommand1(conn,command);
            return 0;
        }
    } 
}
