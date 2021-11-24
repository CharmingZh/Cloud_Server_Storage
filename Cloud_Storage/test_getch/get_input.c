#include <func.h>
#include <termio.h>
#define MAXCOMMAND 20
int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;

    if (tcgetattr(fd, &tm) < 0) {//保存现在的终端设置
        return -1;
    }

    tm_old = tm;
    cfmakeraw(&tm);//更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
    if (tcsetattr(fd, TCSANOW, &tm) < 0) {//设置上更改之后的设置
        return -1;
    }

    ch = getchar();
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//更改设置为最初的样子
        return -1;
    }

    return ch;

}

int change_n(int type,int* n){//0表示-，1表示+
    if(type){
        ++*n;
        if(*n>=MAXCOMMAND){
            *n=0;
        }
    }else{
        --*n;
        if(*n<0){
            *n=MAXCOMMAND-1;
        }
    }
    return 0;
}

int get_input()
{
    int a=0;
    char command[MAXCOMMAND][64];
    for(int i=0;i<MAXCOMMAND;++i){
        memset(command[i],0,sizeof(command[i]));
    }
    int n=0;
    int k=0;
    while((a=getch())!=3){
        if(13==a){
            change_n(1,&n);
            memset(command[n],0,sizeof(command[n]));
            k=0;
            printf("\n");
        }else if(27==a){
            if(91==getch()){
                int c=getch();
                if(65==c){
                    change_n(0,&n);
                    if(strlen(command[n])==0){
                        change_n(1,&n);
                        continue;
                    }
                }else if(66==c){
                    change_n(1,&n);
                    if(strlen(command[n])==0){
                        change_n(0,&n);
                        continue;
                    }
                }
                for(int i=0;i<k;++i){
                    printf("\b \b");
                }
                k=strlen(command[n]);
                printf("%s",command[n]);
            }  
        }else if(8==a || 127==a){
            if(k>0){
                printf("\b \b");
                --k;
                command[n][k]='\0';
            }
        }
        else if(a>=32 && a!=127){
            command[n][k]=a;
            ++k;
            printf("%c",a);
        }
    }
    printf("\n");
    return 0;
}

