/*
 *
 *  dnsa: Domain Name System Admistration
 *  (C) 2014 Iain M. Conochie <iain-AT-thargoid.co.uk>
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
 *  dnsa_net.c: Holds functions for network related funcitons for dnsa
 *
 */
#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include "cmdb.h"
#include "dnsa_data.h"
#include "cmdb_dnsa.h"
#include "dnsa_net.h"

unsigned long int
get_net_range(unsigned long int prefix)
{
        unsigned long int range;
        range = 4294967295UL;
        range = (range >> prefix) + 1;
        return range;
}

int
do_rev_lookup(char *ip, char *host, size_t size)
{
	int retval = 0;
	struct addrinfo hints, *res;
	socklen_t len = sizeof(struct sockaddr_in6);
	socklen_t hlen = (socklen_t)size;

	if (!(ip) || !(host))
		return NULL_POINTER_PASSED;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if ((retval = getaddrinfo(ip, NULL, &hints, &res)) != 0) {
		fprintf(stderr, "Getaddrinfo in do_rev_lookup: %s\n",
		  gai_strerror(retval));
		return NET_FUNC_FAILED;
	}
	if ((retval = getnameinfo(res->ai_addr, len, host, hlen, NULL, 0, NI_NAMEREQD)) != 0) {
		fprintf(stderr, "getnameinfo: %s\n", gai_strerror(retval));
		retval = DNS_LOOKUP_FAILED;
	}
	return retval;
}

int
get_port_number(record_row_s *rec, char *name, unsigned short int *port)
{
	char *host;
	int retval = 0;
	size_t len;
	struct addrinfo hints, *srvinfo;
	struct sockaddr_in *ipv4;

	if (!(host = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "host in get_port_number");
	len = strlen(rec->dest);
	if (rec->dest[len - 1] == '.')
		snprintf(host, RBUFF_S, "%s", rec->dest);
	else
		snprintf(host, RBUFF_S, "%s.%s", rec->dest, name);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	if ((strncmp(rec->protocol, "tcp", COMM_S)) == 0)
		hints.ai_socktype = SOCK_STREAM;
	else if ((strncmp(rec->protocol, "udp", COMM_S)) == 0)
		hints.ai_socktype = SOCK_DGRAM;
	else {
		fprintf(stderr, "Unknown protocol type %s in %s\n",
		 rec->protocol, rec->host);
		free(host);
		return WRONG_PROTO;
	}
	hints.ai_flags = AI_PASSIVE;
	if ((retval = getaddrinfo(host, rec->service, &hints, &srvinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(retval));
		return retval;
	}
	if (srvinfo->ai_family == AF_INET) {
		ipv4 = (struct sockaddr_in *)srvinfo->ai_addr;
		*port = (unsigned short int) htons((uint16_t)ipv4->sin_port);
	} else {
		retval = WRONG_PROTO;
	}
	free(host);
	freeaddrinfo(srvinfo);
	return retval;
}

