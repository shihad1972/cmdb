/* 
 *
 *  dnsa: DNS Administration
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
 *  commline.c:
 *  Contains functions to deal with command line arguments and also
 *  to read the values from the configuration file.
 *
 * drzf.c: 
 * 
 * Contains functions to display the reverse zones
 * contained in the database and also to list the reverse
 * zones contained in the database.
 * 
 * Part of the DNSA  program
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "reverse.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"

MYSQL dnsa;
MYSQL_RES *dnsa_res;
MYSQL_ROW dnsa_row;
my_ulonglong dnsa_rows, start;
size_t max, len, tabs, i;
const char *dnsa_query;

int drzf (int id, char *domain, dnsa_config_t *dc)
{
	rev_zone_info_t *rzi;
	rev_record_row_t rev_row, *rr;
	char *dquery, *in_addr;
	
	if (!(dquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dquery in drzf");
	
	dnsa_query = dquery;
	rr = &rev_row;
	
	start = 0;
	
	/* Initialise MYSQL connection and query */
	sprintf(dquery, "SELECT * FROM rev_zones WHERE rev_zone_id = '%d'", id);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, dquery);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
		report_error(NO_DOMAIN, domain);
	} else if (dnsa_rows > 1) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
		report_error(MULTI_DOMAIN, domain);
	}
	/* Get the information for the reverse zone */
	if (!(rzi = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rzi in drzf");
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		fill_rev_zone_data(dnsa_row, rzi);
	}
	mysql_free_result(dnsa_res);
	/* Get the reverse zone records */
	sprintf(dquery, "SELECT host, destination FROM rev_records WHERE rev_zone = '%d'", id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, dquery);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if ((dnsa_rows = mysql_num_rows(dnsa_res)) == 0) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
		printf("No records found for reverse domain %s/%lu\n",
		       domain, rzi->prefix);
		return 0;
	}
	if (!(in_addr = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "in_addr in drzf");
	get_in_addr_string2(in_addr, rzi->net_range, rzi->prefix);
	/* Minimum length of an in-addr.arpa string is 20, and the maximum
	 * is 28. Therefore we have to work out where on the 24 character
	 * tab boundary the output falls */
	len = strlen(in_addr);
	if (len < 23) {
		printf("%s.\t\t%ld IN SOA\t%s. %s. %ld %ld %ld %ld %ld\n",
		       in_addr, rzi->ttl, rzi->pri_dns, rzi->hostmaster,
		       rzi->serial, rzi->refresh, rzi->retry,rzi->expire,
		       rzi->ttl);
	} else {
		printf("%s.\t%ld IN SOA\t%s. %s. %ld %ld %ld %ld %ld\n",
		       in_addr, rzi->ttl, rzi->pri_dns, rzi->hostmaster,
		       rzi->serial, rzi->refresh, rzi->retry,rzi->expire,
		       rzi->ttl);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_row = get_rev_row(dnsa_row);
		len = strlen(rr->host) + strlen(in_addr) + 1;
		if (len < 23) {
			printf("%s.%s\t\t%ld IN\tPTR\t%s\n", rr->host, in_addr,
			       rzi->ttl, rr->dest);
		} else {
			printf("%s.%s\t%ld IN\tPTR\t%s\n", rr->host, in_addr,
			       rzi->ttl, rr->dest);
		}
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
	free(in_addr);
	free(rzi);
	return 0;
}

int list_rev_zones (dnsa_config_t *dc)
{
	char *tmp, *domain;

	if (!(domain = malloc(TBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain in list_rev_zones");
	if (!(tmp = malloc(FILE_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in list_rev_zones");

	dnsa_query = tmp;

	snprintf(tmp, FILE_S, "SELECT net_range, valid FROM rev_zones");
	max = len = 0;
	start = 0;

	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, tmp);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
		report_error(DOMAIN_LIST_FAIL, "in list_rev_zones");
	}
	printf("Listing rev zones from DB %s\n", dc->db);
	printf("Reverse Zone\tValid\n\n");
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		snprintf(domain, TBUFF_S, "%s", dnsa_row[0]);
		len = strlen(domain);
		if (len < 8) {
			domain[len] = '\t';
			domain[len + 1] = '\0';
		}
		printf("%s\t%s\n", domain, dnsa_row[1]);
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, domain);
	free(tmp);
	return 0;
}