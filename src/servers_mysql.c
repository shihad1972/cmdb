/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  servers_mysql.c
 * 
 *  Functions relating to servers in the MySQL database for the cmdb program
 * 
 *  (C) Iain M Conochie 2012 - 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "mysqlfunc.h"
#include "cmdb_mysql.h"
#include "checks.h"

MYSQL cmdb;
MYSQL_RES *cmdb_res;
MYSQL_ROW cmdb_row;
my_ulonglong cmdb_rows;
const char *unix_socket, *cmdb_query;

int display_on_uuid_mysql(char *uuid, cmdb_config_t *config)
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

int display_on_name_mysql(char *name, cmdb_config_t *config)
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

void display_all_mysql_servers(cmdb_config_t *config)
{
	MYSQL serv;
	MYSQL_RES *serv_res;
	MYSQL_ROW serv_row;
	char *serv_query, *serv_name;
	const char *serv_cmdb_query;
	
	if (!(serv_query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "serv_query in display_all_mysql_servers");
	if (!(serv_name = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "serv_uuid in display_all_mysql_servers");
	serv_cmdb_query = serv_query;
	sprintf(serv_query, "SELECT name FROM server");
	cmdb_mysql_init(config, &serv);
	cmdb_mysql_query(&serv, serv_cmdb_query);
	if (!(serv_res = mysql_store_result(&serv)))
		report_error(MY_STORE_FAIL, mysql_error(&serv));
	while ((serv_row = mysql_fetch_row(serv_res))) {
		sprintf(serv_name, "%s", serv_row[0]);
		printf("\n####################\n");
		display_on_name_mysql(serv_name, config);
	}
	mysql_free_result(serv_res);
	mysql_close(&serv);
	mysql_library_end();
	free(serv_query);
	free(serv_name);
}

int insert_server_into_mysql(cmdb_config_t *config, cmdb_server_t *server)
{
	char *query;
	int retval;

	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_server_into_mysql");
	retval = 0;

	snprintf(query, RBUFF_S,
"SELECT server_id FROM server WHERE name = '%s'", server->name);
	cmdb_query = query;
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		free(query);
		mysql_close(&cmdb);
		mysql_library_end();
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	cmdb_rows = mysql_num_rows(cmdb_res);
	if (cmdb_rows > 0)
		retval = SERVER_EXISTS;
	if (retval > 0) {
		free(query);
		mysql_close(&cmdb);
		mysql_library_end();
		return retval;
	}
	snprintf(query, RBUFF_S,
"INSERT INTO server (vendor, make, model, uuid, cust_id, vm_server_id, name) \
VALUES ('%s', '%s', '%s', '%s', %lu, %lu, '%s')",
		server->vendor, server->make, server->model, server->uuid,
		server->cust_id, server->vm_server_id, server->name);
	cmdb_mysql_query(&cmdb, cmdb_query);
	cmdb_rows = mysql_affected_rows(&cmdb);

	if (cmdb_rows != 1)
		retval = MY_INSERT_FAIL;
	if (retval > 0) {
		free(query);
		mysql_close(&cmdb);
		mysql_library_end();
		return retval;
	}
	snprintf(query, RBUFF_S,
"SELECT server_id FROM server WHERE name = '%s'", server->name);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		free(query);
		mysql_close(&cmdb);
		mysql_library_end();
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	cmdb_row = mysql_fetch_row(cmdb_res);
	server->server_id = strtoul(cmdb_row[0], NULL, 10);
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	mysql_library_end();
	free(query);
	return retval;
}

int insert_hardware_into_mysql(cmdb_config_t *config, cmdb_hardware_t *hw)
{
	int retval;
	char *query;

	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_hardware_into_db");
	retval = 0;

	snprintf(query, RBUFF_S,
"INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES \
('%s', '%s', %lu, %lu)", hw->detail, hw->device, hw->server_id, hw->ht_id);
	cmdb_query = query;
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	cmdb_rows = mysql_affected_rows(&cmdb);

	if (cmdb_rows != 1)
		retval = MY_INSERT_FAIL;
	mysql_close(&cmdb);
	mysql_library_end();
	free(query);
	return retval;
}

cmdb_vm_host_t *get_vm_host(cmdb_config_t *config)
{
	char *query, *input;
	int retval;
	unsigned long int id;
	size_t len;
	cmdb_vm_host_t *head, *node, *saved;
	
	head = vm_host_create();
	if (!(input = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_server_to_database");
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_server_to_database");
	snprintf(query, RBUFF_S,
"SELECT vm_server_id, vm_server, type FROM vm_server_hosts");
	cmdb_query = query;
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		free(query);
		mysql_close(&cmdb);
		mysql_library_end();
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0)) {
		mysql_free_result(cmdb_res);
		mysql_close(&cmdb);
		mysql_library_end();
		free(query);
		report_error(NO_VM_HOSTS, query);
	}
	while ((cmdb_row = mysql_fetch_row(cmdb_res))) {
		fill_vm_host_node(head, cmdb_row);
	}
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	mysql_library_end();
	node = head;
	printf("Please input the ID of the VM server host you wish to use\n");
	printf("ID\tName\t\t\tType\n");
	do {
		len = strlen(node->name);
		if ((len / 8) > 1)
			printf("%lu\t%s\t%s\n", node->id, node->name, node->type);
		else if ((len / 8) > 0)
			printf("%lu\t%s\t\t%s\n", node->id, node->name, node->type);
		else
			printf("%lu\t%s\t\t\t%s\n", node->id, node->name, node->type);
		node = node->next;
	} while (node);
	input = fgets(input, MAC_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, ID_REGEX)) < 0) {
		printf("Input %s not valid. Please input the ID of the server you wish to use\n", input);
		input = fgets(input, MAC_S, stdin);
		chomp(input);
	}
	id = strtoul(input, NULL, 10);
	node = head;
	do {
		saved = node->next;
		if (id == node->id) {
			head = node;
			node = saved;
		} else {
			free(node);
			node=saved;
		}
	} while (node);
	free(input);
	free(query);
	return head;
}

void add_hardware_types(cmdb_config_t *config, cmdb_hard_type_t *hthead)
{
	char *query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_hardware_types");
	
	snprintf(query, RBUFF_S,
"SELECT hard_type_id, type, class FROM hard_type");
	cmdb_query = query;
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		free(query);
		mysql_close(&cmdb);
		mysql_library_end();
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0)) {
		mysql_free_result(cmdb_res);
		mysql_close(&cmdb);
		mysql_library_end();
		free(query);
		report_error(NO_HARDWARE_TYPES, query);
	}
	while ((cmdb_row = mysql_fetch_row(cmdb_res)))
		fill_hardware_types(hthead, cmdb_row);
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	mysql_library_end();
	free(query);
}

void fill_hardware_types(cmdb_hard_type_t *head, MYSQL_ROW row)
{
	cmdb_hard_type_t *node, *saved;
	
	if ((strncmp(head->type, "NULL", HOST_S)) == 0)
		node = head;
	else
		node = hard_type_node_create();
	
	node->ht_id = strtoul(row[0], NULL, 10);
	snprintf(node->type, HOST_S, "%s", row[1]);
	snprintf(node->hclass, HOST_S, "%s", row[2]);
	saved = head;
	if (node != head) {
		while (saved->next)
			saved = saved->next;
		saved->next = node;
	}
}

void fill_vm_host_node(cmdb_vm_host_t *head, MYSQL_ROW myrow)
{
	cmdb_vm_host_t *new, *saved;
	
	if ((strncmp(head->name, "NULL", CONF_S) == 0)) {
		new = head;
	} else {
		new = vm_host_create();
	}
	new->id = strtoul(myrow[0], NULL, 10);
	snprintf(new->name, CONF_S, "%s", myrow[1]);
	snprintf(new->type, CONF_S, "%s", myrow[2]);
	saved = head;
	if (new != head) {
		while (saved->next)
			saved = saved->next;
		saved->next = new;
	}
}