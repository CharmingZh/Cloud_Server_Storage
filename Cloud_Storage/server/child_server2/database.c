#include <func.h>
#include "database.h"

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
        //MYSQL_ROW row;
        MYSQL_RES *res;
        res=mysql_store_result(conn);
        return res;
        /* row=mysql_fetch_row(res); */
        /* if(row==NULL){ */
        /*     return NULL; */
        /* }else{ */
        /*     return res; */
        /* } */
    }
}

int mysqlClose(MYSQL* conn){
    mysql_close(conn);
    return 0;
}
