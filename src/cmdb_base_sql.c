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
#include "cmdb_statements.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_MYSQL
# include <mysql.h>
# include "mysqlfunc.h"
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include "cmdb_sqlite.h"
#endif /* HAVE_SQLITE3 */

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
get_query(int type, const char **query, unsigned int *fields)
{
	int retval;
	
	retval = NONE;
	switch(type) {
		case SERVER:
			*query = sql_select[SERVERS];
			*fields = field_numbers[SERVERS];
			break;
		case CUSTOMER:
			*query = sql_select[CUSTOMERS];
			*fields = field_numbers[CUSTOMERS];
			break;
		case CONTACT:
			*query = sql_select[CONTACTS];
			*fields = field_numbers[CONTACTS];
			break;
		case SERVICE:
			*query = sql_select[SERVICES];
			*fields = field_numbers[SERVICES];
			break;
		case SERVICE_TYPE:
			*query = sql_select[SERVICE_TYPES];
			*fields = field_numbers[SERVICE_TYPES];
			break;
		case HARDWARE:
			*query = sql_select[HARDWARES];
			*fields = field_numbers[HARDWARES];
			break;
		case HARDWARE_TYPE:
			*query = sql_select[HARDWARE_TYPES];
			*fields = field_numbers[HARDWARES];
			break;
		case VM_HOST:
			*query = sql_select[VM_HOSTS];
			*fields = field_numbers[VM_HOSTS];
			break;
		default:
			fprintf(stderr, "Unknown query type %d\n", type);
			retval = 1;
			break;
	}
	
	return retval;
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
	return 0;
}

void
store_result_mysql(MYSQL_ROW row, cmdb_t *base, int type, unsigned int fields)
{
	switch(type) {
		case SERVER:
			if (fields != field_numbers[SERVERS])
				break;
			store_server_mysql(row, base);
			break;
		case CUSTOMER:
			if (fields != field_numbers[CUSTOMERS])
				break;
			store_customer_mysql(row, base);
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
	if(list) {
		while(list->next) {
			list = list->next;
		}
		list->next = customer;
	} else {
		base->customer = customer;
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
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, query, NAME_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "run_query_sqlite");
	}
	retval = 0;
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
	return 0;
}

void
store_result_sqlite(sqlite3_stmt *state, cmdb_t *base, int type, unsigned int fields)
{
	switch(type) {
		case SERVER:
			if (fields != field_numbers[SERVERS])
				break;
			store_server_sqlite(state, base);
			break;
		case CUSTOMER:
			if (fields != field_numbers[CUSTOMERS])
				break;
			store_customer_sqlite(state, base);
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

#endif /* HAVE_SQLITE3 */
