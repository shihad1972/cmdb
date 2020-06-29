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
		fprintf(stderr, "Found multiple schemes for server_id %lu\n", server_id);
	}
	snprintf(name, RBUFF_S, "%s", data->fields.text);
	clean_dbdata_struct(data);
	return 0;
}
