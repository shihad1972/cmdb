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
# include "cmdb_mysql.h"
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
get_query(int type, const char **query, unsigned int *fields)
{
	int retval;
	
	retval = NONE;
	switch(type) {
		case SERVER:
			*query = sql_select[SERVER_QUERY];
			*fields = field_numbers[SERVER_QUERY];
			break;
		case CUSTOMER:
			*query = sql_select[CUSTOMER_QUERY];
			*fields = field_numbers[CUSTOMER_QUERY];
			break;
		case CONTACT:
			*query = sql_select[CONTACT_QUERY];
			*fields = field_numbers[CONTACT_QUERY];
			break;
		case SERVICE:
			*query = sql_select[SERVICE_QUERY];
			*fields = field_numbers[SERVICE_QUERY];
			break;
		case SERVICE_TYPE:
			*query = sql_select[SERVICE_TYPE_QUERY];
			*fields = field_numbers[SERVICE_TYPE_QUERY];
			break;
		case HARDWARE:
			*query = sql_select[HARDWARE_QUERY];
			*fields = field_numbers[HARDWARE_QUERY];
			break;
		case HARDWARE_TYPE_QUERY:
			*query = sql_select[HARDWARE_TYPE_QUERY];
			*fields = field_numbers[HARDWARE_TYPE_QUERY];
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
	if (((cmdb_rows = mysql_num_rows(cmdb_res)) == 0)) {
		cmdb_mysql_cleanup_full(&cmdb, cmdb_res);
		report_error(NO_SERVERS, "run_query_mysql");
	}
	while ((cmdb_row = mysql_fetch_row(cmdb_res)))
		store_result_mysql(cmdb_row, base, type, &fields);
	cmdb_mysql_cleanup_full(&cmdb, cmdb_res);
	return 0;
}

void
store_result_mysql(MYSQL_ROW row, cmdb_t *base, int type, unsigned int *fields)
{
	switch(type) {
		case SERVER:
			if (*fields != 8)
				break;
			store_server_mysql(row, base);
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
	fprintf(stderr, "Running query against %s for type %d\n%s\n",
		config->dbtype, type, query);
	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(cmdb, query, NAME_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "run_query_sqlite");
	}
	retval = 0;
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		store_result_sqlite(state, base, type, fields);
	
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	
	return 0;
}

void
store_result_sqlite(sqlite3_stmt *state, cmdb_t *base, int type, unsigned int fields)
{
	switch(type) {
		case SERVER:
			if (fields != 8)
				break;
			store_server_sqlite(state, base);
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
		report_error(MALLOC_FAIL, "server in store_server_mysql");
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

#endif /* HAVE_SQLITE3 */
