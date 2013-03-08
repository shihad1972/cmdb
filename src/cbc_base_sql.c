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
 *  cbc_base_sql.c:
 *
 *  Contains functions which will fill up data structs based on the parameters
 *  supplied. Will also contian conditional code base on database type.
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
#include "cbc_base_sql.h"
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

/**
 * These SQL searches require the cbc_t struct. Each search will fill one of
 * the structs pointed to within cbc_t.
 * The stucts within cbc_t will be malloc'ed by the database store function so
 * only cbc_t needs to be malloc'ed and initialised.
 * These searches return multiple members.
 * Helper functions need to be created for each search to populate the member
 * of cbc_t used.
 */
const char *sql_select[] = { "\
SELECT boot_id, os, os_ver, bt_id, boot_line FROM boot_line","\
SELECT build_id, mac_addr, varient_id, net_inst_int, server_id, os_id,\
 boot_id, ip_id, locale_id FROM build","\
SELECT bd_id, start_ip, end_ip, netmask, gateway, ns, domain, country,\
 language, keymap, ntp_server, config_ntp, ldap_server, ldap_url, ldap_ssl,\
 ldap_dn, ldap_bind, config_ldap, log_server, config_log, smtp_server,\
 config_email, xymon_server, config_xymon, nfs_domain FROM build_domain","\
SELECT ip_id, ip, hostname, domainname, bd_id FROM build_ip","\
SELECT os_id, os, os_version, alias, ver_alias, arch, boot_id, bt_id FROM\
 build_os","\
SELECT bt_id, alias, build_type, arg, url, mirror FROM build_type","\
SELECT disk_id, server_id, device, lvm FROM disk_dev","\
SELECT locale_id, locale, country, language, keymap, os_id, bt_id, timezone\
 FROM locale","\
SELECT pack_id, package, varient_id, os_id FROM packages","\
SELECT def_part_id, minimum, maximum, priority, mount_point, filesystem,\
 def_scheme_id, logical_volume FROM default_part","\
SELECT part_id, minimum, maximum, priority, mount_point, filesystem,\
 server_id, logical_volume FROM seed_part","\
SELECT def_scheme_id, scheme_name, lvm FROM seed_schemes","\
SELECT server_id, vendor, make, model, uuid, cust_id, vm_server_id, name\
 FROM server","\
SELECT varient_id, varient, valias FROM varient","\
SELECT vm_server_id, vm_server, type, server_id FROM vm_server_hosts"
};

const char *sql_insert[] = { "\
INSERT INTO boot_line (os, os_ver, bt_id, boot_line) VALUES (?, ?,\
 ?, ?, ?)","\
INSERT INTO build (mac_addr, varient_id, net_inst_int, server_id, \
 os_id, boot_id, ip_id, locale_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_domain (start_ip, end_ip, netmask, gateway, ns,\
 domain, country, language, keymap, ntp_server, config_ntp, ldap_server,\
 ldap_url, ldap_ssl, ldap_dn, ldap_bind, config_ldap, log_server, config_log,\
 smtp_server, config_email, xymon_server, config_xymon, nfs_domain) VALUES (\
 ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_ip (ip, hostname, domainname, bd_id) VALUES (?, ?, ?,\
 ?, ?)","\
INSERT INTO build_os (os, os_version, alias, ver_alias, arch, boot_id,\
 bt_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_type (alias, build_type, arg, url, mirror) VALUES (\
 ?, ?, ?, ?, ?, ?)","\
INSERT INTO disk_dev (server_id, device, lvm) VALUES (?, ?, ?, ?)","\
INSERT INTO locale (locale, country, language, keymap, os_id,\
 bt_id, timezone) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO packages (package, varient_id, os_id) VALUES (?, ?, ?, ?)\
","\
INSERT INTO default_part (minimum, maximum, priority,\
mount_point, filesystem, def_scheme_id, logical_volume) VALUES (?, ?, ?, ?, ?\
 ?, ?, ?)","\
INSERT INTO seed_part (minimum, maximum, priority, mount_point,\
 filesystem, server_id, logical_volume) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO seed_schemes (scheme_name, lvm) VALUES (?, ?, ?)","\
INSERT INTO server (vendor, make, model, uuid, cust_id,\
 vm_server_id, name) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO varient (varient, valias) VALUES (?, ?, ?)","\
INSERT INTO vm_server_hosts (vm_server, type, server_id) VALUES\
 (?, ?, ?, ?)"
};

#ifdef HAVE_MYSQL

const int mysql_inserts[][24] = {
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG,
  MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  MYSQL_TYPE_STRING} ,
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_LONG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

#endif /* HAVE_MYSQL */

const unsigned int select_fields[] = { 5, 9, 25, 5, 8, 6, 4, 8, 4, 8, 8, 3, 8,
  3, 4 };
const unsigned int insert_fields[] = { 4, 8, 24, 4, 7, 5, 3, 7, 3, 7, 7, 2, 7,
  2, 3 };
int
run_query(cbc_config_t *config, cbc_t *base, int type)
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
run_multiple_query(cbc_config_t *config, cbc_t *base, int type)
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
run_insert(cbc_config_t *config, cbc_t *base, int type)
{
	int retval;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0)) {
		fprintf(stderr, "No database type configured\n");
		return NO_DB_TYPE;
#ifdef HAVE_MYSQL
	} else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0)) {
		retval = run_insert_mysql(config, base, type);
		return retval;
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	} else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0)) {
		retval = run_insert_sqlite(config, base, type);
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
		case BOOT_LINE:
			*query = sql_select[BOOT_LINES];
			*fields = select_fields[BOOT_LINES];
			break;
		case BUILD:
			*query = sql_select[BUILDS];
			*fields = select_fields[BUILDS];
			break;
		case BUILD_DOMAIN:
			*query = sql_select[BUILD_DOMAINS];
			*fields = select_fields[BUILD_DOMAINS];
			break;
		case BUILD_IP:
			*query = sql_select[BUILD_IPS];
			*fields = select_fields[BUILD_IPS];
			break;
		case BUILD_OS:
			*query = sql_select[BUILD_OSS];
			*fields = select_fields[BUILD_OSS];
			break;
		case BUILD_TYPE:
			*query = sql_select[BUILD_TYPES];
			*fields = select_fields[BUILD_TYPES];
			break;
		case DISK_DEV:
			*query = sql_select[DISK_DEVS];
			*fields = select_fields[DISK_DEVS];
			break;
		case LOCALE:
			*query = sql_select[LOCALES];
			*fields = select_fields[LOCALES];
			break;
		case BPACKAGE:
			*query = sql_select[BPACKAGES];
			*fields = select_fields[BPACKAGES];
			break;
		case DPART:
			*query = sql_select[DPARTS];
			*fields = select_fields[DPARTS];
			break;
		case SPART:
			*query = sql_select[SPARTS];
			*fields = select_fields[SPARTS];
			break;
		case SSCHEME:
			*query = sql_select[SSCHEMES];
			*fields = select_fields[SSCHEMES];
			break;
		case CSERVER:
			*query = sql_select[CSERVERS];
			*fields = select_fields[CSERVERS];
			break;
		case VARIENT:
			*query = sql_select[VARIENTS];
			*fields = select_fields[VARIENTS];
			break;
		case VMHOST:
			*query = sql_select[VMHOSTS];
			*fields = select_fields[VMHOSTS];
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
cmdb_mysql_init(cbc_config_t *dc, MYSQL *cbc_mysql)
{
	const char *unix_socket;
	
	unix_socket = dc->socket;
	
	if (!(mysql_init(cbc_mysql))) {
		report_error(MY_INIT_FAIL, mysql_error(cbc_mysql));
	}
	if (!(mysql_real_connect(cbc_mysql, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag)))
		report_error(MY_CONN_FAIL, mysql_error(cbc_mysql));
}

int
run_query_mysql(cbc_config_t *config, cbc_t *base, int type)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	const char *query;
	int retval;
	unsigned int fields;

	retval = 0;
	cmdb_mysql_init(config, &cbc);
	if ((retval = get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = cmdb_mysql_query_with_checks(&cbc, query)) != 0) {
		fprintf(stderr, "Query failed with error code %d\n", retval);
		return retval;
	}
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_cleanup(&cbc);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	fields = mysql_num_fields(cbc_res);
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)) {
		cmdb_mysql_cleanup_full(&cbc, cbc_res);
		report_error(NO_SERVERS, "run_query_mysql");
	}
	while ((cbc_row = mysql_fetch_row(cbc_res)))
		store_result_mysql(cbc_row, base, type, fields);
	cmdb_mysql_cleanup_full(&cbc, cbc_res);
	return NONE;
}

int
run_insert_mysql(cbc_config_t *config, cbc_t *base, int type)
{
	MYSQL cbc;
	MYSQL_STMT *cbc_stmt;
	MYSQL_BIND my_bind[insert_fields[type]];
	const char *query;
	int retval;
	unsigned int i;

	retval = 0;
	memset(my_bind, 0, sizeof(my_bind));
	for (i = 0; i < insert_fields[type]; i++)
		if ((retval = setup_insert_mysql_bind(&my_bind[i], i, type, base)) != 0)
			return retval;
	query = sql_insert[type];
	cmdb_mysql_init(config, &cbc);
	if (!(cbc_stmt = mysql_stmt_init(&cbc)))
		return MY_STATEMENT_FAIL;
	if ((retval = mysql_stmt_prepare(cbc_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_bind_param(cbc_stmt, &my_bind[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_execute(cbc_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));

	mysql_stmt_close(cbc_stmt);
	cmdb_mysql_cleanup(&cbc);

	return retval;
}

int
run_multiple_query_mysql(cbc_config_t *config, cbc_t *base, int type)
{
	int retval;
	retval = NONE;
	if (type & BOOT_LINE)
		if ((retval = run_query_mysql(config, base, BOOT_LINE)) != 0)
			return retval;
	if (type & BUILD)
		if ((retval = run_query_mysql(config, base, BUILD)) != 0)
			return retval;
	if (type & BUILD_DOMAIN)
		if ((retval = run_query_mysql(config, base, BUILD_DOMAIN)) != 0)
			return retval;
	if (type & BUILD_IP)
		if ((retval = run_query_mysql(config, base, BUILD_IP)) != 0)
			return retval;
	if (type & BUILD_OS)
		if ((retval = run_query_mysql(config, base, BUILD_OS)) != 0)
			return retval;
	if (type & BUILD_TYPE)
		if ((retval = run_query_mysql(config, base, BUILD_TYPE)) != 0)
			return retval;
	if (type & DISK_DEV)
		if ((retval = run_query_mysql(config, base, DISK_DEV)) != 0)
			return retval;
	if (type & LOCALE)
		if ((retval = run_query_mysql(config, base, LOCALE)) != 0)
			return retval;
	if (type & BPACKAGE)
		if ((retval = run_query_mysql(config, base, BPACKAGE)) != 0)
			return retval;
	if (type & DPART)
		if ((retval = run_query_mysql(config, base, DPART)) != 0)
			return retval;
	if (type & SPART)
		if ((retval = run_query_mysql(config, base, SPART)) != 0)
			return retval;
	if (type & SSCHEME)
		if ((retval = run_query_mysql(config, base, SSCHEME)) != 0)
			return retval;
	if (type & CSERVER)
		if ((retval = run_query_mysql(config, base, CSERVER)) != 0)
			return retval;
	if (type & VARIENT)
		if ((retval = run_query_mysql(config, base, VARIENT)) != 0)
			return retval;
	if (type & VMHOST)
		if ((retval = run_query_mysql(config, base, VMHOST)) != 0)
			return retval;
	return retval;
}

void
store_result_mysql(MYSQL_ROW row, cbc_t *base, int type, unsigned int fields)
{
	switch(type) {
		case BOOT_LINE:
			if (fields != select_fields[BOOT_LINES])
				break;
			store_boot_line_mysql(row, base);
			break;
		default:
			fprintf(stderr, "Unknown type for storing %d\n",  type);
			break;
	}
}

void
store_boot_line_mysql(MYSQL_ROW row, cbc_t *base)
{
	int retval;
	cbc_boot_line_t *boot, *list;
	
	if (!(boot = malloc(sizeof(cbc_boot_line_t))))
		report_error(MALLOC_FAIL, "boot in store_boot_line_mysql");
	init_boot_line(boot);
	boot->boot_id = strtoul(row[0], NULL, 10);
	snprintf(boot->os, MAC_S, "%s", row[1]);
	snprintf(boot->os_ver, MAC_S, "%s", row[2]);
	boot->bt_id = strtoul(row[3], NULL, 10);
	snprintf(boot->boot_line, RBUFF_S, "%s", row[4]);
	
}

#endif /* HAVE_MYSQL */

#ifdef HAVE_SQLITE3

int
run_query_sqlite(cbc_config_t *config, cbc_t *base, int type)
{
	const char *query, *file;
	int retval;
	unsigned int fields;
	sqlite3 *cbc;
	sqlite3_stmt *state;
	
	retval = 0;
	file = config->file;
	if ((retval = get_query(type, &query, &fields)) != 0) {
		fprintf(stderr, "Unable to get query. Error code %d\n", retval);
		return retval;
	}
	if ((retval = sqlite3_open_v2(file, &cbc, SQLITE_OPEN_READONLY, NULL)) > 0) {
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(cbc, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cbc);
		report_error(SQLITE_STATEMENT_FAILED, "run_query_sqlite");
	}
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		store_result_sqlite(state, base, type, fields);
	
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cbc);
	
	return NONE;
}

int
run_insert_sqlite(cbc_config_t *config, cbc_t *base, int type)
{
	const char *query, *file;
	int retval;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	retval = 0;
	query = sql_insert[type];
	file = config->file;
	if ((retval = sqlite3_open_v2(file, &cbc, SQLITE_OPEN_READWRITE, NULL)) > 0) {
		report_error(CANNOT_OPEN_FILE, file);
	}
	if ((retval = sqlite3_prepare_v2(cbc, query, BUFF_S, &state, NULL)) > 0) {
		retval = sqlite3_close(cbc);
		report_error(SQLITE_STATEMENT_FAILED, "run_search_sqlite");
	}
	if ((retval = setup_insert_sqlite_bind(state, base, type)) != 0) {
		printf("Error binding result! %d\n", retval);
		sqlite3_close(cbc);
		return retval;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cbc));
		retval = sqlite3_finalize(state);
		retval = sqlite3_close(cbc);
		retval = SQLITE_INSERT_FAILED;
		return retval;
	}
	retval = sqlite3_finalize(state);
	retval = sqlite3_close(cbc);
	return retval;
}

int
run_multiple_query_sqlite(cbc_config_t *config, cbc_t *base, int type)
{
	int retval;
	retval = NONE;
	if (type & BOOT_LINE)
		if ((retval = run_query_sqlite(config, base, BOOT_LINE)) != 0)
			return retval;
	if (type & BUILD)
		if ((retval = run_query_sqlite(config, base, BUILD)) != 0)
			return retval;
	if (type & BUILD_DOMAIN)
		if ((retval = run_query_sqlite(config, base, BUILD_DOMAIN)) != 0)
			return retval;
	if (type & BUILD_IP)
		if ((retval = run_query_sqlite(config, base, BUILD_IP)) != 0)
			return retval;
	if (type & BUILD_OS)
		if ((retval = run_query_sqlite(config, base, BUILD_OS)) != 0)
			return retval;
	if (type & BUILD_TYPE)
		if ((retval = run_query_sqlite(config, base, BUILD_TYPE)) != 0)
			return retval;
	if (type & DISK_DEV)
		if ((retval = run_query_sqlite(config, base, DISK_DEV)) != 0)
			return retval;
	if (type & LOCALE)
		if ((retval = run_query_sqlite(config, base, LOCALE)) != 0)
			return retval;
	if (type & BPACKAGE)
		if ((retval = run_query_sqlite(config, base, BPACKAGE)) != 0)
			return retval;
	if (type & DPART)
		if ((retval = run_query_sqlite(config, base, DPART)) != 0)
			return retval;
	if (type & SPART)
		if ((retval = run_query_sqlite(config, base, SPART)) != 0)
			return retval;
	if (type & SSCHEME)
		if ((retval = run_query_sqlite(config, base, SSCHEME)) != 0)
			return retval;
	if (type & CSERVER)
		if ((retval = run_query_sqlite(config, base, CSERVER)) != 0)
			return retval;
	if (type & VARIENT)
		if ((retval = run_query_sqlite(config, base, VARIENT)) != 0)
			return retval;
	if (type & VMHOST)
		if ((retval = run_query_sqlite(config, base, VMHOST)) != 0)
			return retval;
	return retval;
}

void
store_result_sqlite(sqlite3_stmt *state, cbc_t *base, int type, unsigned int fields)
{
	switch(type) {
		case BOOT_LINE:
			if (fields != select_fields[BOOT_LINES])
				break;
			store_boot_line_sqlite(state, base);
			break;
		default:
			fprintf(stderr, "Unknown type for storing %d\n",  type);
			break;
	}
}

#endif