/* 
 *
 *  cmdb: Configuration Management Database
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
 *  customers.c
 * 
 *  Functions relating to customers in the database for the cmdb program
 * 
 *  (C) Iain M Conochie 2012 - 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */


void
display_customer_info(char *name, char *coid, cmdb_config_t *config)
{
	int retval, i;
	cmdb_customer_t *list;
	cmdb_t *cmdb;
	retval = i = 0;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_t in display_customer_info");
	cmdb_init_struct(cmdb);

	cmdb->customer = '\0';
	if ((retval = run_query(config, cmdb, CUSTOMER)) != 0) {
		cmdb_clean_list(cmdb);
		free(cmdb);
		return;
	}
	list = cmdb->customer;
	while(list) {
		if ((strncmp(list->name, name, MAC_S) == 0)) {
			print_customer_details(list);
			list = list->next;
			i++;
		} else if ((strncmp(list->coid, coid, CONF_S) == 0)) {
			print_customer_details(list);
			list = list->next;
			i++;
		} else {
			list = list->next;
		}
	}
	cmdb_clean_list(cmdb);
	free(cmdb);
	if (i == 0)
		printf("No customer found\n");
	return;
}

void
display_all_customers(cmdb_config_t *config)
{
	int retval;
	cmdb_customer_t *list;
	cmdb_t *cmdb;
	retval = 0;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_t in display_customer_info");
	cmdb_init_struct(cmdb);

	cmdb->customer = '\0';
	if ((retval = run_query(config, cmdb, CUSTOMER)) != 0) {
		cmdb_clean_list(cmdb);
		free(cmdb);
		return;
	}
	list = cmdb->customer;
	while(list) {
			printf("%s\n%s\n\n", list->name, list->coid);
			list = list->next;
	}
	cmdb_clean_list(cmdb);
	free(cmdb);
	return;
}

void
clean_customer_list(cmdb_customer_t *list)
{
	int i;
	cmdb_customer_t *customer, *next;

	i = 0;
	customer = list;
	next = customer->next;
	while (customer) {
		free(customer);
		i++;
		customer = next;
		if (next) {
			next = customer->next;
		} else {
			next = '\0';
		}
	}
}

void
print_customer_details(cmdb_customer_t *cust)
{
	printf("Customer Details\n");
	printf("Name:\t\t%s\n", cust->name);
	printf("Address:\t%s\n", cust->address);
	printf("City:\t\t%s\n", cust->city);
	printf("County:\t\t%s\n", cust->county);
	printf("Postcode:\t%s\n", cust->postcode);
	printf("COID:\t\t%s\n", cust->coid);
}
/*
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
			       name, cmdb_row[0], cmdb_row[1], cmdb_row[2]);
		}
	}
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	free(cquery);
	return 0;
}
*/
/*
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
		mysql_close(&cust);
		mysql_library_end();
		free(cust_query);
		free(cust_coid);
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

cmdb_customer_t *create_customer_node(void)
{
	cmdb_customer_t *head;
	
	if (!(head = malloc(sizeof(cmdb_customer_t))))
		report_error(MALLOC_FAIL, "head in create_customer_node");
	head->cust_id = 0;
	snprintf(head->name, HOST_S, "NULL");
	snprintf(head->address, NAME_S, "NULL");
	snprintf(head->city, HOST_S, "NULL");
	snprintf(head->county, MAC_S, "NULL");
	snprintf(head->postcode, RANGE_S, "NULL");
	snprintf(head->coid, RANGE_S, "NULL");
	head->next = NULL;
	
	return head;
}

cmdb_customer_t *add_customer_node(cmdb_customer_t *head, MYSQL_ROW myrow)
{
	cmdb_customer_t *new, *saved;
	if ((strncmp(head->name, "NULL", HOST_S) != 0))
		new = create_customer_node();
	else
		new = head;
	new->cust_id = strtoul(myrow[0], NULL, 10);
	snprintf(new->name, HOST_S, "%s", myrow[1]);
	snprintf(new->address, NAME_S, "%s", myrow[2]);
	snprintf(new->city, HOST_S, "%s", myrow[3]);
	snprintf(new->county, MAC_S, "%s", myrow[4]);
	snprintf(new->postcode, RANGE_S, "%s", myrow[5]);
	snprintf(new->coid, RANGE_S, "%s", myrow[6]);
	
	saved = head;
	while (saved->next)
		saved = saved->next;
	if (new != head)
		saved->next = new;
	return head;
}

unsigned long int get_customer_for_server(cmdb_config_t *config)
{
	cmdb_customer_t *head, *saved;
	size_t len;
	char *query, *coid;
	int retval;
	unsigned long int id;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_customer_for_server");
	if (!(coid = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "coid in get_customer_for_server");
	head = create_customer_node();
	snprintf(query, RBUFF_S,
"SELECT cust_id, name, address, city, county, postcode, coid FROM customer");
	cmdb_query = query;
	cmdb_mysql_init(config, &cmdb);
	cmdb_mysql_query(&cmdb, cmdb_query);
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		mysql_close(&cmdb);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, "in get_customer_for_server");
	}
	if ((cmdb_rows = mysql_num_rows(cmdb_res)) == 0) {
		mysql_free_result(cmdb_res);
		mysql_close(&cmdb);
		mysql_library_end();
		free(query);
		report_error(NO_CUSTOMERS, "NULL");
	}
	printf("Please input the ID of the customer for this server\n");
	printf("ID\t\tCoid\t\tName\n");
	while ((cmdb_row = mysql_fetch_row(cmdb_res))) {
		head = add_customer_node(head, cmdb_row);
	}
	mysql_free_result(cmdb_res);
	mysql_close(&cmdb);
	mysql_library_end();
	saved = head;
	printf("Please input the ID of the customer for this server\n");
	printf("ID\t\tCoid\t\tName\n");
	while (saved) {
		len = strlen(saved->coid);
		if ((saved->cust_id > 9999999)) {
			if ((len / 8) >0) {
				printf("%lu\t%s\t%s\n", 
				 saved->cust_id, saved->coid, saved->name);
			} else {
				printf("%lu\t%s\t\t%s\n",
				 saved->cust_id, saved->coid, saved->name);
			}
		} else {
			if ((len / 8) >0) {
				printf("%lu\t\t%s\t%s\n", 
				 saved->cust_id, saved->coid, saved->name);
			} else {
				printf("%lu\t\t%s\t\t%s\n",
				 saved->cust_id, saved->coid, saved->name);
			}
		}
		saved = saved->next;
	}
	query = fgets(query, CONF_S, stdin);
	chomp(query);
	while ((retval = validate_user_input(query, ID_REGEX)) < 0) {
		while ((retval = validate_user_input(query, COID_REGEX)) < 0) {
			printf("User input not valid!\n");
			printf("Please input the ID or COID of the customer for this server\n");
			query = fgets(query, CONF_S, stdin);
			chomp(query);
		}
	}
	
	if ((retval = validate_user_input(query, ID_REGEX) > 0)) {
		id = strtoul(query, NULL, 10);
		free(coid);
		free(query);
		while (head) {
			saved = head->next;
			free(head);
			head = saved;
		}
		return id;
	} else {
		snprintf(coid, RANGE_S, "%s", query);
		while (saved) {
			if ((strncmp(saved->coid, coid, RANGE_S) == 0)) {
				id = saved->cust_id;
				free(coid);
				free(query);
				while (head) {
					saved = head->next;
					free(head);
					head = saved;
				}
				return id;
			} else {
				saved = saved->next;
			}
		}
	}
	free(coid);
	free(query);
	while (head) {
		saved = head->next;
		free(head);
		head = saved;
	}
	return 0;
}
*/
