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
 *  commline.c
 *
 *  Contains the functions to deal with command line arguments and also to
 *  read the values from the configuration file
 *
 *  Part of the CMDB program
 *
 *  (C) Iain M Conochie 2012 - 2013
 *
 */

#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "cmdb_base_sql.h"

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_t *comp, cmdb_t *base)
{
	int opt, retval;
	
	retval = NONE;
	
	comp->action = NONE;
	comp->type = NONE;
	strncpy(comp->config, "/etc/dnsa/dnsa.conf", CONF_S);
	strncpy(comp->name, "NULL", CONF_S);
	strncpy(comp->id, "NULL", RANGE_S);
	strncpy(comp->vmhost, "NULL", COMM_S);
	if (!(base->server = malloc(sizeof(cmdb_server_t))))
		report_error(MALLOC_FAIL, "base->server in parse_cmdb_comm_line");
	if (!(base->customer = malloc(sizeof(cmdb_customer_t))))
		report_error(MALLOC_FAIL, "base->customer in parse_cmdb_comm_line");
	if (!(base->contact = malloc(sizeof(cmdb_contact_t))))
		report_error(MALLOC_FAIL, "base->contact in parse_cmdb_comm_line");
	if (!(base->service = malloc(sizeof(cmdb_service_t))))
		report_error(MALLOC_FAIL, "base->service in parse_cmdb_comm_line");
	if (!(base->servicetype = malloc(sizeof(cmdb_service_type_t))))
		report_error(MALLOC_FAIL, "base->servicetype in parse_cmdb_comm_line");
	if (!(base->hardware = malloc(sizeof(cmdb_hardware_t))))
		report_error(MALLOC_FAIL, "base->hardware in parse_cmdb_comm_line");
	cmdb_init_server_t(base->server);
	cmdb_init_customer_t(base->customer);
	cmdb_init_service_t(base->service);
	cmdb_init_servicetype_t(base->servicetype);
	cmdb_init_hardware_t(base->hardware);
	cmdb_init_contact_t(base->contact);
	while ((opt = getopt(argc, argv, "n:i:m:V:M:O:C:U:A:T:Y:Z:N:P:E:D:L:B:I:S:dlatscehv")) != -1) {
		switch (opt) {
			case 's':
				comp->type = SERVER;
				break;
			case 'c':
				comp->type = CUSTOMER;
				break;
			case 't':
				comp->type = CONTACT;
				break;
			case 'e':
				comp->type = SERVICE;
				break;
			case 'h':
				comp->type = HARDWARE;
				break;
			case 'v':
				comp->type = VM_HOST;
				break;
			case 'd':
				comp->action = DISPLAY;
				break;
			case 'l':
				comp->action = LIST_OBJ;
				snprintf(comp->name, MAC_S, "all");
				break;
			case 'a':
				comp->action = ADD_TO_DB;
				if ((comp->type != HARDWARE) &&
				 (comp->type != SERVICE) && /*
				 (comp->type != CONTACT) && */
				 (comp->type != NONE) &&
				 (strncmp(comp->id, "NULL", COMM_S) == 0))
					snprintf(comp->id, MAC_S, "NOCOID");
				break;
			case 'n':
				snprintf(comp->name, CONF_S, "%s", optarg);
				break;
			case 'i':
				snprintf(comp->id, CONF_S, "%s", optarg);
				break;
			case 'm':
				snprintf(comp->vmhost, NAME_S, "%s", optarg);
				break;
			case 'V':
				snprintf(base->server->vendor, CONF_S, "%s", optarg);
				break;
			case 'M':
				snprintf(base->server->make, CONF_S, "%s", optarg);
				break;
			case 'O':
				snprintf(base->server->model, CONF_S, "%s", optarg);
				break;
			case 'U':
				snprintf(base->server->uuid, CONF_S, "%s", optarg);
				break;
			case 'C':
				snprintf(base->customer->coid, RANGE_S, "%s", optarg);
				break;
			case 'A':
				snprintf(base->customer->address, NAME_S, "%s", optarg);
				break;
			case 'T':
				snprintf(base->customer->city, HOST_S, "%s", optarg);
				break;
			case 'Y':
				snprintf(base->customer->county, MAC_S, "%s", optarg);
				break;
			case 'Z':
				snprintf(base->customer->postcode, RANGE_S, "%s", optarg);
				break;
			case 'N':
				snprintf(base->contact->name, HOST_S, "%s", optarg);
				break;
			case 'P':
				snprintf(base->contact->phone, MAC_S, "%s", optarg);
				break;
			case 'E':
				snprintf(base->contact->email, HOST_S, "%s", optarg);
				break;
			case 'D':
				if (comp->type == SERVICE) {
					snprintf(base->service->detail, HOST_S, "%s", optarg);
				} else if (comp->type == HARDWARE) {
					snprintf(base->hardware->detail, HOST_S, "%s", optarg);
				} else {
					printf("Please supply type before adding options\n");
					retval = NO_TYPE;
					return retval;
				}
				break;
			case 'I':
				if (comp->type == SERVICE) {
					base->service->service_type_id = strtoul(optarg, NULL, 10);
				} else if (comp->type == HARDWARE) {
					base->hardware->ht_id = strtoul(optarg, NULL, 10);
				} else {
					printf("Please supply type before adding options\n");
					retval = NO_TYPE;
					return retval;
				}
				break;
			case 'L':
				snprintf(base->service->url, HOST_S, "%s", optarg);
				break;
			case 'B':
				snprintf(base->hardware->device, MAC_S, "%s", optarg);
				break;
			case 'S':
				snprintf(base->servicetype->service, RANGE_S, "%s", optarg);
				break;
			default:
				printf("Unknown option: %c\n", opt);
				retval = DISPLAY_USAGE;
				return retval;
				break;
		}
	}

	retval = check_cmdb_comm_options(comp, base);
	return retval;
}

int
check_cmdb_comm_options(cmdb_comm_line_t *comp, cmdb_t *base)
{
	int retval;

	retval = NONE;
	if ((strncmp(comp->name, "NULL", CONF_S) == 0) &&
		(strncmp(comp->id, "NULL", CONF_S) == 0) &&
		(comp->type == NONE && comp->action == NONE))
		retval = DISPLAY_USAGE;
	else if ((comp->type == NONE) && (comp->action != NONE))
		retval = NO_TYPE;
	else if ((comp->action == NONE) && (comp->type != NONE))
		retval = NO_ACTION;
	else if ((comp->action == NONE) && (comp->type == NONE))
		retval = NO_ACTION;
	else if ((strncmp(comp->name, "NULL", CONF_S) == 0) &&
		(strncmp(comp->id, "NULL", CONF_S) == 0) &&
		(comp->type != NONE || comp->action != NONE) &&
		(comp->type != CONTACT))
		retval = NO_NAME_OR_ID;
	else if (comp->action == ADD_TO_DB) {
		if (comp->type == SERVER) {
			snprintf(base->server->name, MAC_S, "%s", comp->name);
			if (strncmp(base->server->make, "NULL", COMM_S) == 0)
				retval = NO_MAKE;
			else if (strncmp(base->server->model, "NULL", COMM_S) == 0)
				retval = NO_MODEL;
			else if (strncmp(base->server->vendor, "NULL", COMM_S) == 0)
				retval = NO_VENDOR;
			else if (strncmp(base->server->uuid, "NULL", COMM_S) == 0)
				retval = NO_UUID;
			else if (strncmp(base->customer->coid, "NULL", COMM_S) == 0)
				retval = NO_COID;
			else if (strncmp(base->server->name, "NULL", COMM_S) == 0)
				retval = NO_NAME;
		} else if (comp->type == CUSTOMER) {
			snprintf(base->customer->name, CONF_S, "%s", comp->name);
			if (strncmp(base->customer->address, "NULL", COMM_S) == 0)
				retval = NO_ADDRESS;
			else if (strncmp(base->customer->city, "NULL", COMM_S) == 0)
				retval = NO_CITY;
			else if (strncmp(base->customer->county, "NULL", COMM_S) == 0)
				retval = NO_COUNTY;
			else if (strncmp(base->customer->postcode, "NULL", COMM_S) == 0)
				retval = NO_POSTCODE;
			else if (strncmp(base->customer->coid, "NULL", COMM_S) == 0)
				retval = NO_COID;
			else if (strncmp(comp->name, "NULL", COMM_S) == 0)
				retval = NO_NAME;
		} else if (comp->type == SERVICE) {
			if ((strncmp(comp->name, "NULL", COMM_S) == 0) && 
			   (strncmp(comp->id, "NULL", COMM_S) == 0)) {
				retval = NO_NAME_COID;
			} else if (strncmp(comp->name, "NULL", COMM_S) == 0) {
				retval = NO_NAME;
			} else if (strncmp(comp->id, "NULL", COMM_S) == 0) {
				retval = NO_COID;
			} else {
				snprintf(base->server->name, MAC_S, "%s", comp->name);
				snprintf(base->customer->coid, RANGE_S, "%s", comp->id);
			}
		} else if (comp->type == CONTACT) {
			if (strncmp(comp->id, "NULL", COMM_S) == 0) {
				retval = NO_COID;
			} else if (strncmp(base->contact->name, "NULL", COMM_S) == 0) {
				retval = NO_CONT_NAME;
			} else if (strncmp(base->contact->phone, "NULL", COMM_S) == 0) {
				retval = NO_PHONE;
			} else if (strncmp(base->contact->email, "NULL", COMM_S) == 0) {
				retval = NO_EMAIL;
			} else {
				snprintf(base->customer->coid, RANGE_S, "%s", comp->id);
			}
		}
	}
	return retval;
}

int
parse_cmdb_config_file(cmdb_config_t *dc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	int retval;
	unsigned long int portno;

	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = -1;
	} else {
		while ((fgets(buff, RANGE_S - 1, cnf))) {
			sscanf(buff, "DBTYPE=%s", dc->dbtype);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "FILE=%s", dc->file);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "PASS=%s", dc->pass);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "HOST=%s", dc->host);	
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "USER=%s", dc->user);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "DB=%s", dc->db);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "SOCKET=%s", dc->socket);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "PORT=%s", port);
		}
		retval = 0;
		fclose(cnf);
	}
	
	/* We need to check the value of portno before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = -2;
	} else {
		dc->port = (unsigned int) portno;
	}
	
	return retval;
}

void
init_cmdb_comm_line_values(cmdb_comm_line_t *cm)
{
	cm->action = 0;
	cm->type = 0;
	snprintf(cm->config, CONF_S, "NULL");
	snprintf(cm->name, CONF_S, "NULL");
	snprintf(cm->id, CONF_S, "NULL");
	snprintf(cm->vmhost, NAME_S, "NULL");
}

void
init_cmdb_config_values(cmdb_config_t *dc)
{
	snprintf(dc->dbtype, RANGE_S, "none");
	snprintf(dc->file, CONF_S, "none");
	snprintf(dc->db, CONF_S, "cmdb");
	snprintf(dc->user, CONF_S, "root");
	snprintf(dc->host, CONF_S, "localhost");
	snprintf(dc->pass, CONF_S, "%s", "");
	snprintf(dc->socket, CONF_S, "%s", "");
	dc->port = 3306;
	dc->cliflag = 0;
}

void
cmdb_init_struct(cmdb_t *cmdb)
{
	cmdb->server = '\0';
	cmdb->customer = '\0';
	cmdb->vmhost = '\0';
	cmdb->hardware = '\0';
	cmdb->hardtype = '\0';
	cmdb->contact = '\0';
	cmdb->service = '\0';
	cmdb->servicetype = '\0';
}

void
cmdb_init_server_t(cmdb_server_t *server)
{
	snprintf(server->vendor, COMM_S, "NULL");
	snprintf(server->make, COMM_S, "NULL");
	snprintf(server->model, COMM_S, "NULL");
	snprintf(server->uuid, COMM_S, "NULL");
	snprintf(server->name, COMM_S, "NULL");
	server->vm_server_id = 0;
	server->next = '\0';
}

void
cmdb_init_customer_t(cmdb_customer_t *cust)
{
	snprintf(cust->address, COMM_S, "NULL");
	snprintf(cust->city, COMM_S, "NULL");
	snprintf(cust->county, COMM_S, "NULL");
	snprintf(cust->postcode, COMM_S, "NULL");
	snprintf(cust->coid, COMM_S, "NULL");
	snprintf(cust->name, COMM_S, "NULL");
	cust->next = '\0';
}

void
cmdb_init_service_t(cmdb_service_t *service)
{
	service->next = '\0';
}

void
cmdb_init_hardware_t(cmdb_hardware_t *hard)
{
	hard->next = '\0';
}

void
cmdb_init_contact_t(cmdb_contact_t *cont)
{
	snprintf(cont->name, COMM_S, "NULL");
	snprintf(cont->phone, COMM_S, "NULL");
	snprintf(cont->email, COMM_S, "NULL");
	cont->next = '\0';
}

void
cmdb_init_hardtype_t(cmdb_hard_type_t *type)
{
	type->next = '\0';
}

void
cmdb_init_servicetype_t(cmdb_service_type_t *type)
{
	snprintf(type->service, COMM_S, "NULL");
	snprintf(type->detail, COMM_S, "NULL");
	type->next = '\0';
}

void
cmdb_init_vmhost_t(cmdb_vm_host_t *type)
{
	type->next = '\0';
}

void
cmdb_main_free(cmdb_comm_line_t *cm, cmdb_config_t *cmc, char *cmdb_config)
{
	free(cm);
	free(cmc);
	free(cmdb_config);
}

void
cmdb_clean_list(cmdb_t *cmdb)
{
	if (cmdb->server)
		clean_server_list(cmdb->server);
	if (cmdb->customer)
		clean_customer_list(cmdb->customer);
	if (cmdb->contact)
		clean_contact_list(cmdb->contact);
	if (cmdb->service)
		clean_service_list(cmdb->service);
	if (cmdb->servicetype)
		clean_service_type_list(cmdb->servicetype);
	if (cmdb->hardware)
		clean_hardware_list(cmdb->hardware);
	if (cmdb->hardtype)
		clean_hardware_type_list(cmdb->hardtype);
	if (cmdb->vmhost)
		clean_vmhost_list(cmdb->vmhost);
	free(cmdb);
}

void
clean_server_list(cmdb_server_t *list)
{
	cmdb_server_t *server, *next;

	server = list;
	next = server->next;
	while (server) {
		free(server);
		server = next;
		if (next) {
			next = server->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_customer_list(cmdb_customer_t *list)
{
	cmdb_customer_t *customer, *next;

	customer = list;
	next = customer->next;
	while (customer) {
		free(customer);
		customer = next;
		if (next) {
			next = customer->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_contact_list(cmdb_contact_t *list)
{
	cmdb_contact_t *contact, *next;

	contact = list;
	next = contact->next;
	while(contact) {
		free(contact);
		contact = next;
		if (next) {
			next = contact->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_service_list(cmdb_service_t *list)
{
	cmdb_service_t *service, *next;

	service = list;
	next = service->next;
	while (service) {
		free(service);
		service = next;
		if (next) {
			next = service->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_service_type_list(cmdb_service_type_t *list)
{
	cmdb_service_type_t *service, *next;

	service = list;
	next = service->next;
	while (service) {
		free(service);
		service = next;
		if (next) {
			next = service->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_hardware_list(cmdb_hardware_t *list)
{
	cmdb_hardware_t *hardware, *next;

	hardware = list;
	next = hardware->next;
	while(hardware) {
		free(hardware);
		hardware = next;
		if (next) {
			next = hardware->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_hardware_type_list(cmdb_hard_type_t *list)
{
	cmdb_hard_type_t *hardware, *next;

	hardware = list;
	next = hardware->next;
	while(hardware) {
		free(hardware);
		hardware = next;
		if (next) {
			next = hardware->next;
		} else {
			next = '\0';
		}
	}
}

void
clean_vmhost_list(cmdb_vm_host_t *list)
{
	cmdb_vm_host_t *vmhost, *next;

	vmhost = list;
	next = vmhost->next;
	while (vmhost) {
		free(vmhost);
		vmhost = next;
		if (next) {
			next = vmhost->next;
		} else {
			next = '\0';
		}
	}
}

