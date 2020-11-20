/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  sql_data.c
 *
 *  Helper functions for libailsasql
 *
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif // HAVE_STDBOOL_H
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /*HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /*HAVE_SQLITE3 */
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"

void
cmdb_clean_ailsa_sql_multi(ailsa_sql_multi_s *data)
{
	if (!(data))
		return;
	if (data->query)
		my_free(data->query);
	if (data->fields)
		my_free(data->fields);
	my_free(data);
}

int
ailsa_get_bdom_list(ailsa_cmdb_s *cbs, AILLIST *list)
{
	if (!(cbs) || !(list))
		return AILSA_NO_DATA;
	int retval;
	size_t total = 5;
	uint32_t ip;
	ailsa_dhcp_s *dhcp;
	AILLIST *dom = ailsa_db_data_list_init();
	AILELEM *e;

	if ((retval = ailsa_basic_query(cbs, BUILD_DOMAIN_NETWORKS, dom)) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_NETWORKS query failed");
		goto cleanup;
	}
	if ((dom->total % total) != 0) {
		ailsa_syslog(LOG_ERR, "BUILD_DOMAIN_NETWORKS returned wrong number of items?");
		goto cleanup;
	}
	e = dom->head;
	while (e) {
		dhcp = ailsa_calloc(sizeof(ailsa_dhcp_s), "dhcp in ailsa_get_bdom_list");
		dhcp->network = ailsa_calloc(INET6_ADDRSTRLEN, "dhcp->network in ailsa_get_bdom_list");
		dhcp->gateway = ailsa_calloc(INET6_ADDRSTRLEN, "dhcp->gateway in ailsa_get_bdom_list");
		dhcp->netmask = ailsa_calloc(INET6_ADDRSTRLEN, "dhcp->netmask in ailsa_get_bdom_list");
		dhcp->nameserver = ailsa_calloc(INET6_ADDRSTRLEN, "dhcp->nameserver in ailsa_get_bdom_list");
		dhcp->dname = strndup(((ailsa_data_s *)e->data)->data->text, DOMAIN_LEN);
		dhcp->ns = ((ailsa_data_s *)e->next->data)->data->number;
		dhcp->gw = ((ailsa_data_s *)e->next->next->next->data)->data->number;
		dhcp->nm = ((ailsa_data_s *)e->next->next->next->next->data)->data->number;
		dhcp->nw = ((ailsa_data_s *)e->next->next->data)->data->number & dhcp->nm;
		ip = htonl((uint32_t)dhcp->ns);
		inet_ntop(AF_INET, &ip, dhcp->nameserver, INET6_ADDRSTRLEN);
		ip = htonl((uint32_t)dhcp->gw);
		inet_ntop(AF_INET, &ip, dhcp->gateway, INET6_ADDRSTRLEN);
		ip = htonl((uint32_t)dhcp->nm);
		inet_ntop(AF_INET, &ip, dhcp->netmask, INET6_ADDRSTRLEN);
		ip = htonl((uint32_t)dhcp->nw);
		inet_ntop(AF_INET, &ip, dhcp->network, INET6_ADDRSTRLEN);
		if ((retval = ailsa_list_insert(list, dhcp)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert into dhcp list");
			goto cleanup;
		}
		e = ailsa_move_down_list(e, total);
	}
	cleanup:
		ailsa_list_full_clean(dom);
		return retval;
}
