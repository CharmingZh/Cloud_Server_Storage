#include <func.h>
#include "pthread_pool.h"
#include "linkList.h"
#include "database.h"
int threadPool_init(pthreadPool_t pPool, int threadNum){//线程池初始化
    pPool->startFlag=0;
    pPool->threadNum=threadNum;
    pPool->pthid=(pthread_t*)calloc(threadNum,sizeof(pthread_t));
    que_init(&pPool->que);
    return 0;
}
void cleanFunc(void *p){//线程清理函数
    pQue_t pQue=(pQue_t)p;
    pthread_mutex_unlock(&pQue->que_mutex);
}

void* threadFunc(void *p){//线程工作函数
    pQue_t pQue=(pQue_t)p;
    int fd=0;
    char comBuf[3][64];
    int arg=0;
    int workdir=0;
    int uid=0;
    MYSQL* conn=NULL;
    int ret=-1;
    while(1){
        pthread_mutex_lock(&pQue->que_mutex);//对任务队列加???
        pthread_cleanup_push(cleanFunc,pQue);
        if(!pQue->size){
            pthread_cond_wait(&pQue->cond,&pQue->que_mutex);//任务队列中没有消息就等待
        }
        ret=queGet(pQue,&fd,comBuf,&arg,&workdir,&uid,&conn);//获取任务队列中的任务
        pthread_mutex_unlock(&pQue->que_mutex);
        if(!ret){
            int oldstate=0;
            if(!strcmp(comBuf[0],"puts")){
                pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldstate);//任务进行过程中屏蔽取消信???
                putsFile(fd,arg,comBuf,&workdir,uid,conn);
                pthread_setcancelstate(oldstate,NULL);

            }else if(!strcmp(comBuf[0],"gets")){
                pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&oldstate);//任务进行过程中屏蔽取消信???
                getsFile(fd,comBuf[1],uid,workdir,0,conn);
                pthread_setcancelstate(oldstate,NULL);

            }
        }
        pthread_cleanup_pop(1);
    }
    pthread_exit(NULL);
}

int threadPool_start(pthreadPool_t pPool){//循环创建子线???
    for(int i=0;i<pPool->threadNum;++i){
        pthread_create(&pPool->pthid[i],NULL,threadFunc,&pPool->que);
    }
    return 0;
}
int wrongCom(int fd,char * buf){//像客户端???送相应的错误信息
    Train_t train;
    memset(&train,0,sizeof(train));
    train.len=-1;//客户端发???
    send(fd,&train.len,4,0);
    strcpy(train.buf,buf);
    train.len=strlen(train.buf);
    send(fd,&train.len,4,0);
    send(fd,train.buf,train.len,0);
    return 0;
}
int recvCommand(int fd,char* buf,int *workDir,int uid,char* wDir,MYSQL* conn,pthreadPool_t pPool){//收到客户端的命令后的处理函数
    int j=0;
    logWrite(conn,uid,buf);
    char comBuf[3][64];//存储命令以及命令参数
    for(int i=0;i<3;++i){
        memset(comBuf[i],0,sizeof(comBuf[i]));//初始化数???
    }
    int arg=0;//命令参数数量，包括命???
    for(int i=0;i<3;++i){//根据空格将收到的字符串分解成命令及命令参???
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
    char command[128]={0};//要用到的数据库操作函???
    if(!strcmp(comBuf[0],"cd")){//收到cd 命令操作
        if(arg!=2){//判断命令后面的参数数量是否正???
            strcpy(wrongMsg,"wrong args!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        if(!strcmp(comBuf[1],".")){//.表示返回根目???
            //通过用户ID找到根目录文件ID和名???
            sprintf(command,
                    "select id,filename from file where id = (select pwd from user where id = \'%d\')",uid);
            res=selectCommand(conn,command);
            row=mysql_fetch_row(res);
            *workDir=atoi(row[0]);
            memset(wDir,0,128*sizeof(char));
            sprintf(wDir,"%s/",row[1]);
        }else if(!strcmp(comBuf[1],"..")){//..表示返回上一级目???
            //通过当前工作目录文件ID找到其上一级目录ID
            sprintf(command,
                    "select parent_id from file where id = \'%d\'",*workDir);
            res=selectCommand(conn,command);
            if(res==NULL || (row=mysql_fetch_row(res))==NULL){//如果没找到说明数据库的数据有???
                strcpy(wrongMsg,"the database is damage");
                wrongCom(fd,wrongMsg);
                return -1;
            }
            if(0==atoi(row[0])){//上一级目录文件ID???，说明此时已经在根目录下
                strcpy(wrongMsg,"Already in the root directory");
                wrongCom(fd,wrongMsg);
                return -1;
            }
            *workDir=atoi(row[0]);
            //重新调整当前工作目录文件ID以及目录字符???
            int i=0;
            for(i=0;wDir[i]!='\0';++i){}
            --i;
            wDir[i]='\0';
            for(;wDir[i]!='/';--i){
                wDir[i]='\0';
            }
        }else{
            //通过命令参数（也就是要进入的下一级目录文件名）以及此时的工作目录文件ID
            //查找在此时工作目录下的对应文件名的目录文件ID
            sprintf(command,
                    "select id from file where filename = \'%s\' and parent_id = \'%d\' and type = 0",comBuf[1],*workDir);
            res=selectCommand(conn,command);
            if(res==NULL || (row=mysql_fetch_row(res))==NULL){
                strcpy(wrongMsg,"this file is not found!");
                wrongCom(fd,wrongMsg);
                return -1;
            }
            *workDir=atoi(row[0]);
            sprintf(wDir,"%s%s/",wDir,comBuf[1]);
        }
        //向客户端发送当前工作目???
        train.len=strlen(wDir);
        strcpy(train.buf,wDir);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        mysql_free_result(res);
        return 0;

    }else if(!strcmp(comBuf[0],"ls")){//列出当前工作目录下的所有文???
        //通过当前工作目录ID查找其下面所有的文件,并且通过排序，把目录文件和普通文件分开
        if(1!=arg){
            strcpy(wrongMsg,"args wrong!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        sprintf(command,
                "select type,filename from file where parent_id = \'%d\' order by type",*workDir);
        res=selectCommand(conn,command);
        if(res==NULL || (row=mysql_fetch_row(res))==NULL){//文件没找到时，返回错误信???
            strcpy(wrongMsg,"there has not file!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        strcpy(train.buf,"directory: ");
        if(0==atoi(row[0])){//type???表示为目录文???
            do{
                strcat(train.buf,row[1]);
                strcat(train.buf," ");
            }while(NULL!=(row=mysql_fetch_row(res)) && 0==atoi(row[0]));//当type不为0时退出循???

        }
        strcat(train.buf,"\r\nfile: ");
        if(row!=NULL){//由于排过序，所以退出循环后，剩下的文件就是普通文???
            do{
                strcat(train.buf,row[1]);
                strcat(train.buf," ");
            }while(NULL!=(row=mysql_fetch_row(res)));
        }else{
            strcat(train.buf,"(NULL)");
        }
        train.len=strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        mysql_free_result(res);
        return 0;       
    }else if(!strcmp(comBuf[0],"pwd")){
        if(1!=arg){
            strcpy(wrongMsg,"args wrong!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        strcpy(train.buf,wDir);
        train.len=strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0;
    }else if(!strcmp(comBuf[0],"puts")){
       //检测当前工作目录下是否有同名文???
        //检测文件的MD5值，看看有没有相匹配的文件，如果有则只需要添加数据库的数据即???
        //如果没有匹配，就开始传输文件，将文件名保存为MD5???
        if(arg!=2){
            strcpy(wrongMsg,"wrong args!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        sprintf(command,"select id from file where parent_id = \'%d\' and filename = \'%s\'",*workDir,comBuf[1]);
        res = selectCommand(conn,command);
        if(NULL==res){
            strcpy(wrongMsg,"mysql is not exist!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        else if((row=mysql_fetch_row(res))!=NULL){
            strcpy(wrongMsg,"filename is exist!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        train.len=-3;
        send(fd,&train.len,4,0);
        int newClientFd=add_tcp(fd);
        pthread_mutex_lock(&pPool->que.que_mutex);
        queInsert(&pPool->que,newClientFd,comBuf,arg,*workDir,uid,conn);
        pthread_cond_signal(&pPool->que.cond);
        pthread_mutex_unlock(&pPool->que.que_mutex);
        return 1;
    }else if(!strcmp(comBuf[0],"gets")){
        if(arg<2 || arg>3){
            strcpy(wrongMsg,"wrong args!");
            wrongCom(fd,wrongMsg);
            return -1;

        }
        if(!strcmp(comBuf[1],"-m")){
            sprintf(command,"select * from file where parent_id=%d and owner_id=%d and filename=\'%s\'",*workDir,uid,comBuf[2]);
            res=selectCommand(conn,command);
            //若文件不存在,报错返回
            row=mysql_fetch_row(res);
            if(row==NULL){
                char buf[30]={0};
                strcpy(buf,"filename is not exist");
                wrongCom(fd,buf);
                return -1;
            }
            train.len=-4;//向客户端发???文件传输标志
            send(fd,&train,4,0);
            
            memset(&train,0,sizeof(train));
            strcpy(train.buf,comBuf[2]);
            train.len=strlen(train.buf);
            send(fd,&train.len,4,0);
            send(fd,train.buf,train.len,0);

            memset(&train,0,sizeof(train));
            strcpy(train.buf,row[4]);
            train.len=strlen(train.buf);
            send(fd,&train.len,4,0);
            send(fd,train.buf,train.len,0);

            memset(&train,0,sizeof(train));
            strcpy(train.buf,row[5]);
            train.len=strlen(train.buf);
            send(fd,&train.len,4,0);
            send(fd,train.buf,train.len,0);

            mysql_free_result(res);
            memset(command,0,sizeof(command));
            sprintf(command,"select * from server");
            
            res=selectCommand(conn,command);
            for(int i=0;i<3;++i){
                row=mysql_fetch_row(res);
                memset(&train,0,sizeof(train));
                strcpy(train.buf,row[1]);
                train.len=strlen(train.buf);
                send(fd,&train.len,4,0);
                send(fd,train.buf,train.len,0);

                memset(&train,0,sizeof(train));
                strcpy(train.buf,row[2]);
                train.len=strlen(train.buf);
                send(fd,&train.len,4,0);
                send(fd,train.buf,train.len,0);
            }

            return 1;
        }
        sprintf(command,"select * from file where parent_id=%d and owner_id=%d and filename=\'%s\'",*workDir,uid,comBuf[1]);
        res=selectCommand(conn,command);
        //若文件不存在,报错返回
        row=mysql_fetch_row(res);
        if(row==NULL){
            char buf[30]={0};
            strcpy(buf,"filename is not exist");
            wrongCom(fd,buf);
            return -1;
        }
        train.len=-2;//向客户端发???文件传输标志
        send(fd,&train,4,0);
        int newClientFd=add_tcp(fd);
        pthread_mutex_lock(&pPool->que.que_mutex);
        queInsert(&pPool->que,newClientFd,comBuf,arg,*workDir,uid,conn);
        pthread_cond_signal(&pPool->que.cond);
        pthread_mutex_unlock(&pPool->que.que_mutex);

        //getsFile(fd,comBuf[1],uid,*workDir,0,conn);
        //检测当前工作目录下是否有该名字的文???
        //如果有，根据文件的MD5值找到该文件并开始传输文???
        //通过命令参数确定传输位置
        return 1;
    }else if(!strcmp(comBuf[0],"rm")){
        //先通过命令检测要删除的文件是否存在，存在就删除该文件在数据库中的数据???
        //同时检测该文件是否有其它用户关联，如没有则将服务器中的文件删除
        rmFile(fd,comBuf[1],comBuf[2],*workDir,uid,conn);
        return 0;
    }else if(!strcmp(comBuf[0],"mkdir")){
        //通过命令检查当前工作目录下是否有文件与命令参数同名???
        //没有则将信息添加进数据表???
        if(2!=arg){
            strcpy(wrongMsg,"args wrong!");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        sprintf(command,
                "select filename from file where parent_id = \'%d\' and type = 0",*workDir);
        res=selectCommand(conn,command);
        if(res==NULL){//数据出错时，返回错误信息
            strcpy(wrongMsg,"the database is damage");
            wrongCom(fd,wrongMsg);
            return -1;
        }
        if((row=mysql_fetch_row(res))!=NULL){//当前工作目录有目录文件时，检测是否重???
            do{
                if(!strcmp(row[0],comBuf[1])){
                    strcpy(wrongMsg,"the directory is already exist!");
                    wrongCom(fd,wrongMsg);
                    return -1;
                }
            }while(NULL!=(row=mysql_fetch_row(res)));
        }
        memset(command,0,sizeof(command));
        sprintf(command,
                "insert into file (parent_id,filename,owner_id,type) values (%d,\'%s\',%d,0)",*workDir,comBuf[1],uid);
        int ret=mysqlCommand1(conn,command);
        if(!ret){
            strcpy(train.buf,"success!");
        }else{
            strcpy(train.buf,"fail!");
        }
        train.len=strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        mysql_free_result(res);
        return 1;
    }
    else if(!strcmp(comBuf[0],"cp")){
        if(!strcmp(comBuf[1],"-r")){
            cp_r(fd,workDir,buf,uid,wDir,conn);
        }else{
            cp(fd,workDir,buf,uid,wDir,conn);
        }

        return 0;
    }else if(!strcmp(comBuf[0],"mv")){
        if(!strcmp(comBuf[1],"-r")){
            mv_r(fd,workDir,buf,uid,wDir,conn);

        }else{
            mv(fd,workDir,buf,uid,wDir,conn);
        }
        return 0;
    }else if(!strcmp(comBuf[0],"help")){
        help(fd,arg);
        return 0;
    }else if(!strcmp(comBuf[0],"man")){
        man(fd,arg,comBuf[1]);
        return 0;
    }
    else{
        int len=-1;
        send(fd,&len,4,0);
        len=13;
        send(fd,&len,4,0);
        char tBuf[15]="wrong command";
        send(fd,tBuf,13,0);
        return -1;
    }
}
