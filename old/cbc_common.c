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
