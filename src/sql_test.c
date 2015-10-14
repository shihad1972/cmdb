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

static unsigned int
get_column(unsigned int table, const unsigned int col)
{
	unsigned int retval, i, sanity;
	sanity = 0;
	for (i = 0; i < NOPROGS; i++)
		sanity += sql_tables[i];
	if (table > sanity)
		return 0;
	for (i = 0, retval = 0; i < table; i++)
		retval +=table_columns[i];
	retval += col;
	return retval;
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

/*
 * Get various indeces for table, column and search
 *
 */

static void
get_search_table_index(unsigned int sno, unsigned int *tab)
{
	unsigned int index, tmp, i;

	for (i = 0, index = 0, tmp = 0; i < NOPROGS; i++) {
		if ((index + sql_searches[i]) <= sno) {
			index += sql_searches[i];
			tmp += sql_tables[i];
		} else {
			break;
		}
	}
	*tab = tmp;
}

static void
get_search_column_index(unsigned int tab, unsigned int *col)
{
	unsigned int index, i;

	for (i = 0, index = 0; i < tab; i++)
		index += table_columns[i];
	*col = index;
}

static void
get_search_query_index(unsigned int sno, unsigned int *index)
{
	unsigned int ind, i;

	for (i = 0, ind = 0; i < NOPROGS; i++) {
		if ((ind + sql_searches[i]) < sno)
			ind += sql_searches[i];
		else
			break;
	}
	*index = ind;
}

static void
get_search_prog(unsigned int sno, unsigned int *prog)
{
	unsigned int pg, i;

	for (i = 0, pg = 0; i < NOPROGS; i++) {
		if ((pg + sql_searches[i]) < sno)
			pg += sql_searches[i];
		else
			break;
	}
	*prog = i;
}

static int
get_join_table_column(int tabno, unsigned int jno, unsigned int sno, unsigned int *tab, unsigned int *col)
{
	int retval = 0;
	unsigned int tindex, i, cindex;

	get_search_table_index(sno, &tindex);
	if (tabno == 0) {
		*tab = search_join_columns[sno][jno][0] + tindex;
		for (i = 0, cindex = 0; i < *tab; i++)
			cindex += table_columns[i];
		*col = search_join_columns[sno][jno][1] + cindex;
	} else if (tabno == 1) {
		*tab = search_join_columns[sno][jno][2] + tindex;
		for (i = 0, cindex = 0; i < *tab; i++)
			cindex += table_columns[i];
		*col = search_join_columns[sno][jno][3] + cindex;
	} else {
		retval = -1;
	}
	return retval;
}

/*
 * Main helper functions for build_sql_search
 * Each one returns a specific part of the search query
 */

static char *
get_sql_modifier(unsigned int sno)
{
	char *mod = NULL;
	unsigned int index, i, mods, query;
	size_t tot = MAC_S;

	for (i = 0, index = 0; index < sno; i++)
		index += sql_searches[i];
	query = sno - index - sql_searches[i];
	for (mods = 0; mods < SQL_MODS; mods++) {
		if (query == sql_modifiers[mods][1] && i == sql_modifiers[mods][0]) {
			mod = cmdb_malloc(tot, "mod in get_sql_modifier");
			if (sql_modifiers[mods][2] == DISTINCT)
				snprintf(mod, tot, "DISTINCT ");
		}
	}
	return mod;
}

static char *
get_first_search_column_table(unsigned int sno)
{
	char *search;
	unsigned int col, tab, prog, ser, ind, i;
	size_t len, pos;

	ind = col = tab = 0;
	search = cmdb_malloc(HOST_S, "search in get_first_search_column");
	get_search_table_index(sno, &ind);
	tab = ind + search_field_columns[sno][0][0];
	get_search_column_index(tab, &col);
	col += search_field_columns[sno][0][1];
	if (search_table_count[sno] > 1)
		snprintf(search, HOST_S, "%s.%s FROM %s %s ",
		 sql_table_alias[tab], sql_columns[col], sql_table_list[tab], sql_table_alias[tab]);
	else
		snprintf(search, HOST_S, "%s FROM %s ", sql_columns[col], sql_table_list[tab]);
	pos = strlen(search);
	get_search_prog(sno, &prog);
	get_search_query_index(sno, &ind);
	ser = sno - ind;
	for (i = 0; i < SQL_MODS; i++) {
		if ((sql_modifiers[i][0] == prog) && (sql_modifiers[i][1] == ser)) {
			if (sql_modifiers[i][2] == COUNT) {
				pos = strlen(search);
				len = strlen("COUNT (*) C ");
				snprintf(search + pos, len + 1, "COUNT (*) C ");
			}
		}
	}
	return search;
}

static char *
get_search_joins(unsigned int sno)
{
	char *join, *buff;
	int retval;
	unsigned int tab, col, i, jns;
	size_t len, tot, jtot, pos, jpos;

	jns = search_table_count[sno] - 1;
	tot = jtot = NAME_S;
	join = cmdb_malloc(tot, "join in get_search_joins");
	pos = jpos = 0;
	for (i = 0; i < jns; i++) {
		buff = cmdb_malloc(tot, "buff in get_search_joins");
		snprintf(buff, RANGE_S, "LEFT JOIN ");
		pos = strlen(buff);
		if ((retval = get_join_table_column(1, i, sno, &tab, &col)) < 0)
			goto cleanup;
		len = strlen(sql_table_list[tab]) + strlen(sql_table_alias[tab]) + 3;
		if (!(buff = check_for_resize(buff, &tot, len + pos)))
			report_error(MALLOC_FAIL, "buff in get_search_joins");
		snprintf(buff + pos, len, "%s %s ", sql_table_list[tab], sql_table_alias[tab]);
		pos += len - 1;
		if ((retval = get_join_table_column(0, i, sno, &tab, &col)) < 0)
			goto cleanup;
		len = strlen(sql_table_alias[tab]) + strlen(sql_columns[col]) + 6;
		if (!(buff = check_for_resize(buff, &tot, len + pos)))
			report_error(MALLOC_FAIL, "buff in get_search_joins");
		snprintf(buff + pos, len, "ON %s.%s=", sql_table_alias[tab], sql_columns[col]);
		pos += len - 1;
		if ((retval = get_join_table_column(1, i, sno, &tab, &col)) < 0)
			goto cleanup;
		len = strlen(sql_table_alias[tab]) + strlen(sql_columns[col]) + 3;
		if (!(buff = check_for_resize(buff, &tot, len + pos)))
			report_error(MALLOC_FAIL, "buff in get_search_joins");
		snprintf(buff + pos, len, "%s.%s ", sql_table_alias[tab], sql_columns[col]);
		len = strlen(buff);
		if (!(join = check_for_resize(join, &jtot, len + pos)))
			report_error(MALLOC_FAIL, "join in get_search_joins");
		snprintf(join + jpos, len + 1, "%s", buff);
		pos += len;
		free(buff);
	}
	return join;
	cleanup:
		free(buff);
		free(join);
		join = NULL;
		return join;
}

static char *
get_search_args(unsigned int sno)
{
	char *args, *buff;
	const char *table, *column;
	unsigned int jns, i, tindex, cindex, tab, col;
	size_t len, tot, atot, pos, apos;

	jns = search_table_count[sno] - 1;
	tot = atot = NAME_S;
	args = cmdb_malloc(atot, "args in get_search_args");
	pos = 0;
	get_search_table_index(sno, &tindex);
	snprintf(args, COMM_S, "WHERE ");
	apos = strlen(args);
	for (i = 0; i < search_args[sno]; i++) {
		buff = cmdb_malloc(tot, "buff in get_search_args");
		tab = tindex + search_arg_columns[sno][i][0];
		get_search_column_index(tab, &cindex);
		col = cindex + search_arg_columns[sno][i][1];
		table = sql_table_alias[tab];
		column = sql_columns[col];
		if (jns > 0) {
			len = strlen(table) + strlen(column) + 11;
			if (!(buff = check_for_resize(buff, &tot, len + pos)))
				report_error(MALLOC_FAIL, "buff in get_search_args");
			snprintf(buff, len, "%s.%s = ? AND ", table, column);
			pos += len - 1;
		} else {
			len = strlen(column) + 10;
			if (!(buff = check_for_resize(buff, &tot, len + pos)))
				report_error(MALLOC_FAIL, "buff in get_search_args");
			snprintf(buff, len, "%s = ? AND ", column);
			pos += len - 1;
		}
		len = strlen(buff) + 1;
		if (!(args = check_for_resize(args, &atot, len + apos )))
			report_error(MALLOC_FAIL, "args in get_search_args");
		snprintf(args + apos, len, "%s", buff);
		apos += len - 1;
		free(buff);
	}
	len = strlen(args) - 5;
	args[len] = '\0';
	return args;
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
	const char *table, *column, *sf, *sv;
	unsigned int uno, fields, statics, tab, col, id, i;
	size_t tot, pos, len;

	for (i = 0, uno = 0; i < prog; i++)
		uno += sql_updates[i];
	uno += no;
	fields = update_fields[uno];
	statics = static_update_fields[uno];
	tot = HOST_S;
	update = cmdb_malloc(tot, "update in build_sql_update");
	pos = strlen("UPDATE ");
	snprintf(update, pos + 1, "UPDATE ");
	tab = update_tables[uno];
	table = sql_table_list[tab];
	len = strlen(table) + 1;
	if (!(update = check_for_resize(update, &tot, pos + len + 1)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 1, "%s ", table);
	pos += len;
	len = strlen("SET ");
	if (!(update = check_for_resize(update, &tot, pos + len + 1)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 1, "SET ");
	pos += len;
	for (i = 0; i < fields; i++) {
		col = get_column(tab, update_field_columns[uno][i]);
		column = sql_columns[col];
		len = strlen(column);
		if (!(update = check_for_resize(update, &tot, pos + len + 7)))
			report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
		snprintf(update + pos, len + 7, "%s = ?, ", column);
		pos += len + 6;
	}
	for (i = 0; i < statics; i++) {
		for (id = 0; id <= sql_static; id++)
			if (uno == sql_statics[id])
				break;
		col = get_column(tab, static_update_columns[id][i]);
		sf = sql_columns[col];
		sv = static_update_values[id][i];
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
	col = get_column(tab, update_arg_column[uno]);
	column = sql_columns[col];
	len = strlen(column) + 4;
	if (!(update = check_for_resize(update, &tot, pos + len + 1)))
		report_error(MALLOC_FAIL, "reallocating update in build_sql_update");
	snprintf(update + pos, len + 1, "%s = ?", column);
	return update;
}

static char *
build_sql_delete(unsigned int prog, unsigned int no)
{
	char *delete;
	const char *table, *tid;
	unsigned int dno, i, id;
	size_t len;

	for (i = 0, dno = 0; i < prog; i++)
		dno += sql_updates[i];
	dno += no;
	table = sql_table_list[dno];
	for (i = 0, id = 0; i < dno; i++)
		id += table_columns[i];
	tid = sql_columns[id];
	len = strlen(table) + strlen(tid);
	len += strlen("DELETE FROM  WHERE  = ?");
	delete = cmdb_malloc(len + 1, "delete in build_sql_delete");
	snprintf(delete, len + 1, "DELETE FROM %s WHERE %s = ?", table, tid);
	return delete;
}
/*
static char *
build_sql_search(unsigned int prog, unsigned int no)
{
	char *search, *buff;
	unsigned int sno, i;
	size_t tot, pos, len;

	for (i = 0, sno = 0; i < prog; i++)
		sno += sql_searches[i];
	sno += no;
	tot = HOST_S;
	search = cmdb_malloc(tot, "search in build_sql_search");
	snprintf(search, COMM_S, "SELECT ");
	pos = strlen(search);
	if ((buff = get_sql_modifier(sno))) {
		len = strlen(buff);
		snprintf(search + pos, len + 1, "%s", buff);
		pos += len;
		free(buff);
	}
	if (!(buff = get_first_search_column_table(sno)))
		report_error(MALLOC_FAIL, "get_first_search_column_table");
	len = strlen(buff);
	snprintf(search + pos, len + 1, "%s", buff);
	pos += len;
	free(buff);
	if (!(buff = get_search_joins(sno)))
		goto cleanup;
	len = strlen(buff);
	if (!(search = check_for_resize(search, &tot, pos + len)))
		report_error(MALLOC_FAIL, "search in build_sql_search");
	snprintf(search + pos, len + 1, "%s", buff);
	free(buff);
	pos += len;
	if (!(buff = get_search_args(sno)))
		goto cleanup;
	len = strlen(buff);
	if (!(search = check_for_resize(search, &tot, pos + len)))
		report_error(MALLOC_FAIL, "search in build_sql_search");
	snprintf(search + pos, len + 1, "%s", buff);
	free(buff);
	return search;
	cleanup:
		free(search);
		search = NULL;
		return search;
} */

int
main(int argc, char *argv[])
{
	int retval = 0;
	unsigned int prog, i;
	char *query;

	if (argc < 2) {
		prog = CMDB;
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
	printf("\nSQL Deletes\n\n");
	for (i = 0; i < sql_tables[prog]; i++) {
		query = build_sql_delete(prog, i);
		printf("%s\n", query);
		free(query);
	}
/*	printf("\nSQL Searches\n\n");
	for (i = 0; i < sql_searches[prog]; i++) {
		query = build_sql_search(prog, i);
		printf("%s\n", query);
		free(query);
	} */
	return retval;
}

