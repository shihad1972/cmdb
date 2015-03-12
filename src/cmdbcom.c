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
 *  cmdbcom.c
 *
 *  Contains the functions to deal with command line arguments and also to
 *  read the values from the configuration file
 *
 *  Part of the CMDB program
 *
 */

#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif /* HAVE_WORDEXP_H */
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comp, cmdb_s *base)
{
	int opt, retval = NONE;

	comp->config = strndup("/etc/dnsa/dnsa.conf", CONF_S);
	while ((opt = getopt(argc, argv,
	 "n:i:m:V:M:O:C:U:A:T:Y:Z:N:P:E:D:L:B:I:S:H:adefhlorstuvx")) != -1) {
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
		} else if (opt == 'a') {
			comp->action = ADD_TO_DB;
		} else if (opt == 'r') {
			comp->action = RM_FROM_DB;
		} else if (opt == 'x') {
			comp->action = MODIFY;
		} else if (opt == 'f') {
			comp->force = 1;
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
			comp->url = strndup(optarg, RBUFF_S);
		} else if (opt == 'B') {
			comp->device = strndup(optarg, MAC_S);
		} else if (opt == 'S') {
			comp->service = strndup(optarg, RANGE_S);
		} else if (opt == 'H') {
			comp->hclass = strndup(optarg, MAC_S);
		} else {
			return DISPLAY_USAGE;
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
	if ((!(comp->name)) && (!(comp->id)) && (comp->type == 0) && 
		(comp->action == 0))
		retval = DISPLAY_USAGE;
	else if (comp->action == CVERSION)
		retval = CVERSION;
	else if ((comp->type == NONE) && (comp->action != NONE))
		retval = NO_TYPE;
	else if ((comp->action == NONE) && (comp->type != NONE))
		retval = NO_ACTION;
	else if ((comp->action == NONE) && (comp->type == NONE)) {
		retval = NO_ACTION;
	} else if (comp->action == LIST_OBJ) {
		if (comp->type == CONTACT)
			if ((!(comp->id)) && (!(comp->coid)))
				retval = NO_COID;
	} else if ((!(comp->name)) && (!(comp->id)) &&
		(comp->type != NONE || comp->action != NONE) &&
		(comp->type != CONTACT))
		retval = NO_NAME_OR_ID;
	else if (comp->action == ADD_TO_DB) {
		if (comp->type == SERVER) {
			retval = fill_server_values(comp, base);
			complete_server_values(base, retval);
		} else if (comp->type == CUSTOMER) {
			retval = fill_customer_values(comp, base);
		} else if (comp->type == SERVICE) {
			retval = fill_service_values(comp, base);
		} else if (comp->type == CONTACT) {
			retval = fill_contact_values(comp, base);
		} else if (comp->type == HARDWARE) {
			retval = fill_hardware_values(comp, base);
		} else if (comp->type == VM_HOST) {
			retval = fill_vmhost_values(comp, base);
		}
	} else if (comp->action == DISPLAY) {
		if (comp->type == CONTACT) {
			if ((!(comp->id)) && (!(comp->coid)))
				retval = NO_COID;
		} else if (!comp->name) {
			retval = NO_NAME;
		}
	} else if (comp->action == RM_FROM_DB) {
		if (comp->type == SERVICE) {
			if ((!(comp->id)) && (!(comp->coid)) && (!(comp->name)))
				retval = NO_NAME_OR_ID;
			else if ((!(comp->service)) && (!(comp->url)) && (comp->force != 1))
				retval = NO_SERVICE_URL;
		}
	}
	return retval;
}

int
check_for_comm_line_errors(int cl, cmdb_comm_line_s *cm)
{
	int retval = NONE;

	if (cl == DISPLAY_USAGE)
		retval = DISPLAY_USAGE;
	else if (cl == CVERSION)
		retval = CVERSION;
	else if ((cl == NO_NAME_OR_ID) && (cm->action != LIST_OBJ))
		retval = NO_NAME_OR_ID;
	else if ((cl == NO_NAME) && (cm->action != DISPLAY))
		retval = NO_NAME;
	else if (cl == NO_SERVICE_URL)
		retval = NO_SERVICE_URL;
	else if ((cl & NO_COID) && (cm->action == ADD_TO_DB) && (cm->type = CONTACT))
		retval = NO_COID;
	else if (cm->type == SERVICE) {
		if (cm->action == ADD_TO_DB) {
			if ((cl & NO_COID) && (cl & NO_NAME))
				retval = NO_NAME_COID;
			else if (cl & NO_COID)
				retval = NO_COID;
			else if (cl & NO_NAME)
				retval = NO_NAME;
			else if (cl & NO_SERVICE)
				retval = NO_SERVICE;
		} else if (cm->action == DISPLAY) {
			if ((cl & NO_COID) && (cl & NO_NAME))
				retval = NO_NAME_COID;
		}
	} else if (cm->type == CONTACT) {
		if (cl & NO_COID)
			retval = NO_COID;
	} else if (cl == NO_TYPE)
		retval = cl;
	return retval;
}

int
parse_cmdb_config_file(cmdb_config_s *dc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	int retval;
#ifdef HAVE_WORDEXP_H
	char **uconf;
	wordexp_t p;
#endif /* HAVE_WORDEXP_H */
	
	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		retval = CONF_ERR;
	} else {
		retval = read_cmdb_config_values(dc, cnf);
		fclose(cnf);
	}
#ifdef HAVE_WORDEXP
	if ((retval = wordexp("~/.dnsa.conf", &p, 0)) == 0) {
		uconf = p.we_wordv;
		if ((cnf = fopen(*uconf, "r"))) {
			if ((retval = read_cmdb_config_values(dc, cnf)) != 0)
				retval = UPORT_ERR;
			fclose(cnf);
		}
		wordfree(&p);
	}
#endif /* HAVE_WORDEXP */
	return retval;
}

int
read_cmdb_config_values(cmdb_config_s *dc, FILE *cnf)
{
	char buff[CONF_S] = "";
	char port[CONF_S] = "";
	int retval = 0;
	unsigned long int portno;
	
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
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
	} else {
		dc->port = (unsigned int) portno;
	}
	return retval;
}

void
init_cmdb_comm_line_values(cmdb_comm_line_s *cm)
{
	memset(cm, 0, sizeof *cm);
}

void
init_cmdb_config_values(cmdb_config_s *dc)
{
	memset(dc, 0, sizeof *dc);
	snprintf(dc->dbtype, RANGE_S, "none");
	snprintf(dc->file, CONF_S, "none");
	snprintf(dc->db, CONF_S, "cmdb");
	snprintf(dc->user, CONF_S, "root");
	snprintf(dc->host, CONF_S, "localhost");
	snprintf(dc->pass, CONF_S, "%s", "");
	snprintf(dc->socket, CONF_S, "%s", "");
	dc->port = 3306;
}

void
cmdb_setup_config(cmdb_config_s **cf, cmdb_comm_line_s **com, cmdb_s **cmdb)
{
	if (!(*cf = malloc(sizeof(cmdb_config_s))))
		report_error(MALLOC_FAIL, "cf in cmdb_setup_config");
	init_cmdb_config_values(*cf);
	if (!(*com = malloc(sizeof(cmdb_comm_line_s))))
		report_error(MALLOC_FAIL, "com in cmdb_setup_config");
	init_cmdb_comm_line_values(*com);
	if (!(*cmdb = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "cbc in cmdb_setup_config");
	cmdb_init_struct(*cmdb);
}

void
cmdb_init_struct(cmdb_s *cmdb)
{
	memset(cmdb, 0, sizeof *cmdb);
}

void
cmdb_init_server_t(cmdb_server_s *server)
{
	memset(server, 0, sizeof *server);
	snprintf(server->vendor, COMM_S, "NULL");
	snprintf(server->make, COMM_S, "NULL");
	snprintf(server->model, COMM_S, "NULL");
	snprintf(server->uuid, COMM_S, "NULL");
	snprintf(server->name, COMM_S, "NULL");
}

void
cmdb_init_customer_t(cmdb_customer_s *cust)
{
	memset(cust, 0, sizeof *cust);
	snprintf(cust->address, COMM_S, "NULL");
	snprintf(cust->city, COMM_S, "NULL");
	snprintf(cust->county, COMM_S, "NULL");
	snprintf(cust->postcode, COMM_S, "NULL");
	snprintf(cust->coid, COMM_S, "NULL");
	snprintf(cust->name, COMM_S, "NULL");
}

void
cmdb_init_service_t(cmdb_service_s *service)
{
	memset(service, 0, sizeof *service);
	snprintf(service->detail, COMM_S, "NULL");
	snprintf(service->url, COMM_S, "NULL");
}

void
cmdb_init_hardware_t(cmdb_hardware_s *hard)
{
	memset(hard, 0, sizeof *hard);
	snprintf(hard->detail, COMM_S, "NULL");
	snprintf(hard->device, COMM_S, "NULL");
}

void
cmdb_init_contact_t(cmdb_contact_s *cont)
{
	memset (cont, 0, sizeof *cont);
	snprintf(cont->name, COMM_S, "NULL");
	snprintf(cont->phone, COMM_S, "NULL");
	snprintf(cont->email, COMM_S, "NULL");
}

void
cmdb_init_hardtype_t(cmdb_hard_type_s *type)
{
	memset(type, 0, sizeof(cmdb_hard_type_s));
	snprintf(type->type, COMM_S, "NULL");
	snprintf(type->hclass, COMM_S, "NULL");
}

void
cmdb_init_servicetype_t(cmdb_service_type_s *type)
{
	memset(type, 0, sizeof(cmdb_service_type_s));
	snprintf(type->service, COMM_S, "NULL");
	snprintf(type->detail, COMM_S, "NULL");
}

void
cmdb_init_vmhost_t(cmdb_vm_host_s *type)
{
	memset(type, 0, sizeof(cmdb_vm_host_s));
	snprintf(type->name, COMM_S, "NULL");
	snprintf(type->type, COMM_S, "NULL");
}

void
cmdb_main_free(cmdb_comm_line_s *cm, cmdb_config_s *cmc, char *cmdb_config)
{
	clean_cmdb_comm_line(cm);
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
			next = NULL;
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
			next = NULL;
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
			next = NULL;
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
			next = NULL;
		}
	}
}

void
clean_service_type_list(cmdb_service_type_s *list)
{
	cmdb_service_type_s *service, *next;

	service = list;
	next = service->next;
	while (service) {
		free(service);
		service = next;
		if (next) {
			next = service->next;
		} else {
			next = NULL;
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
			next = NULL;
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
			next = NULL;
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
			next = NULL;
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
		if (validate_user_input(cm->vendor, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vendor");
#endif /* HAVE_LIBPCRE */
		snprintf(server->vendor, CONF_S, "%s", cm->vendor);
	} else {
		retval = retval | NO_VENDOR;
	}
	if (cm->make) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->make, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "make");
#endif /* HAVE_LIBPCRE */
		snprintf(server->make, CONF_S, "%s", cm->make);
	} else {
		retval = retval | NO_MAKE;
	}
	if (cm->model) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->model, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "model");
#endif /* HAVE_LIBPCRE */
		snprintf(server->model, CONF_S, "%s", cm->model);
	} else {
		retval = retval | NO_MODEL;
	}
	if (cm->uuid) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->uuid, UUID_REGEX) < 0)
			if (validate_user_input(cm->uuid, FS_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "uuid");
#endif /* HAVE_LIBPCRE */
		snprintf(server->uuid, HOST_S, "%s", cm->uuid);
	} else {
		retval = retval | NO_UUID;
	}
	if (cm->name) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "name");
#endif /* HAVE_LIBPCRE */
		snprintf(server->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	if (cm->coid) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->coid, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "coid");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->coid, RANGE_S, "%s", cm->coid);
	} else {
		retval = retval | NO_COID;
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
		if (validate_user_input(cm->address, ADDRESS_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "address");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->address, NAME_S, "%s", cm->address);
	} else {
		retval = retval | CBC_NO_ADDRESS;
	}
	if (cm->city) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->city, ADDRESS_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "city");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->city, HOST_S, "%s", cm->city);
	} else {
		retval = retval | NO_CITY;
	}
	if (cm->county) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->county, ADDRESS_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "county");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->county, MAC_S, "%s", cm->county);
	} else {
		retval = retval | NO_COUNTY;
	}
	if (cm->postcode) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->postcode, POSTCODE_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "postcode");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->postcode, RANGE_S, "%s", cm->postcode);
	} else {
		retval = retval | NO_POSTCODE;
	}
	if (cm->coid) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->coid, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "coid");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->coid, RANGE_S, "%s", cm->coid);
	} else {
		retval = retval | NO_COID;
	}
	if (cm->name) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->name, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "customer name");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	return retval;
}

int
fill_service_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;
	cmdb_service_s *servi;

	if (!(servi = malloc(sizeof(cmdb_service_s))))
		report_error(MALLOC_FAIL, "servi in fill_service_values");
	cmdb_init_service_t(servi);
	cmdb->service = servi;
	if (cm->detail) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->detail, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service detail");
#endif /* HAVE_LIBPCRE */
		snprintf(servi->detail, HOST_S, "%s", cm->detail);
	} else {
		retval = retval | NO_DETAIL;
	}
	if (cm->url) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->url, URL_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service url");
#endif /* HAVE_LIBPCRE */
		snprintf(servi->url, URL_S, "%s", cm->url);
	} else {
		retval = retval | NO_URL;
	}
	if (cm->service) {
		cmdb_service_type_s *stype;
		if (!(stype = malloc(sizeof(cmdb_service_type_s))))
			report_error(MALLOC_FAIL, "stype in fill_service_values");
		cmdb_init_servicetype_t(stype);
		cmdb->servicetype = stype;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->service, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service service");
#endif /* HAVE_LIBPCRE */
		snprintf(stype->service, RANGE_S, "%s", cm->service);
	} else {
		retval = retval | NO_SERVICE;
	}
	if (cm->name) {
		cmdb_server_s *server;
		if (!(server = malloc(sizeof(cmdb_server_s))))
			report_error(MALLOC_FAIL, "server in fill_service_values");
		cmdb_init_server_t(server);
		cmdb->server = server;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service name");
#endif /* HAVE_LIBPCRE */
		snprintf(server->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	if (cm->coid) {
		cmdb_customer_s *cust;
		if (!(cust = malloc(sizeof(cmdb_customer_s))))
			report_error(MALLOC_FAIL, "cust in fill_service_values");
		cmdb_init_customer_t(cust);
		cmdb->customer = cust;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->coid, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service coid");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->coid, RANGE_S, "%s", cm->coid);
	} else if (cm->id) {
		cmdb_customer_s *cust;
		if (!(cust = malloc(sizeof(cmdb_customer_s))))
			report_error(MALLOC_FAIL, "cust in fill_service_values");
		cmdb_init_customer_t(cust);
		cmdb->customer = cust;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->id, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service coid");
#endif /* HAVE_LIBPCRE */
		snprintf(cust->coid, RANGE_S, "%s", cm->id);
	} else {
		retval = retval | NO_COID;
	}
	if (cm->sid != 0)
		servi->service_id = cm->sid;
	else
		retval = retval | NO_ID;
	return retval;
}

int
fill_contact_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;
	cmdb_contact_s *cont;

	if (!(cont = malloc(sizeof(cmdb_contact_s))))
		report_error(MALLOC_FAIL, "cont in fill_contact_values");
	cmdb_init_contact_t(cont);
	cmdb->contact = cont;
	if (cm->name) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->name, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact name");
#endif /* HAVE_LIBPCRE */
		snprintf(cont->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	if (cm->phone) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->phone, PHONE_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact phone");
#endif /* HAVE_LIBPCRE */
		snprintf(cont->phone, MAC_S, "%s", cm->phone);
	} else {
		retval = retval | NO_PHONE;
	}
	if (cm->email) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->email, EMAIL_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact email");
#endif /* HAVE_LIBPCRE */
		snprintf(cont->email, HOST_S, "%s", cm->email);
	} else {
		retval = retval | NO_EMAIL;
	}
	if (cm->id) {
		cmdb_customer_s *cust;
		if (!(cust = malloc(sizeof(cmdb_customer_s))))
			report_error(MALLOC_FAIL, "cust in fill_contact_values");
		cmdb_init_customer_t(cust);
		cmdb->customer = cust;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->id, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact COID");
#endif /*HAVE_LIBPCRE */
		snprintf(cust->coid, RANGE_S, "%s", cm->id);
	} else {
		retval = retval | NO_COID;
	}
	return retval;
}

int
fill_hardware_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;
	cmdb_hardware_s *hard;

	if (!(hard = malloc(sizeof(cmdb_hardware_s))))
		report_error(MALLOC_FAIL, "hard in fill_hardware_values");
	cmdb_init_hardware_t(hard);
	cmdb->hardware = hard;
	if (cm->detail) {
#ifdef HAVE_LIBPCRE
		/* Hard coded net device. Should change this sometime */
		if (cm->sid == 1) {
			if (validate_user_input(cm->detail, MAC_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "hardware detail");
		} else {
			if (validate_user_input(cm->detail, ADDRESS_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "hardware detail");
		}
#endif /* HAVE_LIBPCRE */
		snprintf(hard->detail, HOST_S, "%s", cm->detail);
	} else {
		retval = retval | NO_DETAIL;
	}
	if (cm->device) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->device, DEV_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "hardware device");
#endif /* HAVE_LIBPCRE */
		snprintf(hard->device, MAC_S, "%s", cm->device);
	} else {
		retval = retval | NO_DEVICE;
	}
	if (cm->name) {
		cmdb_server_s *server;
		if (!(server = malloc(sizeof(cmdb_server_s))))
			report_error(MALLOC_FAIL, "server in fill_hardware_values");
		cmdb_init_server_t(server);
		cmdb->server = server;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "hardware server");
#endif /* HAVE_LIBPCRE */
		snprintf(server->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval |  NO_NAME;
	}
	if (cm->sid != 0)
		hard->ht_id = cm->sid;
	else
		retval = retval | NO_ID;
	return retval;
}

int
fill_vmhost_values(cmdb_comm_line_s *cm, cmdb_s *cmdb)
{
	int retval = NONE;
	cmdb_vm_host_s *vmhost;

	if (!(vmhost = malloc(sizeof(cmdb_vm_host_s))))
		report_error(MALLOC_FAIL, "vmhost in fill_vmhost_values");
	cmdb_init_vmhost_t(vmhost);
	cmdb->vmhost = vmhost;
	if (cm->vmhost) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->vmhost, DOMAIN_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vmhost hostname");
#endif /* HAVE_LIBPCRE */
		snprintf(vmhost->name, RBUFF_S, "%s", cm->vmhost);
	} else {
		retval = retval | NO_VHOST;
	}
	if (cm->model) {
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->model, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vmhost type");
#endif /* HAVE_LIBPCRE */
		snprintf(vmhost->type, MAC_S, "%s", cm->model);
	} else {
		retval = retval | NO_TYPE;
	}
	if (cm->name) {
		cmdb_server_s *server;
		if (!(server = malloc(sizeof(cmdb_server_s))))
			report_error(MALLOC_FAIL, "server in fill_vmhost_values");
		cmdb_init_server_t(server);
		cmdb->server = server;
#ifdef HAVE_LIBPCRE
		if (validate_user_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vmhost name");
#endif /* HAVE_LIBPCRE */
		snprintf(server->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	return retval;
}

void
complete_server_values(cmdb_s *cmdb, int cl)
{
	if (cl & NO_VENDOR)
		snprintf(cmdb->server->vendor, COMM_S, "none");
	if (cl & NO_MAKE)
		snprintf(cmdb->server->make, COMM_S, "none");
	if (cl & NO_MODEL)
		snprintf(cmdb->server->model, COMM_S, "none");
	if (cl & NO_UUID)
		snprintf(cmdb->server->uuid, COMM_S, "none");
}
