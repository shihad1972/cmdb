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

static char *
build_sql_query(unsigned int prog, unsigned int no)
{
	char *query;
	unsigned int qno, i, cno;
	size_t tot, pos, size;
	for (i = 0, qno = 0; i < prog; i++)
		qno += select_queries[i];
	qno += no;
	for (i = 0, cno = 0; i < qno; i++)
		cno += select_fields[i];
	fprintf(stderr, "For query no %u, we start at %u\n", qno, cno);
	tot = BUFF_S;
	pos = 0;
	query = cmdb_malloc(tot, "query in build_sql_query");
	pos = strlen("SELECT ");
	snprintf(query, RANGE_S, "SELECT ");
	for (i = 0; i < select_fields[qno]; i++) {
		size = strlen(sql_columns[cno + i]);
		if (tot <= (pos + size + 1)) {
			tot *= 2;
			query = realloc(query, tot);
		}
		snprintf(query + pos, HOST_S, "%s, ", sql_columns[cno + i]);
		pos += size + 2;
	}
	pos -=2;
	size = strlen(" FROM ");
	if (tot <= (pos + size + 1)) {
		tot *= 2;
		query = realloc(query, tot);
	}
	snprintf(query + pos, size + 1, " FROM ");
	pos += size;
	size = strlen(sql_table_list[qno]);
	if (tot <= (pos + size + 1)) {
		tot *= 2;
		query = realloc(query, tot);
	}
	snprintf(query + pos, size + 1, "%s", sql_table_list[qno]);
	return query;
}


int
main(int argc, char *argv[])
{
	int retval = 0;
	unsigned int prog, i;
	char *query;

	if (argc < 2) {
		printf("Doing cbc\n");
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
	printf("SQL Queries for program %s\n\n", argv[1]);
	for (i = 0; i < select_queries[prog]; i++) {
		query = build_sql_query(prog, i);
		printf("%s\n", query);
		free(query);
	}
	return retval;
}

