/* 
 *
 *  cmdb : Configuration Management Database
 *  Copyright (C) 2016 - 2020 Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cmdb.c
 *
 *  Contains main functions for cmdb2 program
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#include <cmdb_cmdb.h>

static int
cmdb_server_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_customer_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_contact_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_service_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_service_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_hardware_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_hardware_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
cmdb_vm_host_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc);

static int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comp);

static int
check_cmdb_comm_options(cmdb_comm_line_s *comp);

static int
validate_cmdb_comm_line(cmdb_comm_line_s *comp);

static void
clean_cmdb_comm_line(cmdb_comm_line_s *list);

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_start_syslog(basename(argv[0]));
	cmdb_comm_line_s *cm = ailsa_calloc(sizeof(cmdb_comm_line_s), "cm in main");
	size_t len = sizeof(ailsa_cmdb_s);
	ailsa_cmdb_s *cc = ailsa_calloc(len, "cc in main");

	if ((retval = parse_cmdb_command_line(argc, argv, cm)) != 0) {
		display_command_line_error(retval, argv[0]);
		goto cleanup;
	}
	parse_cmdb_config(cc);
	switch(cm->type) {
	case SERVER:
		retval = cmdb_server_actions(cm, cc);
		break;
	case CUSTOMER:
		retval = cmdb_customer_actions(cm, cc);
		break;
	case CONTACT:
		retval = cmdb_contact_actions(cm, cc);
		break;
	case SERVICE:
		retval = cmdb_service_actions(cm, cc);
		break;
	case SERVICE_TYPE:
		retval = cmdb_service_type_actions(cm, cc);
		break;
	case HARDWARE:
		retval = cmdb_hardware_actions(cm, cc);
		break;
	case HARDWARE_TYPE:
		retval = cmdb_hardware_type_actions(cm, cc);
		break;
	case VM_HOST:
		retval = cmdb_vm_host_actions(cm, cc);
		break;
	default:
		ailsa_syslog(LOG_ERR, "Unknown type %d", cm->type);
		goto cleanup;
	}

	cleanup:
		ailsa_clean_cmdb(cc);
		clean_cmdb_comm_line(cm);
		return retval;
}

static int
cmdb_server_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	switch(cm->action) {
	case CMDB_ADD:
		retval = cmdb_add_server_to_database(cm, cc);
		break;
	case CMDB_LIST:
		cmdb_list_servers(cc);
		break;
	case CMDB_DISPLAY:
		cmdb_display_server(cm, cc);
		break;
	case CMDB_RM:
		retval = cmdb_remove_server_from_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_customer_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	switch(cm->action) {
	case CMDB_ADD:
		retval = cmdb_add_customer_to_database(cm, cc);
		break;
	case CMDB_LIST:
		cmdb_list_customers(cc);
		break;
	case CMDB_DISPLAY:
		cmdb_display_customer(cm, cc);
		break;
	case CMDB_DEFAULT:
		retval = cmdb_set_default_customer(cm, cc);
		break;
	case CMDB_VIEW_DEFAULT:
		cmdb_display_default_customer(cc);
		break;
	case CMDB_RM:
		retval = cmdb_remove_customer_from_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_contact_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) | !(cc))
		return AILSA_NO_DATA;
	switch(cm->action) {
	case CMDB_ADD:
		retval = cmdb_add_contacts_to_database(cm, cc);
		break;
	case CMDB_LIST:
		cmdb_list_contacts_for_customer(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_service_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) | !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case CMDB_LIST:
		cmdb_list_services_for_server(cm, cc);
		break;
	case CMDB_ADD:
		cmdb_add_services_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_hardware_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case CMDB_LIST:
		cmdb_list_hardware_for_server(cm, cc);
		break;
	case CMDB_ADD:
		cmdb_add_hardware_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_vm_host_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case CMDB_LIST:
		cmdb_list_vm_server_hosts(cc);
		break;
	case CMDB_DISPLAY:
		cmdb_display_vm_server(cm, cc);
		break;
	case CMDB_ADD:
		retval = cmdb_add_vm_host_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_service_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case CMDB_LIST:
		cmdb_list_service_types(cc);
		break;
	case CMDB_ADD:
		retval = cmdb_add_service_type_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
cmdb_hardware_type_actions(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
	int retval = 0;
	switch(cm->action) {
	case CMDB_LIST:
		cmdb_list_hardware_types(cc);
		break;
	case CMDB_ADD:
		retval = cmdb_add_hardware_type_to_database(cm, cc);
		break;
	default:
		display_type_error(cm->type);
		retval = AILSA_WRONG_TYPE;
		break;
	}
	return retval;
}

static int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comp)
{
	const char *optstr = "i:k:n:x:y:A:B:C:D:E:H:I:L:M:N:O:P:S:T:U:V:Y:Z:adefghjlmorqstuvwz";
	int opt, retval;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"display",		no_argument, 		NULL,	'd'},
		{"service",		no_argument,		NULL,	'e'},
		{"force",		no_argument,		NULL,	'f'},
		{"hardtype",		no_argument,		NULL,	'g'},
		{"hard-type",		no_argument,		NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"identity",		required_argument,	NULL,	'i'},
		{"service-type",	no_argument,		NULL,	'j'},
		{"servicetype",		no_argument,		NULL,	'j'},
		{"alias",		required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"name",		required_argument,	NULL,	'n'},
		{"vm",			no_argument,		NULL,	'o'},
		{"view-default",	no_argument,		NULL,	'q'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"server",		no_argument,		NULL,	's'},
		{"contact",		no_argument,		NULL,	't'},
		{"customer",		no_argument,		NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"hardware",		no_argument,		NULL,	'w'},
		{"virtmachine",		required_argument,	NULL,	'x'},
		{"type",		required_argument,	NULL,	'y'},
		{"set-default",		no_argument,		NULL,	'z'},
		{"address",		required_argument,	NULL,	'A'},
		{"device",		required_argument,	NULL,	'B'},
		{"coid",		required_argument,	NULL,	'C'},
		{"detail",		required_argument,	NULL,	'D'},
		{"description",		required_argument,	NULL,	'D'},
		{"email",		required_argument,	NULL,	'E'},
		{"class",		required_argument,	NULL,	'H'},
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
#ifdef HAVE_GETOPT_H
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		switch (opt) {
		case 's':
			comp->type = SERVER;
			break;
		case 'u':
			comp->type = CUSTOMER;
			break;
		case 't':
			comp->type = CONTACT;
			break;
		case 'e':
			comp->type = SERVICE;
			break;
		case 'j':
			comp->type = SERVICE_TYPE;
			break;
		case 'w':
			comp->type = HARDWARE;
			break;
		case 'g':
			comp->type = HARDWARE_TYPE;
			break;
		case 'o':
			comp->type = VM_HOST;
			break;
		case 'd':
			comp->action = CMDB_DISPLAY;
			break;
		case 'v':
			comp->action = AILSA_VERSION;
			break;
		case 'l':
			comp->action = CMDB_LIST;
			break;
		case 'a':
			comp->action = CMDB_ADD;
			break;
		case 'r':
			comp->action = CMDB_RM;
			break;
		case 'z':
			comp->action = CMDB_DEFAULT;
			break;
		case 'q':
			comp->action = CMDB_VIEW_DEFAULT;
			break;
		case 'm':
			comp->action = CMDB_MOD;
			break;
		case 'h':
			return AILSA_DISPLAY_USAGE;
		case 'f':
			comp->force = 1;
			break;
		case 'n':
			comp->name = strndup(optarg, HOST_LEN);
			break;
		case 'i':
			comp->id = strndup(optarg, CONFIG_LEN);
			break;
		case 'x':
			comp->vmhost = strndup(optarg, HOST_LEN);
			break;
		case 'y':
			comp->shtype = strndup(optarg, MAC_LEN);
			break;
		case 'V':
			comp->vendor = strndup(optarg, CONFIG_LEN);
			break;
		case 'M':
			comp->make = strndup(optarg, CONFIG_LEN);
			break;
		case 'O':
			comp->model = strndup(optarg, CONFIG_LEN);
			break;
		case 'U':
			comp->uuid = strndup(optarg, CONFIG_LEN);
			break;
		case 'C':
			comp->coid = strndup(optarg, SERVICE_LEN);
			break;
		case 'A':
			comp->address = strndup(optarg, CONFIG_LEN);
			break;
		case 'T':
			comp->city = strndup(optarg, HOST_LEN);
			break;
		case 'Y':
			comp->county = strndup(optarg, MAC_LEN);
			break;
		case 'Z':
			comp->postcode = strndup(optarg, SERVICE_LEN);
			break;
		case 'N':
			comp->fullname = strndup(optarg, HOST_LEN);
			break;
		case 'P':
			comp->phone = strndup(optarg, MAC_LEN);
			break;
		case 'E':
			comp->email = strndup(optarg, HOST_LEN);
			break;
		case 'D':
			comp->detail = strndup(optarg, HOST_LEN);
			break;
		case 'I':
			comp->sid = strtoul(optarg, NULL, 10);
			break;
		case 'L':
			comp->url = strndup(optarg, CONFIG_LEN);
			break;
		case 'B':
			comp->device = strndup(optarg, MAC_LEN);
			break;
		case 'S':
			comp->service = strndup(optarg, SERVICE_LEN);
			break;
		case 'H':
			comp->hclass = strndup(optarg, MAC_LEN);
			break;
		default:
			return AILSA_DISPLAY_USAGE;
		}
	}

	if ((retval = check_cmdb_comm_options(comp)) != 0)
		return retval;
	retval = validate_cmdb_comm_line(comp);
	return retval;
}

static int
check_cmdb_comm_options(cmdb_comm_line_s *comp)
{
	int retval;

	retval = NONE;
	if ((!(comp->name)) && (!(comp->id)) && (comp->type == 0) && 
		(comp->action == 0))
		retval = AILSA_DISPLAY_USAGE;
	else if (comp->action == AILSA_VERSION)
		retval = AILSA_VERSION;
	else if (comp->action == NONE)
		retval = AILSA_NO_ACTION;
	else if (comp->type == NONE)
		retval = AILSA_NO_TYPE;
	else if (comp->action == CMDB_LIST) {
		if (comp->type == CONTACT) {
			if ((!(comp->id)) && (!(comp->coid)))
				retval = AILSA_NO_COID;
		} else if (comp->type == SERVICE) {
			if (!(comp->id) && !(comp->name))
				retval = AILSA_NO_NAME_OR_ID;
		} else if (comp->type == HARDWARE) {
			if (!(comp->id) && !(comp->name))
				retval = AILSA_NO_NAME_OR_ID;
		}
	} else if (comp->action == CMDB_ADD) {
		if (comp->type == CONTACT) {
			if (!(comp->fullname))
				retval = AILSA_NO_CONTACT_NAME;
			else if (!(comp->email))
				retval = AILSA_NO_EMAIL_ADDRESS;
			else if (!(comp->phone))
				retval = AILSA_NO_PHONE_NUMBER;
		} else if (comp->type == SERVER) {
			if (!(comp->name))
				retval = AILSA_NO_NAME;
			if (!(comp->make))
				comp->make = strdup("none");
			if (!(comp->model))
				comp->model = strdup("none");
			if (!(comp->vendor))
				comp->vendor = strdup("none");
			if (!(comp->uuid)) {
				ailsa_syslog(LOG_INFO, "Auto generating UUID for server");
				comp->uuid = ailsa_gen_uuid_str();
			}
		} else if (comp->type == SERVICE_TYPE) {
			if (!(comp->detail))
				retval = AILSA_NO_DETAIL;
			else if (!(comp->service))
				retval = AILSA_NO_SERVICE;
		} else if (comp->type == HARDWARE_TYPE) {
			if (!(comp->hclass))
				retval = AILSA_NO_CLASS;
			else if (!(comp->shtype))
				retval = AILSA_NO_TYPE;
		} else if (comp->type == VM_HOST) {
			if (!(comp->name))
				retval = AILSA_NO_NAME;
			else if (!(comp->shtype))
				retval = AILSA_NO_VHOST_TYPE;
		} else if (comp->type == HARDWARE) {
			if (!(comp->name))
				retval = AILSA_NO_NAME;
			else if (!(comp->detail))
				retval = AILSA_NO_DETAIL;
			else if (!(comp->device))
				retval = AILSA_NO_DEVICE;
			else if (!(comp->hclass) && !(comp->sid))
				retval = AILSA_NO_ID_OR_CLASS;
		} else if (comp->type == SERVICE) {
			if (!(comp->name))
				retval = AILSA_NO_NAME;
			else if (!(comp->detail))
				retval = AILSA_NO_DETAIL;
			else if (!(comp->url))
				retval = AILSA_NO_SERVICE_URL;
			else if (!(comp->service))
				retval = AILSA_NO_SERVICE;
		} else if (comp->type == CUSTOMER) {
			if (!(comp->fullname))
				retval = AILSA_NO_FULLNAME;
			else if (!(comp->coid))
				retval = AILSA_NO_COID;
			else if (!(comp->county))
				comp->county = strdup("none");
			else if (!(comp->address))
				comp->address = strdup("none");
			else if (!(comp->city))
				comp->city = strdup("none");
			else if (!(comp->postcode))
				comp->postcode = strdup("none");
		}
	} else if (comp->action == CMDB_DISPLAY) {
		if ((comp->type != SERVER) && (comp->type != CUSTOMER) && (comp->type != VM_HOST)) {
			retval = AILSA_WRONG_TYPE_DISPLAY;
		} else if (comp->type == CUSTOMER) {
			if (!comp->coid)
				retval = AILSA_NO_COID;
		} else if (!comp->name) {
			retval = AILSA_NO_NAME;
		}
	} else if (comp->action  == CMDB_VIEW_DEFAULT) {
		if (comp->type != CUSTOMER)
			retval = AILSA_WRONG_TYPE;
	} else if (comp->action == CMDB_DEFAULT) {
		if (!(comp->coid))
			retval = AILSA_NO_COID;
		else if (comp->type != CUSTOMER)
			retval = AILSA_WRONG_TYPE;
	} else if ((!(comp->name)) && (!(comp->id)) && 
		(comp->type != NONE || comp->action != NONE) &&
		((comp->type != CONTACT) && (comp->type != CUSTOMER))) {
		retval = AILSA_NO_NAME_OR_ID;
	} else if (comp->action == CMDB_RM) {
		if (comp->type == SERVICE) {
			if ((!(comp->id)) && (!(comp->coid)) && (!(comp->name)))
				retval = AILSA_NO_NAME_OR_ID;
			else if ((!(comp->service)) && (!(comp->url)) && (comp->force != 1))
				retval = AILSA_NO_SERVICE_URL;
		} else if (comp->type == SERVER) {
			if (!(comp->name))
				retval = AILSA_NO_NAME;
		} else if (comp->type == HARDWARE) {
			if (!(comp->name))
				retval = AILSA_NO_NAME;
			else if ((!(comp->device)) && (!(comp->detail)))
				retval = AILSA_NO_DEVICE_DETAIL;
		}
	}
	return retval;
}

static int
validate_cmdb_comm_line(cmdb_comm_line_s *comp)
{
	if (!(comp))
		return AILSA_NO_DATA;

	if (comp->vmhost)
		if ((ailsa_validate_input(comp->vmhost, DOMAIN_REGEX) != 0) &&
		    (ailsa_validate_input(comp->vmhost, NAME_REGEX) != 0))
			return VMHOST_INVALID;
	if (comp->vendor)
		if (ailsa_validate_input(comp->vendor, CUSTOMER_REGEX) != 0)
			return VENDOR_INVALID;
	if (comp->make)
		if (ailsa_validate_input(comp->make, CUSTOMER_REGEX) != 0)
			return MAKE_INVALID;
	if (comp->model)
		if (ailsa_validate_input(comp->model, CUSTOMER_REGEX) != 0)
			return MODEL_INVALID;
	if (comp->uuid)
		if (ailsa_validate_input(comp->uuid, UUID_REGEX) != 0)
			return UUID_INPUT_INVALID;
	if (comp->name)
		if (ailsa_validate_input(comp->name, NAME_REGEX) != 0)
			return SERVER_NAME_INVALID;
	if (comp->fullname)
		if (ailsa_validate_input(comp->fullname, CUSTOMER_REGEX) != 0)
			return CUSTOMER_NAME_INVALID;
	if (comp->address)
		if (ailsa_validate_input(comp->address, ADDRESS_REGEX) != 0)
			return ADDRESS_INVALID;
	if (comp->city)
		if (ailsa_validate_input(comp->city, ADDRESS_REGEX) != 0)
			return CITY_INVALID;
	if (comp->email)
		if (ailsa_validate_input(comp->email, EMAIL_REGEX) != 0)
			return EMAIL_ADDRESS_INVALID;
	if (comp->detail)
		if ((ailsa_validate_input(comp->detail, MAC_REGEX) != 0) &&
		    (ailsa_validate_input(comp->detail, ADDRESS_REGEX) != 0) &&
		    (ailsa_validate_input(comp->detail, CUSTOMER_REGEX) != 0))
			return DETAIL_INVALID;
	if (comp->hclass)
		if (ailsa_validate_input(comp->hclass, CUSTOMER_REGEX) != 0)
			return HCLASS_INVALID;
	if (comp->url)
		if (ailsa_validate_input(comp->url, URL_REGEX) != 0)
			return URL_INVALID;
	if (comp->device)
		if (ailsa_validate_input(comp->device, DEV_REGEX) != 0)
			return DEVICE_INVALID;
	if (comp->phone)
		if (ailsa_validate_input(comp->phone, PHONE_REGEX) != 0)
			return PHONE_NUMBER_INVALID;
	if (comp->postcode)
		if (ailsa_validate_input(comp->postcode, POSTCODE_REGEX) != 0)
			return POSTCODE_INVALID;
	if (comp->county)
		if (ailsa_validate_input(comp->county, ADDRESS_REGEX) != 0)
			return COUNTY_INVALID;
	if (comp->coid)
		if (ailsa_validate_input(comp->coid, COID_REGEX) != 0)
			return COID_INVALID;
	if (comp->service)
		if (ailsa_validate_input(comp->service, NAME_REGEX) != 0)
			return SERVICE_INPUT_INVALID;
	if (comp->shtype)
		if (ailsa_validate_input(comp->shtype, ADDRESS_REGEX) != 0)
			return TYPE_INVALID;
	return 0;
}

static void
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
	CLEAN_COMM_LIST(list, shtype);
	CLEAN_COMM_LIST(list, hclass);
	CLEAN_COMM_LIST(list, url);
	CLEAN_COMM_LIST(list, device);
	CLEAN_COMM_LIST(list, phone);
	CLEAN_COMM_LIST(list, postcode);
	CLEAN_COMM_LIST(list, coid);
	CLEAN_COMM_LIST(list, service);
	CLEAN_COMM_LIST(list, uuid);
	CLEAN_COMM_LIST(list, county);
	CLEAN_COMM_LIST(list, fullname);
	free(list);
}

#ifdef CLEAN_COMM_LIST
# undef CLEAN_COMM_LIST
#endif /* CLEAN_COMM_LIST */
