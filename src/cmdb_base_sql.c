/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_base_sql.c:
 *
 *  Contains functions which will fill up data structs based on the parameters
 *  supplied. Will also contian conditional code base on database type.
 */
#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_MYSQL
# include <mysql.h>
# include "mysqlfunc.h"
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */
const struct cmdb_server_t servers;
const struct cmdb_customer_t customers;
const struct cmdb_contact_t contacts;
const struct cmdb_service_t services;
const struct cmdb_service_type_t servtypes;
const struct cmdb_hardware_t hardwares;
const struct cmdb_hard_type_t hardtypes;
const struct cmdb_vm_host_t vmhosts;

const char *sql_select[] = { "\
SELECT server_id, vendor, make, model, uuid, cust_id, vm_server_id, name \
FROM server ORDER BY cust_id","\
SELECT cust_id, name, address, city, county, postcode, coid FROM customer \
ORDER BY coid","\
SELECT cont_id, name, phone, email, cust_id FROM contacts","\
SELECT service_id, server_id, cust_id, service_type_id, detail, url FROM \
services ORDER BY service_type_id","\
SELECT service_type_id, service, detail FROM service_type","\
SELECT hard_id, detail, device, server_id, hard_type_id FROM hardware \
ORDER BY device DESC, hard_type_id","\
SELECT hard_type_id, type, class FROM hard_type","\
SELECT vm_server_id, vm_server, type, server_id FROM vm_server_hosts"
};

const char *sql_insert[] = { "\
INSERT INTO server (name, vendor, make, model, uuid, cust_id, vm_server_id) VALUES \
(?,?,?,?,?,?,?)","\
INSERT INTO customer (name, address, city, county, postcode, coid) VALUES \
(?,?,?,?,?,?)","\
INSERT INTO contacts (name, phone, email, cust_id) VALUES (?,?,?,?)","\
INSERT INTO services (server_id, cust_id, service_type_id, detail, url) \
VALUES (?,?,?,?,?)","\
INSERT INTO service_type (service, detail) VALUES (?,?)","\
INSERT INTO hardware (detail, device, server_id, hard_type_id) VALUES \
(?,?,?,?)","\
INSERT INTO hard_type (type, class) VALUES (?,?)","\
INSERT INTO vm_server_hosts (vm_server, type, server_id) VALUES (?,?,?)"
};

#ifdef HAVE_MYSQL

const int mysql_inserts[8][7] = {
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_LONG , MYSQL_TYPE_LONG},
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , 0},
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_LONG , 0 , 0 , 0} ,
{MYSQL_TYPE_LONG , MYSQL_TYPE_LONG , MYSQL_TYPE_LONG , MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , 0 , 0} ,
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , 0 , 0 , 0 , 0 , 0} ,
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_LONG , MYSQL_TYPE_LONG, 0 , 0 , 0} ,
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , 0 , 0 , 0 , 0 , 0} ,
{MYSQL_TYPE_STRING , MYSQL_TYPE_STRING , MYSQL_TYPE_LONG, 0 , 0 , 0 , 0}
};

#endif /* HAVE_MYSQL */

const char *sql_search[] = { "\
SELECT server_id FROM server WHERE name = ?","\
SELECT cust_id FROM customer WHERE coid = ?","\
SELECT service_type_id FROM service_type WHERE service = ?","\
SELECT hard_type_id FROM hard_type WHERE class = ?","\
SELECT vm_server_id FROM vm_server_hosts WHERE vm_server = ?"
};

/* Number of returned fields for the above SELECT queries */
const unsigned int select_fields[] = { 8,7,5,6,3,5,3,4 };

const unsigned int insert_fields[] = { 7,6,4,5,2,4,2,3 };

const unsigned int search_fields[] = { 1,1,1,1,1 };

const unsigned int search_args[] = { 1,1,1,1,1 };


int
run_query(cmdb_config_t *config, cmdb_t *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return NONE;
}

int
run_multiple_query(cmdb_config_t *config, cmdb_t *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_multiple_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_multiple_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return NONE;
}

int
run_search(cmdb_config_t *config, cmdb_t *base, int type)
{
	int retval;

	if ((strncmp(config->dbtype, "none", RANGE_S) ==0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_search_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_search_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
run_insert(cmdb_config_t *config, cmdb_t *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_insert_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_insert_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
get_query(int type, const char **query, unsigned int *fields)
{
	int retval;
	
	retval = NONE;
	switch(type) {
		case SERVER:
			*query = sql_select[SERVERS];
			*fields = select_fields[SERVERS];
			break;
		case CUSTOMER:
			*query = sql_select[CUSTOMERS];
			*fields = select_fields[CUSTOMERS];
			break;
		case CONTACT:
			*query = sql_select[CONTACTS];
			*fields = select_fields[CONTACTS];
			break;
		case SERVICE:
			*query = sql_select[SERVICES];
			*fields = select_fields[SERVICES];
			break;
		case SERVICE_TYPE:
			*query = sql_select[SERVICE_TYPES];
			*fields = select_fields[SERVICE_TYPES];
			break;
		case HARDWARE:
			*query = sql_select[HARDWARES];
			*fields = select_fields[HARDWARES];
			break;
		case HARDWARE_TYPE:
			*query = sql_select[HARDWARE_TYPES];
			*fields = select_fields[HARDWARES];
			break;
		case VM_HOST:
			*query = sql_select[VM_HOSTS];
			*fields = select_fields[VM_HOSTS];
			break;
		default:
			fprintf(stderr, "Unknown query type %d\n", type);
			retval = 1;
			break;
	}
	
	return retval;
}

void
get_search(int type, size_t *fields, size_t *args, void **input, void **output, cmdb_t *base)
{
	switch (type) {
		case SERVER_ID_ON_NAME:
			*input = &(base->server->name);
			*output = &(base->server->server_id);
			*fields = strlen(base->server->name);
			*args = sizeof(base->server->server_id);
			break;
		case CUST_ID_ON_COID:
			*input = &(base->customer->coid);
			*output = &(base->customer->cust_id);
			*fields = strlen(base->customer->coid);
			*args = sizeof(base->customer->cust_id);
			break;
		case SERV_TYPE_ID_ON_SERVICE:
			*input = &(base->servicetype->service);
			*output = &(base->servicetype->service_id);
			*fields = strlen(base->servicetype->service);
			*args = sizeof(base->servicetype->service_id);
			break;
		case HARD_TYPE_ID_ON_HCLASS:
			*input = &(base->hardtype->hclass);
			*output = &(base->hardtype->ht_id);
			*fields = strlen(base->hardtype->hclass);
			*args = sizeof(base->hardtype->ht_id);
			break;
		case VM_ID_ON_NAME:
			*input = &(base->vmhost->name);
			*output = &(base->vmhost->id);
			*fields = strlen(base->vmhost->name);
			*args = sizeof(base->vmhost->id);
			break;
		default:
			fprintf(stderr, "Unknown query %d\n", type);
			exit (NO_QUERY);
	}
}

/* MySQL functions */
#ifdef HAVE_MYSQL


void
cmdb_mysql_init(cmdb_config_t *dc, MYSQL *cmdb_mysql)
{
	const char *unix_socket;
	
	unix_socket = dc->socket;
	
	if (!(mysql_init(cmdb_mysql))) {
		report_error(MY_INIT_FAIL, mysql_error(cmdb_mysql));
	}
	if (!(mysql_real_connect(cmdb_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(cmdb_mysql));
	
}

int
run_query_mysql(cmdb_config_t *config, cmdb_t *base, int type)
{
	MYSQL cmdb;
	MYSQL_RES *cmdb_res;
	MYSQL_ROW cmdb_row;
	my_ulonglong cmdb_rows;
	const char *query;
	int retval;
	unsigned int fields;
	
	retval = 0;
	cmdb_mysql_init(config, &cmdb);
	if ((retval = get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = cmdb_mysql_query_with_checks(&cmdb, query)) != 0) {
		fprintf(stderr, "Query failed with error code %d\n", retval);
		return retval;
	}
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		cmdb_mysql_cleanup(&cmdb);
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	fields = mysql_num_fields(cmdb_res);
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0)) {
		cmdb_mysql_cleanup_full(&cmdb, cmdb_res);
		report_error(NO_SERVERS, "run_query_mysql");
	}
	while ((cmdb_row = mysql_fetch_row(cmdb_res)))
		store_result_mysql(cmdb_row, base, type, fields);
	cmdb_mysql_cleanup_full(&cmdb, cmdb_res);
	return 0;
}

int
run_multiple_query_mysql(cmdb_config_t *config, cmdb_t *base, int type)
{
	int retval;
	if ((type & SERVER) == SERVER)
		if ((retval = run_query_mysql(config, base, SERVER)) != 0)
			return retval;
	if ((type & CUSTOMER) == CUSTOMER)
		if ((retval = run_query_mysql(config, base, CUSTOMER)) != 0)
			return retval;
	if ((type & CONTACT) == CONTACT)
		if ((retval = run_query_mysql(config, base, CONTACT)) != 0)
			return retval;
	if ((type & SERVICE) == SERVICE) {
		if ((retval = run_query_mysql(config, base, SERVICE_TYPE)) != 0)
			return retval;
		if ((retval = run_query_mysql(config, base, SERVICE)) != 0)
			return retval;
	}
	if ((type & HARDWARE) == HARDWARE) {
		if ((retval = run_query_mysql(config, base, HARDWARE_TYPE)) != 0)
			return retval;
		if ((retval = run_query_mysql(config, base, HARDWARE)) != 0)
			return retval;
	}
	if ((type & VM_HOST) == VM_HOST)
		if ((retval = run_query_mysql(config, base, VM_HOST)) != 0)
			return retval;
	return 0;
}

int
run_search_mysql(cmdb_config_t *config, cmdb_t *base, int type)
{
	MYSQL cmdb;
	MYSQL_STMT *cmdb_stmt;
	MYSQL_BIND my_bind[2];
	const char *query;
	int retval;
	size_t arg_len, res_len;
	void *input, *output;
	
	cmdb_mysql_init(config, &cmdb);
	memset(my_bind, 0, sizeof(my_bind));
/* 
Will need to check if we have char or int here. Hard coded char for search,
and int for result, which is OK when searching on name and returning id
*/	
	query = sql_search[type];
	get_search(type, &arg_len, &res_len, &input, &output, base);
	my_bind[0].buffer_type = MYSQL_TYPE_STRING;
	my_bind[0].buffer = input;
	my_bind[0].buffer_length = arg_len;
	my_bind[0].is_unsigned = 0;
	my_bind[0].is_null = 0;
	my_bind[0].length = 0;
	my_bind[1].buffer_type = MYSQL_TYPE_LONG;
	my_bind[1].buffer = output;
	my_bind[1].buffer_length = res_len;
	my_bind[1].is_unsigned = 1;
	my_bind[1].is_null = 0;
	my_bind[1].length = 0;
	
	
	if (!(cmdb_stmt = mysql_stmt_init(&cmdb)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(cmdb_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_bind_param(cmdb_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_bind_result(cmdb_stmt, &my_bind[1])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_execute(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_store_result(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_fetch(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));

	mysql_stmt_free_result(cmdb_stmt);
	mysql_stmt_close(cmdb_stmt);
	cmdb_mysql_cleanup(&cmdb);
	return 0;
}

int
run_insert_mysql(cmdb_config_t *config, cmdb_t *base, int type)
{
	MYSQL cmdb;
	MYSQL_STMT *cmdb_stmt;
	MYSQL_BIND my_bind[insert_fields[type]];
	const char *query;
	int retval;
	unsigned int i;

	memset(my_bind, 0, sizeof(my_bind));
	for (i=0; i<insert_fields[type]; i++) 
		if ((retval = setup_insert_mysql_bind(&my_bind[i], i, type, base)) != 0)
			return retval;
	query = sql_insert[type];
	cmdb_mysql_init(config, &cmdb);
	if (!(cmdb_stmt = mysql_stmt_init(&cmdb)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(cmdb_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_bind_param(cmdb_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_execute(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	
	mysql_stmt_close(cmdb_stmt);
	cmdb_mysql_cleanup(&cmdb);
	return retval;
}

int
setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, cmdb_t *base)
{
	int retval;

	retval = 0;
	void *buffer;
	bind->buffer_type = mysql_inserts[type][i];
	if (bind->buffer_type == MYSQL_TYPE_STRING)
		bind->is_unsigned = 0;
	else if (bind->buffer_type == MYSQL_TYPE_LONG)
		bind->is_unsigned = 1;
	else
		bind->is_unsigned = 0;
	bind->is_null = 0;
	bind->length = 0;
	if ((retval = setup_insert_mysql_bind_buffer(type, &buffer, base, i)) != 0)
		return retval;
	bind->buffer = buffer;
	if (bind->buffer_type == MYSQL_TYPE_LONG)
		bind->buffer_length = sizeof(unsigned long int);
	else if (bind->buffer_type == MYSQL_TYPE_STRING)
		bind->buffer_length = strlen(buffer);
	return retval;
}

int
setup_insert_mysql_bind_buffer(int type, void **buffer, cmdb_t *base, unsigned int i)
{
	int retval;

	retval = 0;
	if (type == SERVERS)
		setup_insert_mysql_bind_buff_server(buffer, base, i);
	else if (type == CUSTOMERS)
		setup_insert_mysql_bind_buff_customer(buffer, base, i);
	else if (type == CONTACTS)
		setup_insert_mysql_bind_buff_contact(buffer, base, i);
	else if (type == SERVICES)
		setup_insert_mysql_bind_buff_service(buffer, base, i);
	else if (type == HARDWARES)
		setup_insert_mysql_bind_buff_hardware(buffer, base, i);
	else
		retval = NO_TYPE;
	return retval;
}

void
setup_insert_mysql_bind_buff_server(void **buffer, cmdb_t *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->server->name);
	else if (i == 1)
		*buffer = &(base->server->vendor);
	else if (i == 2)
		*buffer = &(base->server->make);
	else if (i == 3)
		*buffer = &(base->server->model);
	else if (i == 4)
		*buffer = &(base->server->uuid);
	else if (i == 5)
		*buffer = &(base->server->cust_id);
	else if (i == 6)
		*buffer = &(base->server->vm_server_id);
}

void
setup_insert_mysql_bind_buff_customer(void **buffer, cmdb_t *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->customer->name);
	else if (i == 1)
		*buffer = &(base->customer->address);
	else if (i == 2)
		*buffer = &(base->customer->city);
	else if (i == 3)
		*buffer = &(base->customer->county);
	else if (i == 4)
		*buffer = &(base->customer->postcode);
	else if (i == 5)
		*buffer = &(base->customer->coid);
}

void
setup_insert_mysql_bind_buff_contact(void **buffer, cmdb_t *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->contact->name);
	else if (i == 1)
		*buffer = &(base->contact->phone);
	else if (i == 2)
		*buffer = &(base->contact->email);
	else if (i == 3)
		*buffer = &(base->contact->cust_id);
}

void
setup_insert_mysql_bind_buff_service(void **buffer, cmdb_t *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->service->server_id);
	else if (i == 1)
		*buffer = &(base->service->cust_id);
	else if (i == 2)
		*buffer = &(base->service->service_type_id);
	else if (i == 3)
		*buffer = &(base->service->detail);
	else if (i == 4)
		*buffer = &(base->service->url);
}

void
setup_insert_mysql_bind_buff_hardware(void **buffer, cmdb_t *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->hardware->detail);
	else if (i == 1)
		*buffer = &(base->hardware->device);
	else if (i == 2)
		*buffer = &(base->hardware->server_id);
	else if (i == 3)
		*buffer = &(base->hardware->ht_id);
}
void
store_result_mysql(MYSQL_ROW row, cmdb_t *base, int type, unsigned int fields)
{
	switch(type) {
		case SERVER:
			if (fields != select_fields[SERVERS])
				break;
			store_server_mysql(row, base);
			break;
		case CUSTOMER:
			if (fields != select_fields[CUSTOMERS])
				break;
			store_customer_mysql(row, base);
			break;
		case CONTACT:
			if (fields != select_fields[CONTACTS])
				break;
			store_contact_mysql(row, base);
			break;
		case SERVICE:
			if (fields != select_fields[SERVICES])
				break;
			store_service_mysql(row, base);
			break;
		case SERVICE_TYPE:
			if (fields != select_fields[SERVICE_TYPES])
				break;
			store_service_type_mysql(row, base);
			break;
		case HARDWARE:
			if (fields != select_fields[HARDWARES])
				break;
			store_hardware_mysql(row, base);
			break;
		case HARDWARE_TYPE:
			if (fields != select_fields[HARDWARE_TYPES])
				break;
			store_hardware_type_mysql(row, base);
			break;
		case VM_HOST:
			if (fields != select_fields[VM_HOSTS])
				break;
			store_vm_hosts_mysql(row, base);
			break;
		default:
			fprintf(stderr, "Unknown type %d\n",  type);
			break;
	}
			
}

void
store_server_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_server_t *server, *list;

	if (!(server = malloc(sizeof(cmdb_server_t))))
		report_error(MALLOC_FAIL, "server in store_server_mysql");
	server->server_id = strtoul(row[0], NULL, 10);
	snprintf(server->vendor, CONF_S, "%s", row[1]);
	snprintf(server->make, CONF_S, "%s", row[2]);
	snprintf(server->model, CONF_S, "%s", row[3]);
	snprintf(server->uuid, CONF_S, "%s", row[4]);
	server->cust_id = strtoul(row[5], NULL, 10);
	server->vm_server_id = strtoul(row[6], NULL, 10);
	snprintf(server->name, MAC_S, "%s", row[7]);
	server->next = '\0';
	list = base->server;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = server;
	} else {
		base->server = server;
	}
}

void
store_customer_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_customer_t *customer, *list;

	if (!(customer = malloc(sizeof(cmdb_customer_t))))
		report_error(MALLOC_FAIL, "customer in store_customer_mysql");
	customer->cust_id = strtoul(row[0], NULL, 10);
	snprintf(customer->name, HOST_S, "%s", row[1]);
	snprintf(customer->address, NAME_S, "%s", row[2]);
	snprintf(customer->city, HOST_S, "%s", row[3]);
	snprintf(customer->county, MAC_S, "%s", row[4]);
	snprintf(customer->postcode, RANGE_S, "%s", row[5]);
	snprintf(customer->coid, RANGE_S, "%s", row[6]);
	customer->next = '\0';
	list = base->customer;
	if (list) {
		while(list->next) {
			list = list->next;
		}
		list->next = customer;
	} else {
		base->customer = customer;
	}
}

void
store_contact_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_contact_t *contact, *list;

	if (!(contact = malloc(sizeof(cmdb_contact_t))))
		report_error(MALLOC_FAIL, "contact in store_contact_mysql");
	contact->cont_id = strtoul(row[0], NULL, 10);
	snprintf(contact->name, HOST_S, "%s", row[1]);
	snprintf(contact->phone, MAC_S, "%s", row[2]);
	snprintf(contact->email, HOST_S, "%s", row[3]);
	contact->cust_id = strtoul(row[4], NULL, 10);
	contact->next = '\0';
	list = base->contact;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = contact;
	} else {
		base->contact = contact;
	}
}

void
store_service_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_service_t *service, *list;
	cmdb_service_type_t *type;

	if (!(service = malloc(sizeof(cmdb_service_t))))
		report_error(MALLOC_FAIL, "service in store_service_sqlite");
	service->service_id = strtoul(row[0], NULL, 10);
	service->server_id = strtoul(row[1], NULL, 10);
	service->cust_id = strtoul(row[2], NULL, 10);
	service->service_type_id = strtoul(row[3], NULL, 10);
	snprintf(service->detail, HOST_S, "%s", row[4]);
	snprintf(service->url, HOST_S, "%s", row[5]);
	service->next = '\0';
	type = base->servicetype;
	if (type) {
		while (service->service_type_id != type->service_id)
			type = type->next;
		service->servicetype = type;
	} else {
		service->servicetype = '\0';
	}
	list = base->service;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = service;
	} else {
		base->service = service;
	}
}

void
store_service_type_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_service_type_t *service, *list;

	if (!(service = malloc(sizeof(cmdb_service_type_t))))
		report_error(MALLOC_FAIL, "service in store_service_type_sqlite");

	service->service_id = strtoul(row[0], NULL, 10);
	snprintf(service->service, RANGE_S, "%s", row[1]);
	snprintf(service->detail, MAC_S, "%s", row[2]);
	service->next = '\0';
	list = base->servicetype;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = service;
	} else {
		base->servicetype = service;
	}
}

void
store_hardware_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_hardware_t *hard, *list;
	cmdb_hard_type_t *type;

	if (!(hard = malloc(sizeof(cmdb_hardware_t))))
		report_error(MALLOC_FAIL, "hardware in store_hardware_mysql");

	hard->hard_id = strtoul(row[0], NULL, 10);
	snprintf(hard->detail, HOST_S, "%s", row[1]);
	snprintf(hard->device, MAC_S, "%s", row[2]);
	hard->server_id = strtoul(row[3], NULL, 10);
	hard->ht_id = strtoul(row[4], NULL, 10);
	hard->next = '\0';
	type = base->hardtype;
	if (type) {
		while (hard->ht_id != type->ht_id)
			type = type->next;
		hard->hardtype = type;
	} else {
		hard->hardtype = '\0';
	}
	list = base->hardware;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = hard;
	} else {
		base->hardware = hard;
	}
}

void
store_hardware_type_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_hard_type_t *hard, *list;

	if (!(hard = malloc(sizeof(cmdb_hard_type_t))))
		report_error(MALLOC_FAIL, "hardware in store_hardware_type_mysql");

	hard->ht_id = strtoul(row[0], NULL, 10);
	snprintf(hard->type, MAC_S, "%s", row[1]);
	snprintf(hard->hclass, HOST_S, "%s", row[2]);
	hard->next = '\0';
	list = base->hardtype;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = hard;
	} else {
		base->hardtype = hard;
	}
}

void
store_vm_hosts_mysql(MYSQL_ROW row, cmdb_t *base)
{
	cmdb_vm_host_t *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cmdb_vm_host_t))))
		report_error(MALLOC_FAIL, "vmhost in store_vm_hosts_mysql");
	vmhost->id = strtoul(row[0], NULL, 10);
	snprintf(vmhost->name, RBUFF_S, "%s", row[1]);
	snprintf(vmhost->type, CONF_S, "%s", row[2]);
	vmhost->server_id = strtoul(row[3], NULL, 10);
	vmhost->next = '\0';
	list = base->vmhost;
	if (list) {
		while(list->next) {
			list = list->next;
		}
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	} 
}

#endif /* HAVE_MYSQL */


/* SQLite functions */
#ifdef HAVE_SQLITE3

int
run_query_sqlite(cmdb_config_t *config, cmdb_t *base, int type)
{
	const char *query, *file;
	int retval;
	unsigned int fields;
	sqlite3 *cmdb;
	sqlite3_stmt *state;
	
	retval = 0;
	file = config->file;
	if ((retval = get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "run_query_sqlite");
	}
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		store_result_sqlite(state, base, type, fields);
	
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	
	return 0;
}

int
run_multiple_query_sqlite(cmdb_config_t *config, cmdb_t *base, int type)
{
	int retval;
	if ((type & SERVER) == SERVER)
		if ((retval = run_query_sqlite(config, base, SERVER)) != 0)
			return retval;
	if ((type & CUSTOMER) == CUSTOMER)
		if ((retval = run_query_sqlite(config, base, CUSTOMER)) != 0)
			return retval;
	if ((type & CONTACT) == CONTACT)
		if ((retval = run_query_sqlite(config, base, CONTACT)) != 0)
			return retval;
	if ((type & SERVICE) == SERVICE) {
		if ((retval = run_query_sqlite(config, base, SERVICE_TYPE)) != 0)
			return retval;
		if ((retval = run_query_sqlite(config, base, SERVICE)) != 0)
			return retval;
	}
	if ((type & HARDWARE) == HARDWARE) {
		if ((retval = run_query_sqlite(config, base, HARDWARE_TYPE)) != 0)
			return retval;
		if ((retval = run_query_sqlite(config, base, HARDWARE)) != 0)
			return retval;
	}
	if ((type & VM_HOST) == VM_HOST)
		if ((retval = run_query_sqlite(config, base, VM_HOST)) != 0)
			return retval;
	return 0;
}

int
run_search_sqlite(cmdb_config_t *config, cmdb_t *base, int type)
{
	const char *query, *file;
	int retval;
	unsigned long int result;
	size_t fields, args;
	void *input, *output;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	retval = 0;
	file = config->file;
	query = sql_search[type];

	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "run_search_sqlite");
	}
/*
   As in the MySQL function we assume that we are sending text and recieving
   numerical data. Searching on name for ID is ok for this
*/
	get_search(type, &fields, &args, &input, &output, base);
	if ((retval = sqlite3_bind_text(state, 1, input, (int)strlen(input), SQLITE_STATIC)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_BIND_FAILED, "run_search_sqlite");
	}
	if ((retval = sqlite3_step(state)) == SQLITE_ROW) {
		result = (unsigned long int)sqlite3_column_int(state, 0);
		switch (type) {
			case SERVER_ID_ON_NAME:
				base->server->server_id = result;
				break;
			case CUST_ID_ON_COID:
				base->customer->cust_id = result;
				break;
			case SERV_TYPE_ID_ON_SERVICE:
				base->servicetype->service_id = result;
				break;
			case HARD_TYPE_ID_ON_HCLASS:
				base->hardtype->ht_id = result;
				break;
			case VM_ID_ON_NAME:
				base->vmhost->id = result;
				break;
		}
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	return retval;
}

int
run_insert_sqlite(cmdb_config_t *config, cmdb_t *base, int type)
{
	const char *query, *file;
	int retval;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	retval = 0;
	query = sql_insert[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READWRITE, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "run_search_sqlite");
	}
	if ((retval = setup_insert_sqlite_bind(state, base, type)) != 0) {
		printf("Error binding result! %d\n", retval);
		sqlite3_close(cmdb);
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cmdb));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(cmdb);
		retval = SQLITE_INSERT_FAILED;
		return retval;
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	return retval;
}

int
setup_insert_sqlite_bind(sqlite3_stmt *state, cmdb_t *cmdb, int type)
{
	int retval;
	if (type == SERVERS) {
		retval = setup_bind_sqlite_server(state, cmdb->server);
	} else if (type == CUSTOMERS) {
		retval = setup_bind_sqlite_customer(state, cmdb->customer);
	} else if (type == CONTACTS) {
		retval = setup_bind_sqlite_contact(state, cmdb->contact);
	} else if (type == SERVICES) {
		retval = setup_bind_sqlite_service(state, cmdb->service);
	} else if (type == HARDWARES) {
		retval = setup_bind_sqlite_hardware(state, cmdb->hardware);
	} else {
		retval = NO_TYPE;
	}
	return retval;
}

void
store_result_sqlite(sqlite3_stmt *state, cmdb_t *base, int type, unsigned int fields)
{
	switch(type) {
		case SERVER:
			if (fields != select_fields[SERVERS])
				break;
			store_server_sqlite(state, base);
			break;
		case CUSTOMER:
			if (fields != select_fields[CUSTOMERS])
				break;
			store_customer_sqlite(state, base);
			break;
		case CONTACT:
			if(fields != select_fields[CONTACTS])
				break;
			store_contact_sqlite(state, base);
			break;
		case SERVICE:
			if (fields != select_fields[SERVICES])
				break;
			store_service_sqlite(state, base);
			break;
		case SERVICE_TYPE:
			if (fields != select_fields[SERVICE_TYPES])
				break;
			store_service_type_sqlite(state, base);
			break;
		case HARDWARE:
			if (fields != select_fields[HARDWARES])
				break;
			store_hardware_sqlite(state, base);
			break;
		case HARDWARE_TYPE:
			if (fields != select_fields[HARDWARE_TYPES])
				break;
			store_hardware_type_sqlite(state, base);
			break;
		case VM_HOST:
			if (fields != select_fields[VM_HOSTS])
				break;
			store_vm_hosts_sqlite(state, base);
			break;
		default:
			fprintf(stderr, "Unknown type %d\n",  type);
			break;
	}
}

void
store_server_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_server_t *server, *list;

	if (!(server = malloc(sizeof(cmdb_server_t))))
		report_error(MALLOC_FAIL, "server in store_server_sqlite");
	server->server_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(server->vendor, CONF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(server->make, CONF_S, "%s", sqlite3_column_text(state, 2));
	snprintf(server->model, CONF_S, "%s", sqlite3_column_text(state, 3));
	snprintf(server->uuid, CONF_S, "%s", sqlite3_column_text(state, 4));
	server->cust_id = (unsigned long int) sqlite3_column_int(state, 5);
	server->vm_server_id = (unsigned long int) sqlite3_column_int(state, 6);
	snprintf(server->name, MAC_S, "%s", sqlite3_column_text(state, 7));
	server->next = '\0';
	list = base->server;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = server;
	} else {
		base->server = server;
	}
}

void
store_customer_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_customer_t *cust, *list;

	if (!(cust = malloc(sizeof(cmdb_customer_t))))
		report_error(MALLOC_FAIL, "cust in store_customer_sqlite");
	cust->cust_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(cust->name, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(cust->address, NAME_S, "%s", sqlite3_column_text(state, 2));
	snprintf(cust->city, HOST_S, "%s", sqlite3_column_text(state, 3));
	snprintf(cust->county, MAC_S, "%s", sqlite3_column_text(state, 4));
	snprintf(cust->postcode, RANGE_S, "%s", sqlite3_column_text(state, 5));
	snprintf(cust->coid, RANGE_S, "%s", sqlite3_column_text(state, 6));
	cust->next = '\0';
	list = base->customer;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = cust;
	} else {
		base->customer = cust;
	}
}

void
store_contact_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_contact_t *contact, *list;

	if (!(contact = malloc(sizeof(cmdb_contact_t))))
		report_error(MALLOC_FAIL, "contact in store_contact_sqlite");

	contact->cont_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(contact->name, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(contact->phone, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(contact->email, HOST_S, "%s", sqlite3_column_text(state, 3));
	contact->cust_id = (unsigned long int) sqlite3_column_int(state, 4);
	contact->next = '\0';
	list = base->contact;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = contact;
	} else {
		base->contact = contact;
	}
}

void
store_service_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_service_t *service, *list;
	cmdb_service_type_t *type;

	if (!(service = malloc(sizeof(cmdb_service_t))))
		report_error(MALLOC_FAIL, "service in store_service_sqlite");

	service->service_id = (unsigned long int) sqlite3_column_int(state, 0);
	service->server_id = (unsigned long int) sqlite3_column_int(state, 1);
	service->cust_id = (unsigned long int) sqlite3_column_int(state, 2);
	service->service_type_id = (unsigned long int) sqlite3_column_int(state, 3);
	snprintf(service->detail, HOST_S, "%s", sqlite3_column_text(state, 4));
	snprintf(service->url, HOST_S, "%s", sqlite3_column_text(state, 5));
	type = base->servicetype;
	if (type) {
		while (service->service_type_id != type->service_id)
			type = type->next;
		service->servicetype = type;
	} else {
		service->servicetype = '\0';
	}
	service->next = '\0';
	list = base->service;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = service;
	} else {
		base->service = service;
	}
}

void
store_service_type_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_service_type_t *service, *list;

	if (!(service = malloc(sizeof(cmdb_service_type_t))))
		report_error(MALLOC_FAIL, "service in store_service_type_sqlite");

	service->service_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(service->service, RANGE_S, "%s", sqlite3_column_text(state, 1));
	snprintf(service->detail, MAC_S, "%s", sqlite3_column_text(state, 2));
	service->next = '\0';
	list = base->servicetype;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = service;
	} else {
		base->servicetype = service;
	}
}

void
store_hardware_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_hardware_t *hard, *list;
	cmdb_hard_type_t *type;

	if (!(hard = malloc(sizeof(cmdb_hardware_t))))
		report_error(MALLOC_FAIL, "hard in store_hardware_sqlite");

	hard->hard_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(hard->detail, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(hard->device, MAC_S, "%s", sqlite3_column_text(state, 2));
	hard->server_id = (unsigned long int) sqlite3_column_int(state, 3);
	hard->ht_id = (unsigned long int) sqlite3_column_int(state, 4);
	hard->next = '\0';
	type = base->hardtype;
	if (type) {
		while (hard->ht_id != type->ht_id)
			type = type->next;
		hard->hardtype = type;
	} else {
		hard->hardtype = '\0';
	}
	list = base->hardware;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = hard;
	} else {
		base->hardware = hard;
	}
}

void
store_hardware_type_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_hard_type_t *hard, *list;

	if (!(hard = malloc(sizeof(cmdb_hard_type_t))))
		report_error(MALLOC_FAIL, "hard in store_hardware_types_sqlite");

	hard->ht_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(hard->type, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(hard->hclass, HOST_S, "%s", sqlite3_column_text(state, 2));
	hard->next = '\0';
	list = base->hardtype;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = hard;
	} else {
		base->hardtype = hard;
	}
}

void
store_vm_hosts_sqlite(sqlite3_stmt *state, cmdb_t *base)
{
	cmdb_vm_host_t *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cmdb_vm_host_t))))
		report_error(MALLOC_FAIL, "vmhost in store_vm_hosts_sqlite");
	vmhost->id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(vmhost->name, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vmhost->type, CONF_S, "%s", sqlite3_column_text(state, 2));
	vmhost->server_id = (unsigned long int) sqlite3_column_int(state, 3);
	vmhost->next = '\0';
	list = base->vmhost;
	if (list) {
		while(list->next) {
			list = list->next;
		}
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	}
}

int
setup_bind_sqlite_server(sqlite3_stmt *state, cmdb_server_t *server)
{
	int retval;

	retval = 0;
	if ((retval = sqlite3_bind_text(
state, 1, server->name, (int)strlen(server->name), SQLITE_STATIC)) > 0) {
		printf("Cannot bind %s\n", server->name);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, server->vendor, (int)strlen(server->vendor), SQLITE_STATIC)) > 0) {
		printf("Cannot bind %s\n", server->vendor);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, server->make, (int)strlen(server->make), SQLITE_STATIC)) > 0) {
		printf("Cannot bind %s\n", server->make);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, server->model, (int)strlen(server->model), SQLITE_STATIC)) > 0) {
		printf("Cannot bind %s\n", server->model);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, server->uuid, (int)strlen(server->uuid), SQLITE_STATIC)) > 0) {
		printf("Cannot bind %s\n", server->uuid);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 6, (int)server->cust_id)) > 0) {
		printf("Cannot bind %lu\n", server->cust_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 7, (int)server->vm_server_id)) > 0) {
		printf("Cannot bind %lu\n", server->vm_server_id);
		return retval;
		
	}
	return retval;
}

int
setup_bind_sqlite_customer(sqlite3_stmt *state, cmdb_customer_t *cust)
{
	int retval;

	retval = 0;
	if ((retval = sqlite3_bind_text(
state, 1, cust->name, (int)strlen(cust->name), SQLITE_STATIC)) > 0) {
		printf("Cannot bind customer name %s\n", cust->name);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, cust->address, (int)strlen(cust->address), SQLITE_STATIC)) > 0) {
		printf("Cannot bind customer address %s\n", cust->address);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, cust->city, (int)strlen(cust->city), SQLITE_STATIC)) > 0) {
		printf("Cannot bind customer city %s\n", cust->city);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, cust->county, (int)strlen(cust->county), SQLITE_STATIC)) > 0) {
		printf("Cannot bind customer county %s\n", cust->county);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, cust->postcode, (int)strlen(cust->postcode), SQLITE_STATIC)) > 0) {
		printf("Cannot bind customer postcode %s\n", cust->postcode);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 6, cust->coid, (int)strlen(cust->coid), SQLITE_STATIC)) > 0) {
		printf("Cannot bind customer coid %s\n", cust->coid);
		return retval;
	}
	return retval;
}

int
setup_bind_sqlite_contact(sqlite3_stmt *state, cmdb_contact_t *cont)
{
	int retval;

	retval = 0;
	if ((retval = sqlite3_bind_text(
state, 1, cont->name, (int)strlen(cont->name), SQLITE_STATIC)) > 0) {
		printf("Cannot bind cotact name %s\n", cont->name);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, cont->phone, (int)strlen(cont->phone), SQLITE_STATIC)) > 0) {
		printf("Cannot bind contact phone number %s\n", cont->phone);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, cont->email, (int)strlen(cont->email), SQLITE_STATIC)) > 0) {
		printf("Cannot bind contact email %s\n", cont->email);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 4, (int)cont->cust_id)) > 0) {
		printf("Cannot bind cust_id %lu\n", cont->cust_id);
		return retval;
	}
	return retval;
}

int
setup_bind_sqlite_service(sqlite3_stmt *state, cmdb_service_t *service)
{
	int retval;

	retval = 0;
	if ((retval = sqlite3_bind_int(
state, 1, (int)service->server_id)) > 0) {
		printf("Cannot bind server id %lu\n", service->server_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 2, (int)service->cust_id)) > 0) {
		printf("Cannot bind cust id %lu\n", service->cust_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 3, (int)service->service_type_id)) > 0) {
		printf("Cannot bind service type id %lu\n", service->service_type_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, service->detail, (int)strlen(service->detail), SQLITE_STATIC)) > 0) {
		printf("Cannot bind service detail %s\n", service->detail);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, service->url, (int)strlen(service->url), SQLITE_STATIC)) > 0) {
		printf("Cannot bind service url %s\n", service->url);
		return retval;
	}
	return retval;
}

int
setup_bind_sqlite_hardware(sqlite3_stmt *state, cmdb_hardware_t *hard)
{
	int retval;

	retval = 0;
	if ((retval = sqlite3_bind_text(
state, 1, hard->detail, (int)strlen(hard->detail), SQLITE_STATIC)) > 0) {
		printf("Cannot bind hardware detail %s\n", hard->detail);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, hard->device, (int)strlen(hard->device), SQLITE_STATIC)) > 0) {
		printf("Cannot bind hardware device %s\n", hard->device);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 3, (int)hard->server_id)) > 0) {
		printf("Cannot bind hardware server id %lu\n", hard->server_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 4, (int)hard->ht_id)) > 0) {
		printf("Cannot bind hardware ht_id %lu\n", hard->ht_id);
		return retval;
	}
	return retval;
}
#endif /* HAVE_SQLITE3 */
