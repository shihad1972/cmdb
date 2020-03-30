/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  customers.c: Contains functions for customer component of cmdb
 *
 */

#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"
#include "cmdb_data.h"
#include "cmdb_cmdb.h"

static int
cmdb_populate_contact_details(cmdb_comm_line_s *cm, AILLIST *contacts, AILLIST *customer);

int
cmdb_add_customer_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;

	return retval;
}

void
cmdb_list_customers(ailsa_cmdb_s *cc)
{
	int retval = 0;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *name, *city, *coid;
	ailsa_data_s *one, *two, *three;

	if (!(cc))
		goto cleanup;
	if ((retval = ailsa_basic_query(cc, COID_NAME_CITY, list) != 0)){
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	coid = list->head;
	printf("COID\t\tName\n");
	while (coid) {
		name = coid->next;
		if (name)
			city = name->next;
		else
			break;
		one = (ailsa_data_s *)coid->data;
		two = (ailsa_data_s *)name->data;
		three = (ailsa_data_s *)city->data;
		if (strlen(one->data->text) < 8)
			printf("%s\t\t%s of %s\n", one->data->text, two->data->text, three->data->text);
		else
			printf("%s\t%s of %s\n", one->data->text, two->data->text, three->data->text);
		coid=city->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

void
cmdb_list_contacts_for_customer(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	if (!(cc) || !(cm))
		goto cleanup;
	data->data->text = strndup(cm->coid, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_contacts_for_customer");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CONTACT_DETAILS_ON_COID, args, results))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	printf("Contacts for COID %s\n", cm->coid);
	cmdb_display_contacts(results);
	cleanup:
		ailsa_list_destroy(results);
		ailsa_list_destroy(args);
		my_free(results);
		my_free(args);
}

void
cmdb_display_customer(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *customer = ailsa_db_data_list_init();
	AILLIST *contacts = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	if (!(cc) || !(cm))
		goto cleanup;
	data->data->text = strndup(cm->coid, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_contacts_for_customer");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CUSTOMER_DETAILS_ON_COID, args, customer))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CONTACT_DETAILS_ON_COID, args, contacts))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if (customer->total > 0) {
		printf("Details for coid %s\n", cm->coid);
	} else {
		printf("Coid %s not found in DB\n", cm->coid);
		goto cleanup;
	}
	cmdb_display_customer_details(customer);
	if (contacts->total > 0) {
		printf("Contacts:\n");
	} else {
		printf("No contacts for customer\n");
		goto cleanup;
	}
	cmdb_display_contacts(contacts);

	cleanup:
		ailsa_list_destroy(customer);
		ailsa_list_destroy(contacts);
		ailsa_list_destroy(args);
		my_free(customer);
		my_free(contacts);
		my_free(args);
}

void
cmdb_display_customer_details(AILLIST *list)
{
	if (!(list))
		return;
	AILELEM *first, *second;
	ailsa_data_s *one, *two;

	first = list->head;
	if (list->total == 0)
		return;
	one = first->data;
	printf(" %s\n", one->data->text);
	first = first->next;
	one = first->data;
	printf("  %s,\n", one->data->text);
	first = first->next;
	one = first->data;
	printf("  %s,\n", one->data->text);
	first = first->next;
	one = first->data;
	printf("  %s,\n", one->data->text);
	first = first->next;
	one = first->data;
	printf("  %s\n", one->data->text);
	first = first->next;
	one = first->data;
	second = first->next;
	two = second->data;
#ifdef HAVE_MYSQL
	if (two->type == AILSA_DB_TIME) {
		printf("  Created by %s @ %04u-%02u-%02u %02u:%02u:%02u\n", get_uname(one->data->number),
			two->data->time->year, two->data->time->month, two->data->time->day,
			two->data->time->hour, two->data->time->minute, two->data->time->second);
	} else
#endif
		printf("  Created by %s @ %s\n", get_uname(one->data->number), two->data->text);
	first = second->next;
	one = first->data;
	second = first->next;
	two = second->data;
#ifdef HAVE_MYSQL
	if (two->type == AILSA_DB_TIME) {
		printf("  Modified by %s @ %04u-%02u-%02u %02u:%02u:%02u\n", get_uname(one->data->number),
			two->data->time->year, two->data->time->month, two->data->time->day,
			two->data->time->hour, two->data->time->minute, two->data->time->second);
	} else
#endif
		printf("  Modified by %s @ %s\n", get_uname(one->data->number), two->data->text);
}

void
cmdb_display_contacts(AILLIST *list)
{
	if (!(list))
		return;
	AILELEM *phone, *email, *name;
	ailsa_data_s *one, *two, *three;

	name = list->head;
	if (list->total > 0) {
		while (name) {
			if (name->next)
				phone = name->next;
			else
				break;
			if (phone->next)
				email = phone->next;
			else
				break;
			one = name->data;
			two = phone->data;
			three = email->data;
			printf("  %s\t%s\t%s\n", one->data->text, two->data->text, three->data->text);
			name = email->next;
		}
	}
}

int
cmdb_add_contacts_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	uid_t user = getuid();
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *customer = ailsa_db_data_list_init();
	AILLIST *contacts = ailsa_db_data_list_init();
	AILLIST *check_contact = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	if (!(cc) || !(cm)) {
		retval = AILSA_NO_DATA;
		goto cleanup;
	}
	data->data->text = strndup(cm->coid, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_contacts_for_customer");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CUST_ID_ON_COID, args, customer))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if ((retval = cmdb_populate_contact_details(cm, contacts, customer)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot get new contact values");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CONT_ID_ON_CONTACT_DETAILS, contacts, check_contact)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot check for contact in database");
		goto cleanup;
	}
	if (check_contact->total > 0) {
		ailsa_syslog(LOG_INFO, "Contact already in database");
		goto cleanup;
	}
	data = ailsa_db_lint_data_init();
	data->data->number = (unsigned long int)user;
	if ((retval = ailsa_list_insert(contacts, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert cuser into contacts list");
		goto cleanup;
	}
	data = ailsa_db_lint_data_init();
	data->data->number = (unsigned long int)user;
	if ((retval = ailsa_list_insert(contacts, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert muser into contacts list");
		goto cleanup;
	}
	retval = ailsa_insert_query(cc, INSERT_CONTACTS, contacts);
	cleanup:
		ailsa_list_destroy(check_contact);
		ailsa_list_destroy(customer);
		ailsa_list_destroy(contacts);
		ailsa_list_destroy(args);
		my_free(check_contact);
		my_free(customer);
		my_free(contacts);
		my_free(args);
		return retval;
}

static int
cmdb_populate_contact_details(cmdb_comm_line_s *cm, AILLIST *contacts, AILLIST *customer)
{
	if (!(cm) || !(contacts))
		return AILSA_NO_DATA;
	int retval;
	unsigned long int cust_id = 0;
	ailsa_data_s *data = ailsa_db_text_data_init();
	AILELEM *elem;
	ailsa_data_s *some;

	data->data->text = strndup(cm->name, HOST_LEN);
	if ((retval = ailsa_list_insert(contacts, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert name into contacts list");
		return retval;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->phone, MAC_LEN);
	if ((retval = ailsa_list_insert(contacts, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert phone into contacts list");
		return retval;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->email, HOST_LEN);
	if ((retval = ailsa_list_insert(contacts, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert email into contacts list");
		return retval;
	}
	if (customer->total > 0) {
		elem = customer->head;
		some = elem->data;
		cust_id = some->data->number;
		data = ailsa_db_lint_data_init();
		data->data->number = cust_id;
		if ((retval = ailsa_list_insert(contacts, data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert cust_id into contacts list");
			return retval;
		}
	} else {
		ailsa_syslog(LOG_ERR, "Coid %s does not seem to exist\n", cm->coid);
		return retval;
	}
	return retval;
}
