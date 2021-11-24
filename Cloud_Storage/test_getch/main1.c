#include <func.h>
#include <termio.h>

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
        if(*n>=5){
            *n=0;
        }
    }else{
        --*n;
        if(*n<0){
            *n=4;
        }
    }
    return 0;
}

int main(int argc,char*argv[])
{
    int a=0;
    while((a=getch())!=113){
        printf("%d\n",a);
    }
    return 0;
}

