
/*
 *  cbc: create build config
 *  (C) 2015 Iain M Conochie <iain-AT-thargoid-DOT-co-DOT-uk>
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
 *  alisasql.c
 *  various definitions for SQL queries.
 */

#define THIS_SQL_MAX 2097152	// 2MegaBytes
#include <config.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"

/* 
 * We are defining 5 different types of query per program:
 * SELECT (all columns and rows in 1 table)
 * INSERT (insert full new row into table)
 * UPDATE (update row(s) based on data; we return no of rows affected)
 * DELETE (delete row(s) based on id)
 */

enum {
	AIL_SELECT = 0,
	AIL_INSERT = 1,
	AIL_UPDATE = 2,
	AIL_DELETE = 3,
};

// Programs
enum {
	CMDB = 0,
	DNSA = 1,
	CBC = 2
};

enum {
	NOPROGS = 3
};

// Various table variable definitions
const unsigned int sql_tables[] = { 8, 6, 17 };	// No of tables for each program

const unsigned int table_columns[] = {
// cmdb table columns
	9, 11, 3, 9, 3, 10, 12, 8,
// dnsa table columns
	11, 9, 13, 9, 23, 18,
// cbc table columns;
	13, 13, 10, 11, 7, 12, 4, 12, 8, 8, 7, 6, 8, 9, 6, 10, 7
};

const char *sql_table_list[] = {
// cmdb tables 0 - 7
	"contacts",
	"customer",
	"hard_type",
	"hardware",
	"service_type",
	"services",
	"server",
	"vm_server_hosts",
// dnsa tables 8 - 13
	"glue_zones",
	"preferred_a",
	"records",	// 10
	"rev_records",
	"rev_zones",
	"zones",
// cbc tables 14 - 30
	"build",
	"build_domain",
	"build_ip",
	"build_os",
	"build_type",
	"default_part",
	"disk_dev",	// 20
	"locale",
	"packages",
	"part_options",
	"seed_schemes",
	"system_package_args",
	"system_package_conf",
	"system_packages",
	"system_scrips",
	"system_scripts_args",
	"varient"	// 30
};

const char *sql_table_alias[] = {
// cmdb tables
	"con",
	"cus",
	"hdt",
	"hrd",
	"srt",
	"svc",
	"ser",
	"vsh",
// dnsa tables
	"glu",
	"pfa",
	"rec",
	"rrc",
	"rev",
	"zon",
// cbc tables
	"bld",
	"bdd",
	"bip",
	"bos",
	"bty",
	"dfp",
	"did",
	"loc",
	"pak",
	"prt",
	"sch",
	"spa",
	"spc",
	"syp",
	"sys",
	"sca",
	"var"
};

const char *sql_columns[] = {
// cmdb table columns : Starts at 0 :
	"cont_id", "name", "phone", "email", "cust_id", "cuser", "muser", "ctime", "mtime",
	"cust_id", "name", "address", "city", "county", "postcode", "coid", "cuser", "muser", "ctime", "mtime",
	"hard_type_id", "type", "class",
	"hard_id", "detail", "device", "server_id", "hard_type_id", "cuser", "muser", "ctime", "mtime",
	"service_type_id", "service", "detail",
	"service_id", "server_id", "cust_id", "service_type_id", "detail", "url",
	  "cuser", "muser", "ctime", "mtime",
	"server_id", "vendor", "make", "model", "uuid", "cust_id", "vm_server_id",
	  "name", "cuser", "muser", "ctime", "mtime",
	"vm_server_id", "vm_server", "type", "server_id", "cuser", "muser", "ctime", "mtime",
// dnsa table columns : Starts at 8 :
	"id", "name", "zone_id", "pri_dns", "sec_dns", "pri_ns", "sec_ns", "cuser", "muser", "ctime", "mtime",
	"prefa_id", "ip", "ip_addr", "record_id", "fqdn", "cuser", "muser", "ctime", "mtime",
	"id", "zone", "host", "type", "protocol", "service", "pri", "destination",
	  "valid", "cuser", "muser", "ctime", "mtime",		//10
	"rev_record_id", "rev_zone", "host", "destination", "valid", "cuser", "muser", "ctime", "mtime",
	"rev_zone_id", "net_range", "prefix", "net_start", "net_finish", "start_ip", "finish_ip", "pri_dns", 
	  "sec_dns", "serial", "refresh", "retry", "expire", "ttl", "valid", "owner", "updated", "type",
	  "master", "cuser", "muser", "ctime", "mtime",
	"id", "name", "pri_dns", "sec_dns", "serial", "refresh", "retry", "expire", "ttl", "valid", "owner",
	  "updated", "type", "master", "cuser", "muser", "ctime", "mtime",
// cbc table colums : Starts at 14 :
	"build_id", "mac_addr", "varient_id", "net_inst_int", "server_id", "os_id", "ip_id", "locale_id",
	  "def_scheme_id", "cuser", "muser", "ctime", "mtime",
	"bd_id", "start_ip", "end_ip", "netmask", "gateway", "ns", "domain", "ntp_server", "config_ntp",
	  "cuser", "muser", "ctime", "mtime",
	"ip_id", "ip", "hostname", "domainname", "bd_id", "server_id", "cuser", "muser", "ctime", "mtime",
	"os_id", "os", "os_version", "alias", "ver_alias", "arch", "bt_id", "cuser", "muser", "ctime", "mtime",
	"bt_id", "alias", "build_type", "arg", "url", "mirror", "boot_line",
	"def_part_id", "minimum", "maximum", "priority", "mount_point", "filesystem", "def_scheme_id",
	  "logical_volume", "cuser", "muser", "ctime", "mtime",
	"disk_id", "server_id", "device", "lvm",		// 20
	"locale_id", "locale", "country", "language", "keymap", "os_id",
	  "bt_id", "timezone", "cuser", "muser", "ctime", "mtime",
	"pack_id", "package", "varient_id", "os_id", "cuser", "muser", "ctime", "mtime",
	"part_options_id", "def_part_id", "def_scheme_id", "poption", "cuser", "muser", "ctime", "mtime",
	"def_scheme_id", "scheme_name", "lvm", "cuser", "muser", "ctime", "mtime",
	"syspack_id", "name", "cuser", "muser", "ctime", "mtime",
	"syspack_arg_id", "syspack_id", "field", "type", "cuser", "muser", "ctime", "mtime",
	"syspack_conf_id", "syspack_arg_id", "syspack_id", "bd_id", "arg", "cuser", "muser", "ctime", "mtime",
	"systscr_id", "name", "cuser", "muser", "ctime", "mtime",
	"systscr_arg_id", "systscr_id", "bd_id", "bt_id", "arg", "no", "cuser", "muser", "ctime", "mtime",
	"varient_id", "varient", "valias", "cuser", "muser", "ctime",
	  "mtime"						// 30
};

/*
 * SQL INSERTS
 *
 * We assume that we are inserting to all columns apart from id, ctime, and
 * mtime
 * There are some tables which do not have [c|m]user columns. These are listed
 * below and referenced by their sql_table_list[] no.
 *
 * The first digit in the array short_inserts tells us the number of tables
 * with no [c|m]user columns.
 */

const unsigned int short_inserts[] = {
	4, 2, 4, 18, 20
};

/*
 * SQL UPDATES
 *
 * We have various update statements. We will have variables for:
 *   table name
 *   no of column fields
 *   no of static updates in the query
 *   column names to update
 *   static names to update
 *   static value to update to
 *
 * We assume there is only 1 argument and this is the 'id' column for the
 *  table which will be the first column name.
 */

const unsigned int sql_updates[] = {
	7, 7, 12
};

const unsigned int update_tables[] = {
	6, 1, 6, 6, 6, 6, 6,
	13, 13, 13, 13, 12, 12, 13,
	15, 14, 14, 14, 14, 14, 14, 14, 15, 30, 24, 15
};

const unsigned int sql_static = 6;	// length of the below array
const unsigned int sql_statics[] =  { 7, 8, 9, 11, 13, 22 };

const unsigned int update_fields[] = {
// cmdb update fields
	1, 1, 1, 1, 1, 1, 1,
// dnsa update fields
	1, 1, 0, 2, 1, 2, 0,
// cbc update fields
	1, 1, 1, 1, 2, 2, 2, 3, 2, 1, 1, 1
};

// Only expect 1 argument
const unsigned int update_arg_column[] = {
	0, 0, 0, 0, 0, 0, 0,
	0 ,0 ,0 ,0 ,0, 0, 0,
	0, 4, 4, 4, 4, 4, 4, 4, 0, 0, 0, 0
};

const unsigned int static_update_fields[] = {
// cmdb static fields
	0, 0, 0, 0, 0, 0, 0,
// dnsa static fields
	2, 2, 1, 0, 2, 0, 1,
// cbc static fields
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
};

const unsigned int update_field_columns[][5] = {
// cmdb
	{ 9, 0, 0, 0, 0 },
	{ 6, 0, 0, 0, 0 },
	{ 4, 0, 0, 0, 0 },
	{ 2, 0, 0, 0, 0 },
	{ 3, 0, 0, 0, 0 },
	{ 1, 0, 0, 0, 0 },
	{ 5, 0, 0, 0, 0 },
// dnsa
	{ 15, 0, 0, 0, 0 },
	{ 15, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
	{ 4, 15, 0, 0, 0 },
	{ 20, 0, 0, 0, 0 },
	{ 9, 20, 0, 0, 0 },
	{ 0, 0, 0, 0, 0 },
// cbc
	{ 7, 0, 0, 0, 0 },
	{ 2, 0, 0, 0, 0 },
	{ 5, 0, 0, 0, 0 },
	{ 8, 0, 0, 0, 0 },
	{ 2, 5, 0, 0, 0 },
	{ 2, 8, 0, 0, 0 },
	{ 5, 8, 0, 0, 0 },
	{ 2, 5, 8, 0, 0 },
	{ 7, 10, 0, 0, 0 },
	{ 4, 0, 0, 0, 0 },
	{ 4, 0, 0, 0, 0 },
	{ 10, 0, 0, 0, 0 }
};

const unsigned int static_update_columns[][2] = {
	{ 9, 11 },
	{ 9, 11 },
	{ 11, 0 },
	{ 14, 16 },
	{ 9, 0 },
	{ 8, 0 }
};

const char *static_update_values[][2] = {
	{ "yes", "no" },
	{ "unknown", "yes" },
	{ "no", NULL },
	{ "yes", "no" },
	{ "no", NULL },
	{ "1", NULL }
};

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


char *
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

char *
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

char *
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

char *
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

