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
 *  servers.c: Contains non-database functions for manipulating servers
 *  in the cmdb
 *
 *  (C) Iain M Conochie 2013
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
/*
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
	char *input;
	int retval;
	cmdb_server_t *server;
	cmdb_vm_host_t *node;
	cmdb_hardware_t *hard, *hsaved;
	
	retval = 0;
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_server_to_database");
	if (!(server = malloc(sizeof(cmdb_server_t))))
		report_error(MALLOC_FAIL, "server in add_server_to_database");
	
	printf("***CMDB: Add server into the database***\n\n");
	printf("Is this server a virtual machine? (y/n): ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		node = get_vm_host(config);
		server->vm_server_id = node->id;
		free(node);
	}
	server->cust_id = get_customer_for_server(config);
	get_full_server_config(server);
	print_server_details(server);
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		free(input);
		free(server);
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = insert_server_into_mysql(config, server);
#endif *//* HAVE_MYSQL *//*
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = insert_server_into_sqlite(config, server);
#endif *//* HAVE_SQLITE3 */ /*
	} else {
		free(input);
		free(server);
		return DB_TYPE_INVALID;
	} */
/*	retval = insert_server_into_db(config, server); */
/*	if (retval > 0) {
		free(input);
		free(server);
		return retval;
	}
	hard = hard_node_create();
	retval = get_server_hardware(config, hard, server->server_id);
	if (retval != 0) {
		printf("Hardware input failed\n");
		free(input);
		free(server);
		while (hard->next) {
			hsaved = hard->next;
			free(hard);
			hard = hsaved;
		}
		free(hard);
		return retval;
	}
	printf("\n");
	print_hardware_details(hard);
	retval = add_hardware_to_db(config, hard);
	while (hard->next) {
		hsaved = hard->next;
		free(hard);
		hard = hsaved;
	}
	free(hard);
	free(server);
	free(input);
	return retval;
}
*/
/*
int add_hardware_to_db(cmdb_config_t *config, cmdb_hardware_t *hw)
{
	cmdb_hardware_t *next;
	int retval;

	next = hw;
	retval = 0;
	while (next) {
		if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
			retval = NO_DB_TYPE;
			break;
#ifdef HAVE_MYSQL
		} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
			retval = insert_hardware_into_mysql(config, next);
#endif */ /* HAVE_MYSQL */ /*
#ifdef HAVE_SQLITE3
		} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
			retval = insert_hardware_into_sqlite(config, next);
#endif */ /* HAVE_SQLITE3 */ /*
		} else {
			retval = DB_TYPE_INVALID;
			break;
		}
		if (retval > 0) {
			next = 0;
		} else {
			next = next->next;
		}
	}

	return retval;
}
*/
void
print_hardware_details(cmdb_hardware_t *hard)
{
	printf("Network Card for server ID: %lu\n", hard->server_id);
	printf("Device: %s\tMac Address: %s\n", hard->device, hard->detail);
	hard = hard->next;
	printf("Hard Disk for server ID: %lu\n", hard->server_id);
	printf("Device: /dev/%s\tSize: %s\n", hard->device, hard->detail);
}

void
get_full_server_config(cmdb_server_t *server)
{
	char *input;
	int retval;
	
	if (!(input = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_full_server_config");
	
	printf("Please input the vendor of the server:\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the vendor of the server:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->vendor, CONF_S, "%s", input);
	
	printf("Please input the make of the server:\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, MAKE_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the make of the server:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->make, CONF_S, "%s", input);
	
	printf("Please input the model of the server:\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the model of the server:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->model, CONF_S, "%s", input);
	
	printf("Please input the uuid of the server:\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, UUID_REGEX) < 0) && 
		(retval = validate_user_input(input, COID_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the uuid of the server:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->uuid, CONF_S, "%s", input);
	
	printf("Please input the name of the server:\n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	while ((retval = validate_user_input(input, NAME_REGEX) < 0)) {
		printf("User input not valid!\n");
		printf("Please input the model of the server:\n");
		input = fgets(input, CONF_S, stdin);
		chomp(input);
	}
	snprintf(server->name, MAC_S, "%s", input);
	free(input);
}
/*
cmdb_vm_host_t *
vm_host_create(void)
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

int get_server_hardware(cmdb_config_t *config, cmdb_hardware_t *head, unsigned long int id)
{
	int retval;
	cmdb_hard_type_t *hthead, *net, *disk;
	cmdb_hardware_t *new;
	
	retval = 0;
	hthead = hard_type_node_create();
	add_hardware_types(config, hthead);
	printf("\nNow we need to ask you for some hardware\n");
	retval = get_network_device(head);
	if (retval > 0) {
		fprintf(stderr, "Unable to get network device\n");
		retval = 1;
		return retval;
	}
	retval = get_disk_device(head);
	if (retval > 0) {
		fprintf(stderr, "Unable to get disk device\n");
		retval = 1;
		return retval;
	}
	net = get_network_device_id(hthead);
	if (!net) {
		printf("Unable to find network card hardware type\n");
		while (net) {
			net = net->next;
			free(hthead);
			hthead = net;
		}
		retval = 1;
		return retval;
	} else {
		head->ht_id = net->ht_id;
		head->server_id = id;
	}
	disk = get_disk_device_id(hthead);
	if (!disk) {
		printf("Unable to find hard disk hardware type\n");
		while (disk) {
			disk = disk->next;
			free(hthead);
			hthead = disk;
		}
		retval = 1;
		return retval;
	} else {
		new = head->next;
		new->ht_id = disk->ht_id;
		new->server_id = id;
	}
	while (hthead->next) {
		net = hthead->next;
		free(hthead);
		hthead = net;
	}
	free(hthead);
	
	return retval;
}
*/
cmdb_hardware_t * 
hard_node_create(void)
{
	cmdb_hardware_t *hard;
	
	if (!(hard = malloc(sizeof(cmdb_hardware_t))))
		report_error(MALLOC_FAIL, "hard in *hard_node_create");
	snprintf(hard->detail, MAC_S, "NULL");
	snprintf(hard->device, MAC_S, "NULL");
	hard->hard_id = 0;
	hard->ht_id = 0;
	hard->server_id = 0;
	hard->next = NULL;
	return hard;
}

cmdb_hard_type_t * 
hard_type_node_create(void)
{
	cmdb_hard_type_t *hardtype;
	
	if (!(hardtype = malloc(sizeof(cmdb_hard_type_t))))
		report_error(MALLOC_FAIL, "hardtype in hard_type_node_create");
	snprintf(hardtype->type, HOST_S, "NULL");
	snprintf(hardtype->hclass, HOST_S, "NULL");
	hardtype->ht_id = 0;
	hardtype->next = NULL;
	return hardtype;
}

cmdb_hard_type_t * 
get_network_device_id(cmdb_hard_type_t *head)
{
	cmdb_hard_type_t *net;
	
	net = head;
	while (net) {
		if (((strncmp(net->type, "network", HOST_S)) == 0) &&
		 (strncmp(net->hclass, "Network Card", HOST_S)) == 0) {
			return net;
		} else {
			net = net->next;
		}
	}
	net = NULL;
	return net;
}

cmdb_hard_type_t *
get_disk_device_id(cmdb_hard_type_t *head)
{
	cmdb_hard_type_t *disk;
	
	disk = head;
	while (disk) {
		if (((strncmp(disk->type, "storage", HOST_S)) == 0) &&
		 (strncmp(disk->hclass, "Hard Disk", HOST_S)) == 0) {
			return disk;
		} else {
			disk = disk->next;
		}
	}
	disk = NULL;
	return disk;
}

int
get_network_device(cmdb_hardware_t *head)
{
	char *input;
	int retval;
	
	if (!(input = calloc(HOST_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_network_device");
	
	retval = 0;
	printf("Please enter the network device (without the leading /dev/\n");
	input = fgets(head->device, MAC_S, stdin);
	chomp(head->device);
	while ((retval = validate_user_input(head->device, DEV_REGEX) < 0)) {
		printf("Network device %s not valid!\n", head->device);
		printf("Please enter the network device (without the leading /dev/\n");
		input = fgets(head->device, MAC_S, stdin);
		chomp(head->device);
	}
	printf("Please enter the network device MAC Address\n");
	input = fgets(head->detail, HOST_S, stdin);
	chomp(head->detail);
	while ((retval = validate_user_input(head->detail, MAC_REGEX) < 0)) {
		printf("Network device %s not valid!\n", head->device);
		printf("Please enter the network device MAC address\n");
		input = fgets(head->detail, HOST_S, stdin);
		chomp(head->detail);
	}
	free(input);
	return 0;
}

int
get_disk_device(cmdb_hardware_t *head)
{
	char *input;
	int retval;
	cmdb_hardware_t *disk;
	
	if (!(input = calloc(HOST_S + 1, sizeof(char))))
		report_error(MALLOC_FAIL, "input in get_disk_device");
	
	retval = 0;
	disk = hard_node_create();
	head->next = disk;
	printf("Please enter the disk device (without the leading /dev/\n");
	input = fgets(disk->device, MAC_S, stdin);
	chomp(disk->device);
	while ((retval = validate_user_input(disk->device, DEV_REGEX) < 0)) {
		printf("Disk device not valid!\n");
		printf("Please enter the disk device (without the leading /dev/\n");
		input = fgets(disk->device, MAC_S, stdin);
		chomp(disk->device);
	}
	printf("Please enter the disk device capacity (# [TB | GB | MB])\n");
	input = fgets(disk->detail, HOST_S, stdin);
	chomp(disk->detail);
	while ((retval = validate_user_input(disk->detail, CAPACITY_REGEX) < 0)) {
		printf("Disk Capacity not valid!\n");
		printf("Please enter the disk device capacity\n");
		printf("A number followed by one space followed by either TB, GB or MB\n");
		input = fgets(disk->detail, HOST_S, stdin);
		chomp(disk->detail);
	}
	return 0;
}
/*
int display_server_info(char *server, char *uuid, cmdb_config_t *config)
{
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		printf("No dbtype configured to display server info\n");
		return DB_TYPE_INVALID;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		if ((strncmp(server, "NULL", CONF_S)))
			display_on_name_mysql(server, config);
		else if ((strncmp(uuid, "NULL", CONF_S)))
			display_on_uuid_mysql(uuid, config);
#endif  *//* HAVE_MYSQL */ /*
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		if ((strncmp(server, "NULL", CONF_S)))
			display_on_name_sqlite(server, config);
		else if ((strncmp(uuid, "NULL", CONF_S)))
			display_on_uuid_sqlite(uuid, config);
#endif *//* HAVE_SQLITE3 */ /*
	} else {
		printf("Unknown DB type %s to display servers\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	return 0;
}
*/
void
display_server_info(char *name, char *uuid, cmdb_config_t *config)
{
	int retval, i;
	cmdb_server_t *server;
	cmdb_t *cmdb;
	
	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_list in display_server_info");

	cmdb_init_struct(cmdb);
	i = 0;
	if ((retval = run_multiple_query(config, cmdb, SERVER | CUSTOMER | VM_HOST)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	server = cmdb->server;
	while(server) {
		if ((strncmp(server->name, name, MAC_S) == 0)) {
			print_server_details(server, cmdb);
			server = server->next;
			i++;
		} else if ((strncmp(server->uuid, uuid, CONF_S) == 0)) {
			print_server_details(server, cmdb);
			server = server->next;
			i++;
		} else {
			server = server->next;
		}
	}
	cmdb_clean_list(cmdb);
	if (i == 0)
		printf("No Server found\n");
	return;
}

void
display_all_servers(cmdb_config_t *config)
{
	int retval;
	cmdb_t *cmdb;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_list in display_server_info");

	cmdb_init_struct(cmdb);
	if ((retval = run_multiple_query(config, cmdb, SERVER | CUSTOMER)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	print_all_servers(cmdb);

	cmdb_clean_list(cmdb);
	return;
}

void 
print_server_details(cmdb_server_t *server, cmdb_t *base)
{
	cmdb_customer_t *customer = base->customer;
	cmdb_vm_host_t *vmhost = base->vmhost;
	printf("Server Details:\n");
	printf("Name:\t\t%s\n", server->name);
	printf("UUID:\t\t%s\n", server->uuid);
	printf("Vendor:\t\t%s\n", server->vendor);
	printf("Make:\t\t%s\n", server->make);
	printf("Model:\t\t%s\n", server->model);
	while (server->cust_id != customer->cust_id)
		customer = customer->next;
	printf("Customer name:\t%s\n", customer->name);
	printf("COID:\t\t%s\n", customer->coid);
	if (server->vm_server_id > 0) {
		while (server->vm_server_id != vmhost->id) {
			vmhost = vmhost->next;
		}
		printf("VM Server name:\t%s\n", vmhost->name);
	}
}

void
print_all_servers(cmdb_t *cmdb)
{
	unsigned long int id;
	cmdb_server_t *server = cmdb->server;
	cmdb_customer_t *cust = cmdb->customer;
	size_t len;

	printf("Server\t\t\t\tCustomer\n");
	while (server) {
		id = server->cust_id;
		len = strlen(server->name);
		while (id != cust->cust_id && (cust)) {
			cust = cust->next;
		}
		if (len > 23) {
			printf("%s\t%s\n", server->name, cust->coid);
		} else if (len > 15) {
			printf("%s\t\t%s\n", server->name, cust->coid);
		} else if (len > 7) {
			printf("%s\t\t\t%s\n", server->name, cust->coid);
		} else {
			printf("%s\t\t\t\t%s\n", server->name, cust->coid);
		}
		server = server->next;
		cust = cmdb->customer;
	}
		
}
