#include <func.h>
#include <mysql/mysql.h>

MYSQL* mysqlRun()
{
    MYSQL *conn;
    char *server="localhost";
    char *user="root";
    char *password="1234";
    char *database="netdisk";

    conn=mysql_init(NULL);

    if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0)){
        return NULL; 
    }

    return conn;
}

int mysqlCommand1(MYSQL* conn,char* command){
    int queryResult=mysql_query(conn,command);
    if(queryResult){
        return -1;
    }else{
        unsigned long ret= mysql_affected_rows(conn);
        if(ret){
            return 0;
        }else{
            return -1;
        }
    }
}

MYSQL_RES *selectCommand(MYSQL* conn,char* command){
    int queryResult=mysql_query(conn,command);
    if(queryResult){
        return NULL;
    }else{
        return mysql_store_result(conn);
    }
}

int mysqlClose(MYSQL* conn){
    mysql_close(conn);
    return 0;
}
int main(){
    return 0;
}
