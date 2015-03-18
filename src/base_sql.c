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


void
cmdb_prep_db_query(dbdata_s **data, const unsigned int *values[], int query)
{
	unsigned int max = 0;
	max = cmdb_get_max(values[0][query], values[1][query]);
	init_multi_dbdata_struct(data, max);
}

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

int
cmdb_run_mysql_stmt(MYSQL *cmdb, MYSQL_BIND *my_bind, const char *query)
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
cmdb_set_bind_mysql(MYSQL_BIND *mybind, unsigned int i, dbdata_u *data)
{
	size_t len;
	dbdata_u *list;

	if (!(data))
		report_error(CBC_NO_DATA, "data in cmdb_set_bind_mysql");
	list = data;
	if (i == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		mybind->buffer = &(list->number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (i == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		mybind->buffer = list->text;
		if ((len = strlen(list->text)) > 0)
			mybind->buffer_length = len;
		else
			mybind->buffer_length = RBUFF_S;
	} else if (i == DBSHORT) {
		mybind->buffer_type = MYSQL_TYPE_SHORT;
		mybind->is_unsigned = 0;
		mybind->buffer = &(list->small);
		mybind->buffer_length = sizeof(short int);
	} else {
		report_error(DB_TYPE_INVALID, "in cmdb_set_bind_mysql");
	}
	mybind->is_null = 0;
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
		printf("%s\n", sqlite3_errstr(retval));
		retval = sqlite3_close(*cmdb);
		report_error(SQLITE_STATEMENT_FAILED, "error in cmdb_setup_ro_sqlite");
	}
}

void
cmdb_setup_rw_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt)
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
cmdb_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt)
{
	sqlite3_finalize(stmt);
	sqlite3_close(cmdb);
}

#endif /*HAVE_SQLITE3*/

