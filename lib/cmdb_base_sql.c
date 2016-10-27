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
#include <config.h>
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

const char *sql_select[] = { "\
SELECT server_id, vendor, make, model, uuid, cust_id, vm_server_id, name, \
cuser, muser, ctime, mtime FROM server ORDER BY cust_id","\
SELECT cust_id, name, address, city, county, postcode, coid, cuser, muser, \
ctime, mtime FROM customer ORDER BY coid","\
SELECT cont_id, name, phone, email, cust_id, cuser, muser, ctime, mtime \
FROM contacts","\
SELECT service_id, server_id, cust_id, service_type_id, detail, url, cuser, \
muser, ctime, mtime FROM services ORDER BY service_type_id","\
SELECT service_type_id, service, detail FROM service_type","\
SELECT hard_id, detail, device, server_id, hard_type_id, cuser, muser, ctime, \
mtime FROM hardware ORDER BY device DESC, hard_type_id","\
SELECT hard_type_id, type, class FROM hard_type","\
SELECT vm_server_id, vm_server, type, server_id, cuser, muser, ctime, mtime \
FROM vm_server_hosts"
};

const char *sql_insert[] = { "\
INSERT INTO server (name, vendor, make, model, uuid, cust_id, vm_server_id, \
cuser, muser) VALUES (?,?,?,?,?,?,?,?,?)","\
INSERT INTO customer (name, address, city, county, postcode, coid, cuser, \
muser) VALUES (?,?,?,?,?,?,?,?)","\
INSERT INTO contacts (name, phone, email, cust_id, cuser, muser) VALUES \
(?,?,?,?,?,?)","\
INSERT INTO services (server_id, cust_id, service_type_id, detail, url, \
cuser, muser) VALUES (?,?,?,?,?,?,?)","\
INSERT INTO service_type (service, detail) VALUES (?,?)","\
INSERT INTO hardware (detail, device, server_id, hard_type_id, cuser, muser) \
VALUES (?,?,?,?,?,?)","\
INSERT INTO hard_type (type, class) VALUES (?,?)","\
INSERT INTO vm_server_hosts (vm_server, type, server_id, cuser, muser) VALUES (?,?,?,?,?)"
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
  s.service_type_id = st.service_type_id WHERE s.cust_id = ? AND st.service = ?","\
SELECT hard_id FROM hardware WHERE device = ? AND server_id = ?","\
SELECT hard_id FROM hardware WHERE detail = ? AND server_id = ?","\
SELECT hard_id FROM hardware WHERE device = ? AND hard_type_id = ?","\
SELECT hard_id FROM hardware WHERE detail = ? AND hard_type_id = ?"
};

const char *cmdb_sql_update[] = { "\
UPDATE server SET muser = ? WHERE server_id = ?","\
UPDATE customer SET muser = ? WHERE cust_id = ?","\
UPDATE server SET uuid = ? WHERE server_id = ?","\
UPDATE server SET make = ? WHERE server_id = ?","\
UPDATE server SET model = ? WHERE server_id = ?","\
UPDATE server SET vendor = ? WHERE server_id = ?","\
UPDATE server SET cust_id = ? WHERE server_id = ?"
};

/* Number of returned fields for the above SELECT queries */
const unsigned int select_fields[] = { 12,11,9,10,3,9,3,8 };

const unsigned int insert_fields[] = { 9,8,6,7,2,6,2,5 };

const unsigned int search_fields[] = { 1,1,1,1,1,1,1,1,1,1,1 };

const unsigned int search_args[] = { 1,1,1,1,1,1,1,2,1,1,2 };

const unsigned int cmdb_search_fields[] = { 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 };

const unsigned int cmdb_search_args[] = { 1,1,1,1,1,1,1,2,1,1,2,1,1,2,2,2,2,2,2 };

const unsigned int cmdb_delete_args[] = { 1,1,1,1,1,1 };

const unsigned int cmdb_update_args[] = { 2,2,2,2,2,2,2 };

const int cmdb_inserts[][11] = {
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, DBINT, DBINT, DBINT, DBINT },
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0 },
	{ DBINT, DBINT, DBINT, DBTEXT, DBTEXT, DBINT, DBINT, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBINT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0, 0 }
};

const unsigned int cmdb_search_args_type[][2] = {
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
	{ DBINT, DBTEXT },
	{ DBTEXT, DBINT },
	{ DBTEXT, DBINT },
	{ DBTEXT, DBINT },
	{ DBTEXT, DBINT }
};

const unsigned int cmdb_search_fields_type[][1] = {
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
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT }
};

const unsigned int cmdb_delete_args_type[][1] = {
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT },
	{ DBINT }
};

const unsigned int cmdb_update_args_type[][5] = {
	{ DBINT, DBINT, NONE, NONE, NONE },
	{ DBINT, DBINT, NONE, NONE, NONE },
	{ DBTEXT, DBINT, NONE, NONE, NONE },
	{ DBTEXT, DBINT, NONE, NONE, NONE },
	{ DBTEXT, DBINT, NONE, NONE, NONE },
	{ DBTEXT, DBINT, NONE, NONE, NONE },
	{ DBINT, DBINT, NONE, NONE, NONE }
};

int
cmdb_run_query(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval = 0;

	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cmdb_run_query");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cmdb_run_query_mysql(config, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cmdb_run_query_sqlite(config, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cmdb_run_multiple_query(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval = 0;
	
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cmdb_run_multiple_query");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cmdb_run_multiple_query_mysql(config, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cmdb_run_multiple_query_sqlite(config, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cmdb_run_search(cmdb_config_s *cmdb, dbdata_s *data, int type)
{
	int retval = 0;

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cmdb_run_search");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = cmdb_run_search_mysql(cmdb, data, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = cmdb_run_search_sqlite(cmdb, data, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, cmdb->dbtype);
	return retval;
}

int
cmdb_run_insert(cmdb_config_s *config, cmdb_s *base, int type)
{
	int retval = 0;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cmdb_run_insert");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cmdb_run_insert_mysql(config, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cmdb_run_insert_sqlite(config, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cmdb_run_delete(cmdb_config_s *config, dbdata_s *data, int type)
{
	int retval = NONE;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cmdb_run_delete");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cmdb_run_delete_mysql(config, data, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cmdb_run_delete_sqlite(config, data, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cmdb_run_update(cmdb_config_s *config, dbdata_s *data, int type)
{
	int retval = NONE;
	if (strncmp(config->dbtype, "none", RANGE_S) == 0)
		report_error(NO_DB_TYPE, "cmdb_run_update");
#ifdef HAVE_MYSQL
	else if (strncmp(config->dbtype, "mysql", RANGE_S) == 0)
		retval = cmdb_run_update_mysql(config, data, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if (strncmp(config->dbtype, "sqlite", RANGE_S) == 0)
		retval = cmdb_run_update_sqlite(config, data, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cmdb_get_query(int type, const char **query, unsigned int *fields)
{
	int retval = 0;

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
		retval = UNKNOWN_QUERY;
	}
	return retval;
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
	const char *query;
	int retval = 0;
	unsigned int fields;

	cmdb_mysql_init(config, &cmdb);
	if ((retval = cmdb_get_query(type, &query, &fields)) != 0) 
		report_error(retval, "cmdb_run_query_mysql");
	if ((retval = cmdb_mysql_query_with_checks(&cmdb, query)) != 0) {
		report_error(MY_QUERY_FAIL, mysql_error(&cmdb));
	}
	if (!(cmdb_res = mysql_store_result(&cmdb))) {
		cmdb_mysql_cleanup(&cmdb);
		report_error(MY_STORE_FAIL, mysql_error(&cmdb));
	}
	fields = mysql_num_fields(cmdb_res);
	while ((cmdb_row = mysql_fetch_row(cmdb_res)))
		store_result_mysql(cmdb_row, base, type, fields);
	cmdb_mysql_cleanup_full(&cmdb, cmdb_res);
	return retval;
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
	return retval;
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
	unsigned int i, k;
	dbdata_s *list = data;

	memset(args, 0, sizeof(args));
	memset(fields, 0, sizeof(fields));
	for (i = 0; i < cmdb_search_args[type]; i++) {
		k = cmdb_search_args_type[type][i];
		cmdb_set_bind_mysql(&args[i], k, &(list->args));
		list = list->next;
	}
	list = data;
	for (i = 0; i < cmdb_search_fields[type]; i++) {
		cmdb_set_search_fields_mysql(&fields[i], i, j, type, data);
	}
	cmdb_mysql_init(ccs, &cmdb);
	if (!(cmdb_stmt = mysql_stmt_init(&cmdb)))
		report_error(MY_STATEMENT_FAIL, mysql_error(&cmdb));
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
		for (i = 0; i < cmdb_search_fields[type]; i++) {
			cmdb_set_search_fields_mysql(&fields[i], i, j, type, data);
		}
		if ((retval = mysql_stmt_bind_result(cmdb_stmt, &fields[0])) != 0)
			report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	}
	if (retval != MYSQL_NO_DATA)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cmdb_stmt));
	mysql_stmt_free_result(cmdb_stmt);
	mysql_stmt_close(cmdb_stmt);
	cmdb_mysql_cleanup(&cmdb);
	return j;
}

void
cmdb_set_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base)
{
	int j;
	static int m = 0, stype = -1;
	dbdata_s *list, *new;
	list = base;

	if (stype == -1)
		stype = type;
	else if (stype != type) {
		stype = type;
		m = 0;
	}
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
	cmdb_set_bind_mysql(mybind, cmdb_search_fields_type[type][i], &(list->fields));
	m++;
}

int
cmdb_run_insert_mysql(cmdb_config_s *config, cmdb_s *base, int type)
{
	MYSQL cmdb;
	MYSQL_BIND my_bind[insert_fields[type]];
	const char *query;
	int retval;
	unsigned int i;

	memset(my_bind, 0, sizeof(my_bind));
	for (i=0; i<insert_fields[type]; i++) 
		if ((retval = setup_insert_mysql_bind(&my_bind[i], i, type, base)) != 0)
			report_error(DB_TYPE_INVALID, " in cmdb_run_insert_mysql");
	query = sql_insert[type];
	cmdb_mysql_init(config, &cmdb);
	retval = cmdb_run_mysql_stmt(&cmdb, my_bind, query);
	if (retval == 1)
		retval = 0;
	cmdb_mysql_cleanup(&cmdb);
	return retval;
}

int
cmdb_run_delete_mysql(cmdb_config_s *config, dbdata_s *data, int type)
{
	MYSQL cmdb;
	MYSQL_BIND my_bind[cmdb_delete_args[type]];
	const char *query;
	int retval = NONE;
	unsigned int i, k;
	dbdata_s *list = data;

	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < cmdb_delete_args[type]; i++) {
		k = cmdb_delete_args_type[type][i];
		cmdb_set_bind_mysql(&my_bind[i], k, &(list->args));
		list = list->next;
	}
	query = cmdb_sql_delete[type];
	cmdb_mysql_init(config, &cmdb);
	retval = cmdb_run_mysql_stmt(&cmdb, my_bind, query);
	cmdb_mysql_cleanup(&cmdb);
	return retval;
}

int
cmdb_run_update_mysql(cmdb_config_s *config, dbdata_s *data, int type)
{
	MYSQL cmdb;
	MYSQL_BIND my_bind[cmdb_update_args[type]];
	const char *query;
	int retval = NONE;
	unsigned int i, k;
	dbdata_s *list = data;

	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < cmdb_update_args[type]; i++) {
		k = cmdb_update_args_type[type][i];
		cmdb_set_bind_mysql(&my_bind[i], k, &(list->args));
		list = list->next;
	}
	query = cmdb_sql_update[type];
	cmdb_mysql_init(config, &cmdb);
	retval = cmdb_run_mysql_stmt(&cmdb, my_bind, query);
	cmdb_mysql_cleanup(&cmdb);
	return retval;
}

int
setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, cmdb_s *base)
{
	int retval = NONE;
	void *buffer;

	if (cmdb_inserts[type][i] == DBINT) {
		bind->is_unsigned = 1;
		bind->buffer_type = MYSQL_TYPE_LONG;
		bind->buffer_length = sizeof(unsigned long int);
	} else if (cmdb_inserts[type][i] == DBTEXT) {
		bind->is_unsigned = 0;
		bind->buffer_type = MYSQL_TYPE_STRING;
	} else if (cmdb_inserts[type][i] == DBSHORT) {
		bind->is_unsigned = 1;
		bind->buffer_type = MYSQL_TYPE_SHORT;
		bind->buffer_length = sizeof(short int);
	} else {
		return DB_TYPE_INVALID;
	}
	bind->is_null = 0;
	bind->length = 0;
	if ((retval = setup_insert_mysql_bind_buffer(type, &buffer, base, i)) != 0)
		return retval;
	bind->buffer = buffer;
	if (bind->buffer_type == MYSQL_TYPE_STRING)
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
		report_error(UNKNOWN_STRUCT_DB_TABLE, "cmdb_run_insert_mysql");
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
	else if (i == 7)
		*buffer = &(base->server->cuser);
	else if (i == 8)
		*buffer = &(base->server->muser);
	else if (i == 9)
		*buffer = &(base->server->ctime);
	else if (i == 10)
		*buffer = &(base->server->mtime);
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
	else if (i == 6)
		*buffer = &(base->customer->cuser);
	else if (i == 7)
		*buffer = &(base->customer->muser);
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
	else if (i == 4)
		*buffer = &(base->contact->cuser);
	else if (i == 5)
		*buffer = &(base->contact->muser);
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
	else if (i == 5)
		*buffer = &(base->service->cuser);
	else if (i == 6)
		*buffer = &(base->service->muser);
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
	else if (i == 4)
		*buffer = &(base->hardware->cuser);
	else if (i == 5)
		*buffer = &(base->hardware->muser);
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
	else if (i == 3)
		*buffer = &(base->vmhost->cuser);
	else if (i == 4)
		*buffer = &(base->vmhost->muser);
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
		fprintf(stderr, "Unknown type %d. Cannot store.\n",  type);
	}
}

void
store_server_mysql(MYSQL_ROW row, cmdb_s *base)
{
	char *timestamp;
	cmdb_server_s *server, *list;

	if (!(server = calloc(sizeof(cmdb_server_s), sizeof(char))))
		report_error(MALLOC_FAIL, "server in store_server_mysql");
	server->server_id = strtoul(row[0], NULL, 10);
	snprintf(server->vendor, CONF_S, "%s", row[1]);
	snprintf(server->make, CONF_S, "%s", row[2]);
	snprintf(server->model, CONF_S, "%s", row[3]);
	snprintf(server->uuid, CONF_S, "%s", row[4]);
	server->cust_id = strtoul(row[5], NULL, 10);
	server->vm_server_id = strtoul(row[6], NULL, 10);
	snprintf(server->name, HOST_S, "%s", row[7]);
	server->cuser = strtoul(row[8], NULL, 10);
	server->muser = strtoul(row[9], NULL, 10);
	timestamp = row[10];
	convert_time(timestamp, &(server->ctime));
	timestamp = row[11];
	convert_time(timestamp, &(server->mtime));
	server->next = NULL;
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
	char *timestamp;
	cmdb_customer_s *customer, *list;

	if (!(customer = calloc(sizeof(cmdb_customer_s), sizeof(char))))
		report_error(MALLOC_FAIL, "customer in store_customer_mysql");
	customer->cust_id = strtoul(row[0], NULL, 10);
	snprintf(customer->name, HOST_S, "%s", row[1]);
	snprintf(customer->address, NAME_S, "%s", row[2]);
	snprintf(customer->city, HOST_S, "%s", row[3]);
	snprintf(customer->county, MAC_S, "%s", row[4]);
	snprintf(customer->postcode, RANGE_S, "%s", row[5]);
	snprintf(customer->coid, RANGE_S, "%s", row[6]);
	customer->cuser = strtoul(row[7], NULL, 10);
	customer->muser = strtoul(row[8], NULL, 10);
	timestamp = row[9];
	convert_time(timestamp, &(customer->ctime));
	timestamp = row[10];
	convert_time(timestamp, &(customer->mtime));
	customer->next = NULL;
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
	char *timestamp;
	cmdb_contact_s *contact, *list;

	if (!(contact = calloc(sizeof(cmdb_contact_s), sizeof(char))))
		report_error(MALLOC_FAIL, "contact in store_contact_mysql");
	contact->cont_id = strtoul(row[0], NULL, 10);
	snprintf(contact->name, HOST_S, "%s", row[1]);
	snprintf(contact->phone, MAC_S, "%s", row[2]);
	snprintf(contact->email, HOST_S, "%s", row[3]);
	contact->cust_id = strtoul(row[4], NULL, 10);
	contact->cuser = strtoul(row[5], NULL, 10);
	contact->muser = strtoul(row[6], NULL, 10);
	timestamp = row[7];
	convert_time(timestamp, &(contact->ctime));
	timestamp = row[8];
	convert_time(timestamp, &(contact->mtime));
	contact->next = NULL;
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

	if (!(service = calloc(sizeof(cmdb_service_s), sizeof(char))))
		report_error(MALLOC_FAIL, "service in store_service_mysql");
	service->service_id = strtoul(row[0], NULL, 10);
	service->server_id = strtoul(row[1], NULL, 10);
	service->cust_id = strtoul(row[2], NULL, 10);
	service->service_type_id = strtoul(row[3], NULL, 10);
	snprintf(service->detail, HOST_S, "%s", row[4]);
	snprintf(service->url, RBUFF_S, "%s", row[5]);
	service->cuser = strtoul(row[6], NULL, 10);
	service->muser = strtoul(row[7], NULL, 10);
	convert_time(row[8], &(service->ctime));
	convert_time(row[9], &(service->mtime));
	service->next = NULL;
	type = base->servicetype;
	if (type) {
		while (service->service_type_id != type->service_id)
			type = type->next;
		service->servicetype = type;
	} else {
		service->servicetype = NULL;
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

	if (!(service = calloc(sizeof(cmdb_service_type_s), sizeof(char))))
		report_error(MALLOC_FAIL, "service in store_service_type_mysql");

	service->service_id = strtoul(row[0], NULL, 10);
	snprintf(service->service, RANGE_S, "%s", row[1]);
	snprintf(service->detail, MAC_S, "%s", row[2]);
	service->next = NULL;
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

	if (!(hard = calloc(sizeof(cmdb_hardware_s), sizeof(char))))
		report_error(MALLOC_FAIL, "hardware in store_hardware_mysql");

	hard->hard_id = strtoul(row[0], NULL, 10);
	snprintf(hard->detail, HOST_S, "%s", row[1]);
	snprintf(hard->device, MAC_S, "%s", row[2]);
	hard->server_id = strtoul(row[3], NULL, 10);
	hard->ht_id = strtoul(row[4], NULL, 10);
	hard->cuser = strtoul(row[5], NULL, 10);
	hard->muser = strtoul(row[6], NULL, 10);
	convert_time(row[7], &(hard->ctime));
	convert_time(row[8], &(hard->mtime));
	hard->next = NULL;
	type = base->hardtype;
	if (type) {
		while (hard->ht_id != type->ht_id)
			type = type->next;
		hard->hardtype = type;
	} else {
		hard->hardtype = NULL;
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

	if (!(hard = calloc(sizeof(cmdb_hard_type_s), sizeof(char))))
		report_error(MALLOC_FAIL, "hardware in store_hardware_type_mysql");

	hard->ht_id = strtoul(row[0], NULL, 10);
	snprintf(hard->type, MAC_S, "%s", row[1]);
	snprintf(hard->hclass, MAC_S, "%s", row[2]);
	hard->next = NULL;
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
	char *timestamp;
	cmdb_vm_host_s *vmhost, *list;

	if (!(vmhost = calloc(sizeof(cmdb_vm_host_s), sizeof(char))))
		report_error(MALLOC_FAIL, "vmhost in store_vm_hosts_mysql");
	vmhost->id = strtoul(row[0], NULL, 10);
	snprintf(vmhost->name, RBUFF_S, "%s", row[1]);
	snprintf(vmhost->type, MAC_S, "%s", row[2]);
	vmhost->server_id = strtoul(row[3], NULL, 10);
	vmhost->cuser = strtoul(row[4], NULL, 10);
	vmhost->muser = strtoul(row[5], NULL, 10);
	timestamp = row[6];
	convert_time(timestamp, &(vmhost->ctime));
	timestamp = row[7];
	convert_time(timestamp, &(vmhost->mtime));
	vmhost->next = NULL;
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
	if ((retval = cmdb_get_query(type, &query, &fields)) != 0) {
		report_error(retval, "cmdb_run_query_sqlite");
	}
	cmdb_setup_ro_sqlite(query, file, &cmdb, &state);
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		store_result_sqlite(state, base, type, fields);
	cmdb_sqlite_cleanup(cmdb, state);
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
cmdb_run_search_sqlite(cmdb_config_s *ccs, dbdata_s *data, int type)
{
	const char *query = NULL, *file = NULL;
	int retval = NONE, i;
	dbdata_s *list = NULL;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	query = sql_search[type];
	if (ccs)
		file = ccs->file;
	else
		report_error(CBC_NO_DATA, "ccs in cmdb_run_search_sqlite");
	cmdb_setup_ro_sqlite(query, file, &cmdb, &state);
	if (data)
		list = data;
	else
		report_error(CBC_NO_DATA, "data in cmdb_run_search_sqlite");
	for (i = 0; (unsigned)i < cmdb_search_args[type]; i++) {
		if ((retval = set_cmdb_args_sqlite(state, list, cmdb_search_args_type[type][i], i)) < 0)
			break;
		if (list->next) 
			list = list->next;
	}
	if (retval == DB_WRONG_TYPE) {
		cmdb_sqlite_cleanup(cmdb, state);
		report_error(retval, query);
	}
	list = data;
	i = NONE;
	while ((sqlite3_step(state)) == SQLITE_ROW) {
		if ((retval =
		 get_cmdb_search_res_sqlite(state, list, type, i)) != 0)
			break;
		i++;
	}
	cmdb_sqlite_cleanup(cmdb, state);
	if (retval == DB_WRONG_TYPE)
		report_error(retval, query);
	return i;
}

int
cmdb_run_update_sqlite(cmdb_config_s *config, dbdata_s *data, int type)
{
	const char *query = NULL, *file = NULL;
	int retval = NONE, i;
	dbdata_s *list = NULL;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	if (config)
		file = config->file;
	else
		report_error(CBC_NO_DATA, "config in cmdb_run_update_sqlite");
	if (data)
		list = data;
	else
		report_error(CBC_NO_DATA, "data in cmdb_run_update_sqlite");
	query = cmdb_sql_update[type];
	cmdb_setup_rw_sqlite(query, file, &cmdb, &state);
	for (i = 0; (unsigned)i < cmdb_update_args[type]; i++) {
		if ((retval = set_cmdb_args_sqlite(state, list, cmdb_update_args_type[type][i], i)) < 0)
			break;
		if (list->next)
			list = list->next;
	}
	if (retval == DB_WRONG_TYPE) {
		cmdb_sqlite_cleanup(cmdb, state);
		report_error(retval, query);
	}
	list = data;
	i = NONE;
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cmdb));
		cmdb_sqlite_cleanup(cmdb, state);
		return SQLITE_INSERT_FAILED;
	}
	retval = sqlite3_changes(cmdb);
	cmdb_sqlite_cleanup(cmdb, state);
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
			if (cmdb_search_fields_type[type][j] == DBTEXT)
				snprintf(data->fields.text, RBUFF_S, "%s",
				 sqlite3_column_text(state, j));
			else if (cmdb_search_fields_type[type][j] == DBINT)
				data->fields.number =
				 (uli_t)sqlite3_column_int64(state, j);
			else if (cmdb_search_fields_type[type][j] == DBSHORT)
				data->fields.small =
				 (short int)sqlite3_column_int(state, j);
			else
				return DB_WRONG_TYPE;
			list->next = data;
		}
	} else {
		for (j = 0; (unsigned)j < cmdb_search_fields[type]; j++) {
			if (cmdb_search_fields_type[type][j] == DBTEXT)
				snprintf(list->fields.text, RBUFF_S, "%s",
				 sqlite3_column_text(state, j));
			else if (cmdb_search_fields_type[type][j] == DBINT)
				list->fields.number =
				 (uli_t)sqlite3_column_int64(state, j);
			else if (cmdb_search_fields_type[type][j] == DBSHORT)
				list->fields.small =
				 (short int)sqlite3_column_int(state, j);
			else
				return DB_WRONG_TYPE;
		}
	}
	return retval;
}

int
set_cmdb_args_sqlite(sqlite3_stmt *state, dbdata_s *list, unsigned int type, int i)
{
	int retval = NONE;

	if (type == DBTEXT) {
		if ((retval = sqlite3_bind_text(
state, i + 1, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
			fprintf(stderr, "Cannot bind search arg %s: %s\n",
				list->args.text, sqlite3_errstr(retval));
			return retval;
		}
	} else if (type == DBINT) {
		if ((retval = sqlite3_bind_int64(
state, i + 1, (sqlite3_int64)list->args.number)) > 0) {
			fprintf(stderr, "Cannot bind search number arg %lu: %s\n",
				list->args.number, sqlite3_errstr(retval));
			return retval;
		}
	} else if (type == DBSHORT) {
		if ((retval = sqlite3_bind_int(state, i + 1, list->args.small)) > 0) {
			fprintf(stderr, "Cannot bind search small arg %d: %s\n",
				list->args.small, sqlite3_errstr(retval));
			return retval;
		}
	} else {
		retval = DB_WRONG_TYPE;
	}
	return retval;
}

int
cmdb_run_insert_sqlite(cmdb_config_s *config, cmdb_s *base, int type)
{
	const char *query, *file;
	int retval;
	sqlite3 *cmdb;
	sqlite3_stmt *state;

	retval = 0;
	query = sql_insert[type];
	file = config->file;
	cmdb_setup_rw_sqlite(query, file, &cmdb, &state);
	if ((retval = setup_insert_sqlite_bind(state, base, type)) != 0) {
		if (retval == DB_TYPE_INVALID)
			fprintf(stderr, "Unknown data type %d in cmdb_run_insert_sqlite", type);
		else
			fprintf(stderr, "Error binding result! %s\n", sqlite3_errmsg(cmdb));
		sqlite3_close(cmdb);
		return SQLITE_INSERT_FAILED;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cmdb));
		cmdb_sqlite_cleanup(cmdb, state);
		return SQLITE_INSERT_FAILED;
	}
	cmdb_sqlite_cleanup(cmdb, state);
	return NONE;
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

	cmdb_setup_rw_sqlite(query, file, &cmdb, &state);
	for (i = 0; i < cmdb_delete_args[type]; i++) {
		if (!list)
			break;
		if ((retval = set_cmdb_args_sqlite(state, list, cmdb_delete_args_type[type][i], (int)i)) != 0) {
			cmdb_sqlite_cleanup(cmdb, state);
			return retval;
		}
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Received error: %s\n", sqlite3_errmsg(cmdb));
		cmdb_sqlite_cleanup(cmdb, state);
		return NONE;
	}
	retval = sqlite3_changes(cmdb);
	cmdb_sqlite_cleanup(cmdb, state);
	return retval;
}

int
setup_insert_sqlite_bind(sqlite3_stmt *state, cmdb_s *cmdb, int type)
{
	int retval = 0;
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
		report_error(UNKNOWN_STRUCT_DB_TABLE, "cmdb_run_insert_mysql");
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
		fprintf(stderr, "Unknown type %d. Cannot store.\n",  type);
	}
}

void
store_server_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	char *stime;
	cmdb_server_s *server, *list;

	if (!(server = malloc(sizeof(cmdb_server_s))))
		report_error(MALLOC_FAIL, "server in store_server_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "ctime in store_server_sqlite");
	server->server_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(server->vendor, CONF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(server->make, CONF_S, "%s", sqlite3_column_text(state, 2));
	snprintf(server->model, CONF_S, "%s", sqlite3_column_text(state, 3));
	snprintf(server->uuid, CONF_S, "%s", sqlite3_column_text(state, 4));
	server->cust_id = (unsigned long int) sqlite3_column_int(state, 5);
	server->vm_server_id = (unsigned long int) sqlite3_column_int(state, 6);
	snprintf(server->name, HOST_S, "%s", sqlite3_column_text(state, 7));
	server->cuser = (uli_t) sqlite3_column_int(state, 8);
	server->muser = (uli_t) sqlite3_column_int(state, 9);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 10));
	convert_time(stime, &(server->ctime));
	memset(stime, 0, MAC_S);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 11));
	convert_time(stime, &(server->mtime));
	memset(stime, 0, MAC_S);
	server->next = NULL;
	list = base->server;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = server;
	} else {
		base->server = server;
	}
	free(stime);
}

void
store_customer_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	char *stime;
	cmdb_customer_s *cust, *list;

	if (!(cust = malloc(sizeof(cmdb_customer_s))))
		report_error(MALLOC_FAIL, "cust in store_customer_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in store_customer_sqlite");
	cust->cust_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(cust->name, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(cust->address, NAME_S, "%s", sqlite3_column_text(state, 2));
	snprintf(cust->city, HOST_S, "%s", sqlite3_column_text(state, 3));
	snprintf(cust->county, MAC_S, "%s", sqlite3_column_text(state, 4));
	snprintf(cust->postcode, RANGE_S, "%s", sqlite3_column_text(state, 5));
	snprintf(cust->coid, RANGE_S, "%s", sqlite3_column_text(state, 6));
	cust->cuser = (uli_t) sqlite3_column_int(state, 7);
	cust->muser = (uli_t) sqlite3_column_int(state, 8);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 9));
	convert_time(stime, &(cust->ctime));
	memset(stime, 0, MAC_S);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 10));
	convert_time(stime, &(cust->mtime));
	memset(stime, 0, MAC_S);
	cust->next = NULL;
	list = base->customer;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = cust;
	} else {
		base->customer = cust;
	}
	free(stime);
}

void
store_contact_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	char *stime;
	cmdb_contact_s *contact, *list;

	if (!(contact = malloc(sizeof(cmdb_contact_s))))
		report_error(MALLOC_FAIL, "contact in store_contact_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in store_contact_sqlite");
	contact->cont_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(contact->name, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(contact->phone, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(contact->email, HOST_S, "%s", sqlite3_column_text(state, 3));
	contact->cust_id = (unsigned long int) sqlite3_column_int(state, 4);
	contact->cuser = (uli_t) sqlite3_column_int(state, 5);
	contact->muser = (uli_t) sqlite3_column_int(state, 6);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(contact->ctime));
	memset(stime, 0, MAC_S);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 8));
	convert_time(stime, &(contact->mtime));
	memset(stime, 0, MAC_S);
	contact->next = NULL;
	list = base->contact;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = contact;
	} else {
		base->contact = contact;
	}
	free(stime);
}

void
store_service_sqlite(sqlite3_stmt *state, cmdb_s *base)
{
	char *stime;
	cmdb_service_s *service, *list;
	cmdb_service_type_s *type;

	if (!(service = malloc(sizeof(cmdb_service_s))))
		report_error(MALLOC_FAIL, "service in store_service_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in store_service_sqlite");
	service->service_id = (unsigned long int) sqlite3_column_int(state, 0);
	service->server_id = (unsigned long int) sqlite3_column_int(state, 1);
	service->cust_id = (unsigned long int) sqlite3_column_int(state, 2);
	service->service_type_id = (unsigned long int) sqlite3_column_int(state, 3);
	snprintf(service->detail, HOST_S, "%s", sqlite3_column_text(state, 4));
	snprintf(service->url, RBUFF_S, "%s", sqlite3_column_text(state, 5));
	service->cuser = (uli_t) sqlite3_column_int(state, 6);
	service->muser = (uli_t) sqlite3_column_int(state, 7);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 8));
	convert_time(stime, &(service->ctime));
	memset(stime, 0, MAC_S);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 9));
	convert_time(stime, &(service->mtime));
	memset(stime, 0, MAC_S);
	type = base->servicetype;
	if (type) {
		while (service->service_type_id != type->service_id)
			type = type->next;
		service->servicetype = type;
	} else {
		service->servicetype = NULL;
	}
	service->next = NULL;
	list = base->service;
	if (list) {
		while (list->next) {
			list = list->next;
		}
		list->next = service;
	} else {
		base->service = service;
	}
	free(stime);
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
	service->next = NULL;
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
	char *stime;
	cmdb_hardware_s *hard, *list;
	cmdb_hard_type_s *type;

	if (!(hard = malloc(sizeof(cmdb_hardware_s))))
		report_error(MALLOC_FAIL, "hard in store_hardware_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in store_hardware_sqlite");
	hard->hard_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(hard->detail, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(hard->device, MAC_S, "%s", sqlite3_column_text(state, 2));
	hard->server_id = (unsigned long int) sqlite3_column_int(state, 3);
	hard->ht_id = (unsigned long int) sqlite3_column_int(state, 4);
	hard->cuser = (uli_t) sqlite3_column_int(state, 5);
	hard->muser = (uli_t) sqlite3_column_int(state, 6);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(hard->ctime));
	memset(stime, 0, MAC_S);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 8));
	convert_time(stime, &(hard->mtime));
	memset(stime, 0, MAC_S);
	hard->next = NULL;
	type = base->hardtype;
	if (type) {
		while (hard->ht_id != type->ht_id)
			type = type->next;
		hard->hardtype = type;
	} else {
		hard->hardtype = NULL;
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
	free(stime);
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
	hard->next = NULL;
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
	char *stime;
	cmdb_vm_host_s *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cmdb_vm_host_s))))
		report_error(MALLOC_FAIL, "vmhost in store_vm_hosts_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in store_vm_hosts_sqlite");
	vmhost->id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(vmhost->name, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vmhost->type, MAC_S, "%s", sqlite3_column_text(state, 2));
	vmhost->server_id = (unsigned long int) sqlite3_column_int(state, 3);
	vmhost->cuser = (uli_t) sqlite3_column_int(state, 4);
	vmhost->muser = (uli_t) sqlite3_column_int(state, 5);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(vmhost->ctime));
	memset(stime, 0, MAC_S);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(vmhost->mtime));
	memset(stime, 0, MAC_S);
	vmhost->next = NULL;
	list = base->vmhost;
	if (list) {
		while(list->next) {
			list = list->next;
		}
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	}
	free(stime);
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
	if ((retval = sqlite3_bind_int(
state, 8, (int)server->cuser)) > 0) {
		printf("Cannot bind %lu\n", server->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 9, (int)server->muser)) > 0) {
		printf("Cannot bind %lu\n", server->muser);
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
	if ((retval = sqlite3_bind_int(
state, 7, (int)cust->cuser)) > 0) {
		printf("Cannot bind %lu\n", cust->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 8, (int)cust->muser)) > 0) {
		printf("Cannot bind %lu\n", cust->muser);
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
	if ((retval = sqlite3_bind_int(
state, 5, (int)cont->cuser)) > 0) {
		printf("Cannot bind %lu\n", cont->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 6, (int)cont->muser)) > 0) {
		printf("Cannot bind %lu\n", cont->muser);
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
	if ((retval = sqlite3_bind_int(
state, 6, (int)service->cuser)) > 0) {
		printf("Cannot bind %lu\n", service->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 7, (int)service->muser)) > 0) {
		printf("Cannot bind %lu\n", service->muser);
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
	if ((retval = sqlite3_bind_int(
state, 5, (int)hard->cuser)) > 0) {
		printf("Cannot bind %lu\n", hard->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 6, (int)hard->muser)) > 0) {
		printf("Cannot bind %lu\n", hard->muser);
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
	if ((retval = sqlite3_bind_int(
state, 4, (int)vmhost->cuser)) > 0) {
		printf("Cannot bind %lu\n", vmhost->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int(
state, 5, (int)vmhost->muser)) > 0) {
		printf("Cannot bind %lu\n", vmhost->muser);
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
	else if (type == HARDWARE_TYPE)
		;
	else if (type == HARDWARE)
		fprintf(stderr, "No hardware to list\n");
	else if (type == VM_HOST)
		fprintf(stderr, "No vm hosts to list\n");
	else
		fprintf(stderr, "No unknown listing %d\n", type);
}
