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
my_ulonglong cmdb_rows;
const char *unix_socket, *cmdb_query;

int display_customer_info(char *customer, char *coid, cmdb_config_t *config)
{
	if ((strncmp(customer, "NULL", CONF_S)))
		display_customer_info_on_name(customer, config);
	else if ((strncmp(coid, "NULL", CONF_S)))
		display_customer_info_on_coid(coid, config);
	mysql_library_end();
	return 0;
}

int display_customer_info_on_coid(char *coid, cmdb_config_t *config)
{
	char *cquery;
	long unsigned int cust_id;
	
	if (!(cquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cquery in display_customer_info_on_coid");
	cmdb_query = cquery;
	sprintf(cquery, "SELECT address, city, county, postcode, name, cust_id, coid FROM customer WHERE coid = '%s'", coid);
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
		report_error(CUSTOMER_NOT_FOUND, coid);
	else if (cmdb_rows > 1)
		report_error(MULTIPLE_CUSTOMERS, coid);
	cmdb_row = mysql_fetch_row(cmdb_res);
	cust_id = strtoul(cmdb_row[5], NULL, 10);
	display_customer_from_coid(cmdb_row);
	mysql_free_result(cmdb_res);
	sprintf(cquery, "SELECT name, phone, email FROM contacts WHERE cust_id = %ld",
		cust_id);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
		printf("No contacts for customer %s\n", coid);
	else {
		while ((cmdb_row = mysql_fetch_row(cmdb_res))) {
			printf("Contact for %s: %s %s %s\n",
			       coid, cmdb_row[0], cmdb_row[1], cmdb_row[2]);
		}
	}
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	free(cquery);
	return 0;
}

int display_customer_info_on_name(char *name, cmdb_config_t *config)
{
	char *cquery;
	long unsigned int cust_id;
	
	if (!(cquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cquery in display_customer_info_on_coid");
	cmdb_query = cquery;
	sprintf(cquery,
		"SELECT address, city, county, postcode, coid, cust_id, name FROM customer WHERE name = '%s'", name);
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
		report_error(CUSTOMER_NOT_FOUND, name);
	else if (cmdb_rows > 1)
		report_error(MULTIPLE_SERVERS, name);
	cmdb_row = mysql_fetch_row(cmdb_res);
	cust_id = strtoul(cmdb_row[5], NULL, 10);
	display_customer_from_name(cmdb_row);
	mysql_free_result(cmdb_res);
	sprintf(cquery, "SELECT name, phone, email FROM contacts WHERE cust_id = %ld",
		cust_id);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0))
		printf("No contacts for customer %s\n", name);
	else {
		while ((cmdb_row = mysql_fetch_row(cmdb_res))) {
			printf("Contact for %s: %s %s %s\n",
			       cmdb_row[4], cmdb_row[0], cmdb_row[1], cmdb_row[2]);
		}
	}
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	free(cquery);
	return 0;
}
void display_all_customers(cmdb_config_t *config)
{
	MYSQL cust;
	MYSQL_RES *cust_res;
	MYSQL_ROW cust_row;
	char *cust_query, *cust_coid;
	char cust_name[5];
	const char *cust_cmdb_query;
	
	if (!(cust_query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cust_query in display_all_customers");
	if (!(cust_coid = calloc(RANGE_S, sizeof(char)))) 
		report_error(MALLOC_FAIL, "cust_coid in display_all_customers");
	cust_cmdb_query = cust_query;
	sprintf(cust_query, "SELECT coid FROM customer");
	sprintf(cust_name, "NULL");
	cmdb_mysql_init(config, &cust);
	cmdb_mysql_query(&cust, cust_cmdb_query);
	if (!(cust_res = mysql_store_result(&cust))) {
		report_error(MY_STORE_FAIL, mysql_error(&cust));
	}
	while ((cust_row = mysql_fetch_row(cust_res))) {
		sprintf(cust_coid, "%s", cust_row[0]);
		printf("\n####################\n");
		display_customer_info_on_coid(cust_coid, config);
	}
	mysql_free_result(cust_res);
	mysql_close(&cust);
	mysql_library_end();
	free(cust_coid);
	free(cust_query);
}

void display_customer_from_name(char **cust_info)
{
	printf("Customer %s info\nCOID: %s\n", cust_info[6], cust_info[4]);
	printf("Address: %s\n", cust_info[0]);
	printf("City: %s\n", cust_info[1]);
	printf("County: %s\n", cust_info[2]);
	printf("Postcode: %s\n", cust_info[3]);
}

void display_customer_from_coid(char **cust_info)
{
	printf("Customer %s info\nCOID: %s\n", cust_info[4], cust_info[6]);
	printf("Address: %s\n", cmdb_row[0]);
	printf("City: %s\n", cmdb_row[1]);
	printf("County: %s\n", cmdb_row[2]);
	printf("Postcode: %s\n", cmdb_row[3]);
}
