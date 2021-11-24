#include "pthread_pool.h"                                                                                                                 
#include "linkList.h"
#include "database.h"

int wrongCom(int fd,char * buf);
int help(int fd,int args){
    char wrongMsg[64]={0};
    if(args != 1){
        strcpy(wrongMsg,"args wrong!");
        wrongCom(fd,wrongMsg);
        return -1;
    }
    Train_t train;
    memset(&train,0,sizeof(train));

    char buf[2000] = "----------------------------------------------------------------------\
                      \r\nThis is the system's manual pager.\
                      \r\nYou could use 9 utilities below:\
                      \r\n1.cd -- Change the working directory.\
                      \r\n2.ls -- List information about the current directory FILEs.\
                      \r\n3.pwd -- Print the full filename of the current working directory.\
                      \r\n4.puts -- Upload file from localhost to server.\
                      \r\n5.gets -- download file from server to localhost.\
                      \r\n6.rm -- Remove files or directories from the current working directory.\
                      \r\n7.mkdir -- Make directories from the current working directory.\
                      \r\n8.cp -- Copy files from the current working directory to aim dir.\
                      \r\n9.mv -- Move files from the current working directory to aim dir.\
                      \r\n\r\nUse \"man\" utility to get details about these utilities.\
                      \r\n----------------------------------------------------------------------";
    strcpy(train.buf,buf);
    train.len = strlen(train.buf);
    send(fd,&train.len,4,0);
    send(fd,train.buf,train.len,0);
    return 0;
}

int man(int fd,int arg,char *buf){
    if(arg!=2){
        char wrongMsg[64]={0};
        strcpy(wrongMsg,"wrong args!");
        wrongCom(fd,wrongMsg);
        return -1;
    }
    Train_t train;
    memset(&train,0,sizeof(train));

    if(strcmp(buf,"cd") == 0){
        char info[1000] = "----------------------------------------------------------------------\
                         \r\nDetails about cd :\
                         \r\n1. cd . -- Change the working directory into root dir.\
                         \r\n2. cd .. -- Change the working directory into parent dir.\
                         \r\n\t- If already in the root directory,return error info to client.\
                         \r\n3. cd dirname -- Change the working directory into destination dir.\
                         \r\n\t- If the dest dir not exist,return error info to client.\
                         \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0;
    }
    else if(strcmp(buf,"pwd") == 0){
        char info[1000] = "----------------------------------------------------------------------\
                         \r\nDetails about pwd :\
                         \r\nPrint the full filename of the current working directory.\
                         \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0;
    }
    else if(strcmp(buf,"puts") == 0){ 
        char info[1000] = "----------------------------------------------------------------------\
                        \r\nDetails about puts :\
                        \r\nFirstly,Check whether there is a file with the same name in the current\
                        \r\nworking directory.If the same file exists,return error info to client.\
                        \r\nAnd if not,check whether there is a file with the same MD5.When MD5 valu\
                        \r\ne matched,add info into database,not transfer file(flash upload).When\
                        \r\nnot matched,transfer file;\
                        \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0;

    } 
    else if(strcmp(buf,"gets") == 0){ 
        char info[1000] = "----------------------------------------------------------------------\
                         \r\nDetails about gets :\
                         \r\n1. gets filename -- normal download\
                         \r\n2. gets -m filename -- muti-point download\
                         \r\nFirstly,check whether there is a file with the same name in the current\
                         \r\nlocalhost dir.If the same file exists,return error info to client.And \
                         \r\nif not,check whether the file exists in server.If not,return error info\
                         \r\nto client.When file exists,download the file from server.\
                         \r\nIf download is interrupted,it will continue at break point when next d\
                         \r\nownload start.(breakpoint resume)\
                         \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0;
    } 
    else if(strcmp(buf,"rm") == 0){ 
        char info[1000] = "----------------------------------------------------------------------\
                           \r\nDetails about rm :\
                           \r\nremove from the current working dirname.\
                           \r\n1. rm filename\
                           \r\n2. rm -r directory\
                           \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0; 
    } 
    else if(strcmp(buf,"mkdir") == 0){ 
        char info[1000] = "----------------------------------------------------------------------\
                           \r\nDetails about mkdir :\
                           \r\nFirstly,check whether there is a dir with the same name in the current\
                           \r\nworking dir.And if not,create dir.\
                           \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0; 
    } 
    else if(strcmp(buf,"cp") == 0){ 
        char info[1000] = "----------------------------------------------------------------------\
                           \r\nDetails about cp :\
                           \r\nSupport copy file type and dir type,if need to copy dir,add -r after cp.\
                           \r\n1. cp file .\
                           \r\n2. cp file ..\
                           \r\n3. cp file single-level directory\
                           \r\n4. cp file multi-level directory\
                           \r\n5. cp file ./multi of single-level directory\
                           \r\n6. cp file ../multi or single-level directory\
                           \r\n7. cp -r dir dir\
                           \r\nFirstly,check the file to copy whether exist or not.If not,return err\
                           \r\nor info to client.And then check the destDir whether exist or not.\
                           \r\nIf not,return error info to client.\
                           \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0; 
    } 
    else if(strcmp(buf,"mv") == 0){ 
        char info[1000] = "----------------------------------------------------------------------\
                           \r\nDetails about mv :\
                           \r\nSupport move file type and dir type,if need to move dir,add -r after mv.\
                           \r\n1. mv file .\
                           \r\n2. mv file ..\
                           \r\n3. mv file single-level directory\
                           \r\n4. mv file multi-level directory\
                           \r\n5. mv file ./multi of single-level directory\
                           \r\n6. mv file ../multi or single-level directory\
                           \r\n7. mv -r dir dir\
                           \r\nFirstly,check the file to move whether exist or not.If not,return err\
                           \r\nor info to client.And then check the destDir whether exist or not.\
                           \r\nIf not,return error info to client.\
                           \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0; 
    } 
    else if(strcmp(buf,"ls") == 0){       
        char info[1000] = "----------------------------------------------------------------------\
                           \r\nDetails about ls :\
                           \r\nList information about the current directory FILEs.\
                           \r\n----------------------------------------------------------------------";
        strcpy(train.buf,info);
        train.len = strlen(train.buf);
        send(fd,&train.len,4,0);
        send(fd,train.buf,train.len,0);
        return 0;
    } 
    return 0;
}
