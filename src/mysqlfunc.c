/* mysqlfunc.c
 *
 * Contains the functions for mysql access.
 * 
 * Part of the DNSA  program
 *
 * (C) 2012 Iain M Conochie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "dnsa.h"
#include "mysqlfunc.h"

const char *error_string;
char *my_error_code;

void cmdb_mysql_init(dnsa_config_t *dc, MYSQL *cmdb_mysql)
{
	const char *unix_socket;
	
	if (!(my_error_code = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "my_error_code in cmdb_mysql_init");
	
	unix_socket = dc->socket;
	error_string = my_error_code;
	
	if (!(mysql_init(cmdb_mysql))) {
		sprintf(my_error_code, "none");
		report_error(MY_INIT_FAIL, error_string);
	}
	if (!(mysql_real_connect(cmdb_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(cmdb_mysql));
	
	free(my_error_code);
}

void cmdb_mysql_query(MYSQL *mydnsa, const char *query)
{
	int error;
	
	if (!(my_error_code = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "my_error_code in cmdb_mysql_init");
	
	error_string = my_error_code;
	
	error = mysql_query(mydnsa, query);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_string);
	}
	free(my_error_code);
}
