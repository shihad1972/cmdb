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
	if ((retval = run_multiple_query(config, cmdb,
CUSTOMER | CONTACT | SERVICE)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	list = cmdb->customer;
	while(list) {
		if ((strncmp(list->name, name, MAC_S) == 0)) {
			print_customer_details(list, cmdb);
			list = list->next;
			i++;
		} else if ((strncmp(list->coid, coid, CONF_S) == 0)) {
			print_customer_details(list, cmdb);
			list = list->next;
			i++;
		} else {
			list = list->next;
		}
	}
	cmdb_clean_list(cmdb);
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
	size_t len;
	retval = 0;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_t in display_customer_info");
	cmdb_init_struct(cmdb);

	cmdb->customer = '\0';
	if ((retval = run_query(config, cmdb, CUSTOMER)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	list = cmdb->customer;
	while(list) {
		if ((len = strlen(list->coid)) < 8)
			printf("%s\t %s\n", list->coid, list->name);
		else
			printf("%s %s\n", list->coid, list->name);
		list = list->next;
	}
	cmdb_clean_list(cmdb);
	return;
}

int
add_customer_to_database(cmdb_config_t *config, cmdb_t *cmdb)
{
	char *input;
	int retval;
	cmdb_customer_t *cust;

	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_customer_to_database");
	retval = 0;
	cust = cmdb->customer;
	printf("Details provided:\n");
	printf("Name:\t\t%s\n", cust->name);
	printf("Address:\t%s\n", cust->address);
	printf("City:\t\t%s\n", cust->city);
	printf("County:\t\t%s\n", cust->county);
	printf("Postcode:\t%s\n", cust->postcode);
	printf("COID:\t\t%s\n", cust->coid);
	printf("Are these detail correct? (y/n): ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		retval = run_insert(config, cmdb, CUSTOMERS);
	} else {
		retval = 1;
	}
	free(input);

	return retval;
}

void
display_service_types(cmdb_config_t *config)
{
	int retval;
	cmdb_service_type_t *list;
	size_t len;
	cmdb_t *cmdb;

	retval = 0;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb_t in display_customer_info");
	cmdb_init_struct(cmdb);

	cmdb->servicetype = '\0';
	if ((retval = run_query(config, cmdb, SERVICE_TYPE)) != 0) {
		cmdb_clean_list(cmdb);
		return;
	}
	list = cmdb->servicetype;
	printf("ID\tService\t\tDescription\n");
	while(list) {
		if ((len = strlen(list->service)) < 8)
			printf("%lu\t%s\t\t%s\n",
			list->service_id, list->service, list->detail);
		else
			printf("%lu\t%s\t%s\n",
			list->service_id, list->service, list->detail);
		list = list->next;
	}
	cmdb_clean_list(cmdb);
	return;
}

void
display_customer_services(cmdb_config_t *config, char *coid)
{
	int retval;
	cmdb_t *cmdb;
	cmdb_customer_t *cust;
	cmdb_service_t *service;

	if (!(cmdb = malloc(sizeof(cmdb_t))))
		report_error(MALLOC_FAIL, "cmdb in display server services");

	cmdb_init_struct(cmdb);
	if ((retval = run_multiple_query(config, cmdb, CUSTOMER | SERVICE)) != 0) {
		cmdb_clean_list(cmdb);
		printf("Query for server %s services failed\n", coid);
		return;
	}
	cust = cmdb->customer;
	service = cmdb->service;
	printf("Customer %s\n", coid);
	while (cust) {
		if ((strncmp(cust->coid, coid, MAC_S) == 0)) {
			print_services(service, cust->cust_id, CUSTOMER);
			cust = cust->next;
		} else {
			cust = cust->next;
		}
	}
	cmdb_clean_list(cmdb);
}

void
print_customer_details(cmdb_customer_t *cust, cmdb_t *cmdb)
{
	printf("%s: Coid %s\n", cust->name, cust->coid);
	printf("%s\n", cust->address);
	printf("%s, %s\n", cust->city, cust->postcode);
	print_customer_contacts(cmdb->contact, cust->cust_id);
	print_services(cmdb->service, cust->cust_id, CUSTOMER);
}

void
print_customer_contacts(cmdb_contact_t *contacts, unsigned long int cust_id)
{
	int i = 0;
	cmdb_contact_t *list;
	list = contacts;
	while (list) {
		if (list->cust_id == cust_id) {
			i++;
			if (i == 1)
				printf("\nContacts:\n");
			printf("%s, %s, %s\n", list->name, list->phone, list->email);
		}
		list = list->next;
	}
}

int
get_customer(cmdb_config_t *config, cmdb_t *cmdb, char *coid)
{
	int retval;
	cmdb_customer_t *cust, *next;

	if ((retval = run_query(config, cmdb, CUSTOMER)) != 0)
		return retval;

	cust = cmdb->customer;
	next = cust->next;
	while (cust) {
		if (strncmp(coid, cust->coid, RANGE_S) != 0) {
			free(cust);
			cust = next;
			if (cust)
				next = cust->next;
		} else {
			break;
		}
	}
	if (cust) {
		cmdb->customer = cust;
		cust = cust->next;
		while (cust) {
			next = cust->next;
			free(cust);
			cust = next;
		}
	} else {
		cmdb->customer = '\0';
		return CUSTOMER_NOT_FOUND;
	}
	cmdb->customer->next = '\0';
	return 0;
}
/*

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
