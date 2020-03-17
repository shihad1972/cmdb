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
#include <ailsacmdb.h>
#include <ailsasql.h>
#include <cmdb.h>

#ifdef HAVE_MYSQL
# include <mysql.h>
static int
ailsa_simple_select_mysql(ailsa_cmdb_s *config, AILSS *query, AILLIST *results);
#endif /* HAVE_MYSQL */

#ifdef HAVE_SQLITE3
# include <sqlite3.h>
static int
ailsa_simple_select_sqlite(ailsa_cmdb_s *config, AILSS *query, AILLIST *results);
#endif /* HAVE_SQLITE3 */

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

int
ailsa_init_ss(AILSS *data)
{
	int retval = 0;
	AILLIST *fields = NULL, *args = NULL;
	AILSS *tmp = NULL;

	if (!(data))
		return AILSA_NO_DATA;
	else
		tmp = data;
	fields = ailsa_calloc(sizeof(AILLIST), "fields in ailsa_init_ss");
	ailsa_list_init(fields, ailsa_clean_dbv);
	args = ailsa_calloc(sizeof(AILLIST), "args in ailsa_init_ss");
	ailsa_list_init(args, ailsa_clean_dbv);
	tmp->fields = fields;
	tmp->args = args;
	return retval;
}

void
ailsa_clean_ss_data(void *data)
{
	if (!(data))
		return;
	my_free(data);
}

void
ailsa_clean_ss(AILSS *data)
{
	AILSS *a;

	if (!(data))
		return;
	a = data;
	ailsa_list_destroy(a->fields);
	ailsa_list_destroy(a->args);
	if (a->fields)
		my_free(a->fields);
	if (a->args)
		my_free(a->args);
	if (a->query)
		my_free(a->query);
	my_free(a);
}

void
ailsa_clean_dbv(void *dbv)
{
	AILDBV *tmp = dbv;

	if (!(tmp))
		return;
	if (tmp->name)
		my_free(tmp->name);
	my_free(tmp);
}

int
ailsa_simple_select(ailsa_cmdb_s *config, AILSS *query, AILLIST *results)
{
/* Will run an SQL statement of the like:
	SELECT something, something_else FROM table WHERE column = identifier;
*/
	int retval = 0;

	if (!(config) || !(query) || !(results))
		return AILSA_NO_DATA;
	if (strncmp(config->dbtype, "none", CONFIG_LEN) == 0) {
		retval = AILSA_NO_DBTYPE;
		goto cleanup;
#ifdef HAVE_MYSQL
	} else if (strncmp(config->dbtype, "mysql", CONFIG_LEN) == 0) {
		retval = ailsa_simple_select_mysql(config, query, results);
		goto cleanup;
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	} else if (strncmp(config->dbtype, "sqlite", CONFIG_LEN) == 0) {
		retval = ailsa_simple_select_sqlite(config, query, results);
		goto cleanup;
#endif // HAVE_SQLITE3
	} else {
		retval = AILSA_INVALID_DBTYPE;
		goto cleanup;
	}

	cleanup:
		return retval;
}

#ifdef HAVE_MYSQL

static int
ailsa_simple_select_mysql(ailsa_cmdb_s *config, AILSS *query, AILLIST *results)
{
	int retval = 0;

	if (!(config) || !(query) || !(results))
		return AILSA_NO_DATA;
	return retval;
}


#endif // HAVE_MYSQL


#ifdef HAVE_SQLITE3

static int
ailsa_simple_select_sqlite(ailsa_cmdb_s *config, AILSS *query, AILLIST *results)
{
	int retval = 0;

	if (!(config) || !(query) || !(results))
		return AILSA_NO_DATA;
	return retval;
}

#endif // HAVE_SQLITE3
