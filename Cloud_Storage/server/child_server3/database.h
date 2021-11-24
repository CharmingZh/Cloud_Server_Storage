#ifndef __DATABASE_
#define __DATABASE_
#include <mysql/mysql.h>

MYSQL* mysqlRun();
int mysqlCommand1(MYSQL* conn,char* command);
MYSQL_RES *selectCommand(MYSQL* conn,char* command);
int mysqlClose(MYSQL* conn);
int logWrite(MYSQL * conn,int  user_id,char *op);


#endif
