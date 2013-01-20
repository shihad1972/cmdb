/* base_mysql.c
 *
 *
 * Base mysql functions for dnsa, cbc and cmdb
 *
 * (C) 2013 Iain M Conochie
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "mysqlfunc.h"


void cmdb_mysql_query(MYSQL *mycmdb, const char *query)
{
	int error;
	
	error = mysql_query(mycmdb, query);
	if ((error != 0))
		report_error(MY_QUERY_FAIL, mysql_error(mycmdb));
}

int cmdb_mysql_query_with_checks(MYSQL *mycmdb, const char *query)
{
	int error;
	
	error = mysql_query(mycmdb, query);
	if ((error != 0))
		return error;
	else
		return 0;
}

void cmdb_mysql_clean(MYSQL *cmdb_mysql, char *query)
{
	mysql_close(cmdb_mysql);
	mysql_library_end();
	free(query);
}

void cmdb_mysql_clean_full(MYSQL_RES *cmdb_res, MYSQL *cmdb_mysql, char *query)
{
	mysql_free_result(cmdb_res);
	mysql_close(cmdb_mysql);
	mysql_library_end();
	free(query);
}
