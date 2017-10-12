#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <mntent.h>
#include <netdb.h>
#include <setjmp.h>
#include <signal.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <utime.h>
#include <limits.h>

#include <errno.h>
#include <sys/vfs.h>

#include <mysql.h>

#include <iostream>
#include <string>

using namespace std;


string ip;
int port;
string username;
string password;
string testdb;

#define STRING_SIZE 50

#define DROP_SAMPLE_TABLE "DROP TABLE IF EXISTS test_json"
#define CREATE_SAMPLE_TABLE "CREATE TABLE test_json(id INT primary key AUTO_INCREMENT, value json)shardkey=id"
//#define CREATE_SAMPLE_TABLE "CREATE TABLE test_json(id INT primary key AUTO_INCREMENT, value json)"
#define INSERT_SAMPLE "INSERT INTO test_json(value) VALUES(?)"



void dotest()
{
    //int timeout = 10;
    MYSQL* mysql = new MYSQL;
    mysql = mysql_init(mysql);
    //mysql_options (mysql , MYSQL_OPT_CONNECT_TIMEOUT , (const char *) &timeout);
    //mysql_options (mysql , MYSQL_OPT_READ_TIMEOUT , (const char *) &timeout);
    //mysql_options (mysql , MYSQL_OPT_WRITE_TIMEOUT , (const char *) &timeout);
    MYSQL* tmp = mysql_real_connect(mysql, ip.c_str(), username.c_str(), password.c_str(), testdb.c_str(), port, NULL, 0);
    if(tmp)
    {
        MYSQL_STMT    *stmt;
        MYSQL_BIND    bind[3];
        my_ulonglong  affected_rows;
        int           param_count;
        //short         small_data;
        //int           int_data;
        char          str_data[STRING_SIZE];
        unsigned long str_length;
        bool       is_null;
        //const ulong type= (ulong)CURSOR_TYPE_READ_ONLY;
        //int prefetch_rows = 2;

        if (mysql_query(mysql, DROP_SAMPLE_TABLE))
        {
            fprintf(stderr, " DROP TABLE failed\n");
            fprintf(stderr, " %s\n", mysql_error(mysql));
            //exit(0);
        }

        if (mysql_query(mysql, CREATE_SAMPLE_TABLE))
        {
            fprintf(stderr, " CREATE TABLE failed\n");
            fprintf(stderr, " %s\n", mysql_error(mysql));
        }

        /* Prepare an INSERT query with 3 parameters */
        /* (the TIMESTAMP column is not named; the server */
        /*  sets it to the current date and time) */
        stmt = mysql_stmt_init(mysql);
        if (!stmt)
        {
            fprintf(stderr, " mysql_stmt_init(), out of memory\n");
            exit(0);
        }
        if (mysql_stmt_prepare(stmt, INSERT_SAMPLE, strlen(INSERT_SAMPLE)))
        {
            fprintf(stderr, " mysql_stmt_prepare(), INSERT failed\n");
            fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        }
        fprintf(stdout, " prepare, INSERT successful\n");

        //mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void*) &type);
        //mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS,(void*) &prefetch_rows);

        /* Get the parameter count from the statement */
        param_count= mysql_stmt_param_count(stmt);
        fprintf(stdout, " total parameters in INSERT: %d\n", param_count);

        if (param_count != 1) /* validate parameter count */
        {
            fprintf(stderr, " invalid parameter count returned by MySQL\n");
        }

        /* Bind the data for all 3 parameters */

        memset(bind, 0, sizeof(bind));

        /* STRING PARAM */
        bind[0].buffer_type= MYSQL_TYPE_JSON;
        bind[0].buffer= (char *)str_data;
        bind[0].buffer_length= STRING_SIZE;
        bind[0].is_null= 0;
        bind[0].length= &str_length;

        /* Bind the buffers */
        if (mysql_stmt_bind_param(stmt, bind))
        {
            fprintf(stderr, " mysql_stmt_bind_param() failed\n");
            fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        }

        strncpy(str_data, "1234", STRING_SIZE); /* string  */
        str_length= strlen(str_data);

        /* INSERT SMALLINT data as NULL */
        is_null= 1;

        /* Execute the INSERT statement - 1*/
        for(int i = 0; i < 5; ++i)
        {
            if (mysql_stmt_execute(stmt))
            {
                fprintf(stderr, " mysql_stmt_execute(), 1 failed\n");
                fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
            }
        }

        /* Get the number of affected rows */
        affected_rows= mysql_stmt_affected_rows(stmt);
        fprintf(stdout, " total affected rows(insert 1): %lu\n",
                (unsigned long) affected_rows);

        if (affected_rows != 1) /* validate affected rows */
        {
            fprintf(stderr, " invalid affected rows by MySQL\n");
        }

        /* Close the statement */
        if (mysql_stmt_close(stmt))
        {
            /* mysql_stmt_close() invalidates stmt, so call          */
            /* mysql_error(mysql) rather than mysql_stmt_error(stmt) */
            fprintf(stderr, " failed while closing the statement\n");
            fprintf(stderr, " %s\n", mysql_error(mysql));
        }
    }
    if(mysql)
        delete mysql;
}

void dotest1()
{
    //int timeout = 10;
    MYSQL* mysql = new MYSQL;
    //MYSQL_RES *result;
    //MYSQL_ROW row;
    //my_bool reconnect = 0;
    MYSQL_STMT    *stmt;
    MYSQL_BIND    bind[1];
    MYSQL_BIND    bResult[2];
    //unsigned long length[1];
    int        int_data;
    mysql = mysql_init(mysql);
    //const char *normalSql = "select id from t1 where value=1";
    const char *stmtSql = "select id, value from test_json where value = ?";
    //mysql_options (mysql , MYSQL_OPT_CONNECT_TIMEOUT , (const char *) &timeout);
    //mysql_options (mysql , MYSQL_OPT_READ_TIMEOUT , (const char *) &timeout);
    //mysql_options (mysql , MYSQL_OPT_WRITE_TIMEOUT , (const char *) &timeout);
    MYSQL* tmp = mysql_real_connect(mysql, ip.c_str(), username.c_str(), password.c_str(), testdb.c_str(), port, NULL, 0);
    if(tmp)
    {
        /*
        // 1. normal buffer
        mysql_query(mysql, normalSql);
        result = mysql_store_result(mysql);
        while(row = mysql_fetch_row(result)) {
            printf("1 - %d\n", atoi(row[0]));
        }
        mysql_free_result(result);

        // 2. normal unbuffer
        mysql_query(mysql, normalSql);
        result = mysql_use_result(mysql);
        while(row = mysql_fetch_row(result)) {
            printf("2 - %d\n", atoi(row[0]));
        }
        mysql_free_result(result);


        //3. stmt buffer
        stmt = mysql_stmt_init(mysql);

        mysql_stmt_prepare(stmt, stmtSql, strlen(stmtSql));

        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type= MYSQL_TYPE_LONG;
        bind[0].buffer= (char *)&int_data;
        mysql_stmt_bind_param(stmt, bind);

        memset(bResult, 0, sizeof(bResult));
        bResult[0].buffer_type= MYSQL_TYPE_LONG;
        bResult[0].buffer= (char *)&int_data;
        mysql_stmt_bind_result(stmt, bResult);

        int_data= 1;
        mysql_stmt_execute(stmt);
        mysql_stmt_store_result(stmt);

        while(!mysql_stmt_fetch(stmt)) {
            printf("3 - %d \n", int_data);
        }

        mysql_stmt_close(stmt);
        */
        // 4. stmt unbuffer
        char str_data[STRING_SIZE];
        //unsigned long length[2];
        stmt = mysql_stmt_init(mysql);

        if (mysql_stmt_prepare(stmt, stmtSql, strlen(stmtSql)))
        {
            fprintf(stderr, " mysql_stmt_prepare(), select failed\n");
            fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        }
        fprintf(stdout, " prepare select sucessfull\n");

        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type= MYSQL_TYPE_LONG;
        bind[0].buffer= (char *)&int_data;
        //bind[0].buffer_length= 4;
        if(mysql_stmt_bind_param(stmt, bind))
        {
            fprintf(stderr, " mysql_stmt_bind_param() failed\n");
            fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        }

        /* Get the parameter count from the statement */
        int param_count= mysql_stmt_param_count(stmt);
        fprintf(stdout, " total parameters in select: %d\n", param_count);

        if (param_count != 1) /* validate parameter count */
        {
            fprintf(stderr, " invalid parameter count returned by MySQL\n");
        }

        memset(bResult, 0, sizeof(bResult));
        bResult[0].buffer_type= MYSQL_TYPE_LONG;
        bResult[0].buffer= (char *)&int_data;

        bResult[1].buffer_type= MYSQL_TYPE_JSON;
        bResult[1].buffer= (char *)str_data;
        bResult[1].buffer_length= STRING_SIZE;

        if(mysql_stmt_bind_result(stmt, bResult))
        {
            fprintf(stderr, " mysql_stmt_bind_result() failed\n");
            fprintf(stderr, " %s\n", mysql_stmt_error(stmt));
        }

        int_data= 1234;
        mysql_stmt_execute(stmt);

        while(!mysql_stmt_fetch(stmt)) {
            printf("4 - %d %s \n", int_data, str_data); 
        }
        
        mysql_stmt_close(stmt);

        //unsigned long type = (unsigned long) CURSOR_TYPE_READ_ONLY;
        /*
        // 5. stmt server cursor default
        stmt = mysql_stmt_init(mysql);

        mysql_stmt_prepare(stmt, stmtSql, strlen(stmtSql));

        mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void*) &type);

        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type= MYSQL_TYPE_LONG;
        bind[0].buffer= (char *)&int_data;
        mysql_stmt_bind_param(stmt, bind);

        memset(bResult, 0, sizeof(bResult));
        bResult[0].buffer_type= MYSQL_TYPE_LONG;
        bResult[0].buffer= (char *)&int_data;
        mysql_stmt_bind_result(stmt, bResult);

        int_data= 1;
        mysql_stmt_execute(stmt);

        while(!mysql_stmt_fetch(stmt)) {
            printf("5 - %d \n", int_data); 
        }

        mysql_stmt_close(stmt);
        */

        // 6. stmt server cursor setting
        /*stmt = mysql_stmt_init(mysql);

        mysql_stmt_prepare(stmt, stmtSql, strlen(stmtSql));

        type = (unsigned long) CURSOR_TYPE_READ_ONLY;
        mysql_stmt_attr_set(stmt, STMT_ATTR_CURSOR_TYPE, (void*) &type);

        unsigned long prefetch_rows = 2;
        mysql_stmt_attr_set(stmt, STMT_ATTR_PREFETCH_ROWS, (void*) &prefetch_rows);

        memset(bind, 0, sizeof(bind));
        bind[0].buffer_type= MYSQL_TYPE_LONG;
        bind[0].buffer= (char *)&int_data;
        mysql_stmt_bind_param(stmt, bind);

        memset(bResult, 0, sizeof(bResult));
        bResult[0].buffer_type= MYSQL_TYPE_LONG;
        bResult[0].buffer= (char *)&int_data;
        mysql_stmt_bind_result(stmt, bResult);

        int_data= 1;
        mysql_stmt_execute(stmt);

        while(!mysql_stmt_fetch(stmt)) {
            printf("6 - %d \n", int_data); 
        }

        mysql_stmt_close(stmt);
        */

        mysql_close(mysql);

    }
    if(mysql)
        delete mysql;
}



int main ( int argc , char **argv ) {
    ip = string(argv[1]);
    port = atoi(argv[2]);
    username = string(argv[3]);
    password = string(argv[4]);
    testdb = string(argv[5]);
    dotest();
    dotest1();
    return 0;
}

