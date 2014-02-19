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
 *  cmdbcom.c
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
#include "base_sql.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comp, cmdb_s *base)
{
	int opt, retval = NONE;

	comp->config = strndup("/etc/dnsa/dnsa.conf", CONF_S);
	while ((opt = getopt(argc, argv,
	 "n:i:m:V:M:O:C:U:A:T:Y:Z:N:P:E:D:L:B:I:S:H:adehlorstuv")) != -1) {
		if (opt == 's') {
			comp->type = SERVER;
		} else if (opt == 'u') {
			comp->type = CUSTOMER;
		} else if (opt == 't') {
			comp->type = CONTACT;
		} else if (opt == 'e') {
			comp->type = SERVICE;
		} else if (opt == 'h') {
			comp->type = HARDWARE;
		} else if (opt == 'o') {
			comp->type = VM_HOST;
		} else if (opt == 'd') {
			comp->action = DISPLAY;
		} else if (opt == 'v') {
			comp->action = CVERSION;
		} else if (opt == 'l') {
			comp->action = LIST_OBJ;
			comp->name = strndup("all", COMM_S);
		} else if (opt == 'a') {
			comp->action = ADD_TO_DB;
/*			if ((comp->type != HARDWARE) &&
			 (comp->type != SERVICE) && 
			 (comp->type != NONE) &&
			 (strncmp(comp->id, "NULL", COMM_S) == 0))
				snprintf(comp->id, MAC_S, "NOCOID"); */
		} else if (opt == 'r') {
			comp->action = RM_FROM_DB;
		} else if (opt == 'n') {
			comp->name = strndup(optarg, HOST_S);
		} else if (opt == 'i') {
			comp->id = strndup(optarg, CONF_S);
		} else if (opt == 'm') {
			comp->vmhost = strndup(optarg, HOST_S);
		} else if (opt == 'V') {
			comp->vendor = strndup(optarg, CONF_S);
		} else if (opt == 'M') {
			comp->make = strndup(optarg, CONF_S);
		} else if (opt == 'O') {
			comp->model = strndup(optarg, CONF_S);
		} else if (opt == 'U') {
			comp->uuid = strndup(optarg, CONF_S);
		} else if (opt == 'C') {
			comp->coid = strndup(optarg, RANGE_S);
		} else if (opt == 'A') {
			comp->address = strndup(optarg, NAME_S);
		} else if (opt == 'T') {
			comp->city = strndup(optarg, HOST_S);
		} else if (opt == 'Y') {
			comp->county = strndup(optarg, MAC_S);
		} else if (opt == 'Z') {
			comp->postcode = strndup(optarg, RANGE_S);
		} else if (opt == 'N') {
			comp->name = strndup(optarg, HOST_S);
		} else if (opt == 'P') {
			comp->phone = strndup(optarg, MAC_S);
		} else if (opt == 'E') {
			comp->email = strndup(optarg, HOST_S);
		} else if (opt == 'D') {
			comp->detail = strndup(optarg, HOST_S);
		} else if (opt == 'I') {
			comp->sid = strtoul(optarg, NULL, 10);
		} else if (opt == 'L') {
			comp->url = strndup(optarg, HOST_S);
		} else if (opt == 'B') {
			comp->device = strndup(optarg, MAC_S);
		} else if (opt == 'S') {
			comp->service = strndup(optarg, RANGE_S);
		} else if (opt == 'H') {
			comp->hclass = strndup(optarg, HOST_S);
		} else {
			printf("Unknown option: %c\n", opt);
			retval = DISPLAY_USAGE;
			return retval;
		}
	}

	retval = check_cmdb_comm_options(comp, base);
	return retval;
}

int
check_cmdb_comm_options(cmdb_comm_line_s *comp, cmdb_s *base)
{
	int retval;

	retval = NONE;
	if ((!(comp->name)) && (!(comp->id)) &&	(comp->type == 0) && 
		(comp->action == 0))
		retval = DISPLAY_USAGE;
	else if (comp->action == CVERSION)
		retval = CVERSION;
	else if ((comp->type == NONE) && (comp->action != NONE))
		retval = NO_TYPE;
	else if ((comp->action == NONE) && (comp->type != NONE))
		retval = NO_ACTION;
	else if ((comp->action == NONE) && (comp->type == NONE))
		retval = NO_ACTION;
	else if ((!(comp->name)) && (!(comp->id)) &&
		(comp->type != NONE || comp->action != NONE) &&
		(comp->type != CONTACT))
		retval = NO_NAME_OR_ID;
	else if (comp->action == ADD_TO_DB) {
		if (comp->type == SERVER) {
			retval = fill_server_values(comp, base);
		} else if (comp->type == CUSTOMER) {
			retval = fill_customer_values(comp, base);
		} else if (comp->type == SERVICE) {
			retval = fill_service_values(comp, base);
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
		} else if (comp->type == HARDWARE) {
			if (strncmp(comp->name, "NULL", COMM_S) == 0) {
				retval = NO_NAME;
			} else if (strncmp(base->hardware->detail, "NULL", COMM_S) == 0) {
				retval = NO_DETAIL;
			} else if (strncmp(base->hardware->device, "NULL", COMM_S) == 0) {
				*(base->hardware->device) = '\0';
				snprintf(base->server->name, MAC_S, "%s", comp->name);
			} else {
				snprintf(base->server->name, MAC_S, "%s", comp->name);
			}
			if (strncmp(base->hardtype->hclass, "NULL", COMM_S) == 0) {
				if (base->hardware->ht_id == 0) {
					retval = NO_CLASS;
				}
			}
		} else if (comp->type == VM_HOST) {
			if (strncmp(comp->name, "NULL", COMM_S) == 0) {
				retval = NO_NAME;
			} else if (strncmp(base->server->model, "NULL", COMM_S) == 0) {
				retval = NO_MODEL;
			}
		}
	}
	return retval;
}

int
parse_cmdb_config_file(cmdb_config_s *dc, char *config)
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
init_cmdb_comm_line_values(cmdb_comm_line_s *cm)
{
	cm->action = 0;
	cm->type = 0;
	cm->vmhost = cm->config = cm->vendor = '\0';
	cm->make = cm->model = cm->id = cm->stype = '\0';
	cm->name = cm->address = cm->city = cm->email = '\0';
	cm->email = cm->detail = cm->hclass = cm->url = '\0';
	cm->device = cm->phone = cm->postcode = cm->coid = '\0';
	cm->service = cm->uuid = cm->county = '\0';
}

void
init_cmdb_config_values(cmdb_config_s *dc)
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
cmdb_init_struct(cmdb_s *cmdb)
{
	cmdb->server = '\0';
	cmdb->customer = '\0';
	cmdb->contact = '\0';
	cmdb->service = '\0';
	cmdb->servicetype = '\0';
	cmdb->hardware = '\0';
	cmdb->hardtype = '\0';
	cmdb->vmhost = '\0';
}

void
cmdb_init_server_t(cmdb_server_s *server)
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
cmdb_init_customer_t(cmdb_customer_s *cust)
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
cmdb_init_service_t(cmdb_service_s *service)
{
	snprintf(service->detail, COMM_S, "NULL");
	snprintf(service->url, COMM_S, "NULL");
	service->service_id = 0;
	service->server_id = 0;
	service->cust_id = 0;
	service->service_type_id = 0;
	service->next = '\0';
}

void
cmdb_init_hardware_t(cmdb_hardware_s *hard)
{
	snprintf(hard->detail, COMM_S, "NULL");
	snprintf(hard->device, COMM_S, "NULL");
	hard->hard_id = 0;
	hard->server_id = 0;
	hard->ht_id = 0;
	hard->next = '\0';
}

void
cmdb_init_contact_t(cmdb_contact_s *cont)
{
	snprintf(cont->name, COMM_S, "NULL");
	snprintf(cont->phone, COMM_S, "NULL");
	snprintf(cont->email, COMM_S, "NULL");
	cont->next = '\0';
}

void
cmdb_init_hardtype_t(cmdb_hard_type_s *type)
{
	snprintf(type->type, COMM_S, "NULL");
	snprintf(type->hclass, COMM_S, "NULL");
	type->ht_id = 0;
	type->next = '\0';
}

void
cmdb_init_servicetype_t(cmdb_service_sype_t *type)
{
	snprintf(type->service, COMM_S, "NULL");
	snprintf(type->detail, COMM_S, "NULL");
	type->next = '\0';
}

void
cmdb_init_vmhost_t(cmdb_vm_host_s *type)
{
	snprintf(type->name, COMM_S, "NULL");
	snprintf(type->type, COMM_S, "NULL");
	type->id = 0;
	type->server_id = 0;
	type->next = '\0';
}

void
cmdb_main_free(cmdb_comm_line_s *cm, cmdb_config_s *cmc, char *cmdb_config)
{
	free(cm);
	free(cmc);
	free(cmdb_config);
}

void
cmdb_clean_list(cmdb_s *cmdb)
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
clean_server_list(cmdb_server_s *list)
{
	cmdb_server_s *server, *next;

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
clean_customer_list(cmdb_customer_s *list)
{
	cmdb_customer_s *customer, *next;

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
clean_contact_list(cmdb_contact_s *list)
{
	cmdb_contact_s *contact, *next;

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
clean_service_list(cmdb_service_s *list)
{
	cmdb_service_s *service, *next;

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
clean_service_type_list(cmdb_service_sype_t *list)
{
	cmdb_service_sype_t *service, *next;

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
clean_hardware_list(cmdb_hardware_s *list)
{
	cmdb_hardware_s *hardware, *next;

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
clean_hardware_type_list(cmdb_hard_type_s *list)
{
	cmdb_hard_type_s *hardware, *next;

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
clean_vmhost_list(cmdb_vm_host_s *list)
{
	cmdb_vm_host_s *vmhost, *next;

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

void
clean_cmdb_comm_line(cmdb_comm_line_s *list)
{
#ifndef CLEAN_COMM_LIST
# define CLEAN_COMM_LIST(list, member) {                \
	if (list->member)                               \
		free(list->member);                     \
}
#endif /* CLEAN_COMM_LIST */
	CLEAN_COMM_LIST(list, vmhost);
	CLEAN_COMM_LIST(list, config);
	CLEAN_COMM_LIST(list, vendor);
	CLEAN_COMM_LIST(list, make);
	CLEAN_COMM_LIST(list, model);
	CLEAN_COMM_LIST(list, id);
	CLEAN_COMM_LIST(list, stype);
	CLEAN_COMM_LIST(list, name);
	CLEAN_COMM_LIST(list, address);
	CLEAN_COMM_LIST(list, city);
	CLEAN_COMM_LIST(list, email);
	CLEAN_COMM_LIST(list, detail);
	CLEAN_COMM_LIST(list, hclass);
	CLEAN_COMM_LIST(list, url);
	CLEAN_COMM_LIST(list, device);
	CLEAN_COMM_LIST(list, phone);
	CLEAN_COMM_LIST(list, postcode);
	CLEAN_COMM_LIST(list, coid);
	CLEAN_COMM_LIST(list, service);
	CLEAN_COMM_LIST(list, uuid);
	CLEAN_COMM_LIST(list, county);
}

#ifdef CLEAN_COMM_LIST
# undef CLEAN_COMM_LIST
#endif /* CLEAN_COMM_LIST */

int
fill_server_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;
	cmdb_server_s *server;
	cmdb_customer_s *cust;
	cmdb_vm_host_s *vm;

	if (!(server = malloc(sizeof(cmdb_server_s))))
		report_error(MALLOC_FAIL, "server in fill_server_values");
	if (!(cust = malloc(sizeof(cmdb_customer_s))))
		report_error(MALLOC_FAIL, "cust in fill_server_values");
	if (cm->vmhost) {
		if (!(vm = malloc(sizeof(cmdb_vm_host_s))))
			report_error(MALLOC_FAIL, "vm in fill_server_values");
		cmdb_init_vmhost_t(vm);
		cmdb->vmhost = vm;
	}
	cmdb_init_server_t(server);
	cmdb_init_customer_t(cust);
	cmdb->server = server;
	cmdb->customer = cust;
	if (cm->vendor) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->vendor, CUSTOMER_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "vendor");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(server->vendor, CONF_S, "%s", cm->vendor);
	} else {
		fprintf(stderr, "No vendor supplied. Setting to none\n");
		snprintf(server->vendor, CONF_S, "none");
	}
	if (cm->make) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->make, CUSTOMER_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "make");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(server->make, CONF_S, "%s", cm->make);
	} else {
		fprintf(stderr, "No make supplied. Setting to none\n");
		snprintf(server->make, COMM_S, "none");
	}
	if (cm->model) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->model, CUSTOMER_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "model");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(server->model, CONF_S, "%s", cm->model);
	} else {
		fprintf(stderr, "No model supplied. Setting to none\n");
		snprintf(server->model, COMM_S, "none");
	}
	if (cm->uuid) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->uuid, UUID_REGEX)) < 0) {
			if ((retval = validate_user_input(cm->uuid, FS_REGEX)) < 0)
				report_error(USER_INPUT_INVALID, "uuid");
			else
				retval = NONE;
		} else {
			retval = NONE;
		}
#endif /* HAVE_LIBPCRE */
		snprintf(server->uuid, HOST_S, "%s", cm->uuid);
	} else {
		fprintf(stderr, "No UUID supplied. Setting to none\n");
		snprintf(server->uuid, COMM_S, "none");
	}
	if (cm->name) {
		if ((retval = validate_user_input(cm->name, NAME_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "name");
		else
			retval = NONE;
	} else {
		clean_cmdb_comm_line(cm);
		cmdb_clean_list(cmdb);
		return NO_NAME;
	}
	if (cm->name) {
		if ((retval = validate_user_input(cm->coid, COID_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "coid");
		else
			retval = NONE;
	} else {
		clean_cmdb_comm_line(cm);
		cmdb_clean_list(cmdb);
		return NO_COID;
	}
	return retval;
}

int
fill_customer_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;
	cmdb_customer_s *cust;

	if (!(cust = malloc(sizeof(cmdb_customer_s))))
		report_error(MALLOC_FAIL, "cust in fill_customer_values");
	cmdb_init_customer_t(cust);
	cmdb->customer = cust;
	if (cm->address) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->address, ADDRESS_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "address");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(cust->address, NAME_S, "%s", cm->address);
	} else {
		fprintf(stderr, "No address supplied. Setting to none\n");
		snprintf(cust->address, COMM_S, "none");
	}
	if (cm->city) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->city, ADDRESS_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "city");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(cust->city, HOST_S, "%s", cm->city);
	} else {
		fprintf(stderr, "No city supplied. Setting to none\n");
		snprintf(cust->city, COMM_S, "none");
	}
	if (cm->county) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->county, ADDRESS_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "county");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(cust->county, MAC_S, "%s", cm->county);
	} else {
		fprintf(stderr, "No county supplied. Setting to none\n");
		snprintf(cust->county, COMM_S, "none");
	}
	if (cm->postcode) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->postcode, POSTCODE_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "postcode");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(cust->postcode, RANGE_S, "%s", cm->postcode);
	} else {
		fprintf(stderr, "No postcode supplied. Setting to none\n");
		snprintf(cust->postcode, COMM_S, "none");
	}
	if (cm->coid) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->coid, COID_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "coid");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(cust->coid, RANGE_S, "%s", cm->coid);
	} else {
		clean_cmdb_comm_line(cm);
		cmdb_clean_list(cmdb);
		return NO_COID;
	}
	if (cm->name) {
#ifdef HAVE_LIBPCRE
		if ((retval = validate_user_input(cm->name, CUSTOMER_REGEX)) < 0)
			report_error(USER_INPUT_INVALID, "customer name");
		else
			retval = NONE;
#endif /* HAVE_LIBPCRE */
		snprintf(cust->name, HOST_S, "%s", cm->name);
	} else {
		clean_cmdb_comm_line(cm);
		cmdb_clean_list(cmdb);
		return NO_NAME;
	}
	return retval;
}

int
fill_service_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;

	return retval;
}
