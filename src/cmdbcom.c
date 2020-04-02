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
#include <syslog.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <cmdb.h>
#include <cmdb_data.h>
#include <cmdb_sql.h>

static int
check_cmdb_comm_options(cmdb_comm_line_s *comp);

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_s *comp)
{
	const char *optstr = "c:i:k:n:m:x:y:A:B:C:D:E:H:I:L:M:N:O:P:S:T:U:V:Y:Z:adefhjlorstuvwz";
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
		{"service-type",	no_argument,		NULL,	'j'},
		{"servicetype",		no_argument,		NULL,	'j'},
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
		{"type",		required_argument,	NULL,	'y'},
		{"hardtype",		no_argument,		NULL,	'z'},
		{"hard-type",		no_argument,		NULL,	'z'},
		{"hardwaretype",	no_argument,		NULL,	'z'},
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
		else if (opt == 'u')
			comp->type = CUSTOMER;
		else if (opt == 't')
			comp->type = CONTACT;
		else if (opt == 'e')
			comp->type = SERVICE;
		else if (opt == 'j')
			comp->type = SERVICE_TYPE;
		else if (opt == 'w')
			comp->type = HARDWARE;
		else if (opt == 'z')
			comp->type = HARDWARE_TYPE;
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
		else if (opt == 'y')
			comp->shtype = strndup(optarg, MAC_S);
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
	else if (comp->action == NONE)
		retval = NO_ACTION;
	else if (comp->type == NONE)
		retval = NO_TYPE;
	else if (comp->action == LIST_OBJ) {
		if (comp->type == CONTACT) {
			if ((!(comp->id)) && (!(comp->coid)))
				retval = NO_COID;
		} else if (comp->type == SERVICE) {
			if (!(comp->id) && !(comp->name))
				retval = NO_NAME_OR_ID;
		} else if (comp->type == HARDWARE) {
			if (!(comp->id) && !(comp->name))
				retval = NO_NAME_OR_ID;
		}
	} else if (comp->action == ADD_TO_DB) {
		if (comp->type == CONTACT) {
			if (!(comp->name))
				retval = NO_CONT_NAME;
			else if (!(comp->email))
				retval = NO_EMAIL;
			else if (!(comp->phone))
				retval = NO_PHONE;
			else if (!(comp->coid))
				retval = NO_COID;
		} else if (comp->type == SERVER) {
			if (!(comp->name))
				retval = NO_NAME;
			else if (!(comp->coid))
				retval = NO_COID;
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
				retval = NO_DETAIL;
			else if (!(comp->service))
				retval = NO_SERVICE;
		} else if (comp->type == HARDWARE_TYPE) {
			if (!(comp->hclass))
				retval = NO_CLASS;
			else if (!(comp->shtype))
				retval = NO_TYPE;
		} else if (comp->type == VM_HOST) {
			if (!(comp->name))
				retval = NO_NAME;
			else if (!(comp->shtype))
				retval = NO_VHOST_TYPE;
		} else if (comp->type == HARDWARE) {
			if (!(comp->name))
				retval = NO_NAME;
			else if (!(comp->detail))
				retval = NO_DETAIL;
			else if (!(comp->device))
				retval = NO_DEVICE;
			else if (!(comp->hclass) && !(comp->sid))
				retval = NO_ID_OR_CLASS;
		} else if (comp->type == SERVICE) {
			if (!(comp->name))
				retval = NO_NAME;
			else if (!(comp->coid))
				retval = NO_COID;
			else if (!(comp->detail))
				retval = NO_DETAIL;
			else if (!(comp->url))
				retval = NO_SERVICE_URL;
			else if (!(comp->service))
				retval = NO_SERVICE;
		} else if (comp->type == CUSTOMER) {
			if (!(comp->name))
				retval = NO_NAME;
			else if (!(comp->coid))
				retval = NO_COID;
			else if (!(comp->county))
				retval = NO_COUNTY;
			else if (!(comp->address))
				retval = CBC_NO_ADDRESS;
			else if (!(comp->city))
				retval = NO_CITY;
			else if (!(comp->postcode))
				retval = NO_POSTCODE;
		}
	} else if (comp->action == DISPLAY) {
		if ((comp->type != SERVER) && (comp->type != CUSTOMER) && (comp->type != VM_HOST)) {
			retval = WRONG_TYPE_FOR_DISPLAY;
		} else if (comp->type == CUSTOMER) {
			if (!comp->coid)
				retval = NO_COID;
		} else if (!comp->name) {
			retval = NO_NAME;
		}
	} else if ((!(comp->name)) && (!(comp->id)) && 
		(comp->type != NONE || comp->action != NONE) &&
		(comp->type != CONTACT)) {
		retval = NO_NAME_OR_ID;
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
	free(list);
}

#ifdef CLEAN_COMM_LIST
# undef CLEAN_COMM_LIST
#endif /* CLEAN_COMM_LIST */
