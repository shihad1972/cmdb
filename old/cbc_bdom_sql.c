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

const char *cbc_sql_update[] = { "\
UPDATE build_domain SET ntp_server = ? WHERE domain = ?","\
UPDATE build_domain SET ntp_server = ? WHERE bd_id = ?"
};
const char *cbc_sql_delete[] = { "\
DELETE FROM build_domain WHERE domain = ?","\
DELETE FROM build_domain WHERE bd_id = ?"
};
const char *cbc_sql_search[] = { "\
SELECT config_ldap, ldap_ssl, ldap_server, ldap_dn, ldap_bind FROM\
build_domain WHERE domain = ?","\
SELECT config_ldap, ldap_ssl, ldap_server, ldap_dn, ldap_bind FROM\
build_domain WHERE bd_id = ?","\
SELECT COUNT(*) c FROM build_domain WHERE domain = ?"
};

const unsigned int cbc_update_args[] = {
	2, 2
};
const unsigned int cbc_delete_args[] = {
	1, 1
};
const unsigned int cbc_search_args[] = {
	1, 1, 1
};
const unsigned int cbc_search_fields[] = {
	5, 5, 1
};

const unsigned int cbc_update_types[][2] = {
	{ DBTEXT, DBTEXT } ,
	{ DBTEXT, DBINT }
};
const unsigned int cbc_delete_types[][2] = {
	{ DBTEXT, NONE } ,
	{ DBINT, NONE }
};
const unsigned int cbc_search_arg_types[][2] = {
	{ DBTEXT, NONE } ,
	{ DBINT, NONE } ,
	{ DBTEXT, NONE }
};
const unsigned int cbc_search_field_types[][5] = {
	{ DBSHORT, DBSHORT, DBTEXT, DBTEXT, DBTEXT } ,
	{ DBSHORT, DBSHORT, DBTEXT, DBTEXT, DBTEXT } ,
	{ DBINT, NONE, NONE, NONE, NONE }
};

int
cbc_run_delete(cbc_config_s *ccs, dbdata_s *base, int type)
{
	int retval;
	if ((strncmp(ccs->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(ccs->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cbc_run_delete_mysql(ccs, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(ccs->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cbc_run_delete_sqlite(ccs, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", ccs->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
cbc_run_search(cbc_config_s *ccs, dbdata_s *base, int type)
{
	int retval;
	if ((strncmp(ccs->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(ccs->dbtype, "mysql", RANGE_S) == 0)) {
		retval = cbc_run_search_mysql(ccs, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(ccs->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = cbc_run_search_sqlite(ccs, base, type);
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
cbc_run_delete_mysql(cbc_config_s *ccs, dbdata_s *data, int type)
{
	MYSQL cbc;
	MYSQL_STMT *cbc_stmt;
	MYSQL_BIND mybind[cbc_delete_args[type]];
	const char *query = cbc_sql_delete[type];
	int retval = 0;
	unsigned int i;
	dbdata_s *list = data;

	memset(mybind, 0, sizeof(mybind));
	for (i = 0; i < cbc_delete_args[type]; i++) {
		if (cbc_delete_types[type][i] == DBINT) {
			mybind[i].buffer_type = MYSQL_TYPE_LONG;
			mybind[i].is_null = 0;
			mybind[i].length = 0;
			mybind[i].is_unsigned = 1;
			mybind[i].buffer = &(list->args.number);
			mybind[i].buffer_length = sizeof(unsigned long int);
			list = list->next;
		} else if (cbc_delete_types[type][i] == DBTEXT) {
			mybind[i].buffer_type = MYSQL_TYPE_STRING;
			mybind[i].is_null = 0;
			mybind[i].length = 0;
			mybind[i].is_unsigned = 0;
			mybind[i].buffer = &(list->args.text);
			mybind[i].buffer_length = strlen(list->args.text);
			list = list->next;
		} else if (cbc_delete_types[type][i] == DBSHORT) {
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

int
cbc_run_search_mysql(cbc_config_s *ccs, dbdata_s *data, int type)
{
	MYSQL cbc;
	MYSQL_STMT *cbc_stmt;
	MYSQL_BIND args[cbc_search_args[type]];
	MYSQL_BIND fields[cbc_search_fields[type]];
	const char *query = cbc_sql_search[type];
	int retval = 0, j = 0;
	unsigned int i;

	memset(args, 0, sizeof(args));
	memset(fields, 0, sizeof(fields));
	for (i = 0; i < cbc_search_args[type]; i++)
		if ((retval = set_dom_search_args_mysql(&args[i], i, type, data)) != 0)
			return retval;
	for (i = 0; i < cbc_search_fields[type]; i++)
		if ((retval = set_dom_search_fields_mysql(&fields[i], i, j, type, data)) != 0)
			return retval;
	cbc_mysql_init(ccs, &cbc);
	if (!(cbc_stmt = mysql_stmt_init(&cbc)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(cbc_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_bind_param(cbc_stmt, &args[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_bind_result(cbc_stmt, &fields[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_execute(cbc_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_store_result(cbc_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	while ((retval = mysql_stmt_fetch(cbc_stmt)) == 0) {
		j++;
		for (i = 0; i < cbc_search_fields[type]; i++)
			if ((retval = set_dom_search_fields_mysql(&fields[i], i, j, type, data)) != 0)
				return retval;
		if ((retval = mysql_stmt_bind_result(cbc_stmt, &fields[0])) != 0)
			report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	}
	if (retval != MYSQL_NO_DATA)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	else
		retval = NONE;
	mysql_stmt_free_result(cbc_stmt);
	mysql_stmt_close(cbc_stmt);
	cmdb_mysql_cleanup(&cbc);
	return j;
}

int
set_dom_search_args_mysql(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base)
{
	int retval = 0;
	unsigned int j;
	void *buffer;
	dbdata_s *list = base;

	mybind->is_null = 0;
	mybind->length = 0;
	for (j = 0; j < i; j++)
		list = list->next;
	if (cbc_search_arg_types[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		buffer = &(list->args.number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (cbc_search_arg_types[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		buffer = &(list->args.text);
		mybind->buffer_length = strlen(buffer);
	} else if (cbc_search_arg_types[type][i] == DBSHORT) {
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
set_dom_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base)
{
	int retval = 0, j;
	static int m = 0;
	void *buffer;
	dbdata_s *list, *new;
	list = base;

	mybind->is_null = 0;
	mybind->length = 0;
	if (k > 0) {
		if (!(new = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "new in set_dom_search_fields_mysql");
		init_dbdata_struct(new);
		while (list->next) {
			list = list->next;
		}
		list->next = new;
		list = base;
	}
	for (j = 0; j < m; j++)
		list = list->next;
	if (cbc_search_field_types[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		buffer = &(list->fields.number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (cbc_search_field_types[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		buffer = &(list->fields.text);
		mybind->buffer_length = RBUFF_S;
	} else if (cbc_search_field_types[type][i] == DBSHORT) {
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

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
cbc_run_delete_sqlite(cbc_config_s *ccs, dbdata_s *data, int type)
{
	const char *query = cbc_sql_delete[type], *file = ccs->file;
	int retval = 0;
	unsigned int i;
	dbdata_s *list = data;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	if ((retval = sqlite3_open_v2(file, &cbc, SQLITE_OPEN_READWRITE, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(cbc, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cbc);
		report_error(SQLITE_STATEMENT_FAILED, "cbc_run_delete_sqlite");
	}
	for (i = 1; i <= cbc_delete_args[type]; i++) {
		if (!list)
			break;
		if (cbc_delete_types[type][i - 1] == DBINT) {
			if ((sqlite3_bind_int64(state, (int)i, list->args.number)) > 0) {
				fprintf(stderr, "Cannot bind arg %ud\n", i);
				return retval;
			}
		} else if (cbc_delete_types[type][i - 1] == DBTEXT) {
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

int
cbc_run_search_sqlite(cbc_config_s *ccs, dbdata_s *data, int type)
{
	const char *query = cbc_sql_search[type], *file = ccs->file;
	int retval = 0, i;
	dbdata_s *list = data;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	if ((retval = sqlite3_open_v2(file, &cbc, SQLITE_OPEN_READONLY, NULL)) > 0)
		report_error(FILE_O_FAIL, file);
	if ((retval = sqlite3_prepare_v2(cbc, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cbc);
		report_error(SQLITE_STATEMENT_FAILED, "cbc_run_search_sqlite");
	}
	for (i = 0; (unsigned long)i < cbc_search_args[type]; i++) {
		set_cbc_search_sqlite(state, list, type, i);
		list = list->next;
	}
	list = data;
	i = 0;
	while ((sqlite3_step(state)) == SQLITE_ROW) {
		get_cbc_search_res_sqlite(state, list, type, i);
		i++;
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cbc);
	return i;
}

int
set_cbc_search_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = 0;

	if (cbc_search_arg_types[type][i] == DBTEXT) {
		if ((retval = sqlite3_bind_text(
state, i + 1, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
			fprintf(stderr, "Cannot bind search text arg %s\n",
				list->args.text);
			return retval;
		}
	} else if (cbc_search_arg_types[type][i] == DBINT) {
		if ((retval = sqlite3_bind_int64(
state, i + 1, (sqlite3_int64)list->args.number)) > 0) {
			fprintf(stderr, "Cannot bind search number arg %lu\n",
				list->args.number);
			return retval;
		}
	} else if (cbc_search_arg_types[type][i] == DBSHORT) {
		if ((retval = sqlite3_bind_int(state, i + 1, list->args.small)) > 0) {
			fprintf(stderr, "Cannot bind search small arg %d\n",
				list->args.small);
			return retval;
		}
	} else {
		return WRONG_TYPE;
	}
	return retval;
}

int
get_cbc_search_res_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = 0, j, k;
	unsigned int u;
	dbdata_s *data;

	if (i > 0) {
		for (k = 1; k <= i; k++) {
			for (u = 1; u <= cbc_search_fields[type]; u++)
				if ((u != cbc_search_fields[type]) || ( k != i))
					list = list->next;
		}
		for (j = 0; (unsigned)j < cbc_search_fields[type]; j++) {
			if (!(data = malloc(sizeof(dbdata_s))))
				report_error(MALLOC_FAIL, "dbdata_s in get_cbc_search_res_sqlite");
			init_dbdata_struct(data);
			if (cbc_search_field_types[type][j] == DBTEXT)
				snprintf(data->fields.text, RBUFF_S, "%s", sqlite3_column_text(state, j));
			else if (cbc_search_field_types[type][j] == DBINT)
				data->fields.number = (unsigned long int)sqlite3_column_int64(state, j);
			else if (cbc_search_field_types[type][j] == DBSHORT)
				data->fields.small = (short int)sqlite3_column_int(state, j);
			else
				return WRONG_TYPE;
			list->next = data;
			list = list->next;
		}
	} else {
		for (j = 0; (unsigned)j < cbc_search_fields[type]; j++) {
			if (cbc_search_field_types[type][j] == DBTEXT)
				snprintf(list->fields.text, RBUFF_S, "%s", sqlite3_column_text(state, j));
			else if (cbc_search_field_types[type][j] == DBINT)
				list->fields.number = (unsigned long int)sqlite3_column_int64(state, j);
			else if (cbc_search_field_types[type][j] == DBSHORT)
				list->fields.small = (short int)sqlite3_column_int(state, j);
			else
				return WRONG_TYPE;
			list = list->next;
		}
	}
	return retval;
}

# endif /* HAVE_SQLITE3 */
