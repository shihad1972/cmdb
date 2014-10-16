/* 

 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "base_sql.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int
add_server_to_database(cmdb_config_s *config, cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	char *input;
	const unsigned int args = cmdb_search_args[VM_ID_ON_NAME];
	const unsigned int fields = cmdb_search_fields[VM_ID_ON_NAME];
	int retval = 0;
	unsigned int max;
	cmdb_vm_host_s *vmhost;
//	dbdata_s *data = '\0';
	dbdata_s *data;
	
	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_server_to_database");
	if (!(vmhost = malloc(sizeof(cmdb_vm_host_s))))
		report_error(MALLOC_FAIL, "vmhost in add_server_to_database");
	cmdb_init_vmhost_t(vmhost);
	cmdb->vmhost = vmhost;
	max = cmdb_get_max(args, fields);
	init_multi_dbdata_struct(&data, max);
/* Check is a server with this name already in the DB */
	snprintf(data->args.text, HOST_S, "%s", cmdb->server->name);
	if ((retval = cmdb_run_search(config, data, SERVER_ID_ON_NAME)) != 0) {
		clean_dbdata_struct(data);
		free(input);
		fprintf(stderr, "Server %s exists in database\n",
cmdb->server->name);
		return SERVER_EXISTS;
	} else {
/* FIXME: We should not have to clean here - this is the db search routine */
		clean_dbdata_struct(data->next);
		memset(data, 0, sizeof(dbdata_s));
	}
	snprintf(data->args.text, RANGE_S, "%s", cm->coid);
	if (cmdb->customer) {
		retval = cmdb_run_search(config, data, CUST_ID_ON_COID);
		if (data->fields.number == 0) {
			fprintf(stderr, "Unable to find COID %s\n",
			 cmdb->customer->coid);
			free(input);
			clean_dbdata_struct(data);
			return NO_COID;
		} else {
			cmdb->customer->cust_id = data->fields.number;
		}
/* FIXME: We should not have to clean here - this is the db search routine */
		clean_dbdata_struct(data->next);
		memset(data, 0, sizeof(dbdata_s));
	}
/* Check for vmhost. if so this server is a virtual machine */
	if (cm->vmhost) {
		snprintf(data->args.text, RBUFF_S, "%s", cm->vmhost);
		retval = cmdb_run_search(config, data, VM_ID_ON_NAME);
		if (data->fields.number == 0) {
			fprintf(stderr, "Unable to find vmhost %s\n",
			 cm->vmhost);
			free(input);
			clean_dbdata_struct(data);
			return NO_VM_HOSTS;
		}
	}
	if (cmdb->customer)
		cmdb->server->cust_id = cmdb->customer->cust_id;
	if (cmdb->vmhost)
		cmdb->server->vm_server_id = cmdb->vmhost->id;
	cmdb->server->cuser = cmdb->server->muser = (unsigned long int)getuid();
	retval = cmdb_run_insert(config, cmdb, SERVERS);
	clean_dbdata_struct(data);
	free(input);
	return retval;
}

int
remove_server_from_database(cmdb_config_s *config, cmdb_comm_line_s *cm)
{
	int retval = NONE;
	dbdata_s data;
	memset(&data, 0, sizeof(data));
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
	clean_dbdata_struct(data.next);
	return NONE;
}

int
update_server_in_database(cmdb_config_s *config, cmdb_comm_line_s *cm)
{
	int retval = NONE;
	unsigned long int ids[2];

	if ((ids[1] = cmdb_get_server_id(config, cm->name)) == 0)
		return SERVER_NOT_FOUND;
	ids[0] = (unsigned long int)getuid();
	if (cm->make)
		retval = update_member_on_id(config, cm->make, ids[1], UP_SERVER_MAKE);
	if (cm->model)
		retval = update_member_on_id(config, cm->model, ids[1], UP_SERVER_MODEL);
	if (cm->uuid)
		retval = update_member_on_id(config, cm->uuid, ids[1], UP_SERVER_UUID);
	if (cm->vendor)
		retval = update_member_on_id(config, cm->vendor, ids[1], UP_SERVER_VENDOR);
	if (retval == 0)
		set_server_updated(config, ids);
	return retval;
}

int
add_hardware_to_database(cmdb_config_s *config, cmdb_s *cmdb)
{
	const unsigned int args = cmdb_search_args[SERVER_ID_ON_NAME];
	const unsigned int fields = cmdb_search_fields[SERVER_ID_ON_NAME];
	int retval = NONE;
	unsigned int max = cmdb_get_max(args, fields);
	unsigned long int user[2];
	dbdata_s *data;

	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, RBUFF_S, "%s", cmdb->server->name);
	retval = cmdb_run_search(config, data, SERVER_ID_ON_NAME);
	if (data->fields.number == 0) {
		printf("Unable to retrieve server_id for server %s\n",
		 cmdb->server->name);
		clean_dbdata_struct(data);
		return SERVER_NOT_FOUND;
	} else
		cmdb->hardware->server_id = cmdb->server->server_id = data->fields.number;
	clean_dbdata_struct(data->next);
	memset(data, 0, sizeof *data);
	data->args.number = (uli_t)cmdb->hardware->ht_id;
	cmdb->hardware->cuser = cmdb->hardware->muser = cmdb->server->muser = (uli_t)getuid();
	user[0] = cmdb->server->muser;
	user[1] = cmdb->server->server_id;
	if ((retval = cmdb_run_search(config, data, HCLASS_ON_HARD_TYPE_ID)) == 0) {
		fprintf(stderr, "Cannot find hardware class\n");
		clean_dbdata_struct(data);
		return NONE;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple hardware classes\nUsing %s\n", data->fields.text );
		snprintf(cmdb->hardtype->hclass, MAC_S, "%s", data->fields.text);
	} else {
		if (!(cmdb->hardtype)) {
			cmdb_hard_type_s *hardt;
			if (!(hardt = malloc(sizeof(cmdb_hard_type_s))))
				report_error(MALLOC_FAIL, "hardt in add_hardware_to_database");
			cmdb_init_hardtype_t(hardt);
			cmdb->hardtype = hardt;
		}
		snprintf(cmdb->hardtype->hclass, MAC_S, "%s", data->fields.text);
	}
	printf("Adding to DB....\n");
	retval = cmdb_run_insert(config, cmdb, HARDWARES);
	set_server_updated(config, user);
	clean_dbdata_struct(data);
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
	if ((retval = cmdb_run_multiple_query(config,
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
	if ((retval = cmdb_run_multiple_query(config, cmdb, SERVER | CUSTOMER)) != 0) {
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
	if ((retval = cmdb_run_query(config, cmdb, HARDWARE_TYPE)) != 0) {
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
	if ((retval = cmdb_run_multiple_query(config, cmdb, SERVER | HARDWARE)) != 0) {
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
	if ((retval = cmdb_run_multiple_query(config, cmdb, SERVER | SERVICE)) != 0) {
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
	char *uname, *crtime;
	uid_t uid;
	time_t cmtime;
	struct passwd *user;
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
	uid = (uid_t)server->cuser;
	user = getpwuid(uid);
	uname = user->pw_name;
	cmtime = (time_t)server->ctime;
	crtime = ctime(&cmtime);
	printf("Created user:\t%s at %s", uname, crtime);
	uid = (uid_t)server->muser;
	user = getpwuid(uid);
	uname = user->pw_name;
	cmtime = (time_t)server->mtime;
	crtime = ctime(&cmtime);
	printf("Modified by:\t%s at %s", uname, crtime);
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
	if ((retval = cmdb_run_query(config, cmdb, VM_HOST)) != 0) {
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
	const unsigned int args = cmdb_search_args[VM_ID_ON_NAME];
	const unsigned int fields = cmdb_search_fields[VM_ID_ON_NAME];
	int retval = NONE;
	unsigned int max = cmdb_get_max(args, fields);
	dbdata_s *data;

	if (!(base->vmhost->type))
		return NO_MODEL;
	else if (strncmp(base->vmhost->type, "NULL", COMM_S) == 0)
		return NO_MODEL; 
	if (strncmp(cm->name, "NULL", COMM_S) == 0)
		return NO_NAME;
	else
		init_multi_dbdata_struct(&data, max);
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
	base->vmhost->server_id = data->fields.number;
	base->vmhost->cuser = base->vmhost->muser = (unsigned long int)getuid();
	if ((retval = cmdb_run_insert(cmc, base, VM_HOSTS)) != 0)
		printf("Error adding to database\n");
	else
		printf("VM host %s added to database\n", cm->name);
	clean_dbdata_struct(data);
	return retval;
}

void
set_server_updated(cmdb_config_s *config, unsigned long int *ids)
{
	int retval;
	dbdata_s *data = '\0';
	unsigned long int *user = ids;

	init_multi_dbdata_struct(&data, cmdb_update_args[UP_SERVER_MUSER]);
	data->args.number = *user;
	user++;
	data->next->args.number = *user;
	if ((retval = cmdb_run_update(config, data, UP_SERVER_MUSER)) < 1)
		fprintf(stderr, "Unable to set server updated\n");
	else if (retval > 1)
		fprintf(stderr, "More that 1 server updated??\n");
	clean_dbdata_struct(data);
}

unsigned long int
cmdb_get_server_id(cmdb_config_s *config, char *server)
{
	int retval;
	unsigned long int server_id;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, cmdb_search_args[SERVER_ID_ON_NAME]);
	snprintf(data->args.text, RBUFF_S, "%s", server);
	if ((retval = cmdb_run_search(config, data, SERVER_ID_ON_NAME)) == 0)
		fprintf(stderr, "Server %s does not exist!", server);
	else if (retval > 1)
		fprintf(stderr, "Multiple servers for %s!", server);
	server_id = data->fields.number;
	clean_dbdata_struct(data);
	return server_id;
}

int
update_member_on_id(cmdb_config_s *config, char *member, unsigned long int id, int type)
{
	int retval = NONE;
	dbdata_s *data = '\0';

	init_multi_dbdata_struct(&data, cmdb_update_args[type]);
	snprintf(data->args.text, RBUFF_S, "%s", member);
	data->next->args.number = id;
	if ((retval = cmdb_run_update(config, data, type)) < 1) {
		fprintf(stderr, "Unable updated with %s\n", member);
		retval = CANNOT_UPDATE;
	} else if (retval > 1) {
		fprintf(stderr, "More that 1 server updated??\n");
		retval = MULTIPLE_SERVERS;
	} else {
		printf("Updated to %s\n", member);
		retval = NONE;
	}
	clean_dbdata_struct(data);
	return retval;
}

