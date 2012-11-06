/* drf.c:
 * 
 * Contains functions to display the forward zones and list the
 * forward zones contained in the database. 
 * 
 * (C) Iain M Conochie 2012 <iain@ailsatech.net>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "forward.h"

int dzf (char *domain, dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW my_row;
	zone_info_t zone_info, *zi;
	record_row_t row_data, *rd;
	size_t min, max, len, tabs, i;
	my_ulonglong dnsa_rows, start;
	int error;
	char *tmp, *host, *error_code;
	const char *unix_socket, *dnsaquery, *error_str;
	unix_socket = dc->socket;
	tmp = malloc(FILE_S * sizeof(char));
	host = malloc(TBUFF_S * sizeof(char));
	error_code = malloc(RBUFF_S * sizeof(char));
	error_str = error_code;
	zi = &zone_info;
	rd = &row_data;
	min = strlen(domain) + 2;
	max = min;
	start = 0;
	/* Initialise MYSQL connection and query */
	sprintf(tmp, "SELECT * FROM zones WHERE name = '%s'", domain);
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa,
		dc->host, dc->user, dc->pass, dc->db, dc->port, unix_socket, dc->cliflag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&dnsa));
	}
	dnsaquery = tmp;
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
		report_error(NO_DOMAIN, domain);
	else if (dnsa_rows > 1)
		report_error(MULTI_DOMAIN, domain);
	/* Get the zone info from the DB */
	while ((my_row = mysql_fetch_row(dnsa_res)))
		zone_info = fill_zone_data(my_row);
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d ORDER BY type", zi->id);
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
		report_error(NO_RECORDS, domain);
	/** Check for the size of the sting $host.$domain. and if bigger than
	 ** the previous largest then save the result. This will go through all
	 ** the records and then find the largest string. We can then format
	 ** the output correctly
	 **/
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(my_row);
		sprintf(host, "%s.%s.", rd->host, domain);
		len = strlen(host);
		if (len > max) {
			max = len;
		}
	}
	sprintf(host, "%s.", zi->name);
	len = strlen(host);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		host[len + i] = '\t';
	}
	host[len + i] = '\0';
	printf("%s\t%d\tIN\tSOA\t%s. hostmaster.%s. ",
	       host, zi->ttl, zi->pri_dns, zi->name);
	printf("%d %d %d %d %d\n", zi->serial, zi->refresh, zi->retry,
	       zi->expire, zi->ttl);
	mysql_data_seek(dnsa_res, start);	/* rewind MYSQL results */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(my_row);
		sprintf(host, "%s.%s.", rd->host, domain);
		len = strlen(host);
		tabs = (max / 8) - (len / 8);
		for (i = 0; i < tabs; i++) {
			host[len + i] = '\t';
		}
		host[len + i] = '\0';
		if ((rd->pri == 0)) { /* Not an MX record so no PRI */
			printf("%s\t%d\tIN\t%s\t%s\n", host,
			       zi->ttl, rd->type, rd->dest);
		} else if ((rd->pri > 0)) { /* MX record so add PRI */
			printf("%s\t%d\tIN\t%s\t%d\t%s\n", host,
			       zi->ttl, rd->type, rd->pri, rd->dest);
		}
	}
	sprintf(host, "%s.", zi->name);
	len = strlen(host);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		host[len + i] = '\t';
	}
	host[len + i] = '\0';
	printf("%s\t%d\tIN\tSOA\t%s. hostmaster.%s. ",
	       host, zi->ttl, zi->pri_dns, zi->name);
	printf("%d %d %d %d %d\n", zi->serial, zi->refresh, zi->retry,
	       zi->expire, zi->ttl);
	free(tmp);
	free(host);
	free(error_code);
	return error;
}

int list_zones (dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW my_row;
	my_ulonglong dnsa_rows, start;
	size_t max, len, tabs, i;
	int error;
	char *tmp, *domain, *error_code;
	const char *unix_socket, *dnsaquery, *error_str;
	unix_socket = dc->socket;
	max = len = 0;
	start = 0;
	tmp = malloc(FILE_S * sizeof(char));
	domain = malloc(TBUFF_S * sizeof(char));
	error_code = malloc(RBUFF_S * sizeof(char));
	error_str = error_code;
	printf("Listing zones from database %s\n", dc->db);
	sprintf(tmp, "SELECT name, valid FROM zones ORDER BY name");
	dnsaquery = tmp;
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa,
		dc->host, dc->user, dc->pass, dc->db, dc->port, unix_socket, dc->cliflag ))) {
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
	 * longest domain */
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		sprintf(domain, "%s", my_row[0]);
		len = strlen(domain);
		if (len > max)
			max = len;
	}
	sprintf(domain, "Domain");
	len = strlen(domain);
	tabs = (max / 8) - (len / 8);
	for (i = 0; i < tabs; i++) {
		domain[len + i] = '\t';
	}
	domain[len + i] = '\0';
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