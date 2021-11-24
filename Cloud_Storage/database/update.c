#include <mysql/mysql.h>
#include <stdio.h>
#include <string.h>
int main(int argc,char* argv[])
{
	if(argc!=2)
	{
		printf("error args\n");
		return -1;
	}

	MYSQL *conn;
	MYSQL_RES *res;
	MYSQL_ROW row;
	char* server="localhost";
	char* user="root";
	char* password="root";
	char* database="lol";
	char query[200]="update member set name ='"; 
	sprintf(query,"%s%s%s",query,argv[1],"' where id=7");
	puts(query);

	int queryResult;

	conn=mysql_init(NULL);

	if(!mysql_real_connect(conn,server,user,password,database,0,NULL,0))
	{
		printf("Error connecting to database:%s\n",mysql_error(conn));
	}
    else
    {
		printf("Connected...\n");
	}

	queryResult = mysql_query(conn, query);
	if(queryResult)
	{
		printf("Error making query:%s\n",mysql_error(conn));
	}
    else
    {
        int ret = mysql_affected_rows(conn);
        if(ret)
        {
            printf("update success\n");
        }
        else
        {
            printf("update fail, mysql_affected_rows:%d\n", ret);
        }
	}
	mysql_close(conn);
	return 0;
}
