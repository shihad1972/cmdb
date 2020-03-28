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
 *  servers.c: Contains non-database functions for manipulating servers
 *  in the cmdb
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
cmdb_add_server_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	if (!(cm) || !(cc))
		return AILSA_NO_DATA;
		
	return retval;
}

void
cmdb_list_servers(ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *name, *coid;
	ailsa_data_s *one, *two;

	if (!(cc))
		goto cleanup;
	if ((retval = ailsa_basic_query(cc, SERVER_NAME_COID, list) != 0)) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	name = list->head;
	printf("Server Name\t\tCOID\n");
	while (name) {
		coid = name->next;
		one = (ailsa_data_s *)name->data;
		two = (ailsa_data_s *)coid->data;
		if (strlen(one->data->text) < 8)
			printf("%s\t\t\t%s\n",one->data->text, two->data->text);
		else if (strlen(one->data->text) < 16)
			printf("%s\t\t%s\n",one->data->text, two->data->text);
		else
			printf("%s\t%s\n",one->data->text, two->data->text);
		name = coid->next;
		if (name)
			coid = name->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

void
cmdb_list_services_for_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();
	AILELEM *service, *url, *detail;
	ailsa_data_s *one, *two, *three;

	if (!(cm) || !(cc))
		goto cleanup;
	data->data->text = strndup(cm->name, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_services_for_server");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVICES_ON_SERVER, args, results))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	printf("Services for server %s:\n", cm->name);
	service = results->head;
	while (service) {
		url = service->next;
		if (url)
			detail = url->next;
		else
			break;
		one = service->data;
		two = url->data;
		three = detail->data;
		printf("%s service\t@ %s\t%s\n", one->data->text, two->data->text, three->data->text);
		service = detail->next;
	}
	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(results);
		my_free(args);
		my_free(results);
}

void
cmdb_list_hardware_for_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();
	AILELEM *class, *device, *detail;
	ailsa_data_s *one, *two, *three;

	if (!(cm) || !(cc))
		goto cleanup;
	data->data->text = strndup(cm->name, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_hardware_for_server");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, HARDWARE_ON_SERVER, args, results))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	printf("Hardware for server %s:\n", cm->name);
	class = results->head;
	while (class) {
		device = class->next;
		if (device)
			detail = device->next;
		else
			break;
		one = class->data;
		two = device->data;
		three = detail->data;
		printf("  %s /dev/%s %s\n", one->data->text, two->data->text, three->data->text);
		class = detail->next;
	}
	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(results);
		my_free(args);
		my_free(results);
}

void
cmdb_list_vm_server_hosts(ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *name, *type;
	ailsa_data_s *one, *two;
	
	if (!(cc))
		return;
	if ((retval = ailsa_basic_query(cc, VM_SERVERS, list) != 0)) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	name = list->head;
	printf("Virtual Machine hosts:\n");
	while (name) {
		type = name->next;
		one = name->data;
		if (type)
			two = type->data;
		printf("%s, type %s\n", one->data->text, two->data->text);
		name = type->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

