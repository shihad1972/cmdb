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
# include "mysqlfunc.h"
#endif /*HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /*HAVE_SQLITE3 */
#include "cmdb.h"
#include "cmdb_cbc.h"

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
	if ((error != 0))
		return error;
	else
		return 0;
}

void
cmdb_mysql_clean(MYSQL *cmdb_mysql, char *query)
{
	mysql_close(cmdb_mysql);
	mysql_library_end();
	free(query);
}

void
cmdb_mysql_clean_full(MYSQL_RES *cmdb_res, MYSQL *cmdb_mysql, char *query)
{
	mysql_free_result(cmdb_res);
	mysql_close(cmdb_mysql);
	mysql_library_end();
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



#endif /*HAVE_SQLITE3*/

