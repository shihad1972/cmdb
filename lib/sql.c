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
#include "cmdb.h"
#include "base_sql.h"

#ifdef HAVE_MYSQL

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
ailsa_run_mysql_stmt(MYSQL *cmdb, MYSQL_BIND *my_bind, const char *query)
{
	int retval = NONE;
	MYSQL_STMT *stmt;

	if (!(stmt = mysql_stmt_init(cmdb)))
		report_error(MY_STATEMENT_FAIL, mysql_error(cmdb));
	if ((retval = mysql_stmt_prepare(stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(stmt));
	if ((retval = mysql_stmt_bind_param(stmt, my_bind)) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(stmt));
	if ((retval = mysql_stmt_execute(stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(stmt));
	retval = (int)mysql_stmt_affected_rows(stmt);
	mysql_stmt_close(stmt);
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

#endif // HAVE_MYSQL

#ifdef HAVE_SQLITE3

void
ailsa_setup_ro_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
{
	int retval;
	if ((retval = sqlite3_open_v2(file, cmdb, SQLITE_OPEN_READONLY, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(*cmdb, query, BUFF_S, stmt, NULL)) > 0) {
		printf("%s\n", sqlite3_errstr(retval));
		retval = sqlite3_close(*cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "error in cmdb_setup_ro_sqlite");
	}
}

void
ailsa_setup_rw_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
{
	int retval, sqlret = 0;
	if ((retval = sqlite3_open_v2(file, cmdb, SQLITE_OPEN_READWRITE, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_db_config(*cmdb, SQLITE_DBCONFIG_ENABLE_FKEY, 1, &sqlret)) != SQLITE_OK)
		fprintf(stderr, "Cannot enable foreign key support\n");
	if (sqlret == 0)
		fprintf(stderr, "Did not enable foreign key support\n");
	if ((retval = sqlite3_prepare_v2(*cmdb, query, BUFF_S, stmt, NULL)) > 0) {
		printf("%s\n", sqlite3_errstr(retval));
		retval = sqlite3_close(*cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "error in cmdb_setup_rw_sqlite");
	}
}

void
ailsa_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt)
{
	sqlite3_finalize(stmt);
	sqlite3_close(cmdb);
}

#endif /*HAVE_SQLITE3*/

