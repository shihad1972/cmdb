/* servers.c
 * 
 * Functions relating to servers in the database for the cmdb program
 * 
 * (C) 2012 Iain M Conochie
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "mysqlfunc.h"
#include "cmdb_mysql.h"

MYSQL cmdb;
MYSQL_RES *cmdb_res;
MYSQL_ROW cmdb_row;
my_ulonglong cmdb_rows;
char *error_code;
const char *unix_socket, *cmdb_query, *error_str;

int display_server_info(char *server, char *uuid, cmdb_config_t *config)
{
	char *cquery;
	
	if (!(cquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cquery in display_server_info");
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in display_server_info");
	error_str = error_code;
	cmdb_query = cquery;
	if ((strncmp(server, "NULL", CONF_S))) {
		sprintf(cquery, "SELECT vendor, make, model, uuid, name FROM server WHERE name = '%s'",
			server);
		cmdb_mysql_init(config, &cmdb);
		cmdb_mysql_query(&cmdb, cmdb_query);
		if (!(cmdb_res = mysql_store_result(&cmdb))) {
			snprintf(error_code, CONF_S, "%s", mysql_error(&cmdb));
			report_error(MY_STORE_FAIL, error_str);
		}
		
		if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
			report_error(SERVER_NOT_FOUND, server);
		else if (cmdb_rows > 1)
			report_error(MULTIPLE_SERVERS, server);
		
		cmdb_row = mysql_fetch_row(cmdb_res);
		display_server_from_name(cmdb_row);
	} else if ((strncmp(uuid, "NULL", CONF_S))) {
		printf("UUID: %s\n", uuid);
		sprintf(cquery, "SELECT vendor, make, model, name, uuid FROM server WHERE uuid = '%s'",
			uuid);
		cmdb_mysql_init(config, &cmdb);
		cmdb_mysql_query(&cmdb, cmdb_query);
		if (!(cmdb_res = mysql_store_result(&cmdb))) {
			snprintf(error_code, CONF_S, "%s", mysql_error(&cmdb));
			report_error(MY_STORE_FAIL, error_str);
		}
		
		if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
			report_error(SERVER_NOT_FOUND, server);
		else if (cmdb_rows > 1)
			report_error(MULTIPLE_SERVERS, server);
		
		cmdb_row = mysql_fetch_row(cmdb_res);
		display_server_from_uuid(cmdb_row);
	}
	mysql_close(&cmdb);
	free(cquery);
	free(error_code);
	return 0;
}

void display_all_servers(cmdb_config_t *config)
{
}

void display_server_from_name(char **server_info)
{
	printf("Server %s info\n", server_info[4]);
	printf("Vendor: %s\n", server_info[0]);
	printf("Make: %s\n", server_info[1]);
	printf("Model: %s\n", server_info[2]);
	printf("UUID: %s\n", server_info[3]);
}

void display_server_from_uuid(char **server_info)
{
	printf("Server %s info\n", server_info[3]);
	printf("Vendor: %s\n", server_info[0]);
	printf("Make: %s\n", server_info[1]);
	printf("Model: %s\n", server_info[2]);
	printf("UUID: %s\n", server_info[4]);
}


