/* mysqlfunc.c
 *
 * Contains the functions for mysql access for the dnsa program
 *
 * (C) 2012 Iain M Conochie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"
#include "mysqlfunc.h"

void dnsa_mysql_init(dnsa_config_t *dc, MYSQL *dnsa_mysql)
{
	const char *unix_socket, *error_string;
	char *error_code;
	
	if (!(error_code = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in dnsa_mysql_init");
	
	unix_socket = dc->socket;
	error_string = error_code;
	
	if (!(mysql_init(dnsa_mysql))) {
		sprintf(error_code, "none");
		report_error(MY_INIT_FAIL, error_string);
	}
	if (!(mysql_real_connect(dnsa_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(dnsa_mysql));
	
	free(error_code);
}
