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
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

int
ailsa_init_ss(AILSS *data)
{
	int retval = 0;
	AILLIST *fields = NULL;
	AILSS *tmp = NULL;

	if (!(data))
		return AILSA_NO_DATA;
	else
		tmp = data;
	fields = ailsa_calloc(sizeof(AILLIST), "fields in ailsa_init_ss");
	ailsa_list_init(fields, ailsa_clean_dbv);
	tmp->fields = fields;
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
	if (a->fields)
		my_free(a->fields);
	if (a->table)
		my_free(a->table);
	if (a->arg)
		my_free(a->arg);
	if (a->value)
		my_free(a->value);
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

char *
ailsa_build_simple_sql_query(AILSS *query)
{
	ailsa_string_s *str = NULL;
	char buf[MAC_LEN];
	char *qstr = NULL;
	char *tmp = NULL;
	size_t len = 0;
	AILELEM *element = NULL;
	AILDBV *d = NULL;

	if (!(query))
		goto cleanup;
	element = query->fields->head;
	str = ailsa_calloc(sizeof(ailsa_string_s), "str in ailsa_build_simple_sql_query");
	ailsa_init_string(str);
	ailsa_fill_string(str, "SELECT ");
	while (element) {
		d = (AILDBV *)element->data;
		tmp = d->name;
		snprintf(buf, MAC_LEN, "%s ", tmp);
		ailsa_fill_string(str, buf);
		memset(buf, 0, MAC_LEN);
		element = element->next;
	}
	ailsa_fill_string(str, "FROM ");
	snprintf(buf, MAC_LEN, "%s ", query->table);
	ailsa_fill_string(str, buf);
	memset(buf, 0, MAC_LEN);
	ailsa_fill_string(str, "WHERE ");
	snprintf(buf, MAC_LEN, "%s ", query->arg);
	ailsa_fill_string(str, buf);
	memset(buf, 0, MAC_LEN);
	snprintf(buf, MAC_LEN, "= %s", query->value);
	ailsa_fill_string(str, buf);
	len = strlen(str->string);
	qstr = strndup(str->string, len + 1);
#ifdef DEBUG
	fprintf(stderr, "%s\n", qstr);
#endif // DEBUG
	cleanup:
		if (str)
			ailsa_clean_string(str);
		return qstr;
}

int
ailsa_simple_select(ailsa_cmdb_s *config, char *query, void *results)
{
/* Will run an SQL statement of the like:
	SELECT something, something_else FROM table WHERE column = identifier;
*/
	int retval = 0;
	if (!(config) || !(query) || !(results))
		return AILSA_NO_DATA;

	return retval;
}

