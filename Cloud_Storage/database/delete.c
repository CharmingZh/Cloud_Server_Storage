#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>

int main(int argc,char* argv[])
{
	MYSQL *conn;

	char *server="localhost";
	char *user="root";
	char *password="密码";
	char *database="37th";
	char query[200]="delete from equipment where name='duolanjie'";
	int queryRet;

	conn = mysql_init(NULL);

	if(!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf("Error connecting to database: %s\n", mysql_error(conn));
	}
    else
    {
		printf("Connected...\n");
	}

	queryRet = mysql_query(conn, query);
	if(queryRet)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}
    else
    {
        unsigned long ret = mysql_affected_rows(conn);
        if(ret)
        {
            printf("delete success, delete row=%lu\n", ret);
        }
        else
        {
            printf("delete failed,mysql_affected_rows: %lu\n", ret);
        }
	}

	mysql_close(conn);

	return 0;
}
