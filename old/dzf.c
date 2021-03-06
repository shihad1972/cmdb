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
 *  dzf.c:
 * 
 *  Contains functions to display the forward zones and list the
 *  forward zones contained in the database. 
 * 
 *  Part of the DNSA  program
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "forward.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"

MYSQL dnsa;
MYSQL_RES *dnsa_res;
MYSQL_ROW my_row;
my_ulonglong dnsa_rows, start;
size_t max, len, tabs, i;
char *tmp;
const char *unix_socket, *dnsaquery;

int dzf (char *domain, dnsa_config_t *dc)
{
	zone_info_t zone_info, *zi;
	record_row_t row_data, *rd;
	char *host;
	
	if (!(tmp = malloc(FILE_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in dzf");
	if (!(host = malloc(TBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "host in dzf");
	
	unix_socket = dc->socket;
	zi = &zone_info;
	rd = &row_data;
	dnsaquery = tmp;
	
	max = strlen(domain) + 2;
	start = 0;
	
	/* Initialise MYSQL connection and query */
	snprintf(tmp, FILE_S,
"SELECT * FROM zones WHERE name = '%s'", domain);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsaquery);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, tmp);
		free(host);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
		free(host);
		report_error(NO_DOMAIN, domain);
	} else if (dnsa_rows > 1)
		report_error(MULTI_DOMAIN, domain);
	
	/* Get the zone info from the DB */
	while ((my_row = mysql_fetch_row(dnsa_res)))
		zone_info = fill_zone_data(my_row);
	mysql_free_result(dnsa_res);
	snprintf(tmp, FILE_S,
"SELECT * FROM records WHERE zone = %d ORDER BY type", zi->id);
	cmdb_mysql_query(&dnsa, dnsaquery);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, tmp);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
		report_error(NO_RECORDS, domain);
	}
	/*
	 * Check for the size of the string $host.$domain. and if bigger than
	 * the previous largest then save the result. This will go through all
	 * the records and then find the largest string. We can then format
	 * the output correctly
	 */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(my_row);
		snprintf(host, TBUFF_S, "%s.%s.", rd->host, domain);
		len = strlen(host);
		if (len > max) {
			max = len;
		}
	}
	snprintf(host, TBUFF_S, "%s.", zi->name);
	len = strlen(host);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		host[len + i] = '\t';
	}
	host[len + i] = '\0';
	
	printf("%s\t%ld\tIN\tSOA\t%s. hostmaster.%s. ",
	       host, zi->ttl, zi->pri_dns, zi->name);
	printf("%ld %ld %ld %ld %ld\n", zi->serial, zi->refresh, zi->retry,
	       zi->expire, zi->ttl);

	mysql_data_seek(dnsa_res, start);	/* rewind MYSQL results */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(my_row);
		snprintf(host, TBUFF_S, "%s.%s.", rd->host, domain);
		len = strlen(host);
		tabs = (max / 8) - (len / 8);
		for (i = 0; i < tabs; i++) {
			host[len + i] = '\t';
		}
		host[len + i] = '\0';
		if ((rd->pri == 0)) { /* Not an MX record so no PRI */
			printf("%s\t%ld\tIN\t%s\t%s\n", host,
			       zi->ttl, rd->type, rd->dest);
		} else if ((rd->pri > 0)) { /* MX record so add PRI */
			printf("%s\t%ld\tIN\t%s\t%d\t%s\n", host,
			       zi->ttl, rd->type, rd->pri, rd->dest);
		}
	}
	
	snprintf(host, TBUFF_S, "%s.", zi->name);
	len = strlen(host);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		host[len + i] = '\t';
	}
	host[len + i] = '\0';
	
	printf("%s\t%ld\tIN\tSOA\t%s. hostmaster.%s. ",
	       host, zi->ttl, zi->pri_dns, zi->name);
	printf("%ld %ld %ld %ld %ld\n", zi->serial, zi->refresh, zi->retry,
	       zi->expire, zi->ttl);
	cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
	free(host);
	return 0;
}

int list_zones (dnsa_config_t *dc)
{
	char *domain;

	if (!(tmp = malloc(FILE_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in list_zones");
	if (!(domain = malloc(TBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain in list_zones");
	
	unix_socket = dc->socket;
	dnsaquery = tmp;
	
	max = len = 0;
	start = 0;
	
	printf("Listing zones from database %s\n", dc->db);
	snprintf(tmp, FILE_S, "SELECT name, valid FROM zones ORDER BY name");
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsaquery);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, tmp);
		free(domain);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
		free(domain);
		report_error(DOMAIN_LIST_FAIL, domain);
	}
	/* 
	 * To format the output, we need to know the string length of the
	 * longest domain 
	 */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		snprintf(domain, TBUFF_S, "%s", my_row[0]);
		len = strlen(domain);
		if (len > max)
			max = len;
	}
	snprintf(domain, TBUFF_S, "Domain");
	len = strlen(domain);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		domain[len + i] = '\t';
	}
	domain[len + i] = '\0';
	
	printf("%s\tValid\n\n", domain);
	
	mysql_data_seek(dnsa_res, start);	/* rewind MYSQL results */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		snprintf(domain, TBUFF_S, "%s", my_row[0]);
		len = strlen(domain);
		tabs = (max / 8) - (len / 8);
		for (i = 0; i < tabs; i++) {
			domain[len + i] = '\t';
		}
		domain[len + i] = '\0';
		printf("%s\t%s\n", domain, my_row[1]);
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, domain);
	free(tmp);
	return 0;
}