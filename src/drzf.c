/* drzf.c: 
 * 
 * Contains functions to display the reverse zones
 * contained in the database and also to list the reverse
 * zones contained in the database.
 * 
 * (C) Iain M Conochie 2012 <iain@ailsatech.net>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "reverse.h"

int drzf (int id, char *domain, char config[][CONF_S])
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t min, max, len, tabs, i;
	rev_zone_info_t rev_zone_info, *rzi;
	rev_record_row_t rev_row, *rr;
	my_ulonglong dnsa_rows, start;
	int error;
	unsigned int port;
	unsigned long int client_flag;
	char *tmp, *dquery, *error_code, *in_addr;
	const char *unix_socket, *dnsa_query, *error_str;
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	dquery = calloc(BUFF_S, sizeof(char));
	tmp = calloc(BUFF_S, sizeof(char));
	in_addr = calloc(RBUFF_S, sizeof(char));
	error_code = malloc(RBUFF_S * sizeof(char));
	error_str = error_code;
	rzi = &rev_zone_info;
	rr = &rev_row;
	start = 0;
	/* Initialise MYSQL connection and query */
	sprintf(dquery, "SELECT * FROM rev_zones WHERE rev_zone_id = '%d'", id);
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa,
		config[HOST], config[USER], config[PASS], config[DB], port, unix_socket, client_flag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&dnsa));
	}
	dnsa_query = dquery;
	error = mysql_query(&dnsa, dnsa_query);
	snprintf(error_code, CONF_S, "%d", error);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
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
	sprintf(dquery, "SELECT host, destination FROM rev_records WHERE rev_zone = '%d'",
		id);
	error = mysql_query(&dnsa, dnsa_query);
	snprintf(error_code, CONF_S, "%d", error);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	get_in_addr_string(in_addr, rzi->net_range);
	/* calculate how many tabs we will need to format the output */
	len = strlen(in_addr);
	min = len + 1;
	max = min;
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
	free(in_addr);
	free(dquery);
	free(tmp);
	free(error_code);
	return 0;
}

int list_rev_zones (char config[][CONF_S])
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW my_row;
	my_ulonglong dnsa_rows, start;
	size_t max, len, tabs, i;
	int error;
	unsigned int port;
	unsigned long int client_flag;
	char *tmp, *domain, *error_code;
	const char *unix_socket, *dnsaquery, *error_str;
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	max = len = 0;
	start = 0;
	domain = malloc(TBUFF_S * sizeof(char));
	tmp = malloc(FILE_S * sizeof(char));
	error_code = malloc(RBUFF_S * sizeof(char));
	error_str = error_code;
	printf("Listing rev zones from DB %s\n", config[DB]);
	sprintf(tmp, "SELECT net_range, valid FROM rev_zones");
	dnsaquery = tmp;
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa,
		config[HOST], config[USER], config[PASS], config[DB], port, unix_socket, client_flag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&dnsa));
	}
	error = mysql_query(&dnsa, dnsaquery);
	snprintf(error_code, CONF_S, "%d", error);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0))
		report_error(DOMAIN_LIST_FAIL, error_str);
	/* To format the output, we need to know the string length of the
	 * longest reverse domain */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		sprintf(domain, "%s", my_row[0]);
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
	printf("%s\tValid\n\n", domain);
	mysql_data_seek(dnsa_res, start);	/* rewind MYSQL results */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		sprintf(domain, "%s", my_row[0]);
		len = strlen(domain);
		tabs = (max / 8) - (len / 8);
		for (i = 0; i < tabs; i++) {
			domain[len + i] = '\t';
		}
		domain[len + i] = '\0';
		printf("%s\t%s\n", domain, my_row[1]);
	}
	free(domain);
	free(tmp);
	free(error_code);
	return 0;
}