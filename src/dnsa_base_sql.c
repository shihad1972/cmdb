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
SELECT id, zone, host, type, pri, destination, valid FROM records","\
SELECT rev_record_id, rev_zone, host, destination, valid FROM rev_records"
};

const unsigned int select_fields[] = { 12, 17, 7, 5 };


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
}

void
store_record_mysql(MYSQL_ROW row, dnsa_t *base)
{
}

void
store_rev_zone_mysql(MYSQL_ROW row, dnsa_t *base)
{
}

void
store_rev_record_mysql(MYSQL_ROW row, dnsa_t *base)
{
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
}

void
store_rev_zone_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
}

void
store_record_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
}

void
store_rev_record_sqlite(sqlite3_stmt *state, dnsa_t *base)
{
}


#endif /* HAVE_SQLITE3 */