#include <mysql/mysql.h>
#include <string.h>
#include <stdio.h>

int main(int argc,char* argv[])
{
	MYSQL *conn;
	char *server = "localhost";
	char *user = "root";
	char *password = "root";
	char *database = "lol";//要访问的数据库名称
    char query[200]="insert into member (id, name, age, ATK) values (7,'timo', 27, 86)";
	int queryResult;

	conn = mysql_init(NULL);

	if(!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
	{
		printf("Error connecting to database: %s\n", mysql_error(conn));
	}
    else
    {
		printf("Connected...\n");
	}

	queryResult = mysql_query(conn, query);
	if(queryResult)
	{
		printf("Error making query:%s\n", mysql_error(conn));
	}
    else
    {
		printf("insert success\n");
	}
	mysql_close(conn);

	return 0;
}
