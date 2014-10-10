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
 *  base_mysql.c
 *
 *
 *  Base SQL functions for dnsa, cbc and cmdb
 *
 *
 */
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /*HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /*HAVE_SQLITE3 */
#include "cmdb.h"
#include "base_sql.h"


#ifdef HAVE_MYSQL

void
cmdb_mysql_query(MYSQL *mycmdb, const char *query)
{
	int error;
	
	error = mysql_query(mycmdb, query);
	if ((error != 0)) {
		mysql_close(mycmdb);
		mysql_library_end();
		report_error(MY_QUERY_FAIL, mysql_error(mycmdb));
	}
}

int
cmdb_mysql_query_with_checks(MYSQL *mycmdb, const char *query)
{
	int error;
	
	error = mysql_query(mycmdb, query);
	return error;
}

void
cmdb_mysql_clean(MYSQL *cmdb_mysql, char *query)
{
	cmdb_mysql_cleanup(cmdb_mysql);
	free(query);
}

void
cmdb_mysql_clean_full(MYSQL_RES *cmdb_res, MYSQL *cmdb_mysql, char *query)
{
	cmdb_mysql_cleanup_full(cmdb_mysql, cmdb_res);
	free(query);
}

void
cmdb_mysql_cleanup(MYSQL *cmdb)
{
	mysql_close(cmdb);
	mysql_library_end();
}

void
cmdb_mysql_cleanup_full(MYSQL *cmdb, MYSQL_RES *res)
{
	mysql_free_result(res);
	mysql_close(cmdb);
	mysql_library_end();
}

#endif /*HAVE_MYSQL*/
#ifdef HAVE_SQLITE3

void
cmdb_setup_ro_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
{
	int retval;
	if ((retval = sqlite3_open_v2(file, cmdb, SQLITE_OPEN_READONLY, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(*cmdb, query, BUFF_S, stmt, NULL)) > 0) {
		retval = sqlite3_close(*cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "error in cmdb_run_search_sqlite");
	}
}

void
cmdb_setup_rw_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
{
	int retval;
	if ((retval = sqlite3_open_v2(file, cmdb, SQLITE_OPEN_READWRITE, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(*cmdb, query, BUFF_S, stmt, NULL)) > 0) {
		retval = sqlite3_close(*cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "error in cmdb_run_search_sqlite");
	}
}

void
cmdb_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt)
{
	sqlite3_finalize(stmt);
	sqlite3_close(cmdb);
}

#endif /*HAVE_SQLITE3*/

