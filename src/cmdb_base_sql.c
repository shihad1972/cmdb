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
#include "base_sql.h"
#include "cmdb_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */
const struct cmdb_server_s servers;
const struct cmdb_customer_s customers;
const struct cmdb_contact_s contacts;
const struct cmdb_service_s services;
const struct cmdb_service_type_s servtypes;
const struct cmdb_hardware_s hardwares;
const struct cmdb_hard_type_s hardtypes;
const struct cmdb_vm_host_s vmhosts;

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

const char *cmdb_sql_delete[] = { "\
DELETE FROM server WHERE server_id = ?","\
DELETE FROM customer WHERE cust_id = ?","\
DELETE FROM contacts WHERE cont_id = ?","\
DELETE FROM services WHERE service_id = ?","\
DELETE FROM service_type WHERE service_type_id = ?","\
DELETE FROM hardware WHERE hard_id = ?","\
DELETE FROM hard_type WHERE hard_type_id = ?","\
DELETE FROM vm_server_hosts WHERE vm_server_hosts = ?"
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
SELECT vm_server_id FROM vm_server_hosts WHERE vm_server = ?","\
SELECT class FROM hard_type WHERE hard_type_id = ?","\
SELECT cust_id FROM customer WHERE name = ?","\
SELECT cont_id FROM contacts c LEFT JOIN customer s ON s.cust_id = c.cust_id\
  WHERE c.name = ? AND s.coid = ?","\
SELECT service_id FROM services WHERE url = ?","\
SELECT service_id FROM services s LEFT JOIN service_type st ON\
  s.service_type_id = st.service_type_id WHERE st.service = ?","\
SELECT service_id FROM services s LEFT JOIN service_type st ON\
  s.service_type_id = st.service_type_id WHERE s.url = ? AND st.service = ?","\
SELECT service_id FROM services WHERE server_id = ?","\
SELECT service_id FROM services WHERE cust_id = ?","\
SELECT service_id FROM services s LEFT JOIN service_type st ON\
  s.service_type_id = st.service_type_id WHERE s.server_id = ? AND st.service = ?","\
SELECT service_id FROM services s LEFT JOIN service_type st ON\
  s.service_type_id = st.service_type_id WHERE s.cust_id = ? AND st.service = ?"
};

/* Number of returned fields for the above SELECT queries */
const unsigned int select_fields[] = { 8,7,5,6,3,5,3,4 };

const unsigned int insert_fields[] = { 7,6,4,5,2,4,2,3 };

const unsigned int search_fields[] = { 1,1,1,1,1,1,1,1,1,1,1 };

const unsigned int search_args[] = { 1,1,1,1,1,1,1,2,1,1,2 };

const unsigned int cmdb_search_fields[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };

const unsigned int cmdb_search_args[] = { 1,1,1,1,1,1,1,2,1,1,2,1,1,2,2 };

const unsigned int cmdb_delete_args[] = { 1,1,1,1,1,1 };

const unsigned int cmdb_search_arg_types[][2] = {
	{ DBTEXT, NONE },
	{ DBTEXT, NONE },
	{ DBTEXT, NONE },
	{ DBTEXT, NONE },
	{ DBTEXT, NONE },
	{ DBINT, NONE },
	{ DBTEXT, NONE },
	{ DBTEXT, DBTEXT },
	{ DBTEXT, NONE },
	{ DBTEXT, NONE },
	{ DBTEXT, DBTEXT },
	{ DBINT, NONE },
	{ DBINT, NONE },
	{ DBINT, DBTEXT },
	{ DBINT, DBTEXT }
};

const unsigned int cmdb_search_field_types[][1] = {
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBTEXT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT }
};

const unsigned int cmdb_delete_arg_type[][1] = {
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT }
};
	
int
cmdb_run_query(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cmdb_run_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cmdb_run_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return NONE;
}

int
cmdb_run_multiple_query(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cmdb_run_multiple_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cmdb_run_multiple_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return NONE;
}

int
run_search(cmdb_config_s *config, cmdb_s *base, int type)
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
cmdb_run_search(cmdb_config_s *cmdb, dbdata_s *data, int type)
{
	int retval = NONE;

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) ==0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cmdb_run_search_mysql(cmdb, data, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cmdb_run_search_sqlite(cmdb, data, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", cmdb->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
run_insert(cmdb_config_s *config, cmdb_s *base, int type)
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
cmdb_run_delete(cmdb_config_s *config, dbdata_s *data, int type)
{
	int retval = NONE;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cmdb_run_delete_mysql(config, data, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cmdb_run_delete_sqlite(config, data, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	return retval;
}

int
get_query(int type, const char **query, unsigned int *fields)
{
	int retval;
	
	retval = NONE;
	if (type == SERVER) {
		*query = sql_select[SERVERS];
		*fields = select_fields[SERVERS];
	} else if (type == CUSTOMER) {
		*query = sql_select[CUSTOMERS];
		*fields = select_fields[CUSTOMERS];
	} else if (type == CONTACT) {
		*query = sql_select[CONTACTS];
		*fields = select_fields[CONTACTS];
	} else if (type == SERVICE) {
		*query = sql_select[SERVICES];
		*fields = select_fields[SERVICES];
	} else if (type == SERVICE_TYPE) {
		*query = sql_select[SERVICE_TYPES];
		*fields = select_fields[SERVICE_TYPES];
	} else if (type == HARDWARE) {
		*query = sql_select[HARDWARES];
		*fields = select_fields[HARDWARES];
	} else if (type == HARDWARE_TYPE) {
		*query = sql_select[HARDWARE_TYPES];
		*fields = select_fields[HARDWARES];
	} else if (type == VM_HOST) {
		*query = sql_select[VM_HOSTS];
		*fields = select_fields[VM_HOSTS];
	} else {
		fprintf(stderr, "Unknown query type %d\n", type);
		retval = 1;
	}
	return retval;
}

void
get_search(int type, size_t *fields, size_t *args, void **input, void **output, cmdb_s *base)
{
	if (type == SERVER_ID_ON_NAME) {
		*input = &(base->server->name);
		*output = &(base->server->server_id);
		*fields = strlen(base->server->name);
		*args = sizeof(base->server->server_id);
	} else if (type == CUST_ID_ON_COID) {
		*input = &(base->customer->coid);
		*output = &(base->customer->cust_id);
		*fields = strlen(base->customer->coid);
		*args = sizeof(base->customer->cust_id);
	} else if (type == SERV_TYPE_ID_ON_SERVICE) {
		*input = &(base->servicetype->service);
		*output = &(base->servicetype->service_id);
		*fields = strlen(base->servicetype->service);
		*args = sizeof(base->servicetype->service_id);
	} else if (type == HARD_TYPE_ID_ON_HCLASS) {
		*input = &(base->hardtype->hclass);
		*output = &(base->hardtype->ht_id);
		*fields = strlen(base->hardtype->hclass);
		*args = sizeof(base->hardtype->ht_id);
	} else if (type == VM_ID_ON_NAME) {
		*input = &(base->vmhost->name);
		*output = &(base->vmhost->id);
		*fields = strlen(base->vmhost->name);
		*args = sizeof(base->vmhost->id);
	} else {
		fprintf(stderr, "Unknown query %d\n", type);
		exit (NO_QUERY);
	}
}

void
cmdb_init_initial_dbdata(dbdata_s **list, unsigned int type)
{
	unsigned int i = 0, max = 0;
	dbdata_s *data, *dlist;
	dlist = *list = '\0';
	max = (cmdb_search_fields[type] >= cmdb_search_args[type]) ?
		cmdb_search_fields[type] :
		cmdb_search_args[type];
	for (i = 0; i < max; i++) {
		if (!(data = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "Data in init_initial_dbdata");
		init_dbdata_struct(data);
		if (!(*list)) {
			*list = dlist = data;
		} else {
			while (dlist->next)
				dlist = dlist->next;
			dlist->next = data;
		}
	}
}

/* MySQL functions */
#ifdef HAVE_MYSQL


void
cmdb_mysql_init(cmdb_config_s *dc, MYSQL *cmdb_mysql)
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
cmdb_run_query_mysql(cmdb_config_s *config, cmdb_s *base, int type)
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
		show_no_results(type);
	}
	while ((cmdb_row = mysql_fetch_row(cmdb_res)))
		store_result_mysql(cmdb_row, base, type, fields);
	cmdb_mysql_cleanup_full(&cmdb, cmdb_res);
	return 0;
}

int
cmdb_run_multiple_query_mysql(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval;
	if ((type & SERVER) == SERVER)
		if ((retval = cmdb_run_query_mysql(config, base, SERVER)) != 0)
			return retval;
	if ((type & CUSTOMER) == CUSTOMER)
		if ((retval = cmdb_run_query_mysql(config, base, CUSTOMER)) != 0)
			return retval;
	if ((type & CONTACT) == CONTACT)
		if ((retval = cmdb_run_query_mysql(config, base, CONTACT)) != 0)
			return retval;
	if ((type & SERVICE) == SERVICE) {
		if ((retval = cmdb_run_query_mysql(config, base, SERVICE_TYPE)) != 0)
			return retval;
		if ((retval = cmdb_run_query_mysql(config, base, SERVICE)) != 0)
			return retval;
	}
	if ((type & HARDWARE) == HARDWARE) {
		if ((retval = cmdb_run_query_mysql(config, base, HARDWARE_TYPE)) != 0)
			return retval;
		if ((retval = cmdb_run_query_mysql(config, base, HARDWARE)) != 0)
			return retval;
	}
	if ((type & VM_HOST) == VM_HOST)
		if ((retval = cmdb_run_query_mysql(config, base, VM_HOST)) != 0)
			return retval;
	return 0;
}

int
run_search_mysql(cmdb_config_s *config, cmdb_s *base, int type)
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
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
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
cmdb_run_search_mysql(cmdb_config_s *ccs, dbdata_s *data, int type)
{
	MYSQL cmdb;
	MYSQL_STMT *cmdb_stmt;
	MYSQL_BIND args[cmdb_search_args[type]];
	MYSQL_BIND fields[cmdb_search_fields[type]];
	const char *query = sql_search[type];
	int retval = NONE, j = NONE;
	unsigned int i;

	memset(args, 0, sizeof(args));
	memset(fields, 0, sizeof(fields));
	for (i = 0; i < cmdb_search_args[type]; i++)
		if ((retval = cmdb_set_search_args_mysql(&args[i], i, type, data)) != 0)
			return retval;
	for (i = 0; i < cmdb_search_fields[type]; i++)
		if ((retval = cmdb_set_search_fields_mysql(&fields[i], i, j, type, data)) != 0)
			return retval;
	cmdb_mysql_init(ccs, &cmdb);
	if (!(cmdb_stmt = mysql_stmt_init(&cmdb)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(cmdb_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_bind_param(cmdb_stmt, &args[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_bind_result(cmdb_stmt, &fields[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_execute(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_store_result(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	while ((retval = mysql_stmt_fetch(cmdb_stmt)) == 0) {
		j++;
		for (i = 0; i < cmdb_search_fields[type]; i++)
			if ((retval = cmdb_set_search_fields_mysql(&fields[i], i, j, type, data)) != 0)
				return retval;
		if ((retval = mysql_stmt_bind_result(cmdb_stmt, &fields[0])) != 0)
			report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	}
	if (retval != MYSQL_NO_DATA)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	else
		retval = NONE;
	mysql_stmt_free_result(cmdb_stmt);
	mysql_stmt_close(cmdb_stmt);
	cmdb_mysql_cleanup(&cmdb);
	return j;
}

int
cmdb_set_search_args_mysql(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base)
{
	int retval = 0;
	unsigned int j;
	void *buffer;
	dbdata_s *list = base;

	mybind->is_null = 0;
	mybind->length = 0;
	for (j = 0; j < i; j++)
		list = list->next;
	if (cmdb_search_arg_types[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		buffer = &(list->args.number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (cmdb_search_arg_types[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		buffer = &(list->args.text);
		mybind->buffer_length = strlen(buffer);
	} else if (cmdb_search_arg_types[type][i] == DBSHORT) {
		mybind->buffer_type = MYSQL_TYPE_SHORT;
		mybind->is_unsigned = 0;
		buffer = &(list->args.small);
		mybind->buffer_length = sizeof(short int);
	} else {
		return WRONG_TYPE;
	}
	mybind->buffer = buffer;
	return retval;
}

int
cmdb_set_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base)
{
	int retval = 0, j;
	static int m = 0, stype = 0;
	void *buffer;
	dbdata_s *list, *new;
	list = base;

	if (stype == 0)
		stype = type;
	else if (stype != type) {
		stype = type;
		m = 0;
	}
	mybind->is_null = 0;
	mybind->length = 0;
	if (k > 0) {
		if (!(new = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "new in cmdb_set_search_fields_mysql");
		init_dbdata_struct(new);
		while (list->next) {
			list = list->next;
		}
		list->next = new;
		list = base;
	}
	for (j = 0; j < m; j++)
		list = list->next;
	if (cmdb_search_field_types[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		buffer = &(list->fields.number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (cmdb_search_field_types[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		buffer = &(list->fields.text);
		mybind->buffer_length = RBUFF_S;
	} else if (cmdb_search_field_types[type][i] == DBSHORT) {
		mybind->buffer_type = MYSQL_TYPE_SHORT;
		mybind->is_unsigned = 0;
		buffer = &(list->fields.small);
		mybind->buffer_length = sizeof(short int);
	} else {
		return WRONG_TYPE;
	}
	mybind->buffer = buffer;
	m++;

	return retval;
}

int
run_insert_mysql(cmdb_config_s *config, cmdb_s *base, int type)
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
cmdb_run_delete_mysql(cmdb_config_s *config, dbdata_s *data, int type)
{
	MYSQL cmdb;
	MYSQL_STMT *cmdb_stmt;
	MYSQL_BIND my_bind[cmdb_delete_args[type]];
	const char *query;
	int retval = NONE;
	unsigned int i;
	dbdata_s *list = data;

	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < cmdb_delete_args[type]; i++) {
		if (cmdb_delete_arg_type[type][i] == DBINT) {
			my_bind[i].buffer_type = MYSQL_TYPE_LONG;
			my_bind[i].is_null = 0;
			my_bind[i].length = 0;
			my_bind[i].is_unsigned = 1;
			my_bind[i].buffer = &(list->args.number);
			my_bind[i].buffer_length = sizeof(unsigned long int);
			list = list->next;
		} else if (cmdb_delete_arg_type[type][i] == MYSQL_TYPE_STRING) {
			my_bind[i].buffer_type = MYSQL_TYPE_STRING;
			my_bind[i].is_null = 0;
			my_bind[i].length = 0;
			my_bind[i].is_unsigned = 0;
			my_bind[i].buffer = &(list->args.text);
			my_bind[i].buffer_length = strlen(list->args.text);
			list = list->next;
		}
	}
	query = cmdb_sql_delete[type];
	cmdb_mysql_init(config, &cmdb);
	if (!(cmdb_stmt = mysql_stmt_init(&cmdb)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(cmdb_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_bind_param(cmdb_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cmdb_stmt));
	if ((retval = mysql_stmt_execute(cmdb_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	retval = (int)mysql_stmt_affected_rows(cmdb_stmt);
	mysql_stmt_close(cmdb_stmt);
	cmdb_mysql_cleanup(&cmdb);
	return retval;
}

int
setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, cmdb_s *base)
{
	int retval;

	retval = 0;
	void *buffer;
	bind->buffer_type = mysql_inserts[type][i];
	if (bind->buffer_type == MYSQL_TYPE_LONG)
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
setup_insert_mysql_bind_buffer(int type, void **buffer, cmdb_s *base, unsigned int i)
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
	else if (type == VM_HOSTS)
		setup_insert_mysql_bind_buff_vmhost(buffer, base, i);
	else
		retval = NO_TYPE;
	return retval;
}

void
setup_insert_mysql_bind_buff_server(void **buffer, cmdb_s *base, unsigned int i)
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
setup_insert_mysql_bind_buff_customer(void **buffer, cmdb_s *base, unsigned int i)
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
setup_insert_mysql_bind_buff_contact(void **buffer, cmdb_s *base, unsigned int i)
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
setup_insert_mysql_bind_buff_service(void **buffer, cmdb_s *base, unsigned int i)
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
setup_insert_mysql_bind_buff_hardware(void **buffer, cmdb_s *base, unsigned int i)
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
setup_insert_mysql_bind_buff_vmhost(void **buffer, cmdb_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->vmhost->name);
	else if (i == 1)
		*buffer = &(base->vmhost->type);
	else if (i == 2)
		*buffer = &(base->vmhost->server_id);
}

void
store_result_mysql(MYSQL_ROW row, cmdb_s *base, int type, unsigned int fields)
{
	unsigned int required;
	if (type == SERVER) {
		required = select_fields[SERVERS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_server_mysql(row, base);
	} else if (type == CUSTOMER) {
		required = select_fields[CUSTOMERS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_customer_mysql(row, base);
	} else if (type == CONTACT) {
		required = select_fields[CONTACTS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_contact_mysql(row, base);
	} else if (type == SERVICE) {
		required = select_fields[SERVICES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_service_mysql(row, base);
	} else if (type == SERVICE_TYPE) {
		required = select_fields[SERVICE_TYPES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_service_type_mysql(row, base);
	} else if (type == HARDWARE) {
		required = select_fields[HARDWARES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_hardware_mysql(row, base);
	} else if (type == HARDWARE_TYPE) {
		required = select_fields[HARDWARE_TYPES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_hardware_type_mysql(row, base);
	} else if (type == VM_HOST) {
		required = select_fields[VM_HOSTS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_vm_hosts_mysql(row, base);
	} else {
		fprintf(stderr, "Unknown type %d\n",  type);
	}
}

void
store_server_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_server_s *server, *list;

	if (!(server = malloc(sizeof(cmdb_server_s))))
		report_error(MALLOC_FAIL, "server in store_server_mysql");
	server->server_id = strtoul(row[0], NULL, 10);
	snprintf(server->vendor, CONF_S, "%s", row[1]);
	snprintf(server->make, CONF_S, "%s", row[2]);
	snprintf(server->model, CONF_S, "%s", row[3]);
	snprintf(server->uuid, CONF_S, "%s", row[4]);
	server->cust_id = strtoul(row[5], NULL, 10);
	server->vm_server_id = strtoul(row[6], NULL, 10);
	snprintf(server->name, HOST_S, "%s", row[7]);
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
store_customer_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_customer_s *customer, *list;

	if (!(customer = malloc(sizeof(cmdb_customer_s))))
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
store_contact_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_contact_s *contact, *list;

	if (!(contact = malloc(sizeof(cmdb_contact_s))))
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
store_service_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_service_s *service, *list;
	cmdb_service_type_s *type;

	if (!(service = malloc(sizeof(cmdb_service_s))))
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
store_service_type_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_service_type_s *service, *list;

	if (!(service = malloc(sizeof(cmdb_service_type_s))))
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
store_hardware_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_hardware_s *hard, *list;
	cmdb_hard_type_s *type;

	if (!(hard = malloc(sizeof(cmdb_hardware_s))))
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
store_hardware_type_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_hard_type_s *hard, *list;

	if (!(hard = malloc(sizeof(cmdb_hard_type_s))))
		report_error(MALLOC_FAIL, "hardware in store_hardware_type_mysql");

	hard->ht_id = strtoul(row[0], NULL, 10);
	snprintf(hard->type, MAC_S, "%s", row[1]);
	snprintf(hard->hclass, MAC_S, "%s", row[2]);
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
store_vm_hosts_mysql(MYSQL_ROW row, cmdb_s *base)
{
	cmdb_vm_host_s *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cmdb_vm_host_s))))
		report_error(MALLOC_FAIL, "vmhost in store_vm_hosts_mysql");
	vmhost->id = strtoul(row[0], NULL, 10);
	snprintf(vmhost->name, RBUFF_S, "%s", row[1]);
	snprintf(vmhost->type, MAC_S, "%s", row[2]);
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
cmdb_run_query_sqlite(cmdb_config_s *config, cmdb_s *base, int type)
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
		report_error(SQLITE_STATEMENT_FAILED, "cmdb_run_query_sqlite");
	}
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		store_result_sqlite(state, base, type, fields);
	
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	
	return 0;
}

int
cmdb_run_multiple_query_sqlite(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval;
	if ((type & SERVER) == SERVER)
		if ((retval = cmdb_run_query_sqlite(config, base, SERVER)) != 0)
			return retval;
	if ((type & CUSTOMER) == CUSTOMER)
		if ((retval = cmdb_run_query_sqlite(config, base, CUSTOMER)) != 0)
			return retval;
	if ((type & CONTACT) == CONTACT)
		if ((retval = cmdb_run_query_sqlite(config, base, CONTACT)) != 0)
			return retval;
	if ((type & SERVICE) == SERVICE) {
		if ((retval = cmdb_run_query_sqlite(config, base, SERVICE_TYPE)) != 0)
			return retval;
		if ((retval = cmdb_run_query_sqlite(config, base, SERVICE)) != 0)
			return retval;
	}
	if ((type & HARDWARE) == HARDWARE) {
		if ((retval = cmdb_run_query_sqlite(config, base, HARDWARE_TYPE)) != 0)
			return retval;
		if ((retval = cmdb_run_query_sqlite(config, base, HARDWARE)) != 0)
			return retval;
	}
	if ((type & VM_HOST) == VM_HOST)
		if ((retval = cmdb_run_query_sqlite(config, base, VM_HOST)) != 0)
			return retval;
	return 0;
}

int
run_search_sqlite(cmdb_config_s *config, cmdb_s *base, int type)
{
	const char *query = sql_search[type], *file = config->file;
	int retval = NONE;
	unsigned long int result;
	void *input, *output;
	size_t fields, args;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

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
		if (type == SERVER_ID_ON_NAME)
			base->server->server_id = result;
		else if (type == CUST_ID_ON_COID)
			base->customer->cust_id = result;
		else if (type == SERV_TYPE_ID_ON_SERVICE)
			base->servicetype->service_id = result;
		else if (type == HARD_TYPE_ID_ON_HCLASS)
			base->hardtype->ht_id = result;
		else if (type == VM_ID_ON_NAME)
			base->vmhost->id = result;
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cmdb);
	return retval;
}

int
cmdb_run_search_sqlite(cmdb_config_s *ccs, dbdata_s *data, int type)
{
	const char *query = sql_search[type], *file = ccs->file;
	int retval = NONE, i;
	dbdata_s *list = data;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READONLY, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(cmdb, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "cmdb_run_search_sqlite");
	}
	for (i = 0; (unsigned)i < cmdb_search_args[type]; i++) {
		if ((retval = set_cmdb_search_sqlite(state, list, type, i)) < 0)
			break;
		if (list->next) 
			list = list->next;
	}
	if (retval == WRONG_TYPE) {
		sqlite3_finalize(state);
		sqlite3_close(cmdb);
		return retval;
	}
	list = data;
	i = NONE;
	while ((sqlite3_step(state)) == SQLITE_ROW) {
		if ((retval =
		 get_cmdb_search_res_sqlite(state, list, type, i)) != 0)
			break;
		i++;
	}
	if (retval != WRONG_TYPE)
		retval = i;
	sqlite3_finalize(state);
	sqlite3_close(cmdb);
	return retval;
}

int
get_cmdb_search_res_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = NONE, j, k;
	unsigned int u;
	dbdata_s *data;

	if (i > 0) {
		for (k = 1; k <= i; k++) {
			for (u = 1; u <= cmdb_search_fields[type]; u++)
				if ((u != cmdb_search_fields[type]) || (k != i))
					list = list->next;
		}
		for (j = 0; (unsigned)j < cmdb_search_fields[type]; j++) {
			if (!(data = malloc(sizeof(dbdata_s))))
				report_error(MALLOC_FAIL,
				 "dbdata_s in get_cmdb_search_res_sqlite");
			init_dbdata_struct(data);
			if (cmdb_search_field_types[type][j] == DBTEXT)
				snprintf(data->fields.text, RBUFF_S, "%s",
				 sqlite3_column_text(state, j));
			else if (cmdb_search_field_types[type][j] == DBINT)
				data->fields.number =
				 (uli_t)sqlite3_column_int64(state, j);
			else if (cmdb_search_field_types[type][j] == DBSHORT)
				data->fields.small =
				 (short int)sqlite3_column_int(state, j);
			else
				return WRONG_TYPE;
			list->next = data;
		}
	} else {
		for (j = 0; (unsigned)j < cmdb_search_fields[type]; j++) {
			if (cmdb_search_field_types[type][j] == DBTEXT)
				snprintf(list->fields.text, RBUFF_S, "%s",
				 sqlite3_column_text(state, j));
			else if (cmdb_search_field_types[type][j] == DBINT)
				list->fields.number =
				 (uli_t)sqlite3_column_int64(state, j);
			else if (cmdb_search_field_types[type][j] == DBSHORT)
				list->fields.small =
				 (short int)sqlite3_column_int(state, j);
			else
				return WRONG_TYPE;
		}
	}
	return retval;
}

int
set_cmdb_search_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = NONE;

	if (cmdb_search_arg_types[type][i] == DBTEXT) {
		if ((retval = sqlite3_bind_text(
state, i + 1, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
			fprintf(stderr, "Cannot bind search arg %s\n",
				list->args.text);
			return retval;
		}
	} else if (cmdb_search_arg_types[type][i] == DBINT) {
		if ((retval = sqlite3_bind_int64(
state, i + 1, (sqlite3_int64)list->args.number)) > 0) {
			fprintf(stderr, "Cannot bind search number arg %lu\n",
				list->args.number);
			return retval;
		}
	} else if (cmdb_search_arg_types[type][i] == DBSHORT) {
		if ((retval = sqlite3_bind_int(state, i + 1, list->args.small)) > 0) {
			fprintf(stderr, "Cannot bind search small arg %d\n",
				list->args.small);
			return retval;
		}
	} else {
		retval = WRONG_TYPE;
	}
	return retval;
}

int
run_insert_sqlite(cmdb_config_s *config, cmdb_s *base, int type)
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
		report_error(SQLITE_STATEMENT_FAILED, "run_insert_sqlite");
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
cmdb_run_delete_sqlite(cmdb_config_s *config, dbdata_s *data, int type)
{
	const char *query = cmdb_sql_delete[type], *file = config->file;
	int retval = NONE;
	unsigned int i;
	dbdata_s *list = data;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	if ((retval = sqlite3_open_v2(file, &cmdb, SQLITE_OPEN_READWRITE, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(cmdb, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "cmdb_run_delete");
	}
	for (i = 1; i <= cmdb_delete_args[type]; i++) {
		if (!list)
			break;
		if (cmdb_delete_arg_type[type][i - 1] == DBTEXT) {
			if ((sqlite3_bind_text(state, (int)i, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
				fprintf(stderr, "Cannot bind arg %s\n", list->args.text);
				return retval;
			}
		} else if (cmdb_delete_arg_type[type][i - 1] == DBINT) {
			if ((sqlite3_bind_int64(state, (int)i, (sqlite3_int64)list->args.number)) > 0) {
				fprintf(stderr, "Cannot bind arg %lu\n", list->args.number);
				return retval;
			}
		}
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Received error: %s\n", sqlite3_errmsg(cmdb));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(cmdb);
		return NONE;
	}
	retval = sqlite3_changes(cmdb);
	sqlite3_finalize(state);
	sqlite3_close(cmdb);
	return retval;
}

int
setup_insert_sqlite_bind(sqlite3_stmt *state, cmdb_s *cmdb, int type)
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
	} else if (type == VM_HOSTS) {
		retval = setup_bind_sqlite_vmhost(state, cmdb->vmhost);
	} else {
		retval = NO_TYPE;
	}
	return retval;
}

void
store_result_sqlite(sqlite3_stmt *state, cmdb_s *base, int type, unsigned int fields)
{
	unsigned int required;
	if (type == SERVER) {
		required = select_fields[SERVERS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_server_sqlite(state, base);
	} else if (type == CUSTOMER) {
		required = select_fields[CUSTOMERS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_customer_sqlite(state, base);
	} else if (type == CONTACT) {
		required = select_fields[CONTACTS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_contact_sqlite(state, base);
	} else if (type == SERVICE) {
		required = select_fields[SERVICES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_service_sqlite(state, base);
	} else if (type == SERVICE_TYPE) {
		required = select_fields[SERVICE_TYPES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_service_type_sqlite(state, base);
	} else if (type == HARDWARE) {
		required = select_fields[HARDWARES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_hardware_sqlite(state, base);
	} else if (type == HARDWARE_TYPE) {
		required = select_fields[HARDWARE_TYPES];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_hardware_type_sqlite(state, base);
	} else if (type == VM_HOST) {
		required = select_fields[VM_HOSTS];
		if (fields != required)
			cmdb_query_mismatch(fields, required, type);
		store_vm_hosts_sqlite(state, base);
	} else {
		fprintf(stderr, "Unknown type %d\n",  type);
	}
}

void
store_server_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_server_s *server, *list;

	if (!(server = malloc(sizeof(cmdb_server_s))))
		report_error(MALLOC_FAIL, "server in store_server_sqlite");
	server->server_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(server->vendor, CONF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(server->make, CONF_S, "%s", sqlite3_column_text(state, 2));
	snprintf(server->model, CONF_S, "%s", sqlite3_column_text(state, 3));
	snprintf(server->uuid, CONF_S, "%s", sqlite3_column_text(state, 4));
	server->cust_id = (unsigned long int) sqlite3_column_int(state, 5);
	server->vm_server_id = (unsigned long int) sqlite3_column_int(state, 6);
	snprintf(server->name, HOST_S, "%s", sqlite3_column_text(state, 7));
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
store_customer_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_customer_s *cust, *list;

	if (!(cust = malloc(sizeof(cmdb_customer_s))))
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
store_contact_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_contact_s *contact, *list;

	if (!(contact = malloc(sizeof(cmdb_contact_s))))
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
store_service_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_service_s *service, *list;
	cmdb_service_type_s *type;

	if (!(service = malloc(sizeof(cmdb_service_s))))
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
store_service_type_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_service_type_s *service, *list;

	if (!(service = malloc(sizeof(cmdb_service_type_s))))
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
store_hardware_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_hardware_s *hard, *list;
	cmdb_hard_type_s *type;

	if (!(hard = malloc(sizeof(cmdb_hardware_s))))
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
store_hardware_type_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_hard_type_s *hard, *list;

	if (!(hard = malloc(sizeof(cmdb_hard_type_s))))
		report_error(MALLOC_FAIL, "hard in store_hardware_types_sqlite");

	hard->ht_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(hard->type, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(hard->hclass, MAC_S, "%s", sqlite3_column_text(state, 2));
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
store_vm_hosts_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	cmdb_vm_host_s *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cmdb_vm_host_s))))
		report_error(MALLOC_FAIL, "vmhost in store_vm_hosts_sqlite");
	vmhost->id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(vmhost->name, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vmhost->type, MAC_S, "%s", sqlite3_column_text(state, 2));
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
setup_bind_sqlite_server(sqlite3_stmt *state, cmdb_server_s *server)
{
	int retval = NONE;

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
setup_bind_sqlite_customer(sqlite3_stmt *state, cmdb_customer_s *cust)
{
	int retval = NONE;

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
setup_bind_sqlite_contact(sqlite3_stmt *state, cmdb_contact_s *cont)
{
	int retval = NONE;

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
setup_bind_sqlite_service(sqlite3_stmt *state, cmdb_service_s *service)
{
	int retval = NONE;

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
setup_bind_sqlite_hardware(sqlite3_stmt *state, cmdb_hardware_s *hard)
{
	int retval = NONE;

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

int
setup_bind_sqlite_vmhost(sqlite3_stmt *state, cmdb_vm_host_s *vmhost)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_text(
state, 1, vmhost->name, (int)strlen(vmhost->name), SQLITE_STATIC)) > 0) {
		printf("Cannot bind vmhost name %s\n", vmhost->name);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, vmhost->type, (int)strlen(vmhost->type), SQLITE_STATIC)) > 0) {
		printf("Cannot bind vmhost type %s\n", vmhost->type);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)vmhost->server_id)) > 0) {
		printf("Cannot bind vmhost server_id %lu\n", vmhost->server_id);
		return retval;
	}
	return retval;
}

#endif /* HAVE_SQLITE3 */

void
show_no_results(int type)
{
	if (type == SERVER)
		fprintf(stderr, "No servers to list\n");
	else if (type == CUSTOMER)
		fprintf(stderr, "No customers to list\n");
	else if (type == SERVICE)
		fprintf(stderr, "No services to list\n");
	else if (type == CONTACT)
		fprintf(stderr, "No contacts to list\n");
	else if (type == SERVICE_TYPE)
		;
	else
		fprintf(stderr, "No unknown listing %d\n", type);
}
