/* cmdb_mysql.c
 *
 * Contains the functions for mysql access.
 * 
 * Part of the cmdb program
 *
 * (C) 2012 Iain M Conochie
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "mysqlfunc.h"

void cbc_mysql_init(cbc_config_t *dc, MYSQL *cbc_mysql)
{
	const char *unix_socket;
	
	unix_socket = dc->socket;
	
	if (!(mysql_init(cbc_mysql))) {
		report_error(MY_INIT_FAIL, mysql_error(cbc_mysql));
	}
	if (!(mysql_real_connect(cbc_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(cbc_mysql));
}