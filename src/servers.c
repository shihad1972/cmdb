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

int add_server_to_database(cmdb_config_t *config, cmdb_comm_line_t *cm, cmdb_t *cmdb)
{
	char *input;
	int retval;
	cmdb_vm_host_t *vmhost;
	
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_server_to_database");
	if (!(vmhost = malloc(sizeof(cmdb_vm_host_t))))
		report_error(MALLOC_FAIL, "vmhost in add_server_to_database");

	cmdb_init_vmhost_t(vmhost);
	cmdb->vmhost = vmhost;
	retval = 0;
	printf("Details provided:\n");
	printf("Name: %s\n", cmdb->server->name);
	printf("Vendor: %s\n", cmdb->server->vendor);
	printf("Make: %s\n", cmdb->server->make);
	printf("Model: %s\n", cmdb->server->model);
	printf("UUID: %s\n", cmdb->server->uuid);
	printf("COID: %s\n", cmdb->customer->coid);
	if ((retval = run_search(config, cmdb, CUST_ID_ON_COID)) != 0) {
		printf("Unable to retrieve cust_id for COID %s\n",
		 cmdb->customer->coid);
		free(input);
		return retval;
	}
/*	printf("We have a cust_id of %lu\n", cmdb->server->cust_id); */
	if ((strncmp(cm->vmhost, "NULL", COMM_S)) != 0) {
		printf("VM host: %s\n", cm->vmhost);
		snprintf(vmhost->name, RBUFF_S, "%s", cm->vmhost);
		if ((retval = run_search(config, cmdb, VM_ID_ON_NAME)) != 0) {
			printf("Unable to retrieve vmhost %s id\n",
			 cm->vmhost);
			free(input);
			return retval;
		}
/*		printf("We have a vm_host_id of %lu\n",
		 cmdb->vmhost->id); */
	} else {
		printf("No vmhost supplied. This should be a stand alone server\n");
	}
	cmdb->server->cust_id = cmdb->customer->cust_id;
	cmdb->server->vm_server_id = cmdb->vmhost->id;	
	printf("Are these detail correct? (y/n): ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		retval = run_insert(config, cmdb, SERVERS);
	} else {
		retval = 1;
	}
	free(input);
	return retval;
/*	printf("***CMDB: Add server into the database***\n\n");
	printf("Is this server a virtual machine? (y/n): ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		if ((retval = check_for_vm_host(config, cmdb, cm->vmhost)) != 0) {
			free (input);
			cmdb_clean_list(cmdb);
			return NO_VM_HOSTS;
		}
	}
	if ((retval = check_for_coid(config, cmdb, cm->id)) != 0) {
			free(input);
			cmdb_clean_list(cmdb);
			return NO_CUSTOMERS;
	}
	cmdb->server->vm_server_id = cmdb->vmhost->id;
	cmdb->server->cust_id = cmdb->customer->cust_id;
	get_full_server_config(cmdb->server);
	print_server_details(cmdb->server, cmdb);
	printf("Do you wish to continue? (y/n): \n");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0))
		run_insert(config, cmdb, SERVER);
	else
		printf("Not what you want eh?\n"); */
	free(input);
	return 0;
}
/*

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

int
check_for_vm_host(cmdb_config_t *config, cmdb_t *cmdb, char *vmhost)
{
	int retval;
	if ((strncmp(vmhost, "NULL", MAC_S)) == 0) {
		printf("\
Please supply the name of the vmhost with the -m option\n");
		return 1;
	}
	if ((retval = get_vm_host(config, cmdb, vmhost)) != 0)
		return retval;
	return 0;
}

int
check_for_coid(cmdb_config_t *config, cmdb_t *cmdb, char *coid)
{
	int retval;
	if ((strncmp(coid, "NULL", MAC_S)) == 0) {
		printf("\
Please supply the coid of the customer with the -i option\n");
		return 1;
	}
	if ((retval = get_customer(config, cmdb, coid)) != 0)
		return retval;
	return 0;
}

int
get_vm_host(cmdb_config_t *config, cmdb_t *cmdb, char *vmhost)
{
	int retval;
	cmdb_vm_host_t *vm, *next;

	if ((retval = run_query(config, cmdb, VM_HOST)) != 0)
		return retval;

	vm = cmdb->vmhost;
	next = vm->next;
	while (vm) {
		if (strncmp(vmhost, vm->name, NAME_S) != 0) {
			free(vm);
			vm = next;
			next = vm->next;
		} else {
			break;
		}
	}
	if (vm) {
		cmdb->vmhost = vm;
		vm = vm->next;
		while (vm) {
			next = vm->next;
			free(vm);
			vm = next;
		}
	} else {
		cmdb->vmhost = '\0';
		return NO_VM_HOSTS;
	}
	cmdb->vmhost->next = '\0';
	return 0;
}

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
	if ((retval = run_multiple_query(config,
cmdb, SERVER | CUSTOMER | HARDWARE |  SERVICE | VM_HOST)) != 0) {
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
display_hardware_types(cmdb_config_t *config)
{
	int retval;
	cmdb_t *cmdb;
	cmdb_hard_type_t *list;
	size_t len;

	retval = 0;
	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb in display_server_info");

	cmdb_init_struct(cmdb);
	cmdb->hardtype = '\0';
	if ((retval = run_query(config, cmdb, HARDWARE_TYPE)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	list = cmdb->hardtype;
	printf("ID\tType\t\t\tClass\n");
	while (list) {
		if ((len = strlen(list->type)) < 8) {
			printf("%lu\t%s\t\t\t%s\n",
			 list->ht_id, list->type, list->hclass);
		} else if ((len = strlen(list->type)) < 16) {
			printf("%lu\t%s\t\t%s\n",
			 list->ht_id, list->type, list->hclass);
		} else {
			printf("%lu\t%s\t%s\n",
			 list->ht_id, list->type, list->hclass);
		}
		list = list->next;
	}

	cmdb_clean_list(cmdb);
	return;
}

void
display_server_hardware(cmdb_config_t *config, char *name)
{
	int retval;
	cmdb_t *cmdb;
	cmdb_server_t *server;
	cmdb_hardware_t *hardware;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb in display server hardware");

	cmdb_init_struct(cmdb);
	if ((retval = run_multiple_query(config, cmdb, SERVER | HARDWARE)) != 0) {
		cmdb_clean_list(cmdb);
		printf("Query for server %s hardware failed\n", name);
		return;
	}
	server = cmdb->server;
	hardware = cmdb->hardware;
	printf("Server %s\n", name);
	while (server) {
		if ((strncmp(server->name, name, MAC_S) == 0)) {
			retval = print_hardware(hardware, server->server_id);
			server = server->next;
		} else {
			server = server->next;
		}
	}
	if (retval == 0)
		printf("No hardware\n");
	cmdb_clean_list(cmdb);
}

void
display_server_services(cmdb_config_t *config, char *name)
{
	int retval;
	cmdb_t *cmdb;
	cmdb_server_t *server;
	cmdb_service_t *service;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb in display server services");

	cmdb_init_struct(cmdb);
	if ((retval = run_multiple_query(config, cmdb, SERVER | SERVICE)) != 0) {
		cmdb_clean_list(cmdb);
		printf("Query for server %s services failed\n", name);
		return;
	}
	server = cmdb->server;
	service = cmdb->service;
	printf("Server %s\n", name);
	while (server) {
		if ((strncmp(server->name, name, MAC_S) == 0)) {
			retval = print_services(service, server->server_id, SERVER);
			server = server->next;
		} else {
			server = server->next;
		}
	}
	if (retval == 0)
		printf("No services\n");
	cmdb_clean_list(cmdb);
}

void 
print_server_details(cmdb_server_t *server, cmdb_t *base)
{
	cmdb_customer_t *customer = base->customer;
	cmdb_vm_host_t *vmhost = base->vmhost;
	cmdb_hardware_t *hard = base->hardware;
	cmdb_service_t *service = base->service;

	printf("Server Details:\n");
	printf("Name:\t\t%s\n", server->name);
	printf("UUID:\t\t%s\n", server->uuid);
	printf("Vendor:\t\t%s\n", server->vendor);
	printf("Make:\t\t%s\n", server->make);
	printf("Model:\t\t%s\n", server->model);
	while (server->cust_id != customer->cust_id)
		customer = customer->next;
	printf("Customer:\t%s\n", customer->name);
	printf("COID:\t\t%s\n", customer->coid);
	if (server->vm_server_id > 0) {
		while (server->vm_server_id != vmhost->id) {
			vmhost = vmhost->next;
		}
		printf("VM Server:\t%s\n", vmhost->name);
	} else {
		while ((server->server_id != vmhost->server_id) && (vmhost)) {
			vmhost = vmhost->next;
		}
		if (server->server_id == vmhost->server_id) {
			printf("VM Server host type %s\n", vmhost->type);
		}
	}
	print_hardware(hard, server->server_id);
	print_services(service, server->server_id, SERVER);
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

void
display_vm_hosts(cmdb_config_t *config)
{
	int retval;
	cmdb_t *cmdb;
	cmdb_vm_host_t *list;

	retval = 0;
	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_list in display_server_info");

	cmdb_init_struct(cmdb);
	cmdb->vmhost = '\0';
	if ((retval = run_query(config, cmdb, VM_HOST)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	list = cmdb->vmhost;
	print_vm_hosts(list);
	cmdb_clean_list(cmdb);
}

void
print_vm_hosts(cmdb_vm_host_t *vmhost)
{
	if (vmhost) {
		printf("Virtual Machine Hosts\n");
		printf("ID\tType\tName\n");
		while (vmhost) {
			printf("%lu\t%s\t%s\n",
			 vmhost->id, vmhost->type, vmhost->name);
			vmhost = vmhost->next;
		}
	} else {
		printf("No virtual machine hosts\n");
	}
}

int
print_hardware(cmdb_hardware_t *hard, unsigned long int id)
{
	int i = 0;

	while (hard) {
		if (hard->server_id == id) {
			i++;
			if (i == 1)
				printf("\nHardware details:\n");
			printf("%s\t%s\t%s\n",
hard->hardtype->hclass, hard->device, hard->detail);
		}
		hard = hard->next;
	}
	return i;
}

int
print_services(cmdb_service_t *service, unsigned long int id, int type)
{
	int i = 0;

	if (type == SERVER) {
		while (service) {
			if (service->server_id == id) {
				i++;
				if (i == 1)
					printf("\nService Details:\n");
				if ((strlen(service->servicetype->service)) < 7)
					printf("%s\t\t%s\n",
service->servicetype->service, service->url);
				else
					printf("%s\t%s\n",
service->servicetype->service, service->url);
			}
			service = service->next;
		}
	} else if (type == CUSTOMER) {
		while (service) {
			if (service->cust_id == id) {
				i++;
				if (i == 1)
					printf("\nService Details:\n");
				if ((strlen(service->servicetype->service)) < 7)
					printf("%s\t\t%s\n",
service->servicetype->service, service->url);
				else
					printf("%s\t%s\n",
service->servicetype->service, service->url);
			}
			service = service->next;
		}
	}
	return i;
}
