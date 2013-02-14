/* 
 *
 *  dnsa: DNS administration
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
 *  dnsa_base_sql.c:
 *
 *  Contains functions which will fill up data structs based on the parameters
 *  supplied. Will also contian conditional code base on database type.
 */
#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "dnsa_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_MYSQL
# include <mysql.h>
# include "mysqlfunc.h"
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

const char *sql_select[] = { "\
SELECT id, name, pri_dns, sec_dns, serial, refresh, retry, expire, ttl, \
valid, owner, updated FROM zones ORDER BY name","\
SELECT rev_zone_id, net_range, prefix, net_start, net_finish, start_ip, \
finish_ip, pri_dns, sec_dns, serial, refresh, retry, expire, ttl, valid, \
owner, updated FROM rev_zones ORDER BY start_ip","\
SELECT id, zone, host, type, pri, destination, valid FROM records ORDER \
BY zone, type, host","\
SELECT rev_record_id, rev_zone, host, destination, valid FROM rev_records"
};

const char *sql_search[] = { "\
SELECT id FROM zones WHERE name = ?","\
SELECT rev_zone_id FROM rev_zones WHERE net_range = ?"
};

const unsigned int select_fields[] = { 12, 17, 7, 5 };

const unsigned int search_fields[] = { 1, 1 };

const unsigned int search_args[] = { 1, 1 };

const unsigned int search_field_type[][1] = { /* What we are selecting */
	{ DBINT } ,
	{ DBINT }
};

const unsigned int search_arg_type[][1] = { /* What we are searching on */
	{ DBTEXT } ,
	{ DBTEXT }
};

int
run_query(dnsa_config_t *config, dnsa_t *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return NONE;
}

int
run_multiple_query(dnsa_config_t *config, dnsa_t *base, int type)
{
	int retval;
	retval = NONE;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_multiple_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_multiple_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return retval;
}

int
get_query(int type, const char **query, unsigned int *fields)
{
	int retval;
	
	retval = NONE;
	switch(type) {
		case ZONE:
			*query = sql_select[ZONES];
			*fields = select_fields[ZONES];
			break;
		case REV_ZONE:
			*query = sql_select[REV_ZONES];
			*fields = select_fields[REV_ZONES];
			break;
		case RECORD:
			*query = sql_select[RECORDS];
			*fields = select_fields[RECORDS];
			break;
		case REV_RECORD:
			*query = sql_select[REV_RECORDS];
			*fields = select_fields[REV_RECORDS];
			break;
		default:
			fprintf(stderr, "Unknown query type %d\n", type);
			retval = 1;
			break;
	}
	
	return retval;
}

int
run_search(dnsa_config_t *config, dbdata_t *base, int type)
{
	int retval;

	if ((strncmp(config->dbtype, "none", RANGE_S) ==0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_search_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_search_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

#ifdef HAVE_MYSQL
void
cmdb_mysql_init(dnsa_config_t *dc, MYSQL *dnsa_mysql)
{
	const char *unix_socket;
	
	unix_socket = dc->socket;
	
	if (!(mysql_init(dnsa_mysql))) {
		report_error(MY_INIT_FAIL, mysql_error(dnsa_mysql));
	}
	if (!(mysql_real_connect(dnsa_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(dnsa_mysql));
	
}

int
run_query_mysql(dnsa_config_t *config, dnsa_t *base, int type)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	const char *query;
	int retval;
	unsigned int fields;
	
	retval = 0;
	cmdb_mysql_init(config, &dnsa);
	if ((retval = get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = cmdb_mysql_query_with_checks(&dnsa, query)) != 0) {
		fprintf(stderr, "Query failed with error code %d\n", retval);
		return retval;
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_cleanup(&dnsa);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	fields = mysql_num_fields(dnsa_res);
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_cleanup_full(&dnsa, dnsa_res);
		report_error(NO_SERVERS, "run_query_mysql");
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res)))
		store_result_mysql(dnsa_row, base, type, fields);
	cmdb_mysql_cleanup_full(&dnsa, dnsa_res);
	return 0;
}

int
run_multiple_query_mysql(dnsa_config_t *config, dnsa_t *base, int type)
{
	int retval;
	retval = NONE;
	if ((type & ZONE) == ZONE)
		if ((retval = run_query_mysql(config, base, ZONE)) != 0)
			return retval;
	if ((type & REV_ZONE) == REV_ZONE)
		if ((retval = run_query_mysql(config, base, REV_ZONE)) != 0)
			return retval;
	if ((type & RECORD) == RECORD)
		if ((retval = run_query_mysql(config, base, RECORD)) != 0)
			return retval;
	if ((type & REV_RECORD) == REV_RECORD)
		if ((retval = run_query_mysql(config, base, REV_RECORD)) != 0)
			return retval;
	return retval;
}

void
store_result_mysql(MYSQL_ROW row, dnsa_t *base, int type, unsigned int fields)
{
	switch(type) {
		case ZONE:
			if (fields != select_fields[ZONES])
				break;
			store_zone_mysql(row, base);
			break;
		case REV_ZONE:
			if (fields != select_fields[REV_ZONES])
				break;
			store_rev_zone_mysql(row, base);
			break;
		case RECORD:
			if (fields != select_fields[RECORDS])
				break;
			store_record_mysql(row, base);
			break;
		case REV_RECORD:
			if (fields != select_fields[REV_RECORDS])
				break;
			store_rev_record_mysql(row, base);
			break;
		default:
			fprintf(stderr, "Unknown type %d\n",  type);
			break;
	}
			
}

void
store_zone_mysql(MYSQL_ROW row, dnsa_t *base)
{
	int retval;
	zone_info_t *zone, *list;
	
	if (!(zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "zone in store_zone_mysql");
	init_zone_struct(zone);
	zone->id = strtoul(row[0], NULL, 10);
	snprintf(zone->name, RBUFF_S, "%s", row[1]);
	snprintf(zone->pri_dns, RBUFF_S - 1, "%s", row[2]);
	if ((retval = add_trailing_dot(zone->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	snprintf(zone->sec_dns, RBUFF_S - 1, "%s", row[3]);
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(zone->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	zone->serial = strtoul(row[4], NULL, 10);
	zone->refresh = strtoul(row[5], NULL, 10);
	zone->retry = strtoul(row[6], NULL, 10);
	zone->expire = strtoul(row[7], NULL, 10);
	zone->ttl = strtoul(row[8], NULL, 10);
	snprintf(zone->valid, RANGE_S, "%s", row[9]);
	zone->owner = strtoul(row[10], NULL, 10);
	snprintf(zone->updated, RANGE_S, "%s", row[11]);
	list = base->zones;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = zone;
	} else {
		base->zones = zone;
	}
}

void
store_record_mysql(MYSQL_ROW row, dnsa_t *base)
{
	record_row_t *rec, *list;
	
	if (!(rec = malloc(sizeof(record_row_t))))
		report_error(MALLOC_FAIL, "rec in store_record_mysql");
	init_record_struct(rec);
	rec->id = strtoul(row[0], NULL, 10);
	rec->zone = strtoul(row[1], NULL, 10);
	snprintf(rec->host, RBUFF_S, "%s", row[2]);
	snprintf(rec->type, RANGE_S, "%s", row[3]);
	rec->pri = strtoul(row[4], NULL, 10);
	snprintf(rec->dest, RBUFF_S, "%s", row[5]);
	snprintf(rec->valid, RANGE_S, "%s", row[6]);
	list = base->records;
	if (list) {
		while(list->next)
			list = list->next;
		list->next = rec;
	} else {
		base->records = rec;
	}
}

void
store_rev_zone_mysql(MYSQL_ROW row, dnsa_t *base)
{
	int retval;
	rev_zone_info_t *rev, *list;
	
	if (!(rev = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rev in store_rev_zone_mysql");
	init_rev_zone_struct(rev);
	rev->rev_zone_id = strtoul(row[0], NULL, 10);
	snprintf(rev->net_range, RANGE_S, "%s", row[1]);
	rev->prefix = strtoul(row[2], NULL, 10);
	snprintf(rev->net_start, RANGE_S, "%s", row[3]);
	snprintf(rev->net_finish, RANGE_S, "%s", row[4]);
	rev->start_ip = strtoul(row[5], NULL, 10);
	rev->end_ip = strtoul(row[6], NULL, 10);
	snprintf(rev->pri_dns, RBUFF_S - 1, "%s", row[7]);
	if ((retval = add_trailing_dot(rev->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	snprintf(rev->sec_dns, RBUFF_S - 1, "%s", row[8]);
	if (strncmp(rev->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(rev->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	rev->serial = strtoul(row[9], NULL, 10);
	rev->refresh = strtoul(row[10], NULL, 10);
	rev->retry = strtoul(row[11], NULL, 10);
	rev->expire = strtoul(row[12], NULL, 10);
	rev->ttl = strtoul(row[13], NULL, 10);
	snprintf(rev->valid, RANGE_S, "%s", row[14]);
	rev->owner = strtoul(row[15], NULL, 10);
	snprintf(rev->updated, RANGE_S, "%s", row[16]);
	list = base->rev_zones;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = rev;
	} else {
		base->rev_zones = rev;
	}
}

void
store_rev_record_mysql(MYSQL_ROW row, dnsa_t *base)
{
	rev_record_row_t *rev, *list;
	
	if (!(rev = malloc(sizeof(rev_record_row_t))))
		report_error(MALLOC_FAIL, "rev in store_rev_record_mysql");
	init_rev_record_struct(rev);
	rev->record_id = strtoul(row[0], NULL, 10);
	rev->rev_zone = strtoul(row[1], NULL, 10);
	snprintf(rev->host, RBUFF_S, "%s", row[2]);
	snprintf(rev->dest, RBUFF_S, "%s", row[3]);
	snprintf(rev->valid, RANGE_S, "%s", row[4]);
	list = base->rev_records;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = rev;
	} else {
		base->rev_records = rev;
	}
}

int
run_search_mysql(dnsa_config_t *config, dbdata_t *data, int type)
{
	return 0;
}

#endif /* HAVE_MYSQL */

#ifdef HAVE_SQLITE3

int
run_query_sqlite(dnsa_config_t *config, dnsa_t *base, int type)
{
	const char *query, *file;
	int retval;
	unsigned int fields;
	sqlite3 *dnsa;
	sqlite3_stmt *state;
	
	retval = 0;
	file = config->file;
	if ((retval = get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "run_query_sqlite");
	}
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		store_result_sqlite(state, base, type, fields);
	
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(dnsa);
	
	return 0;
}

int
run_multiple_query_sqlite(dnsa_config_t *config, dnsa_t *base, int type)
{
	int retval;
	retval = NONE;
	if ((type & ZONE) == ZONE)
		if ((retval = run_query_sqlite(config, base, ZONE)) != 0)
			return retval;
	if ((type & REV_ZONE) == REV_ZONE)
		if ((retval = run_query_sqlite(config, base, REV_ZONE)) != 0)
			return retval;
	if ((type & RECORD) == RECORD)
		if ((retval = run_query_sqlite(config, base, RECORD)) != 0)
			return retval;
	if ((type & REV_RECORD) == REV_RECORD)
		if ((retval = run_query_sqlite(config, base, REV_RECORD)) != 0)
			return retval;
	return retval;
}

void
store_result_sqlite(sqlite3_stmt *state, dnsa_t *base, int type, unsigned int fields)
{
	switch(type) {
		case ZONE:
			if (fields != select_fields[ZONES])
				break;
			store_zone_sqlite(state, base);
			break;
		case REV_ZONE:
			if (fields != select_fields[REV_ZONES])
				break;
			store_rev_zone_sqlite(state, base);
			break;
		case RECORD:
			if (fields != select_fields[RECORDS])
				break;
			store_record_sqlite(state, base);
			break;
		case REV_RECORD:
			if (fields != select_fields[REV_RECORDS])
				break;
			store_rev_record_sqlite(state, base);
			break;
		default:
			fprintf(stderr, "Unknown type %d\n",  type);
			break;
	}
			
}

void
store_zone_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
	int retval;
	zone_info_t *zone, *list;
	
	if (!(zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "zone in store_zone_sqlite");
	init_zone_struct(zone);
	zone->id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(zone->name, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(zone->pri_dns, RBUFF_S -1, "%s", sqlite3_column_text(state, 2));
	if ((retval = add_trailing_dot(zone->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	snprintf(zone->sec_dns, RBUFF_S - 1, "%s", sqlite3_column_text(state, 3));
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(zone->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	zone->serial = (unsigned long int) sqlite3_column_int(state, 4);
	zone->refresh = (unsigned long int) sqlite3_column_int(state, 5);
	zone->retry = (unsigned long int) sqlite3_column_int(state, 6);
	zone->expire = (unsigned long int) sqlite3_column_int(state, 7);
	zone->ttl = (unsigned long int) sqlite3_column_int(state, 8);
	snprintf(zone->valid, RANGE_S, "%s", sqlite3_column_text(state, 9));
	zone->owner = (unsigned long int) sqlite3_column_int(state, 10);
	snprintf(zone->updated, RANGE_S, "%s", sqlite3_column_text(state, 11));
	list = base->zones;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = zone;
	} else {
		base->zones = zone;
	}
}

void
store_rev_zone_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
	int retval;
	rev_zone_info_t *rev, *list;
	
	if (!(rev = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rev in store_rev_zone_sqlite");
	init_rev_zone_struct(rev);
	rev->rev_zone_id = (unsigned long int) sqlite3_column_int(state, 0);
	snprintf(rev->net_range, RANGE_S, "%s", sqlite3_column_text(state, 1));
	rev->prefix = (unsigned long int) sqlite3_column_int(state, 2);
	snprintf(rev->net_start, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(rev->net_finish, RANGE_S, "%s", sqlite3_column_text(state, 4));
	rev->start_ip = (unsigned long int) sqlite3_column_int(state, 5);
	rev->end_ip = (unsigned long int) sqlite3_column_int(state, 6);
	snprintf(rev->pri_dns, RBUFF_S -1, "%s", sqlite3_column_text(state, 7));
	if ((retval = add_trailing_dot(rev->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	snprintf(rev->sec_dns, RBUFF_S -1, "%s", sqlite3_column_text(state, 8));
	if (strncmp(rev->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(rev->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	rev->serial = (unsigned long int) sqlite3_column_int(state, 9);
	rev->refresh = (unsigned long int) sqlite3_column_int(state, 10);
	rev->retry = (unsigned long int) sqlite3_column_int(state, 11);
	rev->expire = (unsigned long int) sqlite3_column_int(state, 12);
	rev->ttl = (unsigned long int) sqlite3_column_int(state, 13);
	snprintf(rev->valid, RANGE_S, "%s", sqlite3_column_text(state, 14));
	rev->owner = (unsigned long int) sqlite3_column_int(state, 15);
	snprintf(rev->updated, RANGE_S, "%s", sqlite3_column_text(state, 16));
	list = base->rev_zones;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = rev;
	} else {
		base->rev_zones = rev;
	}
}

void
store_record_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
	record_row_t *rec, *list;
	if (!(rec = malloc(sizeof(record_row_t))))
		report_error(MALLOC_FAIL, "rec in store_record_sqlite");
	init_record_struct(rec);
	rec->id = (unsigned long int) sqlite3_column_int(state, 0);
	rec->zone = (unsigned long int) sqlite3_column_int(state, 1);
	snprintf(rec->host, RBUFF_S, "%s", sqlite3_column_text(state, 2));
	snprintf(rec->type, RANGE_S, "%s", sqlite3_column_text(state, 3));
	rec->pri = (unsigned long int) sqlite3_column_int(state, 4);
	snprintf(rec->dest, RBUFF_S, "%s", sqlite3_column_text(state, 5));
	snprintf(rec->valid, RANGE_S, "%s", sqlite3_column_text(state, 6));
	list = base->records;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = rec;
	} else {
		base->records = rec;
	}
}

void
store_rev_record_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
	rev_record_row_t *rev, *list;

	if (!(rev = malloc(sizeof(rev_record_row_t))))
		report_error(MALLOC_FAIL, "rev in store_rev_record_sqlite");
	init_rev_record_struct(rev);
	rev->record_id = (unsigned long int) sqlite3_column_int(state, 0);
	rev->rev_zone = (unsigned long int) sqlite3_column_int(state, 1);
	snprintf(rev->host, RBUFF_S, "%s", sqlite3_column_text(state, 2));
	snprintf(rev->dest, RBUFF_S, "%s", sqlite3_column_text(state, 3));
	snprintf(rev->valid, RANGE_S, "%s", sqlite3_column_text(state, 4));
	list = base->rev_records;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = rev;
	} else {
		base->rev_records = rev;
	}
}

int
run_search_sqlite(dnsa_config_t *config, dbdata_t *data, int type)
{
	return 0;
}

#endif /* HAVE_SQLITE3 */