/*
 *  cbc: Create Build config
 *  (C) 2014 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcnet.c: functions that deal with ip addressing for cbc suite
 * 
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <errno.h>
#include "cmdb.h"
#include "cbcnet.h"
#include "cbc_data.h"

int
get_net_list_for_dhcp(cbc_build_domain_s *bd, cbc_dhcp_s **dh)
{
	int retval = 0;
	struct cbc_iface_s *info = NULL;

	if (!bd)
		return 1;
	get_iface_info(&info);
	retval = get_dhcp_server_info(bd, dh, info);
	clean_cbc_iface(info);
	return retval;
}

void
get_iface_info(cbc_iface_s **info)
{
	struct ifaddrs *iface, *ilist;
	cbc_iface_s *list = NULL, *temp;

	if (getifaddrs(&iface) == -1) 
		report_error(IFACE_LIST_FAILED, "get_net_list_for_dhcp");
	for (ilist = iface; ilist != NULL; ilist = ilist->ifa_next) {
		if (ilist->ifa_addr->sa_family != AF_INET)
			continue;
		if (! strncmp(ilist->ifa_name, "lo", 4))
			continue;
		if (!(temp = malloc(sizeof(cbc_iface_s))))
			report_error(MALLOC_FAIL, "list in get_net_list_for_dhcp");
		init_cbc_iface(temp);
		if (!(list))
			*info = list = temp;
		else {
			while (list->next)
				list = list->next;
			list->next = temp;
		}
		if (fill_iface_info(ilist, temp) != 0)
			report_error(IFACE_FILL, "fill_iface_info");
		list = *info;
	}
	freeifaddrs(iface);
}

int
fill_iface_info(struct ifaddrs *list, cbc_iface_s *info)
{
	int retval = 0;
	struct sockaddr_in *if_ip, *nm_ip;

	if_ip = (struct sockaddr_in *)list->ifa_addr;
	nm_ip = (struct sockaddr_in *)list->ifa_netmask;
	info->ip = ntohl(if_ip->sin_addr.s_addr);
	info->nm = ntohl(nm_ip->sin_addr.s_addr);
	info->nw = info->ip & info->nm;
	info->bc = info->nw | (~info->nm);
	info->sip = info->nw + 1;
	info->fip = info->bc - 1;
	snprintf(info->name, RBUFF_S, "%s", list->ifa_name);
	return retval;
}

int
get_dhcp_server_info(cbc_build_domain_s *bd, cbc_dhcp_s **dh, cbc_iface_s *i)
{
	int retval = 0;
	cbc_build_domain_s *bdl;
	cbc_dhcp_s *list = NULL, *temp;

	if (!(bd))
		return NO_BUILD_DOMAIN;
	if (!(i))
		return NO_IFACE;
	bdl = bd;
	while (bdl) {
		insert_into_dhcp_list(&list, &temp);
// To check if the this network is already added, we need to pass list
		if ((retval = fill_dhcp_server(bdl, i, list)) != 0) {
			remove_from_dhcp_list(&list);
			if (retval == 1)
				fprintf(stderr, "\
Skipping domain %s: No interface\n", bdl->domain);
			else if (retval == 2)
				fprintf(stderr, "\
Netmask does not correlate for domain %s\n", bdl->domain);
			else if (retval == 3)
				fprintf(stderr, "\
Network already has a configuration in dhcpd.networks\n");
		}
		bdl = bdl->next;
		retval = 0;
	}
	*dh = list;
	return retval;
}

void
insert_into_dhcp_list(cbc_dhcp_s **list, cbc_dhcp_s **item)
{
	cbc_dhcp_s *i = NULL, *l = NULL;

	if (!(i = malloc(sizeof(cbc_dhcp_s))))
		report_error(MALLOC_FAIL, "i in insert_into_dhcp_list");
	init_cbc_dhcp(i);
	if (!(i->dom_search = malloc(sizeof(string_l))))
		report_error(MALLOC_FAIL, "dh->dom_search in insert_into_dhcp_list");
	init_string_l(i->dom_search);
	*item = i;
	if (!(*list))
		*list = i;
	else {
		l = *list;
		while (l->next)
			l = l->next;
		l->next = i;
	}
}

void
remove_from_dhcp_list(cbc_dhcp_s **list)
{
	cbc_dhcp_s *l, *p;

	if (*list)
		l = p = *list;
	else
		return;
	while (l->next) {
		p = l;
		l = l->next;
	}
	p->next = NULL;
	if (p == l)
		*list = NULL;
	clean_cbc_dhcp(l);
}

int
fill_dhcp_server(cbc_build_domain_s *bd, cbc_iface_s *i, cbc_dhcp_s *list)
{
	int retval = 0;
	cbc_iface_s *cif = i;
	cbc_dhcp_s *dh, *cp;
	unsigned long int sip, fip;
	
	if (!(cif) || !(list))
		return NULL_POINTER_PASSED;
	dh = list;
	while (dh->next)
		dh = dh->next;
	while (cif) {
		sip = (unsigned long int)cif->sip;
		fip = (unsigned long int)cif->fip;
		if ((bd->start_ip >= sip) && (bd->end_ip <= fip)) {
			cp = list;
			while (cp) {
				if (cp->nw == cif->nw) {
					retval = 3;
					break;
				}
				cp = cp->next;
			}
			if (retval == 3)
				break;
			snprintf(dh->iname, RBUFF_S, "%s", cif->name);
			snprintf(dh->dname, RBUFF_S, "%s", bd->domain);
			dh->gw = bd->gateway;
			dh->ns = bd->ns;
			dh->nm = bd->netmask;
			dh->nw = (unsigned long int)cif->nw;
			dh->ip = (unsigned long int)cif->ip;
			if (dh->nm != (unsigned long int)cif->nm) {
				retval = 2;
				break;
			}
			snprintf(dh->dom_search->string, RBUFF_S, "%s", bd->domain);
		}
		cif = cif->next;
	}
	if ((dh->gw == 0) || (dh->ns == 0) || (dh->nm == 0) || (dh->nw == 0))
		retval = 1;
	return retval;
}

int
decode_http_header(FILE *rx, unsigned long int *len)
{
	char *buf, *t;
	int retval = 1;
	unsigned long int code;

	if (!(buf = malloc(MAXDATASIZE))) {
		perror("malloc in decode_header");
		return retval;
	}
	if (!(fgets(buf, MAXDATASIZE, rx))) {
		fprintf(stderr, "Cannot read HTTP line\n");
		goto cleanup;
	}
	if (!(t = strtok(buf, " "))) {
		fprintf(stderr, "Cannot tokenise response string\n");
		goto cleanup;
	}
	if (!(t = strtok(NULL, " "))) {
		fprintf(stderr, "Only 1 space in response??\n");
		goto cleanup;
	}
	code = strtoul(t, NULL, 10);
	if (code != 200) {
		fprintf(stderr, "Server response code: %lu\n", code);
		goto cleanup;
	}
	while (fgets(buf, MAXDATASIZE, rx)) {
		if (strncmp(buf, "\r\n", 2) == 0) {
			retval = 0;
			goto cleanup;
		}
		t = strtok(buf, " ");
/*
 * The following does not necessarily have to be provided. The content could
 * be dynamic. Since we are requesting a binary file with a fixed length
 * this should be safe enough. If the file you are requesting is text
 * there is a chance that it could be classified as dynamic and no
 * content length will be provided. In that case you should look for the
 * header:
 *  Transfer-Encoding: chunked
 * In this case, from nginx testing, it seems that after the final '\r\n'
 * of the header, there is the content length of the chunk, on a single line,
 * and encoded in hexadecimal.
 *
 * Full http protocol specification at:
 *
 * http://www.w3.org/Protocols/rfc2616/rfc2616.html
 */
		if (strncmp(t, "Content-Length:", RANGE_S) == 0) {
			if (!(t = strtok(NULL, " "))) {
				fprintf(stderr, "Cannot get length\n");
				goto cleanup;
			}
			*len = strtoul(t, NULL, 10);
		}
	}

	cleanup:
		free(buf);
		return retval;
}

