/* build_info.c
 * 
 * Functions to retrieve build information from the database and
 * to create the specific build files required.
 * 
 * Part of the cbc program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cmdb_mysql.h"

int get_server_name(cbc_comm_line_t *info, cbc_config_t *config)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	char *query;
	const char *cbc_query;
	
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_server_name");
	
	cbc_query = query;
	if (info->server_id != 0)
		sprintf(query, "SELECT * FROM server WHERE server_id = %ld", info->server_id);
	else if ((strncmp(info->name, "NULL", CONF_S)))
		sprintf(query, "SELECT * FROM server WHERE name = '%s'", info->name);
	else if ((strncmp(info->uuid, "NULL", CONF_S)))
		sprintf(query, "SELECT * FROM server WHERE uuid = '%s'", info->uuid);
	
	printf("Query: %s\n", query);
	
	return 0;
}

int get_build_info(cbc_build_t *build_info, char *server_name)
{
	MYSQL build;
	MYSQL_RES *build_res;
	MYSQL_ROW build_row;
	my_ulonglong build_rows;
	int retval;
	
	retval = NONE;
	
	return retval;
}