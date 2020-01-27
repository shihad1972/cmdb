/* 
 * 
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb2com.c
 *
 *  Contains the functions to deal with command line arguments and also to
 *  read the values from the configuration file
 *
 *  Part of the CMDB program
 *
 */

#include <config.h>
#include <configmake.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <cmdb.h>
#include <cmdb_data.h>
#include <cmdb_sql.h>
#include <ailsacmdb.h>

static int
check_cmdb_comm_options(cmdb_comm_line_s *comp);

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comp)
{
	const char *optstr = "c:i:k:n:m:V:M:O:C:U:A:T:Y:Z:N:P:E:D:L:B:I:S:H:adefhlorstuvwxy";
	int opt, retval;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"config",		required_argument,	NULL,	'c'},
		{"display",		no_argument, 		NULL,	'd'},
		{"service",		no_argument,		NULL,	'e'},
		{"force",		no_argument,		NULL,	'f'},
		{"help",		no_argument,		NULL,	'h'},
		{"identity",		required_argument,	NULL,	'i'},
		{"alias",		required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"name",		required_argument,	NULL,	'n'},
		{"vm",			no_argument,		NULL,	'o'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"server",		no_argument,		NULL,	's'},
		{"contact",		no_argument,		NULL,	't'},
		{"customer",		no_argument,		NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"hardware",		no_argument,		NULL,	'w'},
		{"virtmachine",		required_argument,	NULL,	'x'},
		{"servertype",		no_argument,		NULL,	'y'},
		{"address",		required_argument,	NULL,	'A'},
		{"device",		required_argument,	NULL,	'B'},
		{"coid",		required_argument,	NULL,	'C'},
		{"detail",		required_argument,	NULL,	'D'},
		{"description",		required_argument,	NULL,	'D'},
		{"email",		required_argument,	NULL,	'E'},
		{"id",			required_argument,	NULL,	'I'},
		{"url",			required_argument,	NULL,	'L'},
		{"make",		required_argument,	NULL,	'M'},
		{"full-name",		required_argument,	NULL,	'N'},
		{"model",		required_argument,	NULL,	'O'},
		{"phone",		required_argument,	NULL,	'P'},
		{"service-name",	required_argument,	NULL,	'S'},
		{"city",		required_argument,	NULL,	'T'},
		{"uuid",		required_argument,	NULL,	'U'},
		{"vendor",		required_argument,	NULL,	'V'},
		{"county",		required_argument,	NULL,	'Y'},
		{"postcode",		required_argument,	NULL,	'Z'},
		{NULL, 0, NULL, 0}
	};
#endif // HAVE_GETOPT_H
	retval = 0;
	if (!(comp->config)) {
		comp->config = ailsa_calloc(CONF_S, "comp->config in parse_cmdb_command_line");
		get_config_file_location(comp->config);
	}
#ifdef HAVE_GETOPT_H
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 's')
			comp->type = SERVER;
		else if (opt == 'y')
			comp->type = SERVER_TYPE;
		else if (opt == 'u')
			comp->type = CUSTOMER;
		else if (opt == 't')
			comp->type = CONTACT;
		else if (opt == 'e')
			comp->type = SERVICE;
		else if (opt == 'w')
			comp->type = HARDWARE;
		else if (opt == 'o')
			comp->type = VM_HOST;
		else if (opt == 'd')
			comp->action = DISPLAY;
		else if (opt == 'v')
			comp->action = CVERSION;
		else if (opt == 'l')
			comp->action = LIST_OBJ;
		else if (opt == 'a')
			comp->action = ADD_TO_DB;
		else if (opt == 'r')
			comp->action = RM_FROM_DB;
		else if (opt == 'm')
			comp->action = MODIFY;
		else if (opt == 'h')
			return DISPLAY_USAGE;
		else if (opt == 'f')
			comp->force = 1;
		else if (opt == 'c')
			comp->config = strndup(optarg, RBUFF_S);
		else if (opt == 'n')
			comp->name = strndup(optarg, HOST_S);
		else if (opt == 'i')
			comp->id = strndup(optarg, CONF_S);
		else if (opt == 'x')
			comp->vmhost = strndup(optarg, HOST_S);
		else if (opt == 'V')
			comp->vendor = strndup(optarg, CONF_S);
		else if (opt == 'M')
			comp->make = strndup(optarg, CONF_S);
		else if (opt == 'O')
			comp->model = strndup(optarg, CONF_S);
		else if (opt == 'U')
			comp->uuid = strndup(optarg, CONF_S);
		else if (opt == 'C')
			comp->coid = strndup(optarg, RANGE_S);
		else if (opt == 'A')
			comp->address = strndup(optarg, NAME_S);
		else if (opt == 'T')
			comp->city = strndup(optarg, HOST_S);
		else if (opt == 'Y')
			comp->county = strndup(optarg, MAC_S);
		else if (opt == 'Z')
			comp->postcode = strndup(optarg, RANGE_S);
		else if (opt == 'N')
			comp->name = strndup(optarg, HOST_S);
		else if (opt == 'P')
			comp->phone = strndup(optarg, MAC_S);
		else if (opt == 'E')
			comp->email = strndup(optarg, HOST_S);
		else if (opt == 'D')
			comp->detail = strndup(optarg, HOST_S);
		else if (opt == 'I')
			comp->sid = strtoul(optarg, NULL, 10);
		else if (opt == 'L')
			comp->url = strndup(optarg, RBUFF_S);
		else if (opt == 'B')
			comp->device = strndup(optarg, MAC_S);
		else if (opt == 'S')
			comp->service = strndup(optarg, RANGE_S);
		else if (opt == 'H')
			comp->hclass = strndup(optarg, MAC_S);
		else
			return DISPLAY_USAGE;
	}

	retval = check_cmdb_comm_options(comp);
	return retval;
}

int
check_cmdb_comm_options(cmdb_comm_line_s *comp)
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
	else if ((comp->action == NONE) && (comp->type == NONE))
		retval = NO_ACTION;
	else if (comp->action == LIST_OBJ) {
		if (comp->type == CONTACT) {
			if ((!(comp->id)) && (!(comp->coid)))
				retval = NO_COID;
		}
	} else if ((!(comp->name)) && (!(comp->id)) &&
		(comp->type != NONE || comp->action != NONE) &&
		(comp->type != CONTACT))
		retval = NO_NAME_OR_ID;
	else if (comp->action == DISPLAY) {
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
		} else if (comp->type == SERVER) {
			if (!(comp->name))
				retval = NO_NAME;
		} else if (comp->type == HARDWARE) {
			if (!(comp->name))
				retval = NO_NAME;
			else if ((!(comp->device)) && (!(comp->detail)))
				retval = NO_DEVICE | NO_DETAIL;
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
	free(list);
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
	if (cm->name) {
		if (ailsa_validate_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "name");
		snprintf(server->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	if (cm->coid) {
		if (ailsa_validate_input(cm->coid, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "coid");
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
		if (ailsa_validate_input(cm->address, ADDRESS_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "address");
		snprintf(cust->address, NAME_S, "%s", cm->address);
	} else {
		retval = retval | CBC_NO_ADDRESS;
	}
	if (cm->city) {
		if (ailsa_validate_input(cm->city, ADDRESS_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "city");
		snprintf(cust->city, HOST_S, "%s", cm->city);
	} else {
		retval = retval | NO_CITY;
	}
	if (cm->county) {
		if (ailsa_validate_input(cm->county, ADDRESS_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "county");
		snprintf(cust->county, MAC_S, "%s", cm->county);
	} else {
		retval = retval | NO_COUNTY;
	}
	if (cm->postcode) {
		if (ailsa_validate_input(cm->postcode, POSTCODE_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "postcode");
		snprintf(cust->postcode, RANGE_S, "%s", cm->postcode);
	} else {
		retval = retval | NO_POSTCODE;
	}
	if (cm->coid) {
		if (ailsa_validate_input(cm->coid, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "coid");
		snprintf(cust->coid, RANGE_S, "%s", cm->coid);
	} else {
		retval = retval | NO_COID;
	}
	if (cm->name) {
		if (ailsa_validate_input(cm->name, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "customer name");
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
		if (ailsa_validate_input(cm->detail, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service detail");
		snprintf(servi->detail, HOST_S, "%s", cm->detail);
	} else {
		retval = retval | NO_DETAIL;
	}
	if (cm->url) {
		if (ailsa_validate_input(cm->url, URL_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service url");
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
		if (ailsa_validate_input(cm->service, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service service");
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
		if (ailsa_validate_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service name");
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
		if (ailsa_validate_input(cm->coid, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service coid");
		snprintf(cust->coid, RANGE_S, "%s", cm->coid);
	} else if (cm->id) {
		cmdb_customer_s *cust;
		if (!(cust = malloc(sizeof(cmdb_customer_s))))
			report_error(MALLOC_FAIL, "cust in fill_service_values");
		cmdb_init_customer_t(cust);
		cmdb->customer = cust;
		if (ailsa_validate_input(cm->id, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "service coid");
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
		if (ailsa_validate_input(cm->name, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact name");
		snprintf(cont->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	if (cm->phone) {
		if (ailsa_validate_input(cm->phone, PHONE_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact phone");
		snprintf(cont->phone, MAC_S, "%s", cm->phone);
	} else {
		retval = retval | NO_PHONE;
	}
	if (cm->email) {
		if (ailsa_validate_input(cm->email, EMAIL_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact email");
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
		if (ailsa_validate_input(cm->id, COID_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "contact COID");
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
		/* Hard coded net device. Should change this sometime */
		if (cm->sid == 1) {
			if (ailsa_validate_input(cm->detail, MAC_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "hardware detail");
		} else {
			if (ailsa_validate_input(cm->detail, ADDRESS_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "hardware detail");
		}
		snprintf(hard->detail, HOST_S, "%s", cm->detail);
	} else {
		retval = retval | NO_DETAIL;
	}
	if (cm->device) {
		if (ailsa_validate_input(cm->device, DEV_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "hardware device");
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
		if (ailsa_validate_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "hardware server");
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
		if (ailsa_validate_input(cm->vmhost, DOMAIN_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vmhost hostname");
		snprintf(vmhost->name, RBUFF_S, "%s", cm->vmhost);
	} else {
		retval = retval | NO_VHOST;
	}
	if (cm->model) {
		if (ailsa_validate_input(cm->model, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vmhost type");
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
		if (ailsa_validate_input(cm->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "vmhost name");
		snprintf(server->name, HOST_S, "%s", cm->name);
	} else {
		retval = retval | NO_NAME;
	}
	return retval;
}

void
cmdb_main_free(cmdb_comm_line_s *cm, cmdb_config_s *cmc, char *cmdb_config)
{
	clean_cmdb_comm_line(cm);
	free(cm);
	free(cmc);
	free(cmdb_config);
}
/*
int
get_table_id(cmdb_config_s *cbc, int query, char *name, unsigned long int *id)
{
	int retval = 0;
	dbdata_s *data;

	if (!(cbc) || (query < 0))
		return CBC_NO_DATA;
	unsigned int max = cmdb_get_max(cmdb_search_args[query], cmdb_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	if (name) {
		if (cmdb_search_args[query] == 0)
			fprintf(stderr, "Name %s passed for query with no args:\n%s\n",
			 name, sql_search[query]);
		snprintf(data->args.text, RBUFF_S, "%s", name);
	}
	if ((retval = cmdb_run_search(cbc, data, query)) == 0) {
		clean_dbdata_struct(data);
		return -1;
	} else if (retval > 1) {
// Returning the query here useful for debugging; not so much for the user.
		fprintf(stderr, "Multiple id's found for query:\n%s\n", sql_search[query]);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}
*/
