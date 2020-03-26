/*
 *
 *  alisacmdb: Alisatech Configuration Management Database library
 *  Copyright (C) 2015 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  sql.c
 *
 *  Contains the functions to initialise and destroy various data types
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

#include <ailsacmdb.h>
#include <ailsasql.h>
#include <cmdb.h>
#include <base_sql.h>

const struct ailsa_sql_single_s server[12] = {
	{ .string = "name", .type = AILSA_DB_TEXT, .length = HOST_S },
	{ .string = "make", .type = AILSA_DB_TEXT, .length = HOST_S },
	{ .string = "uuid", .type = AILSA_DB_TEXT, .length = HOST_S },
	{ .string = "vendor", .type = AILSA_DB_TEXT, .length = HOST_S },
	{ .string = "model", .type = AILSA_DB_TEXT, .length = MAC_LEN },
	{ .string = "server_id", .type = AILSA_DB_LINT, .length = 0 },
	{ .string = "cust_id", .type = AILSA_DB_LINT, .length = 0 },
	{ .string = "vm_server_id", .type = AILSA_DB_LINT, .length = 0 },
	{ .string = "cuser", .type = AILSA_DB_LINT, .length = 0 },
	{ .string = "muser", .type = AILSA_DB_LINT, .length = 0 },
	{ .string = "ctime", .type = AILSA_DB_TIME, .length = 0 },
	{ .string = "mtime", .type = AILSA_DB_TIME, .length = 0 }
};

const char *basic_queries[] = {
"SELECT s.name, c.coid FROM server s INNER JOIN customer c on c.cust_id = s.cust_id", // SERVER_NAME_COID
"SELECT coid, name, city FROM customer ORDER BY coid", // COID_NAME_CITY
};

const struct ailsa_sql_query_s argument_queries[1] = {
	{ // CONTACT_DETAILS_ON_COID
"SELECT co.name, co.phone, co.email FROM customer cu INNER JOIN contacts co ON co.cust_id = cu.cust_id WHERE cu.coid = ?",
	1,
	{ AILSA_DB_TEXT }
	}
};

#ifdef HAVE_MYSQL
static void
ailsa_mysql_init(ailsa_cmdb_s *dc, MYSQL *sql);
static int
ailsa_basic_query_mysql(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results);
int
ailsa_argument_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results);
static void
ailsa_store_mysql_row(MYSQL_ROW row, AILLIST *results, unsigned int *fields);
#endif

#ifdef HAVE_SQLITE3
static int
ailsa_basic_query_sqlite(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results);
int
ailsa_argument_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results);
static void
ailsa_store_basic_sqlite(sqlite3_stmt *state, AILLIST *results);
static int
ailsa_bind_arguments_sqlite(sqlite3_stmt *state, const struct ailsa_sql_query_s argu, AILLIST *args);
#endif

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *results)
{
	int retval = 0;
	const char *query = basic_queries[query_no];

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_basic_query_mysql(cmdb, query, results);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_basic_query_sqlite(cmdb, query, results);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;
}

int
ailsa_argument_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *args, AILLIST *results)
{
	int retval;
	const struct ailsa_sql_query_s argument = argument_queries[query_no];

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_argument_query_mysql(cmdb, argument, args, results);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_argument_query_sqlite(cmdb, argument, args, results);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;	
}

#ifdef HAVE_MYSQL

static void
ailsa_mysql_init(ailsa_cmdb_s *dc, MYSQL *sql)
{
	const char *unix_socket;

	unix_socket = dc->socket;

	if (!(mysql_init(sql))) {
		ailsa_syslog(LOG_ERR, "Error initalising MySQL: %s", mysql_error(sql));
		exit(MY_INIT_FAIL);
	}
	if (!(mysql_real_connect(sql, dc->host, dc->user, dc->pass,
	  dc->db, dc->port, unix_socket, dc->cliflag))) {
		ailsa_syslog(LOG_ERR, "Cannot connect to MySQL DB: %s", mysql_error(sql));
		exit(MY_CONN_FAIL);
	}
	
}

static int
ailsa_basic_query_mysql(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results)
{
	if (!(cmdb) || !(query) || !(results))
		return AILSA_NO_DATA;

	int retval = 0;
	unsigned int total, i, *fields;
	size_t size;
	MYSQL sql;
	MYSQL_RES *sql_res;
	MYSQL_ROW sql_row;
	MYSQL_FIELD *field;
	my_ulonglong sql_rows;

	ailsa_mysql_init(cmdb, &sql);

	if ((retval = cmdb_mysql_query_with_checks(&sql, query)) != 0) {
		ailsa_syslog(LOG_ERR, "MySQL query failed: %s", mysql_error(&sql));
		exit(MY_QUERY_FAIL);
	}
	if (!(sql_res = mysql_store_result(&sql))) {
		cmdb_mysql_cleanup(&sql);
		ailsa_syslog(LOG_ERR, "MySQL store result failed: %s", mysql_error(&sql));
		exit(MY_STORE_FAIL);
	}

	total = mysql_num_fields(sql_res);
	size = (size_t)(total + 1) * sizeof(unsigned int);
	fields = ailsa_calloc(size, "fields in ailsa_basic_query_mysql");
	for (i = 0; i < total; i++) {
		field = mysql_fetch_field_direct(sql_res, i);
		fields[i + 1] = field->type;
	}
	*fields = i;
	if (((sql_rows = mysql_num_rows(sql_res)) != 0)) {
		while ((sql_row = mysql_fetch_row(sql_res)))
			ailsa_store_mysql_row(sql_row, results, fields);
	}
	cmdb_mysql_cleanup_full(&sql, sql_res);
	my_free(fields);
	return retval;
}

int
ailsa_argument_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results)
{
	if (!(cmdb) || !(args) || !(results))
		return AILSA_NO_DATA;
	int retval = 0;

	return retval;
}

static void
ailsa_store_mysql_row(MYSQL_ROW row, AILLIST *results, unsigned int *fields)
{
	if (!(row) || !(results) || (fields == 0))
		return;
	unsigned int i, n, *p;
	int retval = 0;
	ailsa_data_s *tmp;

	p = fields;
	n = fields[0];
	for (i = 1; i <= n; i++) {
		tmp = ailsa_calloc(sizeof(ailsa_data_s), "tmp in ailsa_store_mysql_row");
		ailsa_init_data(tmp);
		switch(p[i]) {
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_VARCHAR:
			tmp->data->text = strndup(row[i - 1], SQL_TEXT_MAX);
			tmp->type = AILSA_DB_TEXT;
			break;
		case MYSQL_TYPE_LONG:
			tmp->data->number = strtoul(row[i - 1], NULL, 10);
			tmp->type = AILSA_DB_LINT;
			break;
		default:
			ailsa_syslog(LOG_ERR, "Unknown mysql type %u", p[i]);
			break;
		}
		if ((retval = ailsa_list_insert(results, tmp) != 0))
			ailsa_syslog(LOG_ERR, "Cannot insert data into list in store_mysql_row");
	}
}

#endif

#ifdef HAVE_SQLITE3
static int
ailsa_basic_query_sqlite(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results)
{
	if (!(cmdb) || !(query) || !(results))
		return AILSA_NO_DATA;
	int retval = 0;
	const char *file = cmdb->file;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;

	cmdb_setup_ro_sqlite(query, file, &sql, &state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		ailsa_store_basic_sqlite(state, results);
	cmdb_sqlite_cleanup(sql, state);
	if (retval == SQLITE_DONE)
		retval = 0;
	return retval;
}

int
ailsa_argument_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results)
{
	if (!(cmdb) || !(args) || !(results))
		return AILSA_NO_DATA;
	int retval = 0;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;
	const char *query = argument.query;
	const char *file = cmdb->file;

	cmdb_setup_ro_sqlite(query, file, &sql, &state);
	if ((retval = ailsa_bind_arguments_sqlite(state, argument, args)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to bind sqlite arguments: got error %d", retval);
		return retval;
	}
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		ailsa_store_basic_sqlite(state, results);
	cmdb_sqlite_cleanup(sql, state);
	if (retval == SQLITE_DONE)
		retval = 0;
	return retval;
}

static void
ailsa_store_basic_sqlite(sqlite3_stmt *state, AILLIST *results)
{
	ailsa_data_s *tmp;
	if (!(state) || !(results))
		return;
	short int fields, i;
	int type, retval;

	fields = (short int)sqlite3_column_count(state);
	retval = 0;
	for (i=0; i<fields; i++) {
		tmp = ailsa_calloc(sizeof(ailsa_data_s), "tmp in ailsa_store_basic_sqlite");
		ailsa_init_data(tmp);
		type = sqlite3_column_type(state, (int)i);
		switch(type) {
		case SQLITE_INTEGER:
			tmp->data->number = (unsigned long int)sqlite3_column_int(state, (int)i);
			tmp->type = AILSA_DB_LINT;
			break;
	 	case SQLITE_TEXT:  // Can use sqlite3_column_bytes() to get size
			tmp->data->text = strndup((const char *)sqlite3_column_text(state, (int)i), SQL_TEXT_MAX);
			tmp->type = AILSA_DB_TEXT;
			break;
		case SQLITE_FLOAT:
			tmp->data->point = sqlite3_column_double(state, (int)i);
			tmp->type = AILSA_DB_FLOAT;
			break;
		default:
			ailsa_syslog(LOG_ERR, "Unknown sqlite type %d", type);
			break;
		}
		if ((retval = ailsa_list_insert(results, tmp) != 0))
			ailsa_syslog(LOG_ERR, "Cannot insert data %hi into list in ailsa_store_basic_sqlite", i);
	}
}

static int
ailsa_bind_arguments_sqlite(sqlite3_stmt *state, const struct ailsa_sql_query_s argu, AILLIST *args)
{
	if (!(state) || !(args))
		return AILSA_NO_DATA;
	const char *text = NULL;
	int retval = 0;
	short int i = 0, count = argu.number;
	ailsa_data_s *data;
	AILELEM *tmp = args->head;

	for (i = 0; i < count; i++) {
		data = (ailsa_data_s *)tmp->data;
		switch (argu.fields[i]) {
		case AILSA_DB_LINT:
			if ((retval = sqlite3_bind_int64(state, i + 1, (sqlite3_int64)data->data->number)) != 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind integer value in loop %hi", i);
				goto cleanup;
			}
			break;
		case AILSA_DB_TEXT:
			text = data->data->text;
			if ((retval = sqlite3_bind_text(state, i + 1, text, (int)strlen(text), SQLITE_STATIC)) > 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind text value %s", text);
				goto cleanup;
			}
			break;
		case AILSA_DB_FLOAT:
			if ((retval = sqlite3_bind_double(state, i + 1, data->data->point)) != 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind double value %g", data->data->point);
				goto cleanup;
			}
			break;
		default:
			ailsa_syslog(LOG_ERR, "Unknown SQL type %hi", argu.fields[i]);
			retval = AILSA_INVALID_DBTYPE;
			goto cleanup;
		}
	}
	cleanup:
		return retval;
}

#endif
