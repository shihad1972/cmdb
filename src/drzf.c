/* drzf.c: 
 * 
 * Contains functions to display the reverse zones
 * contained in the database and also to list the reverse
 * zones contained in the database.
 * 
 * Part of the DNSA  program
 * 
 * (C) Iain M Conochie 2012
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "dnsa.h"
#include "reverse.h"
#include "mysqlfunc.h"

MYSQL dnsa;
MYSQL_RES *dnsa_res;
MYSQL_ROW dnsa_row;
my_ulonglong dnsa_rows, start;
size_t max, len, tabs, i;
const char *unix_socket, *dnsa_query, *error_str;

int drzf (int id, char *domain, dnsa_config_t *dc)
{
	rev_zone_info_t rev_zone_info, *rzi;
	rev_record_row_t rev_row, *rr;
	char *tmp, *dquery, *error_code, *in_addr;
	
	if (!(dquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dquery in drzf");
	if (!(tmp = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in drzf");
	if (!(in_addr = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "in_addr in drzf");
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in drzf");
	
	error_str = error_code;
	dnsa_query = dquery;
	unix_socket = dc->socket;
	rzi = &rev_zone_info;
	rr = &rev_row;
	
	start = 0;
	
	/* Initialise MYSQL connection and query */
	sprintf(dquery, "SELECT * FROM rev_zones WHERE rev_zone_id = '%d'", id);
	dnsa_mysql_init(dc, &dnsa);
	dnsa_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		report_error(NO_DOMAIN, domain);
	} else if (dnsa_rows > 1) {
		report_error(MULTI_DOMAIN, domain);
	}
	
	/* Get the information for the reverse zone */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_zone_info = fill_rev_zone_data(dnsa_row);
	}
	
	/* Get the reverse zone records */
	sprintf(dquery, "SELECT host, destination FROM rev_records WHERE rev_zone = '%d'", id);
	dnsa_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	
	get_in_addr_string(in_addr, rzi->net_range);
	/* calculate how many tabs we will need to format the output */
	
	max = strlen(in_addr);
	
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_row = get_rev_row(dnsa_row);
		sprintf(tmp, "%s.%s.", rr->host, in_addr);
		len = strlen(tmp);
		if (len > max) {
			max = len;
		}
	}
	sprintf(tmp, "%s.", in_addr);
	len = strlen(tmp);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		tmp[len + i] = '\t';
	}
	tmp[len + i] = '\0';
	
	printf("%s\t%d IN SOA\t%s. %s. ", tmp, rzi->ttl, rzi->pri_dns,
	       rzi->hostmaster);
	printf("%d %d %d %d %d\n", rzi->serial, rzi->refresh, rzi->retry,
	       rzi->expire, rzi->ttl);

	mysql_data_seek(dnsa_res, start);	/* rewind MYSQL results */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_row = get_rev_row(dnsa_row);
		sprintf(tmp, "%s.%s.", rr->host, in_addr);
		len = strlen(tmp);
		tabs = (max / 8) - (len / 8);
		for (i = 0; i < tabs; i++) {
			tmp[len + i] = '\t';
		}
		tmp[len + i] = '\0';
		printf("%s\t%d IN\tPTR\t%s\n", tmp, rzi->ttl,
		       rr->dest);
	}
	sprintf(tmp, "%s.", in_addr);
	len = strlen(tmp);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		tmp[len + i] = '\t';
	}
	tmp[len + i] = '\0';

	printf("%s\t%d IN SOA\t%s. %s. ", tmp, rzi->ttl, rzi->pri_dns,
	       rzi->hostmaster);
	printf("%d %d %d %d %d\n", rzi->serial, rzi->refresh, rzi->retry,
	       rzi->expire, rzi->ttl);

	mysql_close(&dnsa);
	free(in_addr);
	free(dquery);
	free(tmp);
	free(error_code);
	error_str = 0;
	return 0;
}

int list_rev_zones (dnsa_config_t *dc)
{
	char *tmp, *domain, *error_code;

	if (!(domain = malloc(TBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain in list_rev_zones");
	if (!(tmp = malloc(FILE_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in list_rev_zones");
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in list_rev_zones");

	unix_socket = dc->socket;
	dnsa_query = tmp;
	error_str = error_code;

	sprintf(tmp, "SELECT net_range, valid FROM rev_zones");
	max = len = 0;
	start = 0;

	dnsa_mysql_init(dc, &dnsa);
	dnsa_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0))
		report_error(DOMAIN_LIST_FAIL, error_str);
	/* 
	 * To format the output, we need to know the string length of the
	 * longest reverse domain 
	 */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		sprintf(domain, "%s", dnsa_row[0]);
		len = strlen(domain);
		if (len > max)
			max = len;
	}
	sprintf(domain, "Reverse Zone");
	len = strlen(domain);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		domain[len + i] = '\t';
	}

	printf("Listing rev zones from DB %s\n", dc->db);
	printf("%s\tValid\n\n", domain);

	mysql_data_seek(dnsa_res, start);	/* rewind MYSQL results */

	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		sprintf(domain, "%s", dnsa_row[0]);
		len = strlen(domain);
		tabs = (max / 8) - (len / 8);
		for (i = 0; i < tabs; i++) {
			domain[len + i] = '\t';
		}
		domain[len + i] = '\0';
		printf("%s\t%s\n", domain, dnsa_row[1]);
	}
	
	mysql_close(&dnsa);
	free(domain);
	free(tmp);
	free(error_code);
	error_str = 0;
	return 0;
}