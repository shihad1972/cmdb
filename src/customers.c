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
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "base_sql.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */


void
display_customer_info(char *name, char *coid, cmdb_config_s *config)
{
	int retval, i;
	cmdb_customer_s *list;
	cmdb_s *cmdb;
	retval = i = 0;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "cmdb_s in display_customer_info");
	cmdb_init_struct(cmdb);

	cmdb->customer = '\0';
	if ((retval = run_multiple_query(config, cmdb,
CUSTOMER | CONTACT | SERVICE | SERVER)) != 0) {
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
display_all_customers(cmdb_config_s *config)
{
	int retval;
	cmdb_customer_s *list;
	cmdb_s *cmdb;
	size_t len;
	retval = 0;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "cmdb_s in display_customer_info");
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
add_customer_to_database(cmdb_config_s *config, cmdb_s *cmdb)
{
	char *input;
	int retval;
	cmdb_customer_s *cust;

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

int
add_contact_to_database(cmdb_config_s *config, cmdb_s *cmdb)
{
	char *input;
	int retval;
	cmdb_contact_s *cont;

	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_customer_to_database");
	retval = 0;

	cont = cmdb->contact;
	if ((retval = run_search(config, cmdb, CUST_ID_ON_COID)) != 0) {
		printf("Unable to retrieve cust_id for COID %s\n",
		 cmdb->customer->coid);
		free(input);
		return retval;
	}
	cmdb->contact->cust_id = cmdb->customer->cust_id;
	printf("Details Provided:\n");
	printf("Name:\t\t%s\n", cont->name);
	printf("Phone No.\t%s\n", cont->phone);
	printf("Email:\t\t%s\n", cont->email);
	printf("COID:\t\t%s\n", cmdb->customer->coid);
	printf("Are these detail correct? (y/n): ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		retval = run_insert(config, cmdb, CONTACTS);
	} else {
		retval = 1;
	}
	free(input);
	return retval;
}

int
add_service_to_database(cmdb_config_s *config, cmdb_s *cmdb)
{
	char *input;
	int retval;

	if (!(input = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "input in add_customer_to_database");
	retval = 0;
	if ((retval = run_search(config, cmdb, CUST_ID_ON_COID)) != 0) {
		printf("Unable to retrieve cust_id for COID %s\n",
		 cmdb->customer->coid);
		free(input);
		return retval;
	}
	if ((retval = run_search(config, cmdb, SERVER_ID_ON_NAME)) != 0) {
		printf("Unable to retrieve server_id for server %s\n",
		 cmdb->server->name);
		free(input);
		return retval;
	}
	if ((retval = strncmp(cmdb->servicetype->service, "NULL", COMM_S)) != 0) {
		if ((retval = run_search(config, cmdb, SERV_TYPE_ID_ON_SERVICE)) != 0) {
			printf("Unable to retrieve service_type_id for %s\n",
			 cmdb->servicetype->service);
			free(input);
			return retval;
		} else {
			cmdb->service->service_type_id = cmdb->servicetype->service_id;
		}
	}
	cmdb->service->server_id = cmdb->server->server_id;
	cmdb->service->cust_id = cmdb->customer->cust_id;
	printf("Details Provided:\n");
	printf("Server:\t\t%s, id %lu\n", cmdb->server->name, cmdb->service->server_id);
	printf("Customer:\t%s, id %lu\n", cmdb->customer->coid, cmdb->service->cust_id);
	printf("Service:\t%s, id %lu\n", cmdb->servicetype->service, cmdb->service->service_type_id);
	printf("Are these detail correct? (y/n): ");
	input = fgets(input, CONF_S, stdin);
	chomp(input);
	if ((strncmp(input, "y", CH_S)) == 0 || (strncmp(input, "Y", CH_S) == 0)) {
		printf("Adding....\n");
		retval = run_insert(config, cmdb, SERVICES);
	} else {
		retval = 1;
	}

	return retval;
}
void
display_service_types(cmdb_config_s *config)
{
	int retval;
	cmdb_service_sype_t *list;
	size_t len;
	cmdb_s *cmdb;

	retval = 0;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "cmdb_s in display_service_types");
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
display_customer_services(cmdb_config_s *config, char *coid)
{
	int retval;
	cmdb_s *cmdb;
	cmdb_customer_s *cust;
	cmdb_service_s *service;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "cmdb in display_customer_services");

	cmdb_init_struct(cmdb);
	if ((retval = run_multiple_query(config, cmdb, CUSTOMER | SERVICE)) != 0) {
		cmdb_clean_list(cmdb);
		printf("Query for customer %s services failed\n", coid);
		return;
	}
	cust = cmdb->customer;
	service = cmdb->service;
	printf("Customer %s\n", coid);
	while (cust) {
		if ((strncmp(cust->coid, coid, COMM_S) == 0)) {
			retval = print_services(service, cust->cust_id, CUSTOMER);
			cust = cust->next;
		} else {
			cust = cust->next;
		}
	}
	if (retval == 0)
		printf("No services\n");
	cmdb_clean_list(cmdb);
}

void
display_customer_contacts(cmdb_config_s *config, char *coid)
{
	int retval, i;
	cmdb_s *cmdb;
	cmdb_customer_s *cust;
	cmdb_contact_s *contact;

	if (!(cmdb = malloc(sizeof(cmdb_s))))
		report_error(MALLOC_FAIL, "cmdb in display_customer_contacts");

	i = 0;
	cmdb_init_struct(cmdb);
	if ((retval = run_multiple_query(config, cmdb, CUSTOMER | CONTACT)) != 0) {
		cmdb_clean_list(cmdb);
		printf("Query for customer %s contacts failed\n", coid);
		return;
	}
	cust = cmdb->customer;
	contact = cmdb->contact;
	while (cust) {
		if (strncmp(cust->coid, coid, COMM_S) == 0) {
			if (i == 0) {
				printf("Customer %s\n", coid);
			}
			i++;
			retval = print_customer_contacts(contact, cust->cust_id);
			cust = cust->next;
		} else {
			cust = cust->next;
		}
	}
	if (retval == 0)
		printf("No contacts\n");
	cmdb_clean_list(cmdb);
}

void
print_customer_details(cmdb_customer_s *cust, cmdb_s *cmdb)
{
	printf("%s: Coid %s\n", cust->name, cust->coid);
	printf("%s\n", cust->address);
	printf("%s, %s\n", cust->city, cust->postcode);
	print_customer_contacts(cmdb->contact, cust->cust_id);
	print_customer_servers(cmdb->server, cust->cust_id);
	print_services(cmdb->service, cust->cust_id, CUSTOMER);
}

int
print_customer_contacts(cmdb_contact_s *contacts, unsigned long int cust_id)
{
	int i = 0;
	cmdb_contact_s *list;
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
	return i;
}

int
print_customer_servers(cmdb_server_s *server, unsigned long int cust_id)
{
	int i = 0;
	cmdb_server_s *list = server;
	while (list) {
		if (list->cust_id == cust_id) {
			i++;
			if (i == 1)
				printf("\nServers:\n");
			printf("%s\n", list->name);
		}
		list = list->next;
	}
	return i;
}

int
get_customer(cmdb_config_s *config, cmdb_s *cmdb, char *coid)
{
	int retval;
	cmdb_customer_s *cust, *next;

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

