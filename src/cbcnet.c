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
#include <error.h>
#include "cmdb.h"
#include "cbcnet.h"
#include "cbc_data.h"

int
get_net_list_for_dhcp(cbc_build_domain_s *bd, cbc_dhcp_s **dh)
{
	int retval = 0;
	struct cbc_iface_s *info = '\0';

	if (!bd)
		return 1;
	get_iface_info(&info);
	return retval;
}

void
get_iface_info(cbc_iface_s **info)
{
	struct ifaddrs *iface, *ilist;
	cbc_iface_s *list = '\0', *temp;

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
	snprintf(info->name, RBUFF_S, "%s", list->ifa_name);
	return retval;
}

