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

static int
cmdb_populate_server_details(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc, AILLIST *server);

int
cmdb_add_server_to_database(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	int retval = 0;
	AILLIST *args = ailsa_db_data_list_init();
	AILLIST *server = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	if (!(cm) || !(cc)) {
		retval = AILSA_NO_DATA;
		goto cleanup;
	}
	data->data->text = strndup(cm->name, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_display_server");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVER_DETAILS_ON_NAME, args, server)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if (server->total > 0) {
		ailsa_syslog(LOG_ERR, "Server %s already in the database", cm->name);
		goto cleanup;
	}
	if ((retval = cmdb_populate_server_details(cm, cc, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Populate server details failed");
		goto cleanup;
	}
	retval = ailsa_insert_query(cc, INSERT_SERVER, server);
	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(server);
		my_free(args);
		my_free(server);
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
	if (list->total > 0) {
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
	} else {
		fprintf(stderr, "No servers found in the database\n");
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
	if ((retval = ailsa_argument_query(cc, SERVER_DETAILS_ON_NAME, args, server)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, SERVICES_ON_SERVER, args, services)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, HARDWARE_ON_SERVER, args, hardware)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if (server->total > 0) {
		printf("Details for server %s:\n", cm->name);
		cmdb_display_server_details(server);
		if (services->total > 0) {
			printf("Services:\n");
			cmdb_display_services(services);
		} else {
			printf("No Services\n");
		}
		if (hardware->total > 0) {
			printf("Hardware:\n");
			cmdb_display_hardware(hardware);
		} else {
			printf("No Hardware\n");
		}
	} else {
		fprintf(stderr, "Cannot find server %s\n", cm->name);
	}

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
#ifdef HAVE_MYSQL
	if (user->type == AILSA_DB_TIME) {
		printf("  Created by %s @ %04u-%02u-%02u %02u:%02u:%02u\n", get_uname(data->data->number),
			user->data->time->year, user->data->time->month, user->data->time->day,
			user->data->time->hour, user->data->time->minute, user->data->time->second);
	} else
#endif
		printf("  Created by %s @ %s\n", get_uname(data->data->number), user->data->text);
	elem = time->next;
	data = elem->data;
	time = elem->next;
	user = time->data;
#ifdef HAVE_MYSQL
	if (user->type == AILSA_DB_TIME) {
		printf("  Modified by %s @ %04u-%02u-%02u %02u:%02u:%02u\n", get_uname(data->data->number),
			user->data->time->year, user->data->time->month, user->data->time->day,
			user->data->time->hour, user->data->time->minute, user->data->time->second);
	} else
#endif
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
	if ((retval = ailsa_argument_query(cc, SERVICES_ON_SERVER, args, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if (results->total > 0) {
		printf("Services for server %s:\n", cm->name);
		cmdb_display_services(results);
	} else {
		fprintf(stderr, "No Services for server %s\n", cm->name);
	}
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
	if (list->total > 0) {
		printf("Service Types:\n");
		while (name) {
			type = name->next;
			one = name->data;
			if (type)
				two = type->data;
			printf(" %s\t\t%s\n", one->data->text, two->data->text);
			name = type->next;
		}
	} else {
		fprintf(stderr, "No service types found in database\n");
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
	if ((retval = ailsa_argument_query(cc, HARDWARE_ON_SERVER, args, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if (results->total > 0) {
		printf("Hardware for server %s:\n", cm->name);
		cmdb_display_hardware(results);
	} else {
		fprintf(stderr, "No hardware for server %s\n", cm->name);
	}
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
	if (list->total > 0) {
		printf("Hardware Types:\n");
		while (class) {
			type = class->next;
			one = class->data;
			if (type)
				two = type->data;
			printf(" %s\t\t%s\n", one->data->text, two->data->text);
			class = type->next;
		}
	} else {
		fprintf(stderr, "No hardware types found in the database\n");
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
	if (list->total > 0) {
		printf("Virtual Machine hosts:\n");
		while (name) {
			type = name->next;
			one = name->data;
			if (type)
				two = type->data;
			printf(" %s, type %s\n", one->data->text, two->data->text);
			name = type->next;
		}
	} else {
		fprintf(stderr, "No virtual machine hosts found in the database\n");
	}
	cleanup:
		ailsa_list_destroy(list);
		my_free(list);
}

void
cmdb_display_vm_server(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc)
{
	if (!(cm) || !(cc))
		return;
	int retval;
	AILLIST *results = ailsa_db_data_list_init();
	AILLIST *args = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	data->data->text = strndup(cm->name, SQL_TEXT_MAX);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert data into list in cmdb_list_services_for_server");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, VM_HOST_BUILT_SERVERS, args, results)) != 0) {
		ailsa_syslog(LOG_ERR, "SQL Argument query returned %d", retval);
		goto cleanup;
	}
	if (results->total > 0) {
		printf("VM host %s built virtual machines:\n", cm->name);
		cmdb_display_built_vms(results);
	} else {
		fprintf(stderr, "VM Host has no built virtual machines\n");
	}
	cleanup:
		ailsa_list_destroy(args);
		ailsa_list_destroy(results);
		my_free(args);
		my_free(results);
}

void
cmdb_display_built_vms(AILLIST *list)
{
	if (!(list))
		return;
	AILELEM *name, *varient;
	ailsa_data_s *one, *two;

	name = list->head;
	while (name) {
		varient = name->next;
		one = name->data;
		two = varient->data;
		printf("  %s built as %s\n", one->data->text, two->data->text);
		name = varient->next;
	}
}

static int
cmdb_populate_server_details(cmdb_comm_line_s *cm, ailsa_cmdb_s *cc, AILLIST *server)
{
	if (!(cm) || !(cc) || !(server))
		return AILSA_NO_DATA;
	int retval;
	uid_t uid = getuid();
	AILLIST *args = ailsa_db_data_list_init();
	ailsa_data_s *data = ailsa_db_text_data_init();

	data->data->text = strndup(cm->name, HOST_LEN);
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert name into server list");
		goto cleanup;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->make, HOST_LEN);
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert make into server list");
		goto cleanup;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->model, MAC_LEN);
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert model into server list");
		goto cleanup;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->vendor, HOST_LEN);
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert vendor into server list");
		goto cleanup;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->uuid, HOST_LEN);
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert uuid into server list");
		goto cleanup;
	}
	data = ailsa_db_text_data_init();
	data->data->text = strndup(cm->coid, HOST_LEN);
	if ((retval = ailsa_list_insert(args, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert uuid into server list");
		goto cleanup;
	}
	if ((retval = ailsa_argument_query(cc, CUST_ID_ON_COID, args, server)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot run query for cust_id");
		goto cleanup;
	}
	if (cm->vmhost) {
		my_free(data->data->text);
		data->data->text = strndup(cm->vmhost, SQL_TEXT_MAX);
		if ((retval = ailsa_list_insert(args, data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert vmhost into list for VM_SERVER_ID_ON_NAME query");
			goto cleanup;
		}
		if ((retval = ailsa_argument_query(cc, VM_SERVER_ID_ON_NAME, args, server)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot query for vmhost");
			goto cleanup;
		}
	} else {
		data = ailsa_db_lint_data_init();
		data->data->number = 0;
		if ((retval = ailsa_list_insert(server, data)) != 0) {
			ailsa_syslog(LOG_ERR, "Cannot insert empty vm_host_id into list");
			goto cleanup;
		}
	}
	data = ailsa_db_lint_data_init();
	data->data->number = (unsigned long int)uid;
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert empty vm_host_id into list");
		goto cleanup;
	}
	data = ailsa_db_lint_data_init();
	data->data->number = (unsigned long int)uid;
	if ((retval = ailsa_list_insert(server, data)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot insert empty vm_host_id into list");
		goto cleanup;
	}
	cleanup:
		ailsa_list_destroy(args);
		my_free(args);
		return retval;
}
