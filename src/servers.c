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
#include "cmdb_cmdb.h"

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
cmdb_display_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *server = ailsa_db_data_list_init();
	AILLIST *services = ailsa_db_data_list_init();
	AILLIST *hardware = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	if (!(cm) || !(cc))
		goto cleanup;
	data->data->text = strndup(cm->name, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_display_server");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVER_DETAILS_ON_NAME, args, server))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVICES_ON_SERVER, args, services))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, HARDWARE_ON_SERVER, args, hardware))) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	printf("Details for server %s:\n", cm->name);
	cmdb_display_server_details(server);
	printf("Services:\n");
	cmdb_display_services(services);
	printf("Hardware:\n");
	cmdb_display_hardware(hardware);

	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(server);
		ailsa_list_destroy(services);
		ailsa_list_destroy(hardware);
		my_free(args);
		my_free(server);
		my_free(services);
		my_free(hardware);
}

void
cmdb_display_server_details(AILLIST *server)
{
	if (!(server))
		return;
	AILELEM *elem, *time;
	ailsa_data_s *data, *user;

	elem = server->head;
	data = elem->data;
	printf("  Vendor: %s\n", data->data->text);
	elem = elem->next;
	data = elem->data;
	printf("  Make: %s\n", data->data->text);
	elem = elem->next;
	data = elem->data;
	printf("  Model: %s\n", data->data->text);
	elem = elem->next;
	data = elem->data;
	printf("  UUID: %s\n", data->data->text);
	elem = elem->next;
	data = elem->data;
	printf("  Customer COID: %s\n", data->data->text);
	elem = elem->next;
	data = elem->data;
	time = elem->next;
	user = time->data;
	printf("  Created by %s @ %s\n", get_uname(data->data->number), user->data->text);
	elem = time->next;
	data = elem->data;
	time = elem->next;
	user = time->data;
	printf("  Modified by %s @ %s\n", get_uname(data->data->number), user->data->text);
}

void
cmdb_list_services_for_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

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
	cmdb_display_services(results);
	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(results);
		my_free(args);
		my_free(results);
}

void
cmdb_display_services(AILLIST *list)
{
	if (!(list))
		return;
	AILELEM *service, *url, *detail;
	ailsa_data_s *one, *two, *three;

	service = list->head;
	while (service) {
		url = service->next;
		if (url)
			detail = url->next;
		else
			break;
		one = service->data;
		two = url->data;
		three = detail->data;
		printf("  %s service\t@ %s\t%s\n", one->data->text, two->data->text, three->data->text);
		service = detail->next;
	}
}

void
cmdb_list_service_types(ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *name, *type;
	ailsa_data_s *one, *two;

	if (!(cc))
		return;
	if ((retval = ailsa_basic_query(cc, SERVICE_TYPES_ALL, list) != 0)) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	name = list->head;
	printf("Service Types:\n");
	while (name) {
		type = name->next;
		one = name->data;
		if (type)
			two = type->data;
		printf("%s\t\t%s\n", one->data->text, two->data->text);
		name = type->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

void
cmdb_list_hardware_for_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *results = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

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
	cmdb_display_hardware(results);
	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(results);
		my_free(args);
		my_free(results);
}

void
cmdb_display_hardware(AILLIST *list)
{
	if (!(list))
		return;
	AILELEM *class, *device, *detail;
	ailsa_data_s *one, *two, *three;

	class = list->head;
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
}

void
cmdb_list_hardware_types(ailsa_cmdb_s *cc)
{
	int retval;
	AILLIST *list = ailsa_db_data_list_init();
	AILELEM *class, *type;
	ailsa_data_s *one, *two;

	if (!(cc))
		return;
	if ((retval = ailsa_basic_query(cc, HARDWARE_TYPES_ALL, list) != 0)) {
		ailsa_syslog(LOG_ERR, "SQL basic query returned %d", retval);
		goto cleanup;
	}
	class = list->head;
	printf("Hardware Types:\n");
	while (class) {
		type = class->next;
		one = class->data;
		if (type)
			two = type->data;
		printf("%s\t\t%s\n", one->data->text, two->data->text);
		class = type->next;
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
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

