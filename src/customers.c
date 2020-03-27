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
	int retval = 0;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	AILELEM *phone, *email, *name;
	ailsa_data_s *one, *two, *three, *data;

	if (!(cc))
		goto cleanup;
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->coid, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_contacts_for_customer");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CONTACT_DETAILS_ON_COID, args, results))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	name = results->head;
	printf("Name\t\tPhone\t\tEmail\n");
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
		printf("%s\t%s\t%s\n", one->data->text, two->data->text, three->data->text);
		name = email->next;
	}
	cleanup:
		ailsa_list_destroy(results);
		ailsa_list_destroy(args);
		my_free(results);
		my_free(args);
}
