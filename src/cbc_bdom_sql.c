/* 
 *
 *  cbc: Create Build Config
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
 *  cbc_bdom_sql.c:
 *
 *  Contains functions for database access for build domain (cbcdomain).
 */

#include "../config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "cbc_bdom_sql.h"

#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */

#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

const char *cbcdom_sql_update[] = { "\
UPDATE build_domain SET ntp_server = ? WHERE domain = ?","\
UPDATE build_domain SET ntp_server = ? WHERE bd_id = ?"
};
const char *cbcdom_sql_delete[] = { "\
DELETE FROM build_domain WHERE domain = ?","\
DELETE FROM build_domain WHERE bd_id = ?"
};
const char *cbcdom_sql_search[] = { "\
SELECT config_ldap, ldap_ssl, ldap_server, ldap_dn, ldap_bind FROM\
build_domain WHERE domain = ?","\
SELECT config_ldap, ldap_ssl, ldap_server, ldap_dn, ldap_bind FROM\
build_domain WHERE bd_id = ?"
};

const unsigned int cbcdom_update_args[] = {
	2, 2
};
const unsigned int cbcdom_delete_args[] = {
	1, 1
};
const unsigned int cbcdom_search_args[] = {
	1, 1
};
const unsigned int cbcdom_search_fields[] = {
	5, 5
};

const unsigned int cbcdom_update_types[][2] = {
	{ DBTEXT, DBTEXT } ,
	{ DBTEXT, DBINT }
};
const unsigned int cbcdom_delete_types[][2] = {
	{ DBTEXT, NONE } ,
	{ DBINT, NONE }
};
const unsigned int cbcdom_search_arg_types[][2] = {
	{ DBTEXT, NONE } ,
	{ DBINT, NONE }
};
const unsigned int cbcdom_search_field_types[][5] = {
	{ DBSHORT, DBSHORT, DBTEXT, DBTEXT, DBTEXT } ,
	{ DBSHORT, DBSHORT, DBTEXT, DBTEXT, DBTEXT }
};

int
cbcdom_run_delete(cbc_config_s *ccs, dbdata_s *base, int type)
{
	int retval;
	if ((strncmp(ccs->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(ccs->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cbcdom_run_delete_mysql(ccs, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(ccs->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cbcdom_run_delete_sqlite(ccs, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", ccs->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

# ifdef HAVE_MYSQL
#  include <mysql.h>

int
cbcdom_run_delete_mysql(cbc_config_s *ccs, dbdata_s *data, int type)
{
	MYSQL cbc;
	MYSQL_STMT *cbc_stmt;
	MYSQL_BIND mybind[cbcdom_delete_args[type]];
	const char *query = cbcdom_sql_delete[type];
	int retval = 0;
	unsigned int i;
	dbdata_s *list = data;

	memset(mybind, 0, sizeof(mybind));
	for (i = 0; i < cbcdom_delete_args[type]; i++) {
		if (cbcdom_delete_types[type][i] == DBINT) {
			mybind[i].buffer_type = MYSQL_TYPE_LONG;
			mybind[i].is_null = 0;
			mybind[i].length = 0;
			mybind[i].is_unsigned = 1;
			mybind[i].buffer = &(list->args.number);
			mybind[i].buffer_length = sizeof(unsigned long int);
			list = list->next;
		} else if (cbcdom_delete_types[type][i] == DBTEXT) {
			mybind[i].buffer_type = MYSQL_TYPE_STRING;
			mybind[i].is_null = 0;
			mybind[i].length = 0;
			mybind[i].is_unsigned = 0;
			mybind[i].buffer = &(list->args.text);
			mybind[i].buffer_length = strlen(list->args.text);
			list = list->next;
		} else if (cbcdom_delete_types[type][i] == DBSHORT) {
			mybind[i].buffer_type = MYSQL_TYPE_SHORT;
			mybind[i].is_null = 0;
			mybind[i].length = 0;
			mybind[i].is_unsigned = 0;
			mybind[i].buffer = &(list->args.small);
			mybind[i].buffer_length = sizeof(short int);
			list = list->next;
		}
	}
	cbc_mysql_init(ccs, &cbc);
	if (!(cbc_stmt = mysql_stmt_init(&cbc)))
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_prepare(cbc_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_bind_param(cbc_stmt, &mybind[0])) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_execute(cbc_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	retval = (int)mysql_stmt_affected_rows(cbc_stmt);
	mysql_stmt_close(cbc_stmt);
	cmdb_mysql_cleanup(&cbc);
	return retval;
}

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
cbcdom_run_delete_sqlite(cbc_config_s *ccs, dbdata_s *data, int type)
{
	const char *query = cbcdom_sql_delete[type], *file = ccs->file;
	int retval = 0;
	unsigned int i;
	dbdata_s *list = data;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	if ((retval = sqlite3_open_v2(file, &cbc, SQLITE_OPEN_READWRITE, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(cbc, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cbc);
		report_error(SQLITE_STATEMENT_FAILED, "cbcdom_run_delete_sqlite");
	}
	for (i = 1; i <= cbcdom_delete_args[type]; i++) {
		if (!list)
			break;
		if (cbcdom_delete_types[type][i - 1] == DBINT) {
			if ((sqlite3_bind_int64(state, (int)i, list->args.number)) > 0) {
				fprintf(stderr, "Cannot bind arg %ud\n", i);
				return retval;
			}
		} else if (cbcdom_delete_types[type][i - 1] == DBTEXT) {
			if ((sqlite3_bind_text(state, (int)i, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
				fprintf(stderr, "Cannot bind arg %ud\n", i);
				return retval;
			}
		}
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cbc));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(cbc);
		return NONE;
	}
	retval = sqlite3_changes(cbc);
	sqlite3_finalize(state);
	sqlite3_close(cbc);
	return retval;
}

# endif /* HAVE_SQLITE3 */
