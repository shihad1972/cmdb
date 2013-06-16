/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc_dnsa.c
 * 
 *  Functions to likn into dnsa functions for cbc
 * 
 *  part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "../config.h"
#include "base_sql.h"
#include "cbc_data.h"
#include "cmdb.h"
#include "cmdb_cbc.h"

#ifdef HAVE_LIBPCRE

# include "checks.h"

#endif /* HAVE_LIBPCRE */

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"
# include "cbc_dnsa.h"
# include "dnsa_base_sql.h"


void
fill_cbc_fwd_zone(zone_info_s *zone, char *domain, dnsa_config_s *dc)
{
	snprintf(zone->name, RBUFF_S, "%s", domain);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
}

void
copy_cbc_into_dnsa(dnsa_config_s *dc, cbc_config_s *cbc)
{
	snprintf(dc->dbtype, RANGE_S, "%s", cbc->dbtype);
	snprintf(dc->file, CONF_S, "%s", cbc->file);
	snprintf(dc->db, CONF_S, "%s", cbc->db);
	snprintf(dc->user, CONF_S, "%s", cbc->user);
	snprintf(dc->host, CONF_S, "%s", cbc->host);
	snprintf(dc->pass, CONF_S, "%s", cbc->pass);
	snprintf(dc->socket, CONF_S, "%s", cbc->socket);
	dc->port = cbc->port;
	dc->cliflag = cbc->cliflag;
}

int
get_dns_ip_list(cbc_config_s *cbt, cbc_s *details, dbdata_s *data)
{
	int retval = NONE;
	dnsa_s *dnsa;
	dnsa_config_s *dc = '\0';

	if (!(dc = calloc(sizeof(dnsa_config_s), sizeof(char))))
		report_error(MALLOC_FAIL, "dc in get_dns_ip_list");
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in get_dns_ip_list");
	copy_cbc_into_dnsa(dc, cbt);
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_query(dc, dnsa, ALL_A_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		free(dc);
		return retval;
	}
	prep_dnsa_ip_list(data, dnsa, details->bdom);
	dnsa_clean_list(dnsa);
	free(dc);
	return retval;
}

void
prep_dnsa_ip_list(dbdata_s *data, dnsa_s *dnsa, cbc_build_domain_s *build)
{
	int i;
	dbdata_s *pos = data, *list;
	record_row_s *rec = dnsa->records;
	while (pos) {
		if ((pos->fields.number > 0) && (pos->next))
			pos = pos->next;
		else
			break;
	}
	while (rec) {
		add_int_ip_to_fwd_records(rec);
		if ((rec->ip_addr >= build->start_ip) && 
		    (rec->ip_addr <= build->end_ip)) {
			list = data;
			i = FALSE;
			while (list) {
				if (list->fields.number == rec->ip_addr)
					i = TRUE;
				list = list->next;
			}
			if (i == FALSE) {
				if (!(list = calloc(sizeof(dbdata_s), sizeof(char))))
					report_error(MALLOC_FAIL, "list in prep_dnsa_ip_list");
				init_dbdata_struct(list);
				list->fields.number = rec->ip_addr;
				pos->next = list;
				pos = list;
			}
		}
		rec = rec->next;
	}
}

#endif /* HAVE_DNSA */
