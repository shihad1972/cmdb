/*
 *  sqlt: sql tester (C) 2015 Iain M Conochie
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
 * sql_test.c: main file for the sqlt program
 *
 */
#include "../config.h"
#include <errno.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "sql.h"

static size_t
get_size_from_len(size_t len)
{
/* Nicked from stack overflow:
 * http://stackoverflow.com/questions/53161/find-the-highest-order-bit-in-c
 */
	size_t size = 1;

	if (!len)
		return len;
	while (len >>= 1)
		size <<= 1;
	return size * 2;
}

static char *
build_sql_query(unsigned int prog, unsigned int no)
{
	char *query;
	unsigned int qno, cno, i;
	size_t tot, pos, size;
	for (i = 0, qno = 0; i < prog; i++)
		qno += sql_tables[i];
	qno += no;
	for (i = 0, cno = 0; i < qno; i++)
		cno += table_columns[i];
//	fprintf(stderr, "For query no %u, we start at %u\n", qno, cno);
	tot = HOST_S;
	pos = 0;
	query = cmdb_malloc(tot, "query in build_sql_query");
	pos = strlen("SELECT ");
	snprintf(query, RANGE_S, "SELECT ");
	for (i = 0; i < table_columns[qno]; i++) {
		size = strlen(sql_columns[cno + i]);
		if (!(query = check_for_resize(query, &tot, pos + size + 3)))
			report_error(MALLOC_FAIL, "query in build_sql_query");
		snprintf(query + pos, size + 3, "%s, ", sql_columns[cno + i]);
		pos += size + 2;
	}
	pos -= 2;
	size = strlen(" FROM ");
	if (!(query = check_for_resize(query, &tot, pos + size + 3)))
		report_error(MALLOC_FAIL, "query in build_sql_query");
	snprintf(query + pos, size + 3, " FROM ");
	pos += size;
	size = strlen(sql_table_list[qno]);
	if (!(query = check_for_resize(query, &tot, pos + size + 3)))
		report_error(MALLOC_FAIL, "query in build_sql_query");
	snprintf(query + pos, size + 3, "%s", sql_table_list[qno]);
	return query;
}

static char *
add_sql_insert_values(char *insert, unsigned int no, unsigned int count)
{
	size_t size, pos;
	unsigned int i, j;

	pos = strlen(insert);
	size = get_size_from_len(pos);
	if ((size == 0) || (size > THIS_SQL_MAX)) {	// Neither should ever happen...
		if (insert)
			free (insert);
		return NULL;
	}
	for (i = 0; i < count; i++) {
		for (j = 0; j < no; j++) {
			if (j == 0) {
				if (!(insert = check_for_resize(insert, &size, pos + 2)))
					report_error(MALLOC_FAIL, "insert in add_sql_insert_values");
				snprintf(insert + pos, 2, "(");
				pos += 1;
			}
			if (!(insert = check_for_resize(insert, &size, pos + 4)))
				report_error(MALLOC_FAIL, "insert in add_sql_insert_values");
			snprintf(insert + pos, 4, "?, ");
			pos += 3;
		}
		pos -= 2;
		if (!(insert = check_for_resize(insert, &size, pos + 4)))
				report_error(MALLOC_FAIL, "insert in add_sql_insert_values");
		snprintf(insert + pos, 4, "), ");
		pos += 3;
	}
	pos -= 2;
	if (!(insert = check_for_resize(insert, &size, pos + 2)))
			report_error(MALLOC_FAIL, "insert in add_sql_insert_values");
	insert[pos] = '\0';
	return insert;
}

static char *
build_sql_insert(unsigned int prog, unsigned int no, unsigned int count)
{
	char *insert;
	unsigned int ino, cno, fno, i;
	size_t tot, pos, len;
	for (i = 0, ino = 0; i < prog; i++)
		ino += sql_tables[i];
	ino += no;
	for (i = 0, cno = 0; i < ino; i++)
		cno += table_columns[i];
//	fprintf(stderr, "For insert no %u, we start at %u\n", ino, cno);
	tot = HOST_S;
	pos = 0;
	insert = cmdb_malloc(tot, "insert in build_sql_insert");
	pos = strlen("INSERT INTO ");
	snprintf(insert, pos + 1, "INSERT INTO ");
	len = strlen(sql_table_list[ino]);
	if (!(insert = check_for_resize(insert, &tot, pos + len + 3)))
		report_error(MALLOC_FAIL, "insert in build_sql_insert");
	snprintf(insert + pos, len + 3, "%s (", sql_table_list[ino]);
	pos += len + 2;
	for (i = 1;  i <= short_inserts[0]; i++)
		if (ino == short_inserts[i])
			break;
	if (i > short_inserts[0])
		fno = table_columns[ino] - 3;
	else
		fno = table_columns[ino] - 1;
	for (i = 1; i <= fno; i++) {
		len = strlen(sql_columns[cno + i]);
		if (!(insert = check_for_resize(insert, &tot, pos + len + 3)))
			report_error(MALLOC_FAIL, "insert in build_sql_insert");
		snprintf(insert + pos, len + 3, "%s, ", sql_columns[cno + i]);
		pos += len + 2;
	}
	pos -= 2;
	len = strlen(") VALUES ");
	if (!(insert = check_for_resize(insert, &tot, pos + len + 1)))
		report_error(MALLOC_FAIL, "insert in build_sql_insert");
	snprintf(insert + pos, len + 1, ") VALUES ");
	if (!(insert = add_sql_insert_values(insert, fno, count))) {
		return NULL;
	}
	return insert;
}

static char *
build_sql_update(unsigned int prog, unsigned int no)
{
	char *update;
	const char *table, *sf, *sv, *col_id;
	unsigned int uno, fields, statics, i, id;
	size_t tot, pos, len;

	for (i = 0, uno = 0; i < prog; i++)
		uno += sql_updates[i];
	uno += no;
	fields = update_fields[uno];
	statics = static_update_fields[uno];
	tot = HOST_S;
	pos = 0;
	update = cmdb_malloc(tot, "update in build_sql_update");
	pos = strlen("UPDATE ");
	snprintf(update, pos + 1, "UPDATE ");
	table = update_tables[uno];
	len = strlen(table);
	if (!(update = check_for_resize(update, &tot, pos + len + 2)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 2, "%s ", table);
	pos += len + 1;
	len = strlen("SET ");
	if (!(update = check_for_resize(update, &tot, pos + len + 1)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 1, "SET ");
	pos += len;
	for (i = 0; i < fields; i++) {
// check this exists first
		len = strlen(update_field_columns[uno][i]);
		if (!(update = check_for_resize(update, &tot, pos + len + 7)))
			report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
		snprintf(update + pos, len + 7, "%s = ?, ", update_field_columns[uno][i]);
		pos += len + 6;
	}
	for (i = 0; i < statics; i++) {
// check this exists first
		sf = static_update_columns[uno][i];
		sv = static_update_values[uno][i];
		len = strlen(sv) + strlen(sf);
		if (!(update = check_for_resize(update, &tot, pos + len + 8)))
			report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
		snprintf(update + pos, len + 8, "%s = '%s', ", sf, sv);
		pos += len + 7;
	}
	pos -= 2;
	len = strlen(" WHERE ");
	if (!(update = check_for_resize(update, &tot, pos + len + 1)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 1, " WHERE ");
	pos += len;
	i = 0;
	while (strncmp(table, sql_table_list[i], HOST_S) != 0)
		i++;
	for (fields = 0, id = 0; fields < i; fields++)
		id += table_columns[fields];
	col_id = sql_columns[id];
	len = strlen(col_id);
	if (!(update = check_for_resize(update, &tot, pos + len + 5)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 5, "%s = ?", col_id);
	return update;
}

int
main(int argc, char *argv[])
{
	int retval = 0;
	unsigned int prog, i;
	char *query;

	if (argc < 2) {
		prog = CBC;
	} else if (argc > 2) {
		fprintf(stderr, "Usage: %s <prog>)\n", argv[0]);
		exit(1);
	} else {
		if (strncmp(argv[1], "cbc", COMM_S) == 0) {
			prog = CBC;
		} else if (strncmp(argv[1], "dnsa", COMM_S) == 0) {
			prog = DNSA;
		} else if (strncmp(argv[1], "cmdb", COMM_S) == 0) {
			prog = CMDB;
		} else {
			fprintf(stderr, "Unknown prog %s\n", argv[1]);
			exit(1);
		}
	}
	if (argc > 1)
		printf("SQL Queries for program %s\n\n", argv[1]);
	else
		printf("SQL Queries for program cbc\n\n");
	for (i = 0; i < sql_tables[prog]; i++) {
		query = build_sql_query(prog, i);
		printf("%s\n", query);
		free(query);
	}
	printf("\nSQL Inserts\n\n");
	for (i = 0; i < sql_tables[prog]; i++) {
		if (!(query = build_sql_insert(prog, i, 3))) {
// Does not happen. Need to add checks into the called function
			printf("Overflow detected! Quitting\n");
			exit(1);
		}
		printf("%s\n", query);
		free(query);
	}
	printf("\nSQL Updates\n\n");
	for (i = 0; i < sql_updates[prog]; i++) {
		query = build_sql_update(prog, i);
		printf("%s\n", query);
		free(query);
	}
	return retval;
}

