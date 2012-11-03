#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "write_zone.h"

int dzf (char *domain, char config[][CONF_S])
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW my_row;
	zone_info_t zone_info, *zi;
	record_row_t row_data, *rd;
	my_ulonglong dnsa_rows;
	int error;
	unsigned int port;
	unsigned long int client_flag;
	char *tmp, *error_code;
	const char *unix_socket, *dnsaquery, *error_str;
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	tmp = malloc(FILE_S * sizeof(char));
	error_code = malloc(RBUFF_S * sizeof(char));
	error_str = error_code;
	zi = &zone_info;
	rd = &row_data;
	/* Initialise MYSQL connection and query */
	sprintf(tmp, "SELECT * FROM zones WHERE name = '%s'", domain);
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa,
		config[HOST], config[USER], config[PASS], config[DB], port, unix_socket, client_flag ))) {
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
	printf("%s.\t\t%d\tIN\tSOA\t%s. hostmaster.%s. ",
	       zi->name, zi->ttl, zi->pri_dns, zi->name);
	printf("%d %d %d %d %d\n", zi->serial, zi->refresh, zi->retry,
	       zi->expire, zi->ttl);
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d", zi->id);
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
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(my_row);
		if ((rd->pri == 0)) {
			printf("%s.%s.\t%d\tIN\t%s\t%s\n", rd->host, domain,
			       zi->ttl, rd->type, rd->dest);
		} else if ((rd->pri > 0)) {
			printf("%s.%s.\t%d\tIN\t%s\t%d %s\n", rd->host, domain,
			       zi->ttl, rd->type, rd->pri, rd->dest);
		}
	}
	printf("%s.\t\t%d\tIN\tSOA\t%s. hostmaster.%s. ",
	       zi->name, zi->ttl, zi->pri_dns, zi->name);
	printf("%d %d %d %d %d\n", zi->serial, zi->refresh, zi->retry,
	       zi->expire, zi->ttl);
	return error;
}