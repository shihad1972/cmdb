/* 
 *
 *  cbc: Create Build Configuration
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
 *  cbc_common.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc suite of programs
 * 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbc_common.h"

unsigned long int
cbc_get_varient_id(cbc_varient_s *vari, char *name)
{
	cbc_varient_s *list;

	if (!vari)
		return NONE;
	else
		list = vari;
	while (list) {
		if ((strncmp(name, list->varient, HOST_S) == 0) ||
		    (strncmp(name, list->valias, MAC_S) == 0))
			return list->varient_id;
		list = list->next;
	}
	return NONE;
}

void
cbc_set_varient_updated(cbc_config_s *cbc, unsigned long int vid)
{
	int query = UP_VARIENT, retval;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, cbc_update_args[query]);
	data->args.number = (unsigned long int)getuid();
	data->next->args.number = vid;
	if ((retval = cbc_run_update(cbc, data, query)) == 1)
		printf("Varient updated\n");
	else if (retval == 0)
		fprintf(stderr, "Unable to update varient\n");
	else if (retval > 1)
		fprintf(stderr, "Multiple varients updated??\n");
	clean_dbdata_struct(data);
		
}

unsigned long int
search_for_vid(cbc_varient_s *vari, char *varient, char *valias)
{
	char *name;

	if (!(name = cbc_get_varient_name(varient, valias)))
		return NONE;
	return cbc_get_varient_id(vari, name);
}

int
check_for_package(cbc_config_s *cbc, unsigned long int osid, unsigned long int vid, char *pack)
{
	int retval;
	unsigned int type = cbc_search_args[PACK_ID_ON_DETAILS];
	dbdata_s *data;

	init_multi_dbdata_struct(&data, type);
	snprintf(data->args.text, RBUFF_S, "%s", pack);
	data->next->args.number = vid;
	data->next->next->args.number = osid;
	retval = cbc_run_search(cbc, data, PACK_ID_ON_DETAILS);
	clean_dbdata_struct(data);
	return retval;
}

void
check_ip_in_dns(unsigned long int *ip_addr, char *name, char *domain)
{
	int status = 0;
	struct addrinfo hints, *si = NULL, *p;
	char host[256];

	snprintf(host, RBUFF_S, "%s.%s", name, domain);
	memset(&hints, 0, sizeof hints);// make sure the struct is empty
        hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
        hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	if ((status = getaddrinfo(host, NULL, &hints, &si)) == 0) {
		for (p = si; p != NULL; p = p->ai_next) {
// Only IPv4 for now..
			if (p->ai_family == AF_INET) {
				struct in_addr *addr;
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
				addr = &(ipv4->sin_addr);
				*ip_addr = (unsigned long int)ntohl(addr->s_addr);
				printf("Found ip %s in DNS\n", inet_ntoa(*addr));
			}
		}
	}
	if (si)
		freeaddrinfo(si);
}

void
set_build_domain_updated(cbc_config_s *cbt, char *domain, uli_t id)
{
	int retval, query = UP_BDOM_MUSER;
	dbdata_s *data;
	

	if (!(cbt) || (!(domain) && (id == 0)))
		return;
	init_multi_dbdata_struct(&data, cbc_update_args[query]);
	if (id == 0) { // need to get build_domain id
		snprintf(data->args.text, RBUFF_S, "%s", domain);
		if ((retval = cbc_run_search(cbt, data, BD_ID_ON_DOMAIN)) == 0) {
			fprintf(stderr, "Cannot find build domain %s\n", domain);
			clean_dbdata_struct(data);
			return;
		} else { 
			data->next->args.number = data->fields.number;
			memset(&(data->args.text), 0, RBUFF_S);
		}
	} else {
		data->next->args.number = id;
	}
	data->args.number = (unsigned long int)getuid();
	if ((retval = cbc_run_update(cbt, data, query)) == 0)
		fprintf(stderr, "Build domain cannot be updated\n");
	else if (retval == 1)
		printf("Build domain set updated by uid %lu\n", data->args.number);
	else
		fprintf(stderr, "Multiple build domains??\n");
	clean_dbdata_struct(data);
}

char *
cbc_get_varient_name(char *varient, char *valias)
{
	if ((!varient) && (!valias))
		return NULL;
	else if (varient && !valias) {
		if (strncmp(varient, "NULL", COMM_S) != 0)
			return varient;
		else if (strncmp(varient, "NULL", COMM_S) == 0)
			return NULL;
	} else if (!varient && valias) {
		if (strncmp(valias, "NULL", COMM_S) != 0)
			return valias;
		else if (strncmp(varient, "NULL", COMM_S) == 0)
			return NULL;
	} else {
		if (strncmp(valias, "NULL", COMM_S) == 0)
			return varient;
		else if (strncmp(varient, "NULL", COMM_S) == 0)
			return valias;
	}
	return varient;
}

int
get_varient_id(cbc_config_s *cmc, char *var, unsigned long int *varient_id)
{
	int retval = NONE, type;
	unsigned int max;
	dbdata_s *data;

	type = VARIENT_ID_ON_VARIENT;
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, CONF_S, "%s", var);
	if ((retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VARIENT)) == 1) {
		*varient_id = data->fields.number;
		retval = NONE;
	} else if (retval > 1) {
		fprintf(stderr,
			"Multiple variants or aliases found for %s\n", var);
		retval = MULTIPLE_VARIENTS;
	} else {
		if ((retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VALIAS)) == 0) {
			fprintf(stderr,
				"Sorry, but %s is not a valid varient or alias\n", var);
			retval = VARIENT_NOT_FOUND;
		} else if (retval > 1) {
			fprintf(stderr,
				"Multiple variants or aliases found for %s\n", var);
			retval = MULTIPLE_VARIENTS;
		} else {
			*varient_id = data->fields.number;
			retval = NONE;
		}
	}
	clean_dbdata_struct(data);
	return retval;
}

int
get_build_domain_id(cbc_config_s *cbc, char *domain, uli_t *id)
{
	int retval;
	dbdata_s *data;

	if (!(cbc) || !(domain))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, 1);
	snprintf(data->args.text, RBUFF_S, "%s", domain);
	if ((retval = cbc_run_search(cbc, data, BD_ID_ON_DOMAIN)) == 0) {
		fprintf(stderr, "Cannot find build domain %s\n", domain);
		clean_dbdata_struct(data);
		return NO_RECORDS;
	} else if (retval > 1)
		fprintf(stderr, "Multiple build domains found for %s\n", domain);
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_system_package_id(cbc_config_s *cbc, char *package, uli_t *id)
{
	int retval;
	dbdata_s *data;

	if (!(cbc) || !(package))
		return CBC_NO_DATA;
	init_multi_dbdata_struct(&data, 1);
	snprintf(data->args.text, URL_S, "%s", package);
	if ((retval = cbc_run_search(cbc, data, SYSPACK_ID_ON_NAME)) == 0) {
		fprintf(stderr, "Cannot find system package %s\n", package);
		clean_dbdata_struct(data);
		return NO_RECORDS;
	} else if (retval > 1)
		fprintf(stderr, "Multiple system packages found for %s\n", package);
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_syspack_arg_id(cbc_config_s *cbc, char *field, uli_t sp_id, uli_t *id)
{
	int retval, query = SPARG_ON_SPID_AND_FIELD;
	dbdata_s *data;
	unsigned int max;

	if (!(cbc) || !(field) || (sp_id == 0))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = sp_id;
	snprintf(data->next->args.text, URL_S, "%s", field);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find the package argument\n");
		clean_dbdata_struct(data);
		return NO_RECORDS;
	} else if (retval > 1)
		fprintf(stderr, "Found multiple arguments for the packages\n");
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_system_script_id(cbc_config_s *cbc, char *script, uli_t *id)
{
	int retval, query = SCR_ID_ON_NAME;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(script))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, RBUFF_S, "%s", script);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find the script %s\n", script);
		clean_dbdata_struct(data);
		return NO_RECORDS;
	} else if (retval > 1)
		fprintf(stderr, "Found multiple scripts %s\n", script);
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_build_type_id(cbc_config_s *cbc, char *os, uli_t *id)
{
	int retval = 0, query = BUILD_TYPE_ID_ON_ALIAS;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(os))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, RBUFF_S, "%s", os);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find build type for os alias %s\n", os);
		clean_dbdata_struct(data);
		return NO_RECORDS;
	} else if (retval > 1)
		fprintf(stderr, "Found multiple build types for os alias %s\n", os);
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_partition_id(cbc_config_s *cbc, char *name, char *mount, uli_t *id)
{
	int retval = 0, query = DEFP_ID_ON_SCHEME_PART;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(name) || !(mount))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, RBUFF_S, "%s", name);
	snprintf(data->next->args.text, RBUFF_S, "%s", mount);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find partition %s in scheme %s\n",
		 mount, name);
		clean_dbdata_struct(data);
		return PARTITIONS_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Found multiple partitions %s in scheme %s?\n",
		 mount, name);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_scheme_id(cbc_config_s *cbc, char *name, uli_t *id)
{
	int retval = 0, query = DEF_SCHEME_ID_ON_SCH_NAME;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(name))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, RBUFF_S, "%s", name);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find scheme %s\n", name);
		clean_dbdata_struct(data);
		return SCHEME_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Found multiple schemes with name %s?\n", name);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_part_opt_id(cbc_config_s *cbc, char *name, char *part, char *opt, uli_t *id)
{
	int retval = 0, query = PART_OPT_ID;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(name) || !(part) || !(opt))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	if ((retval = get_partition_id(cbc, name, part, &(data->args.number))) != 0)
		return retval;
	if ((retval = get_scheme_id(cbc, name, &(data->next->args.number))) != 0)
		return retval;
	snprintf(data->next->next->args.text, RBUFF_S, "%s", opt);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find option %s for part %s, scheme %s\n",
		 opt, part, name);
		clean_dbdata_struct(data);
		return NO_OPTION;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple options %s for part %s, scheme %s?\n",
		 opt, part, name);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

