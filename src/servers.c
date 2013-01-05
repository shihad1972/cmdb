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
#include "checks.h"

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

int add_server_to_database(cmdb_config_t *config)
{
	char *query, *input;
	int retval;
	cmdb_server_t *server;
	cmdb_vm_host_t *node;
	
	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_server_to_database");
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_server_to_database");
	if (!(server = malloc(sizeof(cmdb_server_t))))
		report_error(MALLOC_FAIL, "server in add_server_to_database");
	
	printf("***CMDB: Add server into the database***\n\n");
	printf("Is this server a virtual machine? (y/n): ");
	fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		node = get_vm_host(config);
		server->vm_server_id = node->id;
	}
	free(node);
	server->cust_id = get_customer_for_server(config);
	get_full_server_config(server);
	print_server_details(server);
	free(server);
	free(input);
	free(query);
	return retval;
}

void print_server_details(cmdb_server_t *server)
{
	printf("Server Details:\n");
	printf("Name:\t%s\n", server->name);
	printf("UUID:\t%s\n", server->uuid);
	printf("Vendor:\t%s\n", server->vendor);
	printf("Make:\t%s\n", server->make);
	printf("Model:\t%s\n", server->model);
	printf("Cust id:\t%lu\n", server->cust_id);
	printf("VM Server ID:\t%lu\n", server->vm_server_id);
}

void get_full_server_config(cmdb_server_t *server)
{
	char *input;
	int retval;
	
	if (!(input = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_full_server_config");
	
	printf("Please input the vendor of the server:\n");
	fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the vendor of the server:\n");
		fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->vendor, CONF_S, "%s", input);
	
	printf("Please input the make of the server:\n");
	fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, MAKE_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the make of the server:\n");
		fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->make, CONF_S, "%s", input);
	
	printf("Please input the model of the server:\n");
	fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the model of the server:\n");
		fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->model, CONF_S, "%s", input);
	
	printf("Please input the uuid of the server:\n");
	fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, UUID_REGEX) < 0) && 
		(retval = validate_user_input(input, COID_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the uuid of the server:\n");
		fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->uuid, CONF_S, "%s", input);
	
	printf("Please input the name of the server:\n");
	fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the model of the server:\n");
		fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->name, CONF_S, "%s", input);
}
cmdb_vm_host_t *vm_host_create(void)
{
	cmdb_vm_host_t *host;
	
	if (!(host = malloc(sizeof(cmdb_vm_host_t))))
		report_error(MALLOC_FAIL, "host in vm_host_create");
	host->next = NULL;
	host->id = 0;
	snprintf(host->name, MAC_S, "NULL");
	snprintf(host->type, MAC_S, "NULL");
	return host;
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
	fgets(input, MAC_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, ID_REGEX)) < 0) {
		printf("Input %s not valid. Please input the ID of the server you wish to use\n", input);
		fgets(input, MAC_S, stdin);
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
