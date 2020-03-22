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
	{ .string = "name", .type = DBTEXT, .length = HOST_S },
	{ .string = "make", .type = DBTEXT, .length = HOST_S },
	{ .string = "uuid", .type = DBTEXT, .length = HOST_S },
	{ .string = "vendor", .type = DBTEXT, .length = HOST_S },
	{ .string = "model", .type = DBTEXT, .length = MAC_LEN },
	{ .string = "server_id", .type = DBINT, .length = 0 },
	{ .string = "cust_id", .type = DBINT, .length = 0 },
	{ .string = "vm_server_id", .type = DBINT, .length = 0 },
	{ .string = "cuser", .type = DBINT, .length = 0 },
	{ .string = "muser", .type = DBINT, .length = 0 },
	{ .string = "ctime", .type = DBTIME, .length = 0 },
	{ .string = "mtime", .type = DBTIME, .length = 0 }
};

const struct ailsa_sql_basic_s basic_queries[1] = {
	{ "\
SELECT s.name, c.coid FROM server s INNER JOIN customer c on c.cust_id = s.cust_id", // SERVER_NAME_COID
	  2,
	  { (short int)DBTEXT, (short int)DBTEXT } 
	}
};

size_t server_fields = sizeof(server) / sizeof(ailsa_sql_single_s);  // Howto get the size of the array

#ifdef HAVE_MYSQL
static void
ailsa_mysql_init(ailsa_cmdb_s *dc, MYSQL *sql);
static int
ailsa_basic_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_basic_s *data, AILLIST *results);
static void
ailsa_store_mysql_row(MYSQL_ROW row, AILLIST *results, unsigned int *fields);
#endif

#ifdef HAVE_SQLITE3
static int
ailsa_basic_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_basic_s *data, AILLIST *results);
static void
ailsa_store_basic_sqlite(sqlite3_stmt *state, const ailsa_sql_basic_s *data, AILLIST *results);
#endif

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, short int query_no, AILLIST *results)
{
	int retval = 0;
	const struct ailsa_sql_basic_s *query = &basic_queries[query_no];

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "ailsa_basic_query");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_basic_query_mysql(cmdb, query, results);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_basic_query_sqlite(cmdb, query, results);
#endif
	else
		report_error(DB_TYPE_INVALID, cmdb->dbtype);
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
ailsa_basic_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_basic_s *data, AILLIST *results)
{
	if (!(cmdb) || !(data) || !(results))
		return AILSA_NO_DATA;

	int retval = 0;
	unsigned int total, i, *fields;
	size_t size;
	const char *query = data->query;
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
ailsa_basic_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_basic_s *data, AILLIST *results)
{
	int retval = 0;
	if (!(cmdb) || !(data) || !(results))
		return AILSA_NO_DATA;
	const char *query = data->query;
	const char *file = cmdb->file;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;

	cmdb_setup_ro_sqlite(query, file, &sql, &state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		ailsa_store_basic_sqlite(state, data, results);
	cmdb_sqlite_cleanup(sql, state);
	if (retval == SQLITE_DONE)
		retval = 0;
	return retval;
}

static void
ailsa_store_basic_sqlite(sqlite3_stmt *state, const ailsa_sql_basic_s *data, AILLIST *results)
{
	ailsa_data_s *tmp;
	if (!(state) || !(data) || !(results))
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
#endif
