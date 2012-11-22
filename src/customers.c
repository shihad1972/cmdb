/* customers.c
 * 
 * Functions relating to customers in the database for the cmdb program
 * 
 * (C) 2012 Iain M Conochie
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "mysqlfunc.h"
#include "cmdb_mysql.h"

MYSQL cmdb;
MYSQL_RES *cmdb_res;
MYSQL_ROW cmdb_row;
my_ulonglong cmdb_rows, start;
size_t max, len, tabs, i;
char *error_code;
const char *unix_socket, *cmdb_query, *error_str;

int display_customer_info(char *customer, char *uuid, cmdb_config_t *config)
{
	char *cquery;
	
	if (!(cquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cquery in display_customer_info");
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in display_customer_info");
	error_str = error_code;
	cmdb_query = cquery;
	if ((strncmp(customer, "NULL", CONF_S))) {
		sprintf(cquery, "SELECT address, city, county, postcode, coid FROM customer WHERE name = '%s'",
			customer);
		cmdb_mysql_init(config, &cmdb);
		cmdb_mysql_query(&cmdb, cmdb_query);
		if (!(cmdb_res = mysql_store_result(&cmdb))) {
			snprintf(error_code, CONF_S, "%s", mysql_error(&cmdb));
			report_error(MY_STORE_FAIL, error_str);
		}
		
		if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
			report_error(CUSTOMER_NOT_FOUND, customer);
		else if (cmdb_rows > 1)
			report_error(MULTIPLE_SERVERS, customer);
		
		cmdb_row = mysql_fetch_row(cmdb_res);
		printf("Customer %s info\nCOID: %s\n", customer, cmdb_row[4]);
		printf("Address: %s\n", cmdb_row[0]);
		printf("City: %s\n", cmdb_row[1]);
		printf("County: %s\n", cmdb_row[2]);
		printf("Postcode: %s\n", cmdb_row[3]);
		mysql_close(&cmdb);
	}
	free(cquery);
	free(error_code);
	return 0;
}