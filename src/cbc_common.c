/* 
 *
 *  cbc: Create Build Configuration
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
 *  cbc_common.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc suite of programs
 * 
 */
#include <config.h>
#include <configmake.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbc_common.h"

static int
read_cbc_config_values(ailsa_cmdb_s *cbc, FILE *cnf);

static int
read_cbc_config_values(ailsa_cmdb_s *cbc, FILE *cnf)
{
	int retval = 0;
	unsigned long int portno;
	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DBTYPE=%s", cbc->dbtype);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PASS=%s", cbc->pass);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "FILE=%s", cbc->file);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "HOST=%s", cbc->host);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "USER=%s", cbc->user);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DB=%s", cbc->db);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "SOCKET=%s", cbc->socket);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PORT=%s", port);
	rewind (cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "TMPDIR=%s", cbc->tmpdir);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "TFTPDIR=%s", cbc->tftpdir);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PXE=%s", cbc->pxe);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "TOPLEVELOS=%s", cbc->toplevelos);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PRESEED=%s", cbc->preseed);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "KICKSTART=%s", cbc->kickstart);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DHCPCONF=%s", cbc->dhcpconf);
	retval = 0;
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
		return retval;
	} else {
		cbc->port = (unsigned int) portno;
	}
	return retval;
}
int
check_for_package(ailsa_cmdb_s *cbc, unsigned long int osid, unsigned long int vid, char *pack)
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

int
get_default_id(ailsa_cmdb_s *cbc, int query, char *name, unsigned long int *id)
{
	int retval = 0;
	dbdata_s *data;

	if (!(cbc) || (query < 0))
		return CBC_NO_DATA;
	unsigned int max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	if (name) {
		if (cbc_search_args[query] == 0)
			fprintf(stderr, "Name %s passed for query with no args:\n%s\n",
			 name, cbc_sql_search[query]);
		snprintf(data->args.text, RBUFF_S, "%s", name);
	}
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
	//	fprintf(stderr, "Cannot find id for query:\n%s\n", cbc_sql_search[query]);
		clean_dbdata_struct(data);
		return -1;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple id's found for query:\n%s\n", cbc_sql_search[query]);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_server_id(ailsa_cmdb_s *cbc, char *server, unsigned long int *id)
{
	int retval = 0, query = SERVER_ID_ON_SNAME;
	unsigned int max;
	dbdata_s *data;

	if (!(cbc) || !(server))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, HOST_S, "%s", server);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find server %s\n", server);
		clean_dbdata_struct(data);
		return SERVER_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple servers found for %s\n", server);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_varient_id(ailsa_cmdb_s *cmc, char *var, unsigned long int *varient_id)
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
get_build_domain_id(ailsa_cmdb_s *cbc, char *domain, uli_t *id)
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
get_system_package_id(ailsa_cmdb_s *cbc, char *package, uli_t *id)
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
get_syspack_arg_id(ailsa_cmdb_s *cbc, char *field, uli_t sp_id, uli_t *id)
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
get_system_script_id(ailsa_cmdb_s *cbc, char *script, uli_t *id)
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
get_build_type_id(ailsa_cmdb_s *cbc, char *os, uli_t *id)
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
get_partition_id(ailsa_cmdb_s *cbc, char *name, char *mount, uli_t *id)
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
get_scheme_id_from_build(ailsa_cmdb_s *cbc, uli_t server_id, uli_t *id)
{
	int retval = 0, query = DEF_SCHEME_ID_FROM_BUILD;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || (server_id == 0))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = server_id;
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find seed scheme for server_id %lu\n",
		 server_id);
		clean_dbdata_struct(data);
		return SCHEME_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Found multiple schemes for server_id %lu\n",
		 server_id);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_scheme_id(ailsa_cmdb_s *cbc, char *name, uli_t *id)
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
get_locale_id(ailsa_cmdb_s *cbc, char *name, uli_t *id)
{
	int retval = 0;
	int query = LOCALE_ID_ON_NAME;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(name))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, HOST_S, "%s", name);
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find locale %s\n", name);
		clean_dbdata_struct(data);
		return LOCALE_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "Found multiple locales with name %s?\n", name);
	}
	*id = data->fields.number;
	clean_dbdata_struct(data);
	return 0;
}

int
get_scheme_name(ailsa_cmdb_s *cbc, uli_t server_id, char *name)
{
	int retval = 0, query = SCHEME_NAME_ON_SERVER_ID;
	dbdata_s *data;
	unsigned int max;
	if (!(cbc) || !(name))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[query], cbc_search_fields[query]);
	init_multi_dbdata_struct(&data, max);
	data->args.number = server_id;
	if ((retval = cbc_run_search(cbc, data, query)) == 0) {
		fprintf(stderr, "Cannot find scheme on server id %lu\n", server_id);
		clean_dbdata_struct(data);
		return SCHEME_NOT_FOUND;
	} else if (retval > 1) {
		fprintf(stderr, "FOund multiple schemes for server_id %lu\n", server_id);
	}
	snprintf(name, RBUFF_S, "%s", data->fields.text);
	clean_dbdata_struct(data);
	return 0;
}

int
get_part_opt_id(ailsa_cmdb_s *cbc, char *name, char *part, char *opt, uli_t *id)
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

int
set_scheme_updated(ailsa_cmdb_s *cbc, char *scheme)
{
	int retval;
	unsigned long int scheme_id;
	dbdata_s *user;

	if (!(cbc) || !(scheme))
		return CBC_NO_DATA;
	if ((retval = get_scheme_id(cbc, scheme, &(scheme_id))) != 0)
		return retval;
	init_multi_dbdata_struct(&user, cbc_update_args[UP_SEEDSCHEME]);
	user->args.number = (unsigned long int)getuid();
	user->next->args.number = scheme_id;
	if ((retval = cbc_run_update(cbc, user, UP_SEEDSCHEME)) == 1) {
		printf("Scheme marked as updated\n");
		retval = 0;
	} else if (retval == 0)
		printf("Scheme not updated\n");
	clean_dbdata_struct(user);
	return retval;
}
// Should get rid of the uli_id *id in this function
void
set_build_domain_updated(ailsa_cmdb_s *cbt, char *domain)
{
	int retval, query = UP_BDOM_MUSER;
	dbdata_s *data;

	if (!(cbt) || (!(domain)))
		return;
	init_multi_dbdata_struct(&data, cbc_update_args[query]);
	snprintf(data->args.text, RBUFF_S, "%s", domain);
	if ((retval = cbc_run_search(cbt, data, BD_ID_ON_DOMAIN)) == 0) {
		fprintf(stderr, "Cannot find build domain %s\n", domain);
		clean_dbdata_struct(data);
		return;
	} else {
		data->next->args.number = data->fields.number;
		memset(&(data->args.text), 0, RBUFF_S);
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
// Should be passing the varient name to this function
void
cbc_set_varient_updated(ailsa_cmdb_s *cbc, unsigned long int vid)
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

int
get_os_id(ailsa_cmdb_s *cmc, char *os[], unsigned long int *os_id)
{
	int retval = NONE, type = OS_ID_ON_NAME;
	unsigned int max;
	dbdata_s *data;

	if (strncmp(os[0], "NULL", COMM_S) == 0) {
		fprintf(stderr, "No architecture provided for OS\n");
		return NO_ARCH;
	}
	if (strncmp(os[1], "NULL", COMM_S) == 0) {
		fprintf(stderr, "No version or version alias provided\n");
		return NO_OS_VERSION;
	}
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	fill_dbdata_os_search(data, os);
	if ((retval = cbc_run_search(cmc, data, OS_ID_ON_NAME)) == 1) {
		*os_id = data->fields.number;
		retval = NONE;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple OS's found!\n");
		retval = MULTIPLE_OS;
	} else {
		if ((retval = cbc_run_search(cmc, data, OS_ID_ON_ALIAS)) == 1) {
			*os_id = data->fields.number;
			retval = NONE;
		} else if (retval > 1) {
			fprintf(stderr, "Multiple OS's found!\n");
			retval = MULTIPLE_OS;
		} else {
			if ((retval = cbc_run_search
			  (cmc, data, OS_ID_ON_NAME_VER_ALIAS)) == 1) {
				*os_id = data->fields.number;
				retval = NONE;
			} else if (retval > 1) {
				fprintf(stderr, "Multiple OS's found!\n");
				retval = MULTIPLE_OS;
			} else {
				if ((retval = cbc_run_search
				 (cmc, data, OS_ID_ON_ALIAS_VER_ALIAS)) == 1) {
					 *os_id = data->fields.number;
					 retval = NONE;
				 } else if (retval > 1) {
					fprintf(stderr,
					 "Multiple OS's found!\n");
					retval = MULTIPLE_OS;
				} else {
					retval = OS_NOT_FOUND;
				 }
			}
		}
	}
	clean_dbdata_struct(data);
	return retval;
}

void
fill_dbdata_os_search(dbdata_s *data, char *os[])
{
        snprintf(data->args.text, CONF_S, "%s", os[2]);
        snprintf(data->next->args.text, MAC_S, "%s", os[1]);
        snprintf(data->next->next->args.text, MAC_S, "%s", os[0]);
}

int
get_os_alias(ailsa_cmdb_s *cbc, char *os, char *alias)
{
	int retval, type = OS_ALIAS_ON_OS;
	unsigned int max;
	dbdata_s *data;

	if (!(cbc) || !(os) || !(alias))
		return CBC_NO_DATA;
	max = cmdb_get_max(cbc_search_args[type], cbc_search_fields[type]);
	init_multi_dbdata_struct(&data, max);
	snprintf(data->args.text, MAC_S, "%s", os);
	if ((retval = cbc_run_search(cbc, data, type)) == 0) {
		clean_dbdata_struct(data);
		return OS_NOT_FOUND;
	}
	snprintf(alias, MAC_S, "%s", data->fields.text);
	clean_dbdata_struct(data);
	return 0;
}

void
check_for_alias(char **what, char *name, char *alias)
{
	if (strncmp(name, "NULL", COMM_S) != 0)
		*what = name;
	else
		*what = alias;
}

int
cbc_add_server(ailsa_cmdb_s *cbc, char *name, long unsigned int *server_id)
{
	int retval = 0;
	cbc_server_s *server;
	cbc_s *base;

	server = ailsa_calloc(sizeof(cbc_server_s), "server in cbc_add_server");
	base = ailsa_calloc(sizeof(cbc_s), "base in cbc_add_server");
	base->server = server;
	snprintf(server->name, HOST_S, "%s", name);
	server->cuser = server->muser = (unsigned long int)getuid();
	if ((retval = cbc_run_insert(cbc, base, CSERVERS)) != 0) {
		fprintf(stderr, "Caanot add server %s to database!\n", name);
	} else {
		printf("Server %s added to database\n", name);
		if (server_id)
			retval = get_server_id(cbc, name, server_id);
	}
	clean_cbc_struct(base);
	return retval;
}

void
init_cbc_config_values(ailsa_cmdb_s *cbc)
{
	memset(cbc, 0, sizeof(ailsa_cmdb_s));
	sprintf(cbc->db, "cmdb");
	sprintf(cbc->dbtype, "none");
	sprintf(cbc->user, "root");
	sprintf(cbc->host, "localhost");
	sprintf(cbc->pass, "%s", "");
	sprintf(cbc->socket, "%s", "");
	sprintf(cbc->tmpdir, "/tmp/cbc");
	sprintf(cbc->tftpdir, "/tftpboot");
	sprintf(cbc->pxe, "pxelinx.cfg");
	sprintf(cbc->toplevelos, "/build");
	sprintf(cbc->dhcpconf, "/etc/dhcpd/dhcpd.hosts");
	sprintf(cbc->preseed, "preseed");
	sprintf(cbc->kickstart, "kickstart");
	cbc->port = 3306;
	cbc->cliflag = 0;
}

int
parse_cbc_config_file(ailsa_cmdb_s *cbc, const char *config)
{
	FILE *cnf;
	int retval;
#ifdef HAVE_WORDEXP_H
	char **uconf;
	wordexp_t p;
#endif /* HAVE_WORDEXP_H */

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
	} else {
		read_cbc_config_values(cbc, cnf);
		fclose(cnf);
	}
#ifdef HAVE_WORDEXP
	if ((retval = wordexp("~/.dnsa.conf", &p, 0)) == 0) {
		uconf = p.we_wordv;
		if ((cnf = fopen(*uconf, "r"))) {
			if ((retval = read_cbc_config_values(cbc, cnf)) != 0)
				retval = UPORT_ERR;
			fclose(cnf);
		}
		wordfree(&p);
	}
#endif /* HAVE_WORDEXP */
	if ((retval = ailsa_add_trailing_slash(cbc->tmpdir)) != 0)
		retval = TMP_ERR;
	if ((retval = ailsa_add_trailing_slash(cbc->tftpdir)) != 0)
		retval = TFTP_ERR;
	if ((retval = ailsa_add_trailing_slash(cbc->pxe)) != 0)
		retval = PXE_ERR;
	if ((retval = ailsa_add_trailing_slash(cbc->toplevelos)) != 0)
		retval = OS_ERR;
	if ((retval = ailsa_add_trailing_slash(cbc->preseed)) != 0)
		retval = PRESEED_ERR;
	if ((retval = ailsa_add_trailing_slash(cbc->kickstart)) !=0)
		retval = KICKSTART_ERR;

	return retval;
}

void
parse_cbc_config_error(int error)
{
	if (error == PORT_ERR)
		fprintf(stderr, "Port higher than 65535!\n");
	else if (error == TMP_ERR)
		fprintf(stderr, "Cannot add trailing / to TMPDIR: > 79 characters\n");
	else if (error == TFTP_ERR)
		fprintf(stderr, "Cannot add trailing / to TFTPDIR: > 79 characters\n");
	else if (error == PXE_ERR)
		fprintf(stderr, "Cannot add trailing / to PXE: > 79 characters\n");
	else if (error == OS_ERR)
		fprintf(stderr, "Cannot add trailing / to TOPLEVELOS: > 79 characters\n");
	else if (error == PRESEED_ERR)
		fprintf(stderr, "Cannot add trailing / to PRESEED: > 79 characters\n");
	else if (error == KICKSTART_ERR)
		fprintf(stderr, "Cannot add trailing / to KICKSTART: > 79 characters\n");
	else
		fprintf(stderr, "Unkown error code: %d\n", error);
}

void
print_cbc_config(ailsa_cmdb_s *cbc)
{
	fprintf(stderr, "########\nConfig Values\n");
	fprintf(stderr, "DB: %s\n", cbc->db);
	fprintf(stderr, "USER: %s\n", cbc->user);
	fprintf(stderr, "PASS: %s\n", cbc->pass);
	fprintf(stderr, "HOST: %s\n", cbc->host);
	fprintf(stderr, "PORT: %d\n", cbc->port);
	fprintf(stderr, "SOCKET: %s\n", cbc->socket);
	fprintf(stderr, "TMPDIR: %s\n", cbc->tmpdir);
	fprintf(stderr, "TFTPDIR: %s\n", cbc->tftpdir);
	fprintf(stderr, "PXE: %s\n", cbc->pxe);
	fprintf(stderr, "TOPLEVELOS: %s\n", cbc->toplevelos);
	fprintf(stderr, "DHCPCONF: %s\n", cbc->dhcpconf);
	fprintf(stderr, "PRESEED: %s\n", cbc->preseed);
	fprintf(stderr, "KICKSTART: %s\n", cbc->kickstart);
	fprintf(stderr, "\n");
}

