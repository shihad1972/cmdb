/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *
 *  Base SQL functions for libailsasql
 *
 *
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#endif // HAVE_STDBOOL_H
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /*HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /*HAVE_SQLITE3 */
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "cmdb.h"

#ifdef HAVE_MYSQL

char mysql_time[MAC_LEN];

void
ailsa_mysql_init(ailsa_cmdb_s *dc, MYSQL *cbc_mysql)
{
	const char *unix_socket;

	unix_socket = dc->socket;

	if (!(mysql_init(cbc_mysql))) {
		report_error(MY_INIT_FAIL, mysql_error(cbc_mysql));
	}
	if (!(mysql_real_connect(cbc_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(cbc_mysql));
}

int
ailsa_mysql_query_with_checks(MYSQL *mycmdb, const char *query)
{
	int error;

	error = mysql_query(mycmdb, query);
	return error;
}

int
ailsa_run_mysql_stmt(MYSQL *cmdb, MYSQL_BIND *my_bind, const struct ailsa_sql_query_s argu, AILLIST *args)
{
	int retval = NONE;
	const char *query = argu.query;
	MYSQL_STMT *stmt = NULL;
	if (!(cmdb) || !(args))
		return -1;

	if (!(stmt = mysql_stmt_init(cmdb))) {
		ailsa_syslog(LOG_ERR, "%s", mysql_error(cmdb));
		retval = -1;
		goto cleanup;
	}
	if ((retval = mysql_stmt_prepare(stmt, query, strlen(query))) != 0) {
		ailsa_syslog(LOG_ERR, "%s", mysql_stmt_error(stmt));
		retval = -1;
		goto cleanup;
	}
	if ((retval = ailsa_bind_params_mysql(stmt, &my_bind, argu, args)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot bind paramaters for sql query");
		retval = -1;
		goto cleanup;
	}
	if ((retval = mysql_stmt_bind_param(stmt, my_bind)) != 0) {
		ailsa_syslog(LOG_ERR, "%s", mysql_stmt_error(stmt));
		retval = -1;
		goto cleanup;
	}
	if ((retval = mysql_stmt_execute(stmt)) != 0) {
		ailsa_syslog(LOG_ERR, "%s", mysql_stmt_error(stmt));
		retval = -1;
		goto cleanup;
	}
	retval = (int)mysql_stmt_affected_rows(stmt);
	my_free(my_bind);
	cleanup:
		if (stmt)
			mysql_stmt_close(stmt);
		return retval;
}

int
ailsa_bind_parameters_mysql(MYSQL_STMT *stmt, MYSQL_BIND **bind, AILLIST *list, unsigned int total, unsigned int *f)
{
	if (!(stmt) || !(list) || !(f) || (total == 0))
		return AILSA_NO_DATA;
	int retval = 0;
	unsigned int i;
	MYSQL_BIND *tmp = ailsa_calloc(sizeof(MYSQL_BIND) * (size_t)total, "tmp in ailsa_bind_parameters_mysql");
	AILELEM *elem = list->head;
	ailsa_data_s *data;
	for (i = 0; i < total; i++) {
		data = elem->data;
		if ((retval = ailsa_set_bind_mysql(&(tmp[i]), data, f[i])) != 0) {
			my_free(tmp);
			return retval;
		}
		elem = elem->next;
	}
	*bind = tmp;
	if ((retval = mysql_stmt_bind_param(stmt, tmp)) != 0)
		ailsa_syslog(LOG_ERR, "Unable to bind parameters: %s", mysql_stmt_error(stmt));
	return retval;
}

void
ailsa_mysql_cleanup(MYSQL *cmdb)
{
        mysql_close(cmdb);
        mysql_library_end();
}

void
ailsa_mysql_cleanup_full(MYSQL *cmdb, MYSQL_RES *res)
{
        mysql_free_result(res);
        mysql_close(cmdb);
        mysql_library_end();
}

char *
ailsa_convert_mysql_time(MYSQL_TIME *time)
{
	memset(mysql_time, 0, MAC_LEN);
	if (!(time))
		return mysql_time;
	sprintf(mysql_time, "%04u-%02u-%02u %02u:%02u:%02u", time->year, time->month, time->day, time->hour, time->minute, time->second);
	return mysql_time;
}


#endif // HAVE_MYSQL

#ifdef HAVE_SQLITE3

int
ailsa_setup_ro_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
{
	int retval;
	if ((retval = sqlite3_open_v2(file, cmdb, SQLITE_OPEN_READONLY, NULL)) > 0) {
		ailsa_syslog(LOG_ERR, "Cannot open SQL file %s", file);
		return FILE_O_FAIL;
	}
	if ((retval = sqlite3_prepare_v2(*cmdb, query, BUFF_S, stmt, NULL)) > 0) {
		ailsa_syslog(LOG_ERR, "Cannot prepare statement for sqlite: %s", sqlite3_errstr(retval));
		retval = sqlite3_close(*cmdb);
		return SQLITE_STATEMENT_FAILED;
	}
	return retval;
}

int
ailsa_setup_rw_sqlite(const char *query, size_t len, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
{
	int retval, sqlret = 0;
	if ((retval = sqlite3_open_v2(file, cmdb, SQLITE_OPEN_READWRITE, NULL)) > 0) {
		ailsa_syslog(LOG_ERR, "Cannot open SQL file %s", file);
		return FILE_O_FAIL;
	}
	if ((retval = sqlite3_db_config(*cmdb, SQLITE_DBCONFIG_ENABLE_FKEY, 1, &sqlret)) != SQLITE_OK)
		ailsa_syslog(LOG_ERR, "Cannot enable foreign key support in sqlite");
	if (sqlret == 0)
		ailsa_syslog(LOG_ERR, "Did not enable foreign key support");
	if ((retval = sqlite3_prepare_v2(*cmdb, query, (int)len, stmt, NULL)) > 0) {
		ailsa_syslog(LOG_ERR, "Cannot prepare statement for sqlite: %s", sqlite3_errstr(retval));
		retval = sqlite3_close(*cmdb);
		return SQLITE_STATEMENT_FAILED;
	}
	return retval;
}

void
ailsa_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt)
{
	sqlite3_finalize(stmt);
	sqlite3_close(cmdb);
}

#endif /*HAVE_SQLITE3*/

