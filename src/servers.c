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
const char *unix_socket, *cmdb_query;

int display_server_info(char *server, char *uuid, cmdb_config_t *config)
{
	if ((strncmp(server, "NULL", CONF_S)))
		display_server_info_on_name(server, config);
	else if ((strncmp(uuid, "NULL", CONF_S)))
		display_server_info_on_uuid(uuid, config);
	mysql_library_end();
	return 0;
}

int display_server_info_on_uuid(char *uuid, cmdb_config_t *config)
{
	char *cquery;
	
	if (!(cquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cquery in display_server_info");
	cmdb_query = cquery;
	printf("UUID: %s\n", uuid);
	sprintf(cquery,
		"SELECT vendor, make, model, name, uuid, name FROM server WHERE uuid = '%s'", uuid);
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
		report_error(SERVER_NOT_FOUND, uuid);
	else if (cmdb_rows > 1)
		report_error(MULTIPLE_SERVERS, uuid);
	cmdb_row = mysql_fetch_row(cmdb_res);
	display_server_from_uuid(cmdb_row);
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	free(cquery);
	return 0;
}

int display_server_info_on_name(char *name, cmdb_config_t *config)
{
	char *cquery;
	
	if (!(cquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cquery in display_server_info");
	cmdb_query = cquery;
	sprintf(cquery,
		"SELECT vendor, make, model, uuid, name FROM server WHERE name = '%s'", name);
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}		
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
		report_error(SERVER_NOT_FOUND, name);
	else if (cmdb_rows > 1)
		report_error(MULTIPLE_SERVERS, name);		
	cmdb_row = mysql_fetch_row(cmdb_res);
	display_server_from_name(cmdb_row);
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	free(cquery);
	return 0;
}

void display_all_servers(cmdb_config_t *config)
{
	MYSQL serv;
	MYSQL_RES *serv_res;
	MYSQL_ROW serv_row;
	char *serv_query, *serv_name;
	const char *serv_cmdb_query;
	
	if (!(serv_query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "serv_query in display_all_servers");
	if (!(serv_name = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "serv_uuid in display_all_servers");
	serv_cmdb_query = serv_query;
	sprintf(serv_query, "SELECT name FROM server");
	cmdb_mysql_init(config, &serv);
	cmdb_mysql_query(&serv, serv_cmdb_query);
	if (!(serv_res = mysql_store_result(&serv)))
		report_error(MY_STORE_FAIL, mysql_error(&serv));
	while ((serv_row = mysql_fetch_row(serv_res))) {
		sprintf(serv_name, "%s", serv_row[0]);
		printf("\n####################\n");
		display_server_info_on_name(serv_name, config);
	}
	mysql_free_result(serv_res);
	mysql_close(&serv);
	mysql_library_end();
	free(serv_query);
	free(serv_name);
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


