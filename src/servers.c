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
#include "base_sql.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int
add_server_to_database(cmdb_config_s *config, cmdb_comm_line_s *cm, cmdb_s *cmdb, int cl)
{
	char *input;
	int retval;
	cmdb_vm_host_s *vmhost;
	
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_server_to_database");
	if (!(vmhost = malloc(sizeof(cmdb_vm_host_s))))
		report_error(MALLOC_FAIL, "vmhost in add_server_to_database");

	cmdb_init_vmhost_t(vmhost);
	cmdb->vmhost = vmhost;
	retval = 0;
	if (cl != 0)
		complete_server_values(cmdb, cl);
	if (cmdb->customer) {
		if ((retval = run_search(config, cmdb, CUST_ID_ON_COID)) != 0) {
			fprintf(stderr, "Unable to retrieve cust_id for COID %s\n",
			 cmdb->customer->coid);
			free(input);
			return retval;
		}
	}
/* Check for vmhost. if so this server is a virtual machine */
	if (cm->vmhost) {
		printf("VM host: %s\n", cm->vmhost);
		snprintf(vmhost->name, RBUFF_S, "%s", cm->vmhost);
		if ((retval = run_search(config, cmdb, VM_ID_ON_NAME)) != 0) {
			fprintf(stderr, "Unable to retrieve vmhost %s id\n",
			 cm->vmhost);
			free(input);
			return retval;
		}
	}
	if (cmdb->customer)
		cmdb->server->cust_id = cmdb->customer->cust_id;
	if (cmdb->vmhost)
		cmdb->server->vm_server_id = cmdb->vmhost->id;
		retval = run_insert(config, cmdb, SERVERS);
	free(input);
	return retval;
}

int
remove_server_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm)
{
	int retval = NONE;
	dbdata_s data;
	if (strncmp(cm->name, "NULL", COMM_S) != 0)
		snprintf(data.args.text, CONF_S, "%s", cm->name);
	else
		return NO_NAME;
	if ((retval = cmdb_run_search(config, &data, SERVER_ID_ON_NAME)) == 0) {
		fprintf(stderr, "No results for server %s\n", data.args.text);
		return SERVER_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple results for server %s\n", data.args.text);
		return MULTIPLE_SERVERS;
	} else
		data.args.number = data.fields.number;
	if ((retval = cmdb_run_delete(config, &data, SERVERS)) == 0) {
		fprintf(stderr, "Server %s not deleted\n", cm->name);
		return SERVER_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple servers with naem %s deleted\n", cm->name);
		return MULTIPLE_SERVERS;
	} else 
		printf("Server %s deleted from database\n", cm->name);
	return NONE;
}

int
add_hardware_to_database(cmdb_config_s *config, cmdb_s *cmdb)
{
	int retval = NONE;
	dbdata_s *data;

	cmdb_init_initial_dbdata(&data, HCLASS_ON_HARD_TYPE_ID);
	if ((retval = run_search(config, cmdb, SERVER_ID_ON_NAME)) != 0) {
		printf("Unable to retrieve server_id for server %s\n",
		 cmdb->server->name);
		return retval;
	} else
		cmdb->hardware->server_id = cmdb->server->server_id;
	data->args.number = (uli_t)cmdb->hardware->ht_id;
	if ((retval = cmdb_run_search(config, data, HCLASS_ON_HARD_TYPE_ID)) == 0) {
		fprintf(stderr, "Cannot find hardware class\n");
		clean_dbdata_struct(data);
		return NONE;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple hardware classes\n");
		fprintf(stderr, "Using %s\n", data->fields.text);
		snprintf(cmdb->hardtype->hclass, MAC_S, "%s", data->fields.text);
	} else {
		if (!(cmdb->hardtype)) {
			cmdb_hard_type_s *hardt;
			if (!(hardt = malloc(sizeof(cmdb_hard_type_s))))
				report_error(MALLOC_FAIL, "hardt in fill_hardware_values");
			cmdb_init_hardtype_t(hardt);
			cmdb->hardtype = hardt;
		}
		snprintf(cmdb->hardtype->hclass, MAC_S, "%s", data->fields.text);
	}
	printf("Adding to DB....\n");
	retval = run_insert(config, cmdb, HARDWARES);
	return retval;
}

void
display_server_info(char *name, char *uuid, cmdb_config_s *config)
{
	int retval, i;
	cmdb_server_s *server;
	cmdb_s *cmdb;
	
	if (!(cmdb = malloc(sizeof(cmdb_s))))
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
		if ((strncmp(server->name, name, HOST_S) == 0)) {
			print_server_details(server, cmdb);
			server = server->next;
			i++;
		} else if (uuid) {
			if ((strncmp(server->uuid, uuid, CONF_S) == 0)) {
				print_server_details(server, cmdb);
				server = server->next;
				i++;
			}
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
display_all_servers(cmdb_config_s *config)
{
	int retval;
	cmdb_s *cmdb;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
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
display_hardware_types(cmdb_config_s *config)
{
	int retval;
	cmdb_s *cmdb;
	cmdb_hard_type_s *list;
	size_t len;

	retval = 0;
	if (!(cmdb = malloc(sizeof(cmdb_s))))
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
display_server_hardware(cmdb_config_s *config, char *name)
{
	int retval;
	cmdb_s *cmdb;
	cmdb_server_s *server;
	cmdb_hardware_s *hardware;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
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
		if ((strncmp(server->name, name, HOST_S) == 0)) {
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
display_server_services(cmdb_config_s *config, char *name)
{
	int retval;
	cmdb_s *cmdb;
	cmdb_server_s *server;
	cmdb_service_s *service;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
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
		if ((strncmp(server->name, name, HOST_S) == 0)) {
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
print_server_details(cmdb_server_s *server, cmdb_s *base)
{
	int retval;
	cmdb_customer_s *customer = base->customer;
	cmdb_vm_host_s *vmhost = base->vmhost;
	cmdb_hardware_s *hard = base->hardware;
	cmdb_service_s *service = base->service;

	printf("Server Details:\n");
	printf("Name:\t\t%s\n", server->name);
	printf("UUID:\t\t%s\n", server->uuid);
	printf("Vendor:\t\t%s\n", server->vendor);
	printf("Make:\t\t%s\n", server->make);
	printf("Model:\t\t%s\n", server->model);
	if (server->cust_id > 0) {
		while (server->cust_id != customer->cust_id)
			customer = customer->next;
		printf("Customer:\t%s\n", customer->name);
		printf("COID:\t\t%s\n", customer->coid);
	} else {
		printf("No Customer associated with this server!\n");
	}
	if (server->vm_server_id > 0) {
		while (server->vm_server_id != vmhost->id) {
			vmhost = vmhost->next;
		}
		printf("VM Server:\t%s\n", vmhost->name);
	} else {
		while (vmhost) {
			if (server->server_id != vmhost->server_id) {
				vmhost = vmhost->next;
			} else {
				if (server->server_id == vmhost->server_id) {
					printf("VM Server host type %s\n", vmhost->type);
				}
				vmhost = vmhost->next;
			}
		}
		if (!vmhost)
			printf("Stand alone server\n");
	}
	if ((retval = print_hardware(hard, server->server_id)) == 0)
		printf("No hardware for this server\n");
	if ((retval = print_services(service, server->server_id, SERVER)) == 0)
		printf("No services for this server\n");
}

void
print_all_servers(cmdb_s *cmdb)
{
	char *customer;
	unsigned long int id;
	cmdb_server_s *server = cmdb->server;
	cmdb_customer_s *cust = cmdb->customer;
	size_t len;

	if (server)
		printf("Server\t\t\t\tCustomer\n");
	while (server) {
		id = server->cust_id;
		len = strlen(server->name);
		while (cust) {
			if (id != cust->cust_id)
				cust = cust->next;
			else
				break;
		}
		if (!(cust))
			customer = strndup("No Customer", HOST_S);
		else
			customer = strndup(cust->coid, HOST_S);
		if (len > 23) {
			printf("%s\t%s\n", server->name, customer);
		} else if (len > 15) {
			printf("%s\t\t%s\n", server->name, customer);
		} else if (len > 7) {
			printf("%s\t\t\t%s\n", server->name, customer);
		} else {
			printf("%s\t\t\t\t%s\n", server->name, customer);
		}
		server = server->next;
		cust = cmdb->customer;
		free(customer);
	}
}

void
display_vm_hosts(cmdb_config_s *config)
{
	int retval;
	cmdb_s *cmdb;
	cmdb_vm_host_s *list;

	retval = 0;
	if (!(cmdb = malloc(sizeof(cmdb_s))))
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
print_vm_hosts(cmdb_vm_host_s *vmhost)
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
print_hardware(cmdb_hardware_s *hard, unsigned long int id)
{
	int i = 0;

	while (hard) {
		if (hard->server_id == id) {
			i++;
			if (i == 1)
				printf("\nHardware details:\n");
			if (strlen(hard->hardtype->hclass) < 8)
				printf("%s\t\t%s\t%s\n",
hard->hardtype->hclass, hard->device, hard->detail);
			else
				printf("%s\t%s\t%s\n",
hard->hardtype->hclass, hard->device, hard->detail);
		}
		hard = hard->next;
	}
	return i;
}

int
print_services(cmdb_service_s *service, unsigned long int id, int type)
{
	int i = 0;

	if (type == SERVER) {
		while (service) {
			if (service->server_id == id) {
				i++;
				if (i == 1)
					printf("\nService Details:\n");
				if ((strlen(service->servicetype->service)) < 8)
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
				if ((strlen(service->servicetype->service)) < 8)
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

int
add_vm_host_to_db(cmdb_config_s *cmc, cmdb_comm_line_s *cm, cmdb_s *base)
{
	int retval = NONE;
	dbdata_s *data;

	if (!(base->server->model))
		return NO_MODEL;
	else if (strncmp(base->server->model, "NULL", COMM_S) == 0)
		return NO_MODEL;
	if (strncmp(cm->name, "NULL", COMM_S) == 0)
		return NO_NAME;
	else
		cmdb_init_initial_dbdata(&data, SERVER_ID_ON_NAME);
	snprintf(data->args.text, NAME_S, "%s", cm->name);
	if ((retval = cmdb_run_search(cmc, data, SERVER_ID_ON_NAME)) == 0) {
		clean_dbdata_struct(data);
		printf("Server %s not found\n", cm->name);
		return SERVER_NOT_FOUND;
	} else if (retval > 1) {
		clean_dbdata_struct(data);
		printf("Multiple servers found for %s\n", cm->name);
		return MULTIPLE_SERVERS;
	} else
		retval = NONE;
	if (!(base->vmhost)) {
		if (!(base->vmhost = malloc(sizeof(cmdb_vm_host_s))))
			report_error(MALLOC_FAIL, "base->vhost in add_vm_host_to_db");
	}
	snprintf(base->vmhost->name, NAME_S, "%s", cm->name);
	snprintf(base->vmhost->type, MAC_S, "%s", base->server->model);
	base->vmhost->server_id = data->fields.number;
	if ((retval = run_insert(cmc, base, VM_HOSTS)) != 0)
		printf("Error adding to database\n");
	else
		printf("VM host %s added to database\n", cm->name);
	clean_dbdata_struct(data);
	return retval;
}
