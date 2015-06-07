/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_data.c: data functions for the cmdb suite of programs
 * 
 *  Part of the CMDB program
 * 
 * 
 */
#include "../config.h"
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include <netdb.h>
#include "cmdb.h"
#include "cmdb_data.h"
#include "cmdb_cmdb.h"

void *
cmdb_malloc(size_t len, const char *msg)
{
	void *data;
	if (!(data = malloc(len))) {
		perror(msg);
		exit(MALLOC_FAIL);
	}
	return data;
}

void
initialise_string_array(char *list[], size_t quantity, size_t quality[])
{
	size_t i;

	for (i = 0; i < quantity; i++)
		list[i] = cmdb_malloc(quality[i], "initialise_string_array");
}

void
cmdb_prep_db_query(dbdata_s **data, const unsigned int *values[], int query)
{
	unsigned int max = 0;
	max = cmdb_get_max(values[0][query], values[1][query]);
	init_multi_dbdata_struct(data, max);
}

unsigned int
cmdb_get_max_val(const unsigned int *search[], int query)
{
	unsigned int max;
	max = cmdb_get_max(search[0][query], search[1][query]);
	return max;
}

unsigned int
cmdb_get_max(const unsigned int args, const unsigned int fields)
{
	unsigned int max;

	max = (fields >= args) ? fields :  args ;
	return max;
}

int
check_data_length(dbdata_s *data, unsigned int len)
{
	unsigned int i = 0;
	if (!(data))
		report_error(CBC_NO_DATA, "data in check_data_length");
	dbdata_s *list = data;
	while (list) {
		i++;
		list = list->next;
	}
	if ((i % len) != 0)
		return 1;
	else
		return 0;
}

dbdata_s *
move_down_list_data(dbdata_s *data, unsigned int len)
{
	dbdata_s *list;
	unsigned int i;
	if (!(data))
		report_error(CBC_NO_DATA, "data in move_down_list_data");
	if (check_data_length(data, len) != 0)
		report_error(CBC_DATA_WRONG_COUNT, "move_down_list_data");
	list = data;
	for (i = len; i != 0; i--) {
		list = list->next;
	}
	return list;
}

void
resize_string_buff(string_len_s *build)
{
	char *tmp;

	build->len *=2;
	tmp = realloc(build->string, build->len * sizeof(char));
	if (!tmp)
		report_error(MALLOC_FAIL, "tmp in resize_string_buff");
	else
		build->string = tmp;
}

void
cmdb_init_struct(cmdb_s *cmdb)
{
	memset(cmdb, 0, sizeof *cmdb);
}

void
cmdb_init_server_t(cmdb_server_s *server)
{
	memset(server, 0, sizeof *server);
	snprintf(server->vendor, COMM_S, "NULL");
	snprintf(server->make, COMM_S, "NULL");
	snprintf(server->model, COMM_S, "NULL");
	snprintf(server->uuid, COMM_S, "NULL");
	snprintf(server->name, COMM_S, "NULL");
}

void
cmdb_init_customer_t(cmdb_customer_s *cust)
{
	memset(cust, 0, sizeof *cust);
	snprintf(cust->address, COMM_S, "NULL");
	snprintf(cust->city, COMM_S, "NULL");
	snprintf(cust->county, COMM_S, "NULL");
	snprintf(cust->postcode, COMM_S, "NULL");
	snprintf(cust->coid, COMM_S, "NULL");
	snprintf(cust->name, COMM_S, "NULL");
}

void
cmdb_init_service_t(cmdb_service_s *service)
{
	memset(service, 0, sizeof *service);
	snprintf(service->detail, COMM_S, "NULL");
	snprintf(service->url, COMM_S, "NULL");
}

void
cmdb_init_hardware_t(cmdb_hardware_s *hard)
{
	memset(hard, 0, sizeof *hard);
	snprintf(hard->detail, COMM_S, "NULL");
	snprintf(hard->device, COMM_S, "NULL");
}

void
cmdb_init_contact_t(cmdb_contact_s *cont)
{
	memset (cont, 0, sizeof *cont);
	snprintf(cont->name, COMM_S, "NULL");
	snprintf(cont->phone, COMM_S, "NULL");
	snprintf(cont->email, COMM_S, "NULL");
}

void
cmdb_init_hardtype_t(cmdb_hard_type_s *type)
{
	memset(type, 0, sizeof(cmdb_hard_type_s));
	snprintf(type->type, COMM_S, "NULL");
	snprintf(type->hclass, COMM_S, "NULL");
}

void
cmdb_init_servicetype_t(cmdb_service_type_s *type)
{
	memset(type, 0, sizeof(cmdb_service_type_s));
	snprintf(type->service, COMM_S, "NULL");
	snprintf(type->detail, COMM_S, "NULL");
}

void
cmdb_init_vmhost_t(cmdb_vm_host_s *type)
{
	memset(type, 0, sizeof(cmdb_vm_host_s));
	snprintf(type->name, COMM_S, "NULL");
	snprintf(type->type, COMM_S, "NULL");
}

void
cmdb_clean_list(cmdb_s *cmdb)
{
	if (cmdb->server)
		clean_server_list(cmdb->server);
	if (cmdb->customer)
		clean_customer_list(cmdb->customer);
	if (cmdb->contact)
		clean_contact_list(cmdb->contact);
	if (cmdb->service)
		clean_service_list(cmdb->service);
	if (cmdb->servicetype)
		clean_service_type_list(cmdb->servicetype);
	if (cmdb->hardware)
		clean_hardware_list(cmdb->hardware);
	if (cmdb->hardtype)
		clean_hardware_type_list(cmdb->hardtype);
	if (cmdb->vmhost)
		clean_vmhost_list(cmdb->vmhost);
	free(cmdb);
}

void
clean_server_list(cmdb_server_s *list)
{
	cmdb_server_s *server, *next;

	server = list;
	next = server->next;
	while (server) {
		free(server);
		server = next;
		if (next) {
			next = server->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_customer_list(cmdb_customer_s *list)
{
	cmdb_customer_s *customer, *next;

	customer = list;
	next = customer->next;
	while (customer) {
		free(customer);
		customer = next;
		if (next) {
			next = customer->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_contact_list(cmdb_contact_s *list)
{
	cmdb_contact_s *contact, *next;

	contact = list;
	next = contact->next;
	while(contact) {
		free(contact);
		contact = next;
		if (next) {
			next = contact->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_service_list(cmdb_service_s *list)
{
	cmdb_service_s *service, *next;

	service = list;
	next = service->next;
	while (service) {
		free(service);
		service = next;
		if (next) {
			next = service->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_service_type_list(cmdb_service_type_s *list)
{
	cmdb_service_type_s *service, *next;

	service = list;
	next = service->next;
	while (service) {
		free(service);
		service = next;
		if (next) {
			next = service->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_hardware_list(cmdb_hardware_s *list)
{
	cmdb_hardware_s *hardware, *next;

	hardware = list;
	next = hardware->next;
	while(hardware) {
		free(hardware);
		hardware = next;
		if (next) {
			next = hardware->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_hardware_type_list(cmdb_hard_type_s *list)
{
	cmdb_hard_type_s *hardware, *next;

	hardware = list;
	next = hardware->next;
	while(hardware) {
		free(hardware);
		hardware = next;
		if (next) {
			next = hardware->next;
		} else {
			next = NULL;
		}
	}
}

void
clean_vmhost_list(cmdb_vm_host_s *list)
{
	cmdb_vm_host_s *vmhost, *next;

	vmhost = list;
	next = vmhost->next;
	while (vmhost) {
		free(vmhost);
		vmhost = next;
		if (next) {
			next = vmhost->next;
		} else {
			next = NULL;
		}
	}
}

