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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "base_sql.h"
#include "dnsa_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

/**
 * These SQL searches require the dnsa_s struct. Each search will fill one of
 * the structs pointed to within dnsa_s.
 * The stucts within dnsa will be malloc'ed by the database store function so
 * only dnsa_s needs to be malloc'ed and initialised.
 * These searches return multiple members.
 * Helper functions need to be created for earch search to populate the member
 * of dnsa_s used.
 */
const char *dnsa_sql_select[] = { "\
SELECT id, name, pri_dns, sec_dns, serial, refresh, retry, expire, ttl, \
valid, owner, updated, type, master FROM zones ORDER BY name","\
SELECT rev_zone_id, net_range, prefix, net_start, net_finish, start_ip, \
finish_ip, pri_dns, sec_dns, serial, refresh, retry, expire, ttl, valid, \
owner, updated, type, master FROM rev_zones ORDER BY start_ip","\
SELECT id, zone, host, type, pri, destination, valid FROM records ORDER \
BY zone, type, host","\
SELECT rev_record_id, rev_zone, host, destination, valid FROM rev_records","\
SELECT name, host, destination, r.id, zone FROM records r, zones z WHERE z.id = r.zone AND r.type = 'A' ORDER BY destination","\
SELECT destination, COUNT(*) c FROM records WHERE type = 'A' GROUP BY destination HAVING c > 1","\
SELECT prefa_id, ip, ip_addr, record_id, fqdn FROM preferred_a","\
SELECT id, zone, pri, destination FROM records WHERE TYPE = 'CNAME'","\
SELECT id, name, zone_id, pri_dns, sec_dns, pri_ns, sec_ns FROM glue_zones"
};
/**
 * These SQL searches require the struct within dnsa_s to be initialised, as
 * the arguments for the query are contained within them. 
 * These searches will only return 1 member, regardless of the number of
 * search results. This can lead to bugs if the database is not in order e.g.
 * if there are multiple zones with the same id.
 */
const char *dnsa_sql_search[] = { "\
SELECT id FROM zones WHERE name = ?","\
SELECT rev_zone_id FROM rev_zones WHERE net_range = ?","\
SELECT prefix FROM rev_zones WHERE net_range = ?"
};
/**
 * These SQL searches return an unknown number of members, with a variable
 * number and type of fields and arguments in the query. The data is sent
 * and returned in a list of dbdata_s structs. The inital list must be created
 * to hold at least the greater of dnsa_extended_search_fields OR 
 * dnsa_extended_search_args. The helper function init_initial_dbdata will do this.
 * To ascertain the argument, fields and types thereof, we use the following 
 * arrays and matrices:
 *   dnsa_extended_search_fields[]
 *   dnsa_extended_search_args[]
 *   dnsa_ext_search_field_type[][]
 *   dnsa_ext_search_arg_type[][]
 */
const char *dnsa_sql_extended_search[] = { "\
SELECT r.host, z.name, r.id FROM records r, zones z WHERE r.destination = ? AND r.zone = z.id","\
SELECT id, host, type, pri, destination FROM records WHERE zone = ?","\
SELECT DISTINCT destination from records WHERE destination > ? AND destination < ?" /*,"\
SELECT fqdn FROM preferred_a WHERE ip = ?" */
};

const char *dnsa_sql_insert[] = {"\
INSERT INTO zones (name, pri_dns, sec_dns, serial, refresh, retry, expire, \
ttl, type, master) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO rev_zones (net_range, prefix, net_start, net_finish, start_ip, \
finish_ip, pri_dns, sec_dns, serial, refresh, retry, expire, ttl, type, master) \
VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO records (zone, host, type, pri, destination) VALUES \
(?, ?, ?, ?, ?)","\
INSERT INTO rev_records (rev_zone, host, destination) VALUES (?, ?, ?)","\
INSERT","\
INSERT","\
INSERT INTO preferred_a (ip, ip_addr, record_id, fqdn) VALUES (?, ?, ?, ?)","\
INSERT","\
INSERT INTO glue_zones(name, zone_id, pri_dns, sec_dns, pri_ns, sec_ns) VALUES (?, ?, ?, ?, ?, ?)"
};

const char *dnsa_sql_update[] = {"\
UPDATE zones SET valid = 'yes', updated = 'no' WHERE id = ?","\
UPDATE zones SET updated = 'yes' WHERE id = ?","\
UPDATE zones SET updated = 'no' WHERE id = ?","\
UPDATE zones SET serial = ? WHERE id = ?","\
UPDATE rev_zones SET valid = 'yes', updated = 'no' WHERE rev_zone_id = ?","\
UPDATE rev_zones SET serial = ? WHERE rev_zone_id = ?","\
UPDATE zones SET valid = 'no' WHERE id = ?"
};

const char *dnsa_sql_delete[] = {"\
DELETE FROM zones WHERE id = ?","\
DELETE FROM rev_zones WHERE rev_zone_id = ?","\
DELETE FROM records WHERE id = ?","\
DELETE FROM rev_records WHERE rev_record_id = ?","\
DELETE","\
DELETE","\
DELETE FROM preferred_a WHERE prefa_id = ?","\
DELETE FROM rev_records WHERE rev_zone = ?","\
DELETE FROM records WHERE zone = ?"
};

#ifdef HAVE_MYSQL

const int mysql_inserts[][15] = {
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, 
    MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
    MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0} ,
{MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 
    MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
    MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
    MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING} , 
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG,
    MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , 
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0} ,
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
{MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0} ,
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
{MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
    MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

#endif /* HAVE_MYSQL */

const unsigned int dnsa_select_fields[] = { 14, 19, 7, 5, 5, 2, 5, 4, 7 };

const unsigned int dnsa_insert_fields[] = { 10, 15, 5, 3, 0, 0, 4, 0, 6 };

const unsigned int dnsa_search_fields[] = { 1, 1, 1 };

const unsigned int dnsa_search_args[] = { 1, 1, 1, 1 };

const unsigned int dnsa_delete_args[] = { 1, 1, 1, 1, 0, 0, 1 };

const unsigned int dnsa_update_args[] = { 1, 1, 1, 2, 1, 2, 1 };

const unsigned int dnsa_extended_search_fields[] = { 3, 5, 1 /*, 1*/ };

const unsigned int dnsa_extended_search_args[] = { 1, 1, 2/* , 1 */ };

const unsigned int dnsa_ext_search_field_type[][5] = { /* What we are selecting */
	{ DBTEXT, DBTEXT, DBINT, NONE, NONE } ,
	{ DBINT, DBTEXT, DBTEXT, DBINT, DBTEXT },
	{ DBTEXT, NONE, NONE, NONE, NONE }/* ,
	{ DBTEXT, NONE, NONE } */
};

const unsigned int dnsa_ext_search_arg_type[][2] = { /* What we are searching on */
	{ DBTEXT, NONE } ,
	{ DBINT, NONE } ,
	{ DBTEXT, DBTEXT } /*,
	{ DBTEXT } */
};

const unsigned int dnsa_update_arg_type[][2] = {
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, DBINT } ,
	{ DBINT, NONE } ,
	{ DBINT, DBINT } ,
	{ DBINT, NONE }
};

const unsigned int dnsa_delete_arg_type[][1] = {
	{ DBINT } ,
	{ DBINT } ,
	{ DBINT } ,
	{ DBINT } ,
	{ NONE } ,
	{ NONE } ,
	{ DBINT } ,
	{ DBINT } ,
	{ DBINT }
};

int
dnsa_run_query(dnsa_config_s *config, dnsa_s *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return NONE;
}

int
dnsa_run_multiple_query(dnsa_config_s *config, dnsa_s *base, int type)
{
	int retval;
	retval = NONE;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_multiple_query_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_multiple_query_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}
	
	return retval;
}

int
dnsa_run_search(dnsa_config_s *config, dnsa_s *base, int type)
{
	int retval;

	if ((strncmp(config->dbtype, "none", RANGE_S) ==0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_search_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_search_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
dnsa_run_extended_search(dnsa_config_s *config, dbdata_s *base, int type)
{
	int retval;

	if ((strncmp(config->dbtype, "none", RANGE_S) ==0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_extended_search_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_extended_search_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
dnsa_run_insert(dnsa_config_s *config, dnsa_s *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_insert_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_insert_sqlite(config, base, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
dnsa_run_update(dnsa_config_s *config, dbdata_s *data, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_update_mysql(config, data, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_update_sqlite(config, data, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
dnsa_run_delete(dnsa_config_s *config, dbdata_s *data, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = dnsa_run_delete_mysql(config, data, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = dnsa_run_delete_sqlite(config, data, type);
		return retval;
#endif /* HAVE_SQLITE3 */
	} else {
		fprintf(stderr, "Unknown database type %s\n", config->dbtype);
		return DB_TYPE_INVALID;
	}

	return NONE;
}

int
dnsa_get_query(int type, const char **query, unsigned int *fields)
{
	int retval;
	
	retval = NONE;
	if (type == ZONE) {
		*query = dnsa_sql_select[ZONES];
		*fields = dnsa_select_fields[ZONES];
	} else if (type == REV_ZONE) {
		*query = dnsa_sql_select[REV_ZONES];
		*fields = dnsa_select_fields[REV_ZONES];
	} else if (type == RECORD) {
		*query = dnsa_sql_select[RECORDS];
		*fields = dnsa_select_fields[RECORDS];
	} else if (type == REV_RECORD) {
		*query = dnsa_sql_select[REV_RECORDS];
		*fields = dnsa_select_fields[REV_RECORDS];
	} else if (type == ALL_A_RECORD) {
		*query = dnsa_sql_select[ALL_A_RECORDS];
		*fields = dnsa_select_fields[ALL_A_RECORDS];
	} else if (type ==  DUPLICATE_A_RECORD) {
		*query = dnsa_sql_select[DUPLICATE_A_RECORDS];
		*fields = dnsa_select_fields[DUPLICATE_A_RECORDS];
	} else if (type == PREFERRED_A) {
		*query = dnsa_sql_select[PREFERRED_AS];
		*fields = dnsa_select_fields[PREFERRED_AS];
	} else if (type == GLUE) {
		*query = dnsa_sql_select[GLUES];
		*fields = dnsa_select_fields[GLUES];
	} else {
		fprintf(stderr, "Unknown query type %d\n", type);
		retval = 1;
	}
	return retval;
}

void
dnsa_get_search(int type, size_t *fields, size_t *args, void **input, void **output, dnsa_s *base)
{
	if (type == ZONE_ID_ON_NAME) {
		*input = &(base->zones->name);
		*output = &(base->zones->id);
		*fields = strlen(base->zones->name);
		*args = sizeof(base->zones->id);
	} else if (type == REV_ZONE_ID_ON_NET_RANGE) {
		*input = &(base->rev_zones->net_range);
		*output = &(base->rev_zones->rev_zone_id);
		*fields = strlen(base->rev_zones->net_range);
		*args = sizeof(base->rev_zones->rev_zone_id);
	} else if (type == REV_ZONE_PREFIX) {
		*input = &(base->rev_zones->net_range);
		*output = &(base->rev_zones->prefix);
		*fields = strlen(base->rev_zones->net_range);
		*args = sizeof(base->rev_zones->prefix);
	} else {
		fprintf(stderr, "Unknown query %d\n", type);
		exit (NO_QUERY);
	}
}

void
dnsa_init_initial_dbdata(dbdata_s **list, int type)
{
	unsigned int i = 0;
	dbdata_s *data, *dlist;
	dlist = *list = '\0';
	for (i = 0; i < dnsa_extended_search_fields[type]; i++) {
		if (!(data = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "data in dnsa_init_initial_dbdata");
		init_dbdata_struct(data);
		if (!(*list)) {
			*list = dlist = data;
		} else {
			while (dlist->next)
				dlist = dlist->next;
			dlist->next = data;
		}
	}
	while (i < dnsa_extended_search_args[type]) {
		if (!(data = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "data in dnsa_init_initial_dbdata");
		init_dbdata_struct(data);
		if (!(*list)) {
			*list = dlist = data;
		} else {
			while (dlist->next)
				dlist = dlist->next;
			dlist->next = data;
		}
		i++;
	}
}

#ifdef HAVE_MYSQL
void
dnsa_mysql_init(dnsa_config_s *dc, MYSQL *dnsa_mysql)
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
dnsa_run_query_mysql(dnsa_config_s *config, dnsa_s *base, int type)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	const char *query;
	int retval;
	unsigned int fields;

	retval = 0;
	dnsa_mysql_init(config, &dnsa);
	if ((retval = dnsa_get_query(type, &query, &fields)) != 0) {
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
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) != 0)) {
		while ((dnsa_row = mysql_fetch_row(dnsa_res)))
			dnsa_store_result_mysql(dnsa_row, base, type, fields);
	}
	cmdb_mysql_cleanup_full(&dnsa, dnsa_res);
	return retval;
}

int
dnsa_run_multiple_query_mysql(dnsa_config_s *config, dnsa_s *base, int type)
{
	int retval;
	retval = NONE;
	if (type & ZONE)
		if ((retval = dnsa_run_query_mysql(config, base, ZONE)) != 0)
			return retval;
	if (type & REV_ZONE)
		if ((retval = dnsa_run_query_mysql(config, base, REV_ZONE)) != 0)
			return retval;
	if (type & RECORD)
		if ((retval = dnsa_run_query_mysql(config, base, RECORD)) != 0)
			return retval;
	if (type & REV_RECORD)
		if ((retval = dnsa_run_query_mysql(config, base, REV_RECORD)) != 0)
			return retval;
	if (type & ALL_A_RECORD)
		if ((retval = dnsa_run_query_mysql(config, base, ALL_A_RECORD)) != 0)
			return retval;
	if (type & DUPLICATE_A_RECORD)
		if ((retval = dnsa_run_query_mysql(config, base, DUPLICATE_A_RECORD)) != 0)
			return retval;
	if (type & PREFERRED_A)
		if ((retval = dnsa_run_query_mysql(config, base, PREFERRED_A)) != 0)
			return retval;
	if (type & GLUES)
		if ((retval = dnsa_run_query_mysql(config, base, GLUE)) != 0)
			return retval;
	return retval;
}

void
dnsa_store_result_mysql(MYSQL_ROW row, dnsa_s *base, int type, unsigned int fields)
{
	unsigned int required;
	if (type == ZONE) {
		required = dnsa_select_fields[ZONES];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_zone_mysql(row, base);
	} else if (type == REV_ZONE) {
		required = dnsa_select_fields[REV_ZONES];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_rev_zone_mysql(row, base);
	} else if (type == RECORD) {
		required = dnsa_select_fields[RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_record_mysql(row, base);
	} else if (type == REV_RECORD) {
		required = dnsa_select_fields[REV_RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_rev_record_mysql(row, base);
	} else if (type == ALL_A_RECORD) {
		required = dnsa_select_fields[ALL_A_RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_all_a_records_mysql(row, base);
	} else if (type == DUPLICATE_A_RECORD) {
		required = dnsa_select_fields[DUPLICATE_A_RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_duplicate_a_record_mysql(row, base);
	} else if (type == PREFERRED_A) {
		required = dnsa_select_fields[PREFERRED_AS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_preferred_a_mysql(row, base);
	} else if (type == GLUE) {
		required = dnsa_select_fields[GLUES];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_glue_mysql(row, base);
	} else {
		dnsa_query_mismatch(NONE, NONE, NONE);
	}
}

void
dnsa_store_zone_mysql(MYSQL_ROW row, dnsa_s *base)
{
	int retval;
	zone_info_s *zone, *list;

	if (!(zone = malloc(sizeof(zone_info_s))))
		report_error(MALLOC_FAIL, "zone in dnsa_store_zone_mysql");
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
	snprintf(zone->type, RANGE_S, "%s", row[12]);
	snprintf(zone->master, RBUFF_S, "%s", row[13]);
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
dnsa_store_record_mysql(MYSQL_ROW row, dnsa_s *base)
{
	record_row_s *rec, *list;

	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "rec in dnsa_store_record_mysql");
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
dnsa_store_rev_zone_mysql(MYSQL_ROW row, dnsa_s *base)
{
	int retval;
	rev_zone_info_s *rev, *list;

	if (!(rev = malloc(sizeof(rev_zone_info_s))))
		report_error(MALLOC_FAIL, "rev in dnsa_store_rev_zone_mysql");
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
	snprintf(rev->type, RANGE_S, "%s", row[17]);
	snprintf(rev->master, RBUFF_S, "%s", row[18]);
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
dnsa_store_rev_record_mysql(MYSQL_ROW row, dnsa_s *base)
{
	rev_record_row_s *rev, *list;

	if (!(rev = malloc(sizeof(rev_record_row_s))))
		report_error(MALLOC_FAIL, "rev in dnsa_store_rev_record_mysql");
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

void
dnsa_store_all_a_records_mysql(MYSQL_ROW row, dnsa_s *base)
{
	record_row_s *rec, *list;

	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "rec in dnsa_store_all_a_records_mysql");
	init_record_struct(rec);
	snprintf(rec->host, RBUFF_S, "%s.%s", row[1], row[0]);
	snprintf(rec->dest, RANGE_S, "%s", row[2]);
	rec->id = strtoul(row[3], NULL, 10);
	rec->zone = strtoul(row[4], NULL, 10);
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
dnsa_store_preferred_a_mysql(MYSQL_ROW row, dnsa_s *base)
{
	preferred_a_s *prefer, *list;

	if (!(prefer = malloc(sizeof(preferred_a_s))))
		report_error(MALLOC_FAIL, "prefer in dnsa_store_preferred_a_sqlite");
	init_preferred_a_struct(prefer);
	prefer->prefa_id = strtoul(row[0], NULL, 10);
	snprintf(prefer->ip, RANGE_S, "%s", row[1]);
	prefer->ip_addr = strtoul(row[2], NULL, 10);
	prefer->record_id = strtoul(row[3], NULL, 10);
	snprintf(prefer->fqdn, RBUFF_S, "%s", row[4]);
	list = base->prefer;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = prefer;
	} else {
		base->prefer = prefer;
	}
}

void
dnsa_store_duplicate_a_record_mysql(MYSQL_ROW row, dnsa_s *base)
{
	record_row_s *rec, *list;

	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "dnsa_store_duplicate_a_record_mysql");
	init_record_struct(rec);
	snprintf(rec->dest, RANGE_S, "%s", row[0]);
	rec->id = strtoul(row[1], NULL, 10);
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
dnsa_store_glue_mysql(MYSQL_ROW row, dnsa_s *base)
{
	glue_zone_info_s *glue, *list;

	if (!(glue = malloc(sizeof(glue_zone_info_s))))
		report_error(MALLOC_FAIL, "dnsa_store_glue_mysql");
	init_glue_zone_struct(glue);
	glue->id = strtoul(row[0], NULL, 10);
	snprintf(glue->name, RBUFF_S, "%s", row[1]);
	glue->zone_id = strtoul(row[2], NULL, 10);
	snprintf(glue->pri_dns, RANGE_S, "%s", row[3]);
	snprintf(glue->sec_dns, RANGE_S, "%s", row[4]);
	snprintf(glue->pri_ns, RBUFF_S "%s", row[5]);
	snprintf(glue->sec_ns, RBUFF_S, "%s", row[6]);
	list = base->glue;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = glue;
	} else {
		base->glue = glue;
	}
}

int
dnsa_run_search_mysql(dnsa_config_s *config, dnsa_s *base, int type)
{
	MYSQL dnsa;
	MYSQL_STMT *dnsa_stmt;
	MYSQL_BIND my_bind[2];
	const char *query;
	int retval;
	size_t arg, res;
	void *input, *output;

	retval = 0;
	dnsa_mysql_init(config, &dnsa);
	memset(my_bind, 0, sizeof(my_bind));
/* 
Will need to check if we have char or int here. Hard coded char for search,
and int for result, which is OK when searching on name and returning id
*/
	query = dnsa_sql_search[type];
	dnsa_get_search(type, &arg, &res, &input, &output, base);
	my_bind[0].buffer_type = MYSQL_TYPE_STRING;
	my_bind[0].buffer = input;
	my_bind[0].buffer_length = arg;
	my_bind[0].is_unsigned = 0;
	my_bind[0].is_null = 0;
	my_bind[0].length = 0;
	my_bind[1].buffer_type = MYSQL_TYPE_LONG;
	my_bind[1].buffer = output;
	my_bind[1].buffer_length = res;
	my_bind[1].is_unsigned = 1;
	my_bind[1].is_null = 0;
	my_bind[1].length = 0;
	
	if (!(dnsa_stmt = mysql_stmt_init(&dnsa)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(dnsa_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_param(dnsa_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_result(dnsa_stmt, &my_bind[1])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_execute(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_store_result(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_fetch(dnsa_stmt)) != 0) {
		if (retval != MYSQL_NO_DATA) {
			report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
		} else {
			retval = NONE;
		}
	}
	mysql_stmt_free_result(dnsa_stmt);
	mysql_stmt_close(dnsa_stmt);
	cmdb_mysql_cleanup(&dnsa);
	return retval;
}

int
dnsa_run_extended_search_mysql(dnsa_config_s *config, dbdata_s *base, int type)
{
	MYSQL dnsa;
	MYSQL_STMT *dnsa_stmt;
	MYSQL_BIND args_bind[dnsa_extended_search_args[type]];
	MYSQL_BIND fields_bind[dnsa_extended_search_fields[type]];
	const char *query;
	int retval, j;
	unsigned int i;

	retval = j = 0;
	memset(args_bind, 0, sizeof(args_bind));
	memset(fields_bind, 0, sizeof(fields_bind));
	for (i = 0; i < dnsa_extended_search_args[type]; i++) 
		if ((retval = dnsa_setup_bind_ext_mysql_args(&args_bind[i], i, type, base)) != 0)
			return retval;
	for (i = 0; i < dnsa_extended_search_fields[type]; i++)
		if ((retval = dnsa_setup_bind_ext_mysql_fields(&fields_bind[i], i, j, type, base)) != 0)
			return retval;
	query = dnsa_sql_extended_search[type];
	dnsa_mysql_init(config, &dnsa);
	if (!(dnsa_stmt = mysql_stmt_init(&dnsa)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(dnsa_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_param(dnsa_stmt, &args_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_result(dnsa_stmt, &fields_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_execute(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_store_result(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	while ((retval = mysql_stmt_fetch(dnsa_stmt)) == 0) {
		j++;
		for (i = 0; i < dnsa_extended_search_fields[type]; i++)
			if ((retval = dnsa_setup_bind_ext_mysql_fields(&fields_bind[i], i, j, type, base)) != 0)
				return retval;
		if ((retval = mysql_stmt_bind_result(dnsa_stmt, &fields_bind[0])) != 0)
			report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	}
	if (retval != MYSQL_NO_DATA) {
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	} else {
		retval = NONE;
	}
	mysql_stmt_free_result(dnsa_stmt);
	mysql_stmt_close(dnsa_stmt);
	cmdb_mysql_cleanup(&dnsa);
	return j;
}

int
dnsa_run_insert_mysql(dnsa_config_s *config, dnsa_s *base, int type)
{
	MYSQL dnsa;
	MYSQL_STMT *dnsa_stmt;
	MYSQL_BIND my_bind[dnsa_insert_fields[type]];
	const char *query;
	int retval;
	unsigned int i;

	retval = 0;
	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < dnsa_insert_fields[type]; i++)
		if ((retval = dnsa_setup_insert_mysql_bind(&my_bind[i], i, type, base)) != 0)
			return retval;
	query = dnsa_sql_insert[type];
	dnsa_mysql_init(config, &dnsa);
	if (!(dnsa_stmt = mysql_stmt_init(&dnsa)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(dnsa_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_param(dnsa_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_execute(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));

	mysql_stmt_close(dnsa_stmt);
	cmdb_mysql_cleanup(&dnsa);

	return retval;
}

int
dnsa_run_update_mysql(dnsa_config_s *config, dbdata_s *data, int type)
{
	MYSQL dnsa;
	MYSQL_STMT *dnsa_stmt;
	MYSQL_BIND my_bind[dnsa_update_args[type]];
	const char *query;
	int retval;
	unsigned int i;
	dbdata_s *list;

	list = data;
	retval = 0;
	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < dnsa_update_args[type]; i++) {
		if (dnsa_update_arg_type[type][i] == DBINT) {
			my_bind[i].buffer_type = MYSQL_TYPE_LONG;
			my_bind[i].is_null = 0;
			my_bind[i].length = 0;
			my_bind[i].is_unsigned = 1;
			my_bind[i].buffer = &(list->args.number);
			my_bind[i].buffer_length = sizeof(unsigned long int);
			list = list->next;
		} else if (dnsa_update_arg_type[type][i] == DBTEXT) {
			my_bind[i].buffer_type = MYSQL_TYPE_STRING;
			my_bind[i].is_null = 0;
			my_bind[i].length = 0;
			my_bind[i].is_unsigned = 0;
			my_bind[i].buffer = &(list->args.text);
			my_bind[i].buffer_length = strlen(list->args.text);
			list = list->next;
		}
	}
	query = dnsa_sql_update[type];
	dnsa_mysql_init(config, &dnsa);
	if (!(dnsa_stmt = mysql_stmt_init(&dnsa)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(dnsa_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_param(dnsa_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_execute(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	
	mysql_stmt_close(dnsa_stmt);
	cmdb_mysql_cleanup(&dnsa);
	return retval;
}

int
dnsa_run_delete_mysql(dnsa_config_s *config, dbdata_s *data, int type)
{
	MYSQL dnsa;
	MYSQL_STMT *dnsa_stmt;
	MYSQL_BIND my_bind[dnsa_delete_args[type]];
	const char *query;
	int retval;
	unsigned int i;
	dbdata_s *list;

	list = data;
	retval = 0;
	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < dnsa_delete_args[type]; i++) {
		if (dnsa_delete_arg_type[type][i] == DBINT) {
			my_bind[i].buffer_type = MYSQL_TYPE_LONG;
			my_bind[i].is_null = 0;
			my_bind[i].length = 0;
			my_bind[i].is_unsigned = 1;
			my_bind[i].buffer = &(list->args.number);
			my_bind[i].buffer_length = sizeof(unsigned long int);
			list = list->next;
		} else if (dnsa_delete_arg_type[type][i] == DBTEXT) {
			my_bind[i].buffer_type = MYSQL_TYPE_STRING;
			my_bind[i].is_null = 0;
			my_bind[i].length = 0;
			my_bind[i].is_unsigned = 0;
			my_bind[i].buffer = &(list->args.text);
			my_bind[i].buffer_length = strlen(list->args.text);
			list = list->next;
		}
	}
	query = dnsa_sql_delete[type];
	dnsa_mysql_init(config, &dnsa);
	if (!(dnsa_stmt = mysql_stmt_init(&dnsa)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(dnsa_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_bind_param(dnsa_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(dnsa_stmt));
	if ((retval = mysql_stmt_execute(dnsa_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(dnsa_stmt));

	retval = (int)mysql_stmt_affected_rows(dnsa_stmt);
	mysql_stmt_close(dnsa_stmt);
	cmdb_mysql_cleanup(&dnsa);
	return retval;
}

int
dnsa_setup_insert_mysql_bind(MYSQL_BIND *mybind, unsigned int i, int type, dnsa_s *base)
{
	int retval;

	retval = 0;
	void *buffer;
	mybind->buffer_type = mysql_inserts[type][i];
	mybind->is_null = 0;
	mybind->length = 0;
	if ((retval = dnsa_setup_insert_mysql_bind_buffer(type, &buffer, base, i)) != 0)
		return retval;
	mybind->buffer = buffer;
	if (mybind->buffer_type == MYSQL_TYPE_STRING) {
		mybind->is_unsigned = 0;
		mybind->buffer_length = strlen(buffer);
	} else if (mybind->buffer_type == MYSQL_TYPE_LONG) {
		mybind->is_unsigned = 1;
		mybind->buffer_length = sizeof(unsigned long int);
	} else {
		retval = WRONG_TYPE;
	}
	return retval;
}

int
dnsa_setup_bind_ext_mysql_args(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base)
{
	int retval;
	unsigned int j;
	void *buffer;
	dbdata_s *list = base;
	
	retval = 0;
	mybind->is_null = 0;
	mybind->length = 0;
	for (j = 0; j < i; j++)
		list = list->next;
	if (dnsa_ext_search_arg_type[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		buffer = &(list->args.number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (dnsa_ext_search_arg_type[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		buffer = &(list->args.text);
		mybind->buffer_length = strlen(buffer);
	} else {
		return WRONG_TYPE;
	}
	mybind->buffer = buffer;
	return retval;
}

int
dnsa_setup_bind_ext_mysql_fields(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base)
{
	int retval, j;
	static int m = 0;
	void *buffer;
	dbdata_s *list, *new;
	list = base;
	
	retval = 0;
	mybind->is_null = 0;
	mybind->length = 0;
	if (k > 0) {
		if (!(new = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "new in dnsa_setup_bind_ext_mysql_fields");
		init_dbdata_struct(new);
		while (list->next) {
			list = list->next;
		}
		list->next = new;
		list = base;
	}
	for (j = 0; j < m; j++)
		list = list->next;
	if (dnsa_ext_search_field_type[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		buffer = &(list->fields.number);
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (dnsa_ext_search_field_type[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		buffer = &(list->fields.text);
		mybind->buffer_length = RBUFF_S;
	} else {
		return WRONG_TYPE;
	}
	mybind->buffer = buffer;
	m++;

	return retval;
}

int
dnsa_setup_insert_mysql_bind_buffer(int type, void **input, dnsa_s *base, unsigned int i)
{
	int retval = 0;
	
	if (type == RECORDS)
		dnsa_setup_insert_mysql_bind_buff_record(input, base, i);
	else if (type == ZONES)
		dnsa_setup_insert_mysql_bind_buff_zone(input, base, i);
	else if (type == REV_ZONES)
		dnsa_setup_insert_mysql_bind_buff_rev_zone(input, base, i);
	else if (type == REV_RECORDS)
		dnsa_setup_insert_mysql_bind_buff_rev_records(input, base, i);
	else if (type == PREFERRED_AS)
		dnsa_setup_insert_mysql_bind_buff_pref_a(input, base, i);
	else if (type == GLUES)
		dnsa_setup_insert_mysql_bind_buff_glue(input, base, i);
	else
		retval = WRONG_TYPE;
	
	return retval;
}

void
dnsa_setup_insert_mysql_bind_buff_record(void **input, dnsa_s *base, unsigned int i)
{
	if (i == 0)
		*input = &(base->records->zone);
	else if (i == 1)
		*input = &(base->records->host);
	else if (i == 2)
		*input = &(base->records->type);
	else if (i == 3)
		*input = &(base->records->pri);
	else if (i == 4)
		*input = &(base->records->dest);
}

void
dnsa_setup_insert_mysql_bind_buff_zone(void **input, dnsa_s *base, unsigned int i)
{
	if (i == 0)
		*input = &(base->zones->name);
	else if (i == 1)
		*input = &(base->zones->pri_dns);
	else if (i == 2)
		*input = &(base->zones->sec_dns);
	else if (i == 3)
		*input = &(base->zones->serial);
	else if (i == 4)
		*input = &(base->zones->refresh);
	else if (i == 5)
		*input = &(base->zones->retry);
	else if (i == 6)
		*input = &(base->zones->expire);
	else if (i == 7)
		*input = &(base->zones->ttl);
	else if (i == 8)
		*input = &(base->zones->type);
	else if (i == 9)
		*input = &(base->zones->master);
}

void
dnsa_setup_insert_mysql_bind_buff_rev_zone(void **input, dnsa_s *base, unsigned int i)
{
	if (i == 0)
		*input = &(base->rev_zones->net_range);
	else if (i == 1)
		*input = &(base->rev_zones->prefix);
	else if (i == 2)
		*input = &(base->rev_zones->net_start);
	else if (i == 3)
		*input = &(base->rev_zones->net_finish);
	else if (i == 4)
		*input = &(base->rev_zones->start_ip);
	else if (i == 5)
		*input = &(base->rev_zones->end_ip);
	else if (i == 6)
		*input = &(base->rev_zones->pri_dns);
	else if (i == 7)
		*input = &(base->rev_zones->sec_dns);
	else if (i == 8)
		*input = &(base->rev_zones->serial);
	else if (i == 9)
		*input = &(base->rev_zones->refresh);
	else if (i == 10)
		*input = &(base->rev_zones->retry);
	else if (i == 11)
		*input = &(base->rev_zones->expire);
	else if (i == 12)
		*input = &(base->rev_zones->ttl);
	else if (i == 13)
		*input = &(base->rev_zones->type);
	else if (i == 14)
		*input = &(base->rev_zones->master);
}

void
dnsa_setup_insert_mysql_bind_buff_rev_records(void **input, dnsa_s *base, unsigned int i)
{
	if (i == 0)
		*input = &(base->rev_records->rev_zone);
	else if (i == 1)
		*input = &(base->rev_records->host);
	else if (i == 2)
		*input = &(base->rev_records->dest);
}

void
dnsa_setup_insert_mysql_bind_buff_pref_a(void **input, dnsa_s *base, unsigned int i)
{
	if (i == 0)
		*input = &(base->prefer->ip);
	else if (i == 1)
		*input = &(base->prefer->ip_addr);
	else if (i == 2)
		*input = &(base->prefer->record_id);
	else if (i == 3)
		*input = &(base->prefer->fqdn);
}

void
dnsa_setup_insert_mysql_bind_buff_glue(void **input, dnsa_s *base, unsigned int i)
{
	if (i == 0)
		*input = &(base->glue->name);
	else if (i == 1)
		*input = &(base->glue->zone_id);
	else if (i == 2)
		*input = &(base->glue->pri_dns);
	else if (i == 3)
		*input = &(base->glue->sec_dns);
	else if (i == 4)
		*input = &(base->glue->pri_ns);
	else if (i == 5)
		*input = &(base->glue->sec_ns);
}

#endif /* HAVE_MYSQL */

#ifdef HAVE_SQLITE3

int
dnsa_run_query_sqlite(dnsa_config_s *config, dnsa_s *base, int type)
{
	const char *query, *file;
	int retval;
	unsigned int fields;
	sqlite3 *dnsa;
	sqlite3_stmt *state;
	
	retval = 0;
	file = config->file;
	if ((retval = dnsa_get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_query_sqlite");
	}
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		dnsa_store_result_sqlite(state, base, type, fields);
	
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(dnsa);
	
	return 0;
}

int
dnsa_run_multiple_query_sqlite(dnsa_config_s *config, dnsa_s *base, int type)
{
	int retval;
	retval = NONE;
	if (type & ZONE)
		if ((retval = dnsa_run_query_sqlite(config, base, ZONE)) != 0)
			return retval;
	if (type & REV_ZONE)
		if ((retval = dnsa_run_query_sqlite(config, base, REV_ZONE)) != 0)
			return retval;
	if (type & RECORD)
		if ((retval = dnsa_run_query_sqlite(config, base, RECORD)) != 0)
			return retval;
	if (type & REV_RECORD)
		if ((retval = dnsa_run_query_sqlite(config, base, REV_RECORD)) != 0)
			return retval;
	if (type & ALL_A_RECORD)
		if ((retval = dnsa_run_query_sqlite(config, base, ALL_A_RECORD)) != 0)
			return retval;
	if (type & DUPLICATE_A_RECORD)
		if ((retval = dnsa_run_query_sqlite(config, base, DUPLICATE_A_RECORD)) != 0)
			return retval;
	if (type & PREFERRED_A)
		if ((retval = dnsa_run_query_sqlite(config, base, PREFERRED_A)) != 0)
			return retval;
	if (type & GLUE)
		if ((retval = dnsa_run_query_sqlite(config, base, GLUE)) != 0)
			return retval;
	return retval;
}

void
dnsa_store_result_sqlite(sqlite3_stmt *state, dnsa_s *base, int type, unsigned int fields)
{
	unsigned int required;
	if (type == ZONE) {
		required = dnsa_select_fields[ZONES];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_zone_sqlite(state, base);
	} else if (type == REV_ZONE) {
		required = dnsa_select_fields[REV_ZONES];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_rev_zone_sqlite(state, base);
	} else if (type == RECORD) {
		required = dnsa_select_fields[RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_record_sqlite(state, base);
	} else if (type == REV_RECORD) {
		required = dnsa_select_fields[REV_RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_rev_record_sqlite(state, base);
	} else if (type == ALL_A_RECORD) {
		required = dnsa_select_fields[ALL_A_RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_all_a_records_sqlite(state, base);
	} else if (type == DUPLICATE_A_RECORD) {
		required = dnsa_select_fields[DUPLICATE_A_RECORDS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_duplicate_a_record_sqlite(state, base);
	} else if (type == PREFERRED_A) {
		required = dnsa_select_fields[PREFERRED_AS];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_preferred_a_sqlite(state, base);
	} else if (type == GLUE) {
		required = dnsa_select_fields[GLUES];
		if (fields != required)
			dnsa_query_mismatch(fields, required, type);
		dnsa_store_glue_sqlite(state, base);
	} else {
		dnsa_query_mismatch(NONE, NONE, NONE);
	}
}

void
dnsa_store_zone_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	int retval;
	zone_info_s *zone, *list;
	
	if (!(zone = malloc(sizeof(zone_info_s))))
		report_error(MALLOC_FAIL, "zone in dnsa_store_zone_sqlite");
	init_zone_struct(zone);
	zone->id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(zone->name, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(zone->pri_dns, RBUFF_S -1, "%s", sqlite3_column_text(state, 2));
	if ((retval = add_trailing_dot(zone->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	snprintf(zone->sec_dns, RBUFF_S - 1, "%s", sqlite3_column_text(state, 3));
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(zone->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	zone->serial = (unsigned long int) sqlite3_column_int64(state, 4);
	zone->refresh = (unsigned long int) sqlite3_column_int64(state, 5);
	zone->retry = (unsigned long int) sqlite3_column_int64(state, 6);
	zone->expire = (unsigned long int) sqlite3_column_int64(state, 7);
	zone->ttl = (unsigned long int) sqlite3_column_int64(state, 8);
	snprintf(zone->valid, RANGE_S, "%s", sqlite3_column_text(state, 9));
	zone->owner = (unsigned long int) sqlite3_column_int(state, 10);
	snprintf(zone->updated, RANGE_S, "%s", sqlite3_column_text(state, 11));
	snprintf(zone->type, RANGE_S, "%s", sqlite3_column_text(state, 12));
	snprintf(zone->master, RBUFF_S, "%s", sqlite3_column_text(state, 13));
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
dnsa_store_rev_zone_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	int retval;
	rev_zone_info_s *rev, *list;
	
	if (!(rev = malloc(sizeof(rev_zone_info_s))))
		report_error(MALLOC_FAIL, "rev in dnsa_store_rev_zone_sqlite");
	init_rev_zone_struct(rev);
	rev->rev_zone_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(rev->net_range, RANGE_S, "%s", sqlite3_column_text(state, 1));
	rev->prefix = (unsigned long int) sqlite3_column_int(state, 2);
	snprintf(rev->net_start, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(rev->net_finish, RANGE_S, "%s", sqlite3_column_text(state, 4));
	rev->start_ip = (unsigned long int) sqlite3_column_int64(state, 5);
	rev->end_ip = (unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(rev->pri_dns, RBUFF_S -1, "%s", sqlite3_column_text(state, 7));
	if ((retval = add_trailing_dot(rev->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	snprintf(rev->sec_dns, RBUFF_S -1, "%s", sqlite3_column_text(state, 8));
	if (strncmp(rev->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(rev->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	rev->serial = (unsigned long int) sqlite3_column_int64(state, 9);
	rev->refresh = (unsigned long int) sqlite3_column_int64(state, 10);
	rev->retry = (unsigned long int) sqlite3_column_int64(state, 11);
	rev->expire = (unsigned long int) sqlite3_column_int64(state, 12);
	rev->ttl = (unsigned long int) sqlite3_column_int64(state, 13);
	snprintf(rev->valid, RANGE_S, "%s", sqlite3_column_text(state, 14));
	rev->owner = (unsigned long int) sqlite3_column_int(state, 15);
	snprintf(rev->updated, RANGE_S, "%s", sqlite3_column_text(state, 16));
	snprintf(rev->type, RANGE_S, "%s", sqlite3_column_text(state, 17));
	snprintf(rev->master, RBUFF_S, "%s", sqlite3_column_text(state, 18));
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
dnsa_store_record_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	record_row_s *rec, *list;
	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "rec in dnsa_store_record_sqlite");
	init_record_struct(rec);
	rec->id = (unsigned long int) sqlite3_column_int64(state, 0);
	rec->zone = (unsigned long int) sqlite3_column_int64(state, 1);
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
dnsa_store_rev_record_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	rev_record_row_s *rev, *list;

	if (!(rev = malloc(sizeof(rev_record_row_s))))
		report_error(MALLOC_FAIL, "rev in dnsa_store_rev_record_sqlite");
	init_rev_record_struct(rev);
	rev->record_id = (unsigned long int) sqlite3_column_int64(state, 0);
	rev->rev_zone = (unsigned long int) sqlite3_column_int64(state, 1);
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

void
dnsa_store_all_a_records_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	record_row_s *rec, *list;

	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "rec in dnsa_store_all_a_records");
	init_record_struct(rec);
	snprintf(rec->host, RBUFF_S, "%s.%s",
		 sqlite3_column_text(state, 1), sqlite3_column_text(state, 0));
	snprintf(rec->dest, RANGE_S, "%s", sqlite3_column_text(state, 2));
	rec->id = (unsigned long int) sqlite3_column_int64(state, 3);
	rec->zone = (unsigned long int) sqlite3_column_int64(state, 4);
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
dnsa_store_preferred_a_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	preferred_a_s *prefer, *list;
	
	if (!(prefer = malloc(sizeof(preferred_a_s))))
		report_error(MALLOC_FAIL, "prefer in dnsa_store_preferred_a_sqlite");
	init_preferred_a_struct(prefer);
	prefer->prefa_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(prefer->ip, RANGE_S, "%s", sqlite3_column_text(state, 1));
	prefer->ip_addr = (unsigned long int) sqlite3_column_int64(state, 2);
	prefer->record_id = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(prefer->fqdn, RBUFF_S, "%s", sqlite3_column_text(state, 4));
	list = base->prefer;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = prefer;
	} else {
		base->prefer = prefer;
	}
}

void
dnsa_store_duplicate_a_record_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	record_row_s *rec, *list;

	if (!(rec = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "dnsa_store_duplicate_a_record_mysql");
	init_record_struct(rec);
	snprintf(rec->dest, RANGE_S, "%s", sqlite3_column_text(state, 0));
	rec->id = (unsigned long int) sqlite3_column_int64(state, 1);
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
dnsa_store_glue_sqlite(sqlite3_stmt *state, dnsa_s *base)
{
	glue_zone_info_s *glue, *list;

	if (!(glue = malloc(sizeof(glue_zone_info_s))))
		report_error(MALLOC_FAIL, "glue in dnsa_store_glue_sqlite");
	init_glue_zone_struct(glue);
	glue->id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(glue->name, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	glue->zone_id = (unsigned long int) sqlite3_column_int64(state, 2);
	snprintf(glue->pri_dns, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(glue->sec_dns, RANGE_S, "%s", sqlite3_column_text(state, 4));
	snprintf(glue->pri_ns, RBUFF_S, "%s", sqlite3_column_text(state, 5));
	snprintf(glue->sec_ns, RBUFF_S, "%s", sqlite3_column_text(state, 6));
	list = base->glue;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = glue;
	} else {
		base->glue = glue;
	}
}

int
dnsa_run_search_sqlite(dnsa_config_s *config, dnsa_s *base, int type)
{
	const char *query, *file;
	int retval;
	unsigned long int result;
	size_t fields, args;
	void *input, *output;
	sqlite3 *dnsa;
	sqlite3_stmt *state;
	
	retval = 0;
	query = dnsa_sql_search[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_search_sqlite");
	}
/*
   As in the MySQL function we assume that we are sending text and recieving
   numerical data. Searching on name for ID is ok for this
*/
	dnsa_get_search(type, &fields, &args, &input, &output, base);
	if ((retval = sqlite3_bind_text(state, 1, input, (int)strlen(input), SQLITE_STATIC)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_search_sqlite");
	}
	if ((retval = sqlite3_step(state)) == SQLITE_ROW) {
		result = (unsigned long int)sqlite3_column_int64(state, 0);
		if (type == ZONE_ID_ON_NAME) 
			base->zones->id = result;
		else if (type == REV_ZONE_ID_ON_NET_RANGE)
			base->rev_zones->rev_zone_id = result;
		else if (type == REV_ZONE_PREFIX)
			base->rev_zones->prefix = result;
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(dnsa);
	return retval;
}

int
dnsa_run_extended_search_sqlite(dnsa_config_s *config, dbdata_s *base, int type)
{
	const char *query, *file;
	int retval, i;
	dbdata_s *list;
	sqlite3 *dnsa;
	sqlite3_stmt *state;

	retval = 0;
	list = base;
	query = dnsa_sql_extended_search[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_search_sqlite");
	}
	for (i = 0; (unsigned long)i < dnsa_extended_search_args[type]; i++) {
		dnsa_setup_bind_extended_sqlite(state, list, type, i);
		list = list->next;
	}
	list = base;
	i = 0;
	while ((sqlite3_step(state)) == SQLITE_ROW) {
		dnsa_get_extended_results_sqlite(state, list, type, i);
		i++;
	}

	retval = sqlite3_finalize(state);
	retval = sqlite3_close(dnsa);
	return i;
}

int
dnsa_run_insert_sqlite(dnsa_config_s *config, dnsa_s *base, int type)
{
	const char *query, *file;
	int retval;
	sqlite3 *dnsa;
	sqlite3_stmt *state;

	retval = 0;
	query = dnsa_sql_insert[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READWRITE, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_search_sqlite");
	}
	if ((retval = dnsa_setup_insert_sqlite_bind(state, base, type)) != 0) {
		printf("Error binding result! %d\n", retval);
		sqlite3_close(dnsa);
		return retval;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(dnsa));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(dnsa);
		retval = SQLITE_INSERT_FAILED;
		return retval;
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(dnsa);
	return retval;
}

int
dnsa_run_update_sqlite(dnsa_config_s *config, dbdata_s *data, int type)
{
	const char *query, *file;
	int retval;
	unsigned int i;
	dbdata_s *list;
	sqlite3 *dnsa;
	sqlite3_stmt *state;

	retval = 0;
	list = data;
	query = dnsa_sql_update[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READWRITE, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_search_sqlite");
	}
	for (i = 1; i <= dnsa_update_args[type]; i++) {
		if (!list)
			break;
		if (dnsa_update_arg_type[type][i - 1] == DBTEXT) {
			if ((sqlite3_bind_text(state, (int)i, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
				fprintf(stderr, "Cannot bind arg\n");
				return retval;
			}
		} else if (dnsa_update_arg_type[type][i - 1] == DBINT) {
			if ((sqlite3_bind_int64(state, (int)i, (sqlite3_int64)list->args.number)) > 0) {
				fprintf(stderr, "Cannot bind arg\n");
				return retval;
			}
		}
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(dnsa));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(dnsa);
		retval = SQLITE_INSERT_FAILED;
		return retval;
	}
	if (retval == SQLITE_DONE) {
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(dnsa);
		return NONE;
	} else {
		sqlite3_finalize(state);
		sqlite3_close(dnsa);
		return retval;
	}
}

int
dnsa_run_delete_sqlite(dnsa_config_s *config, dbdata_s *data, int type)
{
	const char *query, *file;
	int retval;
	unsigned int i;
	dbdata_s *list;
	sqlite3 *dnsa;
	sqlite3_stmt *state;

	retval = 0;
	list = data;
	query = dnsa_sql_delete[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &dnsa, SQLITE_OPEN_READWRITE, NULL)) > 0) {
		report_error(FILE_O_FAIL, file);
	}
	if ((retval = sqlite3_prepare_v2(dnsa, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(dnsa);
		report_error(SQLITE_STATEMENT_FAILED, "dnsa_run_delete_sqlite");
	}
	for (i = 1; i <= dnsa_delete_args[type]; i++) {
		if (!list)
			break;
		if (dnsa_delete_arg_type[type][i - 1] == DBTEXT) {
			if ((sqlite3_bind_text(state, (int)i, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
				fprintf(stderr, "Cannot bind arg\n");
				return retval;
			}
		} else if (dnsa_delete_arg_type[type][i - 1] == DBINT) {
			if ((sqlite3_bind_int64(state, (int)i, (sqlite3_int64)list->args.number)) > 0) {
				fprintf(stderr, "Cannot bind arg\n");
				return retval;
			}
		}
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Received error: %s\n", sqlite3_errmsg(dnsa));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(dnsa);
		return NONE;
	}
	retval = sqlite3_changes(dnsa);
	sqlite3_finalize(state);
	sqlite3_close(dnsa);
	return retval;
}

int
dnsa_setup_insert_sqlite_bind(sqlite3_stmt *state, dnsa_s *base, int type)
{
	int retval;
	if (type == RECORDS)
		retval = dnsa_setup_bind_sqlite_records(state, base->records);
	else if (type == ZONES)
		retval = dnsa_setup_bind_sqlite_zones(state, base->zones);
	else if (type == REV_ZONES)
		retval = dnsa_setup_bind_sqlite_rev_zones(state, base->rev_zones);
	else if (type == REV_RECORDS)
		retval = dnsa_setup_bind_sqlite_rev_records(state, base->rev_records);
	else if (type == PREFERRED_AS)
		retval = dnsa_setup_bind_sqlite_prefer_a(state, base->prefer);
	else if (type == GLUES)
		retval = dnsa_setup_bind_sqlite_glue(state, base->glue);
	else
		retval = WRONG_TYPE;
	return retval;
}

int
dnsa_setup_bind_extended_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval;

	retval = 0;
	if (dnsa_ext_search_arg_type[type][i] == DBTEXT) {
		if ((retval = sqlite3_bind_text(
state, i + 1, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
			fprintf(stderr, "Cannot bind extended text arg %s\n",
				list->args.text);
			return retval;
		}
	} else if (dnsa_ext_search_arg_type[type][i] == DBINT) {
		if ((retval = sqlite3_bind_int64(
state, i + 1, (sqlite3_int64)list->args.number)) > 0) {
			fprintf(stderr, "Cannot bind extended number arg %lu\n",
				list->args.number);
			return retval;
		}
	}
	return retval;
}

int
dnsa_get_extended_results_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval, j, k;
	unsigned int u;
	dbdata_s *data;

	data = list;
	retval = 0;
	if (i > 0) {
		for (k = 1; k <= i; k++) {
			for (u = 1; u <= dnsa_extended_search_fields[type]; u++)
				if ((u != dnsa_extended_search_fields[type]) || (k != i))
					list = list->next;
		}
		for (j = 0; (unsigned)j < dnsa_extended_search_fields[type]; j++) {
			if (!(data = malloc(sizeof(dbdata_s))))
				report_error(MALLOC_FAIL, "data in get_ext_results_sqlite");
			init_dbdata_struct(data);
			if (dnsa_ext_search_field_type[type][j] == DBTEXT) {
				snprintf(data->fields.text, RBUFF_S, "%s", sqlite3_column_text(state, j));
			} else if (dnsa_ext_search_field_type[type][j] == DBINT) {
				data->fields.number = (unsigned long int)sqlite3_column_int64(state, j);
			}
			list->next = data;
			list = list->next;
		}
	} else {
		for (j = 0; (unsigned)j < dnsa_extended_search_fields[type]; j++) {
			if (dnsa_ext_search_field_type[type][j] == DBTEXT) {
				snprintf(list->fields.text, RBUFF_S, "%s", sqlite3_column_text(state, j));
			} else if (dnsa_ext_search_field_type[type][j] == DBINT) {
				list->fields.number = (unsigned long int)sqlite3_column_int64(state, j);
			}
			list = list->next;
		}
	}
	
	return retval;
}

int
dnsa_setup_bind_sqlite_records(sqlite3_stmt *state, record_row_s *record)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_int64(state, 1, (sqlite3_int64)record->zone)) > 0) {
		fprintf(stderr, "Cannot bind zone %lu\n", record->zone);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, record->host, (int)strlen(record->host), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind host %s\n", record->host);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, record->type, (int)strlen(record->type), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind type %s\n", record->type);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 4, (sqlite3_int64)record->pri)) > 0) {
		fprintf(stderr, "Cannot bind pri %lu\n", record->pri);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, record->dest, (int)strlen(record->dest), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind destination %s\n", record->dest);
		return retval;
	}
	return retval;
}

int
dnsa_setup_bind_sqlite_zones(sqlite3_stmt *state, zone_info_s *zone)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_text(
state, 1, zone->name, (int)strlen(zone->name), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind name %s\n", zone->name);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, zone->pri_dns, (int)strlen(zone->pri_dns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind primary dns %s\n", zone->pri_dns);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, zone->sec_dns, (int)strlen(zone->sec_dns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind primary dns %s\n", zone->sec_dns);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 4, (sqlite3_int64)zone->serial)) > 0) {
		fprintf(stderr, "Cannot bind serial %lu\n", zone->serial);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 5, (sqlite3_int64)zone->refresh)) > 0) {
		fprintf(stderr, "Cannot bind refresh %lu\n", zone->refresh);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 6, (sqlite3_int64)zone->retry)) > 0) {
		fprintf(stderr, "Cannot bind retry %lu\n", zone->retry);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 7, (sqlite3_int64)zone->expire)) > 0) {
		fprintf(stderr, "Cannot bind serial %lu\n", zone->expire);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 8, (sqlite3_int64)zone->ttl)) > 0) {
		fprintf(stderr, "Cannot bind serial %lu\n", zone->ttl);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 9, zone->type, (int)strlen(zone->type), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind type %s\n", zone->type);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 10, zone->master, (int)strlen(zone->master), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind master %s\n", zone->master);
		return retval;
	}
	return retval;
}

int
dnsa_setup_bind_sqlite_rev_zones(sqlite3_stmt *state, rev_zone_info_s *zone)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_text(
state, 1, zone->net_range, (int)strlen(zone->net_range), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind net_range %s\n", zone->net_range);
		return retval;
	}
	if ((retval = sqlite3_bind_int(state, 2, (int)zone->prefix)) > 0) {
		fprintf(stderr, "Cannot bind prefix %lu\n", zone->prefix);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, zone->net_start, (int)strlen(zone->net_start), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind net_start %s\n", zone->net_start);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, zone->net_finish, (int)strlen(zone->net_finish), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind net_finish %s\n", zone->net_finish);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 5, (sqlite3_int64)zone->start_ip)) > 0) {
		fprintf(stderr, "Cannot bind start_ip %lu\n", zone->start_ip);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 6, (sqlite3_int64)zone->end_ip)) > 0) {
		fprintf(stderr, "Cannot bind end_ip %lu\n", zone->end_ip);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 7, zone->pri_dns, (int)strlen(zone->pri_dns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind pri_dns %s\n", zone->pri_dns);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 8, zone->sec_dns, (int)strlen(zone->sec_dns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind sec_dns %s\n", zone->sec_dns);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 9, (sqlite3_int64)zone->serial)) > 0) {
		fprintf(stderr, "Cannot bind serial %lu\n", zone->serial);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 10, (sqlite3_int64)zone->refresh)) > 0) {
		fprintf(stderr, "Cannot bind refresh %lu\n", zone->refresh);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 11, (sqlite3_int64)zone->retry)) > 0) {
		fprintf(stderr, "Cannot bind retry %lu\n", zone->retry);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 12, (sqlite3_int64)zone->expire)) > 0) {
		fprintf(stderr, "Cannot bind expire %lu\n", zone->expire);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 13, (sqlite3_int64)zone->ttl)) > 0) {
		fprintf(stderr, "Cannot bind ttl %lu\n", zone->ttl);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 14, zone->type, (int)strlen(zone->type), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind type %s\n", zone->type);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 15, zone->master, (int)strlen(zone->master), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind master %s\n", zone->master);
		return retval;
	}
	return retval;
}

int
dnsa_setup_bind_sqlite_rev_records(sqlite3_stmt *state, rev_record_row_s *rev)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_int64(state, 1, (sqlite3_int64)rev->rev_zone)) > 0) {
		fprintf(stderr, "Cannot bind rev_zone %lu\n", rev->rev_zone);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, rev->host, (int)strlen(rev->host), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind host %s\n", rev->host);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, rev->dest, (int)strlen(rev->dest), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind dest %s\n", rev->dest);
		return retval;
	}
	return retval;
}

int
dnsa_setup_bind_sqlite_prefer_a(sqlite3_stmt *state, preferred_a_s *prefer)
{
	int retval= NONE;

	if ((retval = sqlite3_bind_text(
state, 1, prefer->ip, (int)strlen(prefer->ip), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind ip %s\n", prefer->ip);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 2, (sqlite3_int64)prefer->ip_addr)) > 0) {
		fprintf(stderr, "Cannot bind ip_addr %lu\n", prefer->ip_addr);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 3, (sqlite3_int64)prefer->record_id)) > 0) {
		fprintf(stderr, "Cannot bind record_id %lu\n", prefer->record_id);
		return retval;
	}
	if ((retval - sqlite3_bind_text(
state, 4, prefer->fqdn, (int)strlen(prefer->fqdn), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind fqdn %s\n", prefer->fqdn);
		return retval;
	}
	return retval;
}

int
dnsa_setup_bind_sqlite_glue(sqlite3_stmt *state, glue_zone_info_s *glue)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_text(
state, 1, glue->name, (int)strlen(glue->name), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind name %s\n", glue->name);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 2, (sqlite3_int64)glue->zone_id)) > 0) {
		fprintf(stderr, "Cannot bind zone_id %lu\n", glue->zone_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, glue->pri_dns, (int)strlen(glue->pri_dns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind pri_dns %s\n", glue->pri_dns);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, glue->sec_dns, (int)strlen(glue->sec_dns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind sec_dns %s\n", glue->sec_dns);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, glue->pri_ns, (int)strlen(glue->pri_ns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind pri_ns %s\n", glue->pri_ns);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 6, glue->sec_ns, (int)strlen(glue->sec_ns), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind sec_ns %s\n", glue->sec_ns);
		return retval;
	}
	return retval;
}

#endif /* HAVE_SQLITE3 */
