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
#include "base_sql.h"
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
	if (type == BOOT_LINE) {
		*query = sql_select[BOOT_LINES];
		*fields = select_fields[BOOT_LINES];
	} else if (type == BUILD) {
		*query = sql_select[BUILDS];
		*fields = select_fields[BUILDS];
	} else if (type == BUILD_DOMAIN) {
		*query = sql_select[BUILD_DOMAINS];
		*fields = select_fields[BUILD_DOMAINS];
	} else if (type == BUILD_IP) {
		*query = sql_select[BUILD_IPS];
		*fields = select_fields[BUILD_IPS];
	} else if (type == BUILD_OS) {
		*query = sql_select[BUILD_OSS];
		*fields = select_fields[BUILD_OSS];
	} else if (type == BUILD_TYPE) {
		*query = sql_select[BUILD_TYPES];
		*fields = select_fields[BUILD_TYPES];
	} else if (type == DISK_DEV) {
		*query = sql_select[DISK_DEVS];
		*fields = select_fields[DISK_DEVS];
	} else if (type == LOCALE) {
		*query = sql_select[LOCALES];
		*fields = select_fields[LOCALES];
	} else if (type == BPACKAGE) {
		*query = sql_select[BPACKAGES];
		*fields = select_fields[BPACKAGES];
	} else if (type == DPART) {
		*query = sql_select[DPARTS];
		*fields = select_fields[DPARTS];
	} else if (type == SPART) {
		*query = sql_select[SPARTS];
		*fields = select_fields[SPARTS];
	} else if (type == SSCHEME) {
		*query = sql_select[SSCHEMES];
		*fields = select_fields[SSCHEMES];
	} else if (type == CSERVER) {
		*query = sql_select[CSERVERS];
		*fields = select_fields[CSERVERS];
	} else if (type == VARIENT) {
		*query = sql_select[VARIENTS];
		*fields = select_fields[VARIENTS];
	} else if (type == VMHOST) {
		*query = sql_select[VMHOSTS];
		*fields = select_fields[VMHOSTS];
	} else {
		fprintf(stderr, "Unknown query type %d\n", type);
		retval = 1;
	}
	return retval;
}

#ifdef HAVE_MYSQL

void
cbc_mysql_init(cbc_config_t *dc, MYSQL *cbc_mysql)
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
	cbc_mysql_init(config, &cbc);
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
	cbc_mysql_init(config, &cbc);
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
		case BUILD:
			if (fields != select_fields[BUILDS])
				break;
			store_build_mysql(row, base);
			break;
		case BUILD_DOMAIN:
			if (fields != select_fields[BUILD_DOMAINS])
				break;
			store_build_domain_mysql(row, base);
			break;
		case BUILD_IP:
			if (fields != select_fields[BUILD_IPS])
				break;
			store_build_ip_mysql(row, base);
			break;
		case BUILD_OS:
			if (fields != select_fields[BUILD_OSS])
				break;
			store_build_os_mysql(row, base);
			break;
		case BUILD_TYPE:
			if (fields != select_fields[BUILD_TYPES])
				break;
			store_build_type_mysql(row, base);
			break;
		case DISK_DEV:
			if (fields != select_fields[DISK_DEVS])
				break;
			store_disk_dev_mysql(row, base);
			break;
		case LOCALE:
			if (fields != select_fields[LOCALES])
				break;
			store_locale_mysql(row, base);
			break;
		case BPACKAGE:
			if (fields != select_fields[BPACKAGES])
				break;
			store_package_mysql(row, base);
			break;
		case DPART:
			if (fields != select_fields[DPARTS])
				break;
			store_dpart_mysql(row, base);
			break;
		case SPART:
			if (fields != select_fields[SPARTS])
				break;
			store_spart_mysql(row, base);
			break;
		case SSCHEME:
			if (fields != select_fields[SSCHEMES])
				break;
			store_seed_scheme_mysql(row, base);
			break;
		case CSERVER:
			if (fields != select_fields[CSERVERS])
				break;
			store_server_mysql(row, base);
			break;
		case VARIENT:
			if (fields != select_fields[VARIENTS])
				break;
			store_varient_mysql(row, base);
			break;
		case VMHOST:
			if (fields != select_fields[VMHOSTS])
				break;
			store_vmhost_mysql(row, base);
			break;
		default:
			fprintf(stderr, "Unknown type for storing %d\n",  type);
			break;
	}
}

int
setup_insert_mysql_bind(MYSQL_BIND *mybind, unsigned int i, int type, cbc_t *base)
{
	return NONE;
}

void
store_boot_line_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_boot_line_t *boot, *list;

	if (!(boot = malloc(sizeof(cbc_boot_line_t))))
		report_error(MALLOC_FAIL, "boot in store_boot_line_mysql");
	init_boot_line(boot);
	boot->boot_id = strtoul(row[0], NULL, 10);
	snprintf(boot->os, MAC_S, "%s", row[1]);
	snprintf(boot->os_ver, MAC_S, "%s", row[2]);
	boot->bt_id = strtoul(row[3], NULL, 10);
	snprintf(boot->boot_line, RBUFF_S, "%s", row[4]);
	list = base->bootl;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = boot;
	} else {
		base->bootl = boot;
	}
}

void
store_build_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_build_t *build, *list;

	if (!(build = malloc(sizeof(cbc_build_t))))
		report_error(MALLOC_FAIL, "build in store_build_mysql");
	init_build_struct(build);
	build->build_id = strtoul(row[0], NULL, 10);
	snprintf(build->mac_addr, TYPE_S, "%s", row[1]);
	build->varient_id = strtoul(row[2], NULL, 10);
	snprintf(build->net_int, RANGE_S, "%s", row[3]);
	build->server_id = strtoul(row[4], NULL, 10);
	build->os_id = strtoul(row[5], NULL, 10);
	build->boot_id = strtoul(row[6], NULL, 10);
	build->ip_id = strtoul(row[7], NULL, 10);
	build->locale_id = strtoul(row[8], NULL, 10);
	list = base->build;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = build;
	} else {
		base->build = build;
	}
}

void
store_build_domain_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_build_domain_t *dom, *list;

	if (!(dom = malloc(sizeof(cbc_build_domain_t))))
		report_error(MALLOC_FAIL, "dom in store_build_domain_mysql");
	init_build_domain(dom);
	dom->bd_id = strtoul(row[0], NULL, 10);
	dom->start_ip = strtoul(row[1], NULL, 10);
	dom->end_ip = strtoul(row[2], NULL, 10);
	dom->netmask = strtoul(row[3], NULL, 10);
	dom->gateway = strtoul(row[4], NULL, 10);
	dom->ns = strtoul(row[5], NULL, 10);
	snprintf(dom->domain, RBUFF_S, "%s", row[6]);
	snprintf(dom->country, RANGE_S, "%s", row[7]);
	snprintf(dom->language, RANGE_S, "%s", row[8]);
	snprintf(dom->keymap, RANGE_S, "%s", row[9]);
	if (strncmp("0", row[11], CH_S) == 0) {
		dom->config_ntp = 0;
	} else {
		snprintf(dom->ntp_server, HOST_S, "%s", row[10]);
		dom->config_ntp = 1;
	}
	if (strncmp("0", row[17], CH_S) == 0) {
		dom->config_ldap = 0;
	} else {
		snprintf(dom->ldap_server, URL_S, "%s", row[12]);
		snprintf(dom->ldap_url, URL_S, "%s", row[13]);
		snprintf(dom->ldap_dn, URL_S, "%s", row[15]);
		snprintf(dom->ldap_bind, URL_S, "%s", row[16]);
		dom->config_ldap = 1;
	}
	if (strncmp("0", row[14], CH_S) == 0)
		dom->ldap_ssl = 0;
	else
		dom->ldap_ssl = 1;
	if (strncmp("0", row[19], CH_S) == 0) {
		dom->config_log = 0;
	} else {
		snprintf(dom->log_server, CONF_S, "%s", row[18]);
		dom->config_log = 1;
	}
	if (strncmp("0", row[21], CH_S) == 0) {
		dom->config_email = 0;
	} else {
		snprintf(dom->smtp_server, CONF_S, "%s", row[20]);
		dom->config_email = 1;
	}
	if (strncmp("0", row[23], CH_S) == 0) {
		dom->config_xymon = 0;
	} else {
		snprintf(dom->xymon_server, CONF_S, "%s", row[22]);
		dom->config_xymon = 1;
	}
	snprintf(dom->nfs_domain, CONF_S, "%s", row[24]);
	list = base->bdom;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = dom;
	} else {
		base->bdom = dom;
	}
}

void
store_build_ip_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_build_ip_t *ip, *list;

	if (!(ip = malloc(sizeof(cbc_build_ip_t))))
		report_error(MALLOC_FAIL, "ip in store_build_ip_mysql");
	init_build_ip(ip);
	ip->ip_id = strtoul(row[0], NULL, 10);
	ip->ip = strtoul(row[1], NULL, 10);
	snprintf(ip->host, MAC_S, "%s", row[2]);
	snprintf(ip->domain, RBUFF_S, "%s", row[3]);
	ip->bd_id = strtoul(row[4], NULL, 10);
	list = base->bip;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = ip;
	} else {
		base->bip = ip;
	}
}

void
store_build_os_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_build_os_t *os, *list;

	if (!(os = malloc(sizeof(cbc_build_os_t))))
		report_error(MALLOC_FAIL, "os in store_build_os_mysql");
	init_build_os(os);
	os->os_id = strtoul(row[0], NULL, 10);
	snprintf(os->os, MAC_S, "%s", row[1]);
	snprintf(os->version, MAC_S, "%s", row[2]);
	snprintf(os->alias, MAC_S, "%s", row[3]);
	snprintf(os->ver_alias, MAC_S, "%s", row[4]);
	snprintf(os->arch, MAC_S, "%s", row[5]);
	os->boot_id = strtoul(row[6], NULL, 10);
	os->bt_id = strtoul(row[7], NULL, 10);
	list = base->bos;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = os;
	} else {
		base->bos = os;
	}
}

void
store_build_type_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_build_type_t *type, *list;

	if (!(type = malloc(sizeof(cbc_build_type_t))))
		report_error(MALLOC_FAIL, "type in store_build_type_mysql");
	init_build_type(type);
	type->bt_id = strtoul(row[0], NULL, 10);
	snprintf(type->alias, MAC_S, "%s", row[1]);
	snprintf(type->build_type, MAC_S, "%s", row[2]);
	snprintf(type->arg, RANGE_S, "%s", row[3]);
	snprintf(type->url, CONF_S, "%s", row[4]);
	snprintf(type->mirror, CONF_S, "%s", row[5]);
	list = base->btype;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = type;
	} else {
		base->btype = type;
	}
}

void
store_disk_dev_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_disk_dev_t *disk, *list;

	if (!(disk = malloc(sizeof(cbc_disk_dev_t))))
		report_error(MALLOC_FAIL, "disk in store_disk_dev_mysql");
	init_disk_dev(disk);
	disk->disk_id = strtoul(row[0], NULL, 10);
	disk->server_id = strtoul(row[1], NULL, 10);
	snprintf(disk->device, HOST_S, "%s", row[2]);
	if (strncmp(row[3], "0", CH_S) == 0)
		disk->lvm = 0;
	else
		disk->lvm = 1;
	list = base->diskd;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = disk;
	} else {
		base->diskd = disk;
	}
}

void
store_locale_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_locale_t *loc, *list;

	if (!(loc = malloc(sizeof(cbc_locale_t))))
		report_error(MALLOC_FAIL, "loc in store_locale_mysql");
	init_locale(loc);
	loc->locale_id = strtoul(row[0], NULL, 10);
	snprintf(loc->locale, MAC_S, "%s", row[1]);
	snprintf(loc->country, RANGE_S, "%s", row[2]);
	snprintf(loc->language, RANGE_S, "%s", row[3]);
	snprintf(loc->keymap, RANGE_S, "%s", row[4]);
	loc->os_id = strtoul(row[5], NULL, 10);
	loc->bt_id = strtoul(row[6], NULL, 10);
	snprintf(loc->timezone, HOST_S, "%s", row[7]);
	list = base->locale;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = loc;
	} else {
		base->locale = loc;
	}
}

void
store_package_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_package_t *pack, *list;

	if (!(pack = malloc(sizeof(cbc_package_t))))
		report_error(MALLOC_FAIL, "pack in store_package_mysql");
	init_package(pack);
	pack->pack_id = strtoul(row[0], NULL, 10);
	snprintf(pack->package, HOST_S, "%s", row[1]);
	pack->vari_id = strtoul(row[2], NULL, 10);
	pack->os_id = strtoul(row[3], NULL, 10);
	list = base->package;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = pack;
	} else {
		base->package = pack;
	}
}

void
store_dpart_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_pre_part_t *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_t))))
		report_error(MALLOC_FAIL, "part in store_dpart_mysql");
	init_pre_part(part);
	part->id.def_part_id = strtoul(row[0], NULL, 10);
	part->min= strtoul(row[1], NULL, 10);
	part->max = strtoul(row[2], NULL, 10);
	part->pri = strtoul(row[3], NULL, 10);
	snprintf(part->mount, HOST_S, "%s", row[4]);
	snprintf(part->fs, RANGE_S, "%s", row[5]);
	part->link_id.def_scheme_id = strtoul(row[6], NULL, 10);
	snprintf(part->log_vol, MAC_S, "%s", row[7]);
	list = base->dpart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->dpart = part;
	}
}

void
store_spart_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_pre_part_t *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_t))))
		report_error(MALLOC_FAIL, "part in store_dpart_mysql");
	init_pre_part(part);
	part->id.part_id = strtoul(row[0], NULL, 10);
	part->min= strtoul(row[1], NULL, 10);
	part->max = strtoul(row[2], NULL, 10);
	part->pri = strtoul(row[3], NULL, 10);
	snprintf(part->mount, HOST_S, "%s", row[4]);
	snprintf(part->fs, RANGE_S, "%s", row[5]);
	part->link_id.server_id = strtoul(row[6], NULL, 10);
	snprintf(part->log_vol, MAC_S, "%s", row[7]);
	list = base->spart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->spart = part;
	}
}

void
store_seed_scheme_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_seed_scheme_t *seed, *list;

	if (!(seed = malloc(sizeof(cbc_seed_scheme_t))))
		report_error(MALLOC_FAIL, "seed in store_seed_scheme_mysql");
	init_seed_scheme(seed);
	seed->def_scheme_id = strtoul(row[0], NULL, 10);
	snprintf(seed->name, CONF_S, "%s", row[1]);
	if (strncmp(row[2], "0", CH_S) == 0)
		seed->lvm = 0;
	else
		seed->lvm = 1;
	list = base->sscheme;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = seed;
	} else {
		base->sscheme = seed;
	}
}

void
store_server_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_server_t *server, *list;

	if (!(server = malloc(sizeof(cbc_server_t))))
		report_error(MALLOC_FAIL, "server in store_server_mysql");
	init_cbc_server(server);
	server->server_id = strtoul(row[0], NULL, 10);
	snprintf(server->vendor, CONF_S, "%s", row[1]);
	snprintf(server->make, CONF_S, "%s", row[2]);
	snprintf(server->model, CONF_S, "%s", row[3]);
	snprintf(server->uuid, CONF_S, "%s", row[4]);
	server->cust_id = strtoul(row[5], NULL, 10);
	server->vm_server_id = strtoul(row[6], NULL, 10);
	snprintf(server->name, MAC_S, "%s", row[7]);
	list = base->server;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = server;
	} else {
		base->server = server;
	}
}

void
store_varient_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_varient_t *vari, *list;

	if (!(vari = malloc(sizeof(cbc_varient_t))))
		report_error(MALLOC_FAIL, "vari in store_varient_mysql");
	init_varient(vari);
	vari->varient_id = strtoul(row[0], NULL, 10);
	snprintf(vari->varient, HOST_S, "%s", row[1]);
	snprintf(vari->valias, MAC_S, "%s", row[2]);
	list = base->varient;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vari;
	} else {
		base->varient = vari;
	}
}

void
store_vmhost_mysql(MYSQL_ROW row, cbc_t *base)
{
	cbc_vm_server_hosts *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cbc_vm_server_hosts))))
		report_error(MALLOC_FAIL, "vmhost in store_vmhost_mysql");
	init_vm_hosts(vmhost);
	vmhost->vm_s_id = strtoul(row[0], NULL, 10);
	snprintf(vmhost->vm_server, RBUFF_S, "%s", row[1]);
	snprintf(vmhost->type, HOST_S, "%s", row[2]);
	vmhost->server_id = strtoul(row[4], NULL, 10);
	list = base->vmhost;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	}
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
		report_error(FILE_O_FAIL, file);
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
		report_error(FILE_O_FAIL, file);
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
		case BUILD:
			if (fields != select_fields[BUILDS])
				break;
			store_build_sqlite(state, base);
			break;
		case BUILD_DOMAIN:
			if (fields != select_fields[BUILD_DOMAINS])
				break;
			store_build_domain_sqlite(state, base);
			break;
		case BUILD_IP:
			if (fields != select_fields[BUILD_IPS])
				break;
			store_build_ip_sqlite(state, base);
			break;
		case BUILD_OS:
			if (fields != select_fields[BUILD_OSS])
				break;
			store_build_os_sqlite(state, base);
			break;
		case BUILD_TYPE:
			if (fields != select_fields[BUILD_TYPES])
				break;
			store_build_type_sqlite(state, base);
			break;
		case DISK_DEV:
			if (fields != select_fields[DISK_DEVS])
				break;
			store_disk_dev_sqlite(state, base);
			break;
		case LOCALE:
			if (fields != select_fields[LOCALES])
				break;
			store_locale_sqlite(state, base);
			break;
		case BPACKAGE:
			if (fields != select_fields[BPACKAGES])
				break;
			store_package_sqlite(state, base);
			break;
		case DPART:
			if (fields != select_fields[DPARTS])
				break;
			store_dpart_sqlite(state, base);
			break;
		case SPART:
			if (fields != select_fields[SPARTS])
				break;
			store_spart_sqlite(state, base);
			break;
		case SSCHEME:
			if (fields != select_fields[SSCHEMES])
				break;
			store_seed_scheme_sqlite(state, base);
			break;
		case CSERVER:
			if (fields != select_fields[CSERVERS])
				break;
			store_server_sqlite(state, base);
			break;
		case VARIENT:
			if (fields != select_fields[VARIENTS])
				break;
			store_varient_sqlite(state, base);
			break;
		case VMHOST:
			if (fields != select_fields[VMHOSTS])
				break;
			store_vmhost_sqlite(state, base);
			break;
		default:
			fprintf(stderr, "Unknown type for storing %d\n",  type);
			break;
	}
}

int
setup_insert_sqlite_bind(sqlite3_stmt *state, cbc_t *base, int type)
{
	return NONE;
}

void
store_boot_line_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_boot_line_t *boot, *list;

	if (!(boot = malloc(sizeof(cbc_boot_line_t))))
		report_error(MALLOC_FAIL, "boot in store_boot_line_sqlite");
	init_boot_line(boot);
	boot->boot_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(boot->os, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(boot->os_ver, MAC_S, "%s", sqlite3_column_text(state, 2));
	boot->bt_id = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(boot->boot_line, MAC_S, "%s", sqlite3_column_text(state, 4));
	list = base->bootl;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = boot;
	} else {
		base->bootl = boot;
	}
}

void
store_build_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_build_t *build, *list;

	if (!(build = malloc(sizeof(cbc_build_t))))
		report_error(MALLOC_FAIL, "build in store_build_sqlite");
	init_build_struct(build);
	build->build_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(build->mac_addr, TYPE_S, "%s", sqlite3_column_text(state, 1));
	build->varient_id = (unsigned long int) sqlite3_column_int64(state, 2);
	snprintf(build->net_int, RANGE_S, "%s", sqlite3_column_text(state, 3));
	build->server_id = (unsigned long int) sqlite3_column_int64(state, 4);
	build->os_id = (unsigned long int) sqlite3_column_int64(state, 5);
	build->boot_id = (unsigned long int) sqlite3_column_int64(state, 6);
	build->ip_id = (unsigned long int) sqlite3_column_int64(state, 7);
	build->locale_id = (unsigned long int) sqlite3_column_int64(state, 8);
	list = base->build;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = build;
	} else {
		base->build = build;
	}
}

void
store_build_domain_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_build_domain_t *dom, *list;

	if (!(dom = malloc(sizeof(cbc_build_domain_t))))
		report_error(MALLOC_FAIL, "dom in store_build_domain_sqlite");
	init_build_domain(dom);
	dom->bd_id = (unsigned long int) sqlite3_column_int64(state, 0);
	dom->start_ip = (unsigned long int) sqlite3_column_int64(state, 1);
	dom->end_ip = (unsigned long int) sqlite3_column_int64(state, 2);
	dom->netmask = (unsigned long int) sqlite3_column_int64(state, 3);
	dom->gateway = (unsigned long int) sqlite3_column_int64(state, 4);
	dom->ns = (unsigned long int) sqlite3_column_int64(state, 5);
	snprintf(dom->domain, RBUFF_S, "%s", sqlite3_column_text(state, 6));
	snprintf(dom->country, RBUFF_S, "%s", sqlite3_column_text(state, 7));
	snprintf(dom->language, RBUFF_S, "%s", sqlite3_column_text(state, 8));
	snprintf(dom->keymap, RBUFF_S, "%s", sqlite3_column_text(state, 9));
	if ((dom->config_ntp = (short int) sqlite3_column_int(state, 11)) != 0)
		snprintf(dom->ntp_server, RBUFF_S, "%s",
		 sqlite3_column_text(state, 10));
	if ((dom->config_ldap = (short int) sqlite3_column_int(state, 17)) != 0) {
		snprintf(dom->ldap_server, URL_S, "%s", 
			 sqlite3_column_text(state, 12));
		snprintf(dom->ldap_url, URL_S, "%s", 
			 sqlite3_column_text(state, 13));
		snprintf(dom->ldap_dn, URL_S, "%s",
			 sqlite3_column_text(state, 15));
		snprintf(dom->ldap_bind, URL_S, "%s",
			 sqlite3_column_text(state, 16));
	}
	dom->ldap_ssl = (short int) sqlite3_column_int(state, 14);
	if ((dom->config_log = (short int) sqlite3_column_int(state, 19)) != 0)
		snprintf(dom->log_server, CONF_S, "%s",
			 sqlite3_column_text(state, 18));
	if ((dom->config_email = (short int) sqlite3_column_int(state, 21)) != 0)
		snprintf(dom->smtp_server, CONF_S, "%s",
			 sqlite3_column_text(state, 20));
	if ((dom->config_xymon = (short int) sqlite3_column_int(state, 23)) != 0)
		snprintf(dom->xymon_server, CONF_S, "%s",
			 sqlite3_column_text(state, 22));
	snprintf(dom->nfs_domain, CONF_S, "%s",
		 sqlite3_column_text(state, 24));
	list = base->bdom;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = dom;
	} else {
		base->bdom = dom;
	}
}

void
store_build_ip_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_build_ip_t *ip, *list;

	if (!(ip = malloc(sizeof(cbc_build_ip_t))))
		report_error(MALLOC_FAIL, "ip in store_build_ip_sqlite");
	init_build_ip(ip);
	ip->ip_id = (unsigned long int) sqlite3_column_int64(state, 0);
	ip->ip = (unsigned long int) sqlite3_column_int64(state, 1);
	snprintf(ip->host, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(ip->domain, RBUFF_S, "%s", sqlite3_column_text(state, 3));
	ip->bd_id = (unsigned long int) sqlite3_column_int64(state, 4);
	list = base->bip;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = ip;
	} else {
		base->bip = ip;
	}
}

void
store_build_os_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_build_os_t *os, *list;

	if (!(os = malloc(sizeof(cbc_build_os_t))))
		report_error(MALLOC_FAIL, "os in store_build_os_sqlite");
	init_build_os(os);
	os->os_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(os->os, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(os->version, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(os->alias, MAC_S, "%s", sqlite3_column_text(state, 3));
	snprintf(os->ver_alias, MAC_S, "%s", sqlite3_column_text(state, 4));
	snprintf(os->arch, MAC_S, "%s", sqlite3_column_text(state, 5));
	os->boot_id = (unsigned long int) sqlite3_column_int64(state, 6);
	os->bt_id = (unsigned long int) sqlite3_column_int64(state, 7);
	list = base->bos;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = os;
	} else {
		base->bos = os;
	}
}

void
store_build_type_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_build_type_t *type, *list;

	if (!(type = malloc(sizeof(cbc_build_type_t))))
		report_error(MALLOC_FAIL, "type in store_build_type_sqlite");
	init_build_type(type);
	type->bt_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(type->alias, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(type->build_type, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(type->arg, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(type->url, CONF_S, "%s", sqlite3_column_text(state, 4));
	snprintf(type->mirror, RBUFF_S, "%s", sqlite3_column_text(state, 5));
	list = base->btype;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = type;
	} else {
		base->btype = type;
	}
}

void
store_disk_dev_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_disk_dev_t *disk, *list;

	if (!(disk = malloc(sizeof(cbc_disk_dev_t))))
		report_error(MALLOC_FAIL, "disk in store_disk_dev_sqlite");
	init_disk_dev(disk);
	disk->disk_id = (unsigned long int) sqlite3_column_int64(state, 0);
	disk->server_id = (unsigned long int) sqlite3_column_int64(state, 1);
	snprintf(disk->device, HOST_S, "%s", sqlite3_column_text(state, 2));
	disk->lvm = (short int) sqlite3_column_int(state, 3);
	list = base->diskd;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = disk;
	} else {
		base->diskd = disk;
	}
}

void
store_locale_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_locale_t *loc, *list;

	if (!(loc = malloc(sizeof(cbc_locale_t))))
		report_error(MALLOC_FAIL, "loc in store_locale_sqlite");
	init_locale(loc);
	loc->locale_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(loc->locale, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(loc->country, RANGE_S, "%s", sqlite3_column_text(state, 2));
	snprintf(loc->language, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(loc->keymap, RANGE_S, "%s", sqlite3_column_text(state, 4));
	loc->os_id = (unsigned long int) sqlite3_column_int64(state, 5);
	loc->bt_id = (unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(loc->timezone, HOST_S, "%s", sqlite3_column_text(state, 7));
	list = base->locale;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = loc;
	} else {
		base->locale = loc;
	}
}

void
store_package_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_package_t *pack, *list;

	if (!(pack = malloc(sizeof(cbc_package_t))))
		report_error(MALLOC_FAIL, "pack in store_package_sqlite");
	init_package(pack);
	pack->pack_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(pack->package, HOST_S, "%s", sqlite3_column_text(state, 1));
	pack->vari_id = (unsigned long int) sqlite3_column_int64(state, 2);
	pack->os_id = (unsigned long int) sqlite3_column_int64(state, 3);
	list = base->package;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = pack;
	} else {
		base->package = pack;
	}
}

void
store_dpart_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_pre_part_t *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_t))))
		report_error(MALLOC_FAIL, "part in store_dpart_sqlite");
	init_pre_part(part);
	part->id.def_part_id = 
		(unsigned long int) sqlite3_column_int64(state, 0);
	part->min = (unsigned long int) sqlite3_column_int64(state, 1);
	part->max = (unsigned long int) sqlite3_column_int64(state, 2);
	part->pri = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(part->mount, HOST_S, "%s",
		 sqlite3_column_text(state, 4));
	snprintf(part->fs, RANGE_S, "%s", 
		 sqlite3_column_text(state, 5));
	part->link_id.def_scheme_id = 
		(unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(part->log_vol, MAC_S, "%s", 
		 sqlite3_column_text(state, 7));
	list = base->dpart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->dpart = part;
	}
}

void
store_spart_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_pre_part_t *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_t))))
		report_error(MALLOC_FAIL, "part in store_dpart_sqlite");
	init_pre_part(part);
	part->id.part_id = 
		(unsigned long int) sqlite3_column_int64(state, 0);
	part->min = (unsigned long int) sqlite3_column_int64(state, 1);
	part->max = (unsigned long int) sqlite3_column_int64(state, 2);
	part->pri = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(part->mount, HOST_S, "%s",
		 sqlite3_column_text(state, 4));
	snprintf(part->fs, RANGE_S, "%s", 
		 sqlite3_column_text(state, 5));
	part->link_id.server_id = 
		(unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(part->log_vol, MAC_S, "%s", 
		 sqlite3_column_text(state, 7));
	list = base->spart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->spart = part;
	}
}

void
store_seed_scheme_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_seed_scheme_t *seed, *list;

	if (!(seed = malloc(sizeof(cbc_seed_scheme_t))))
		report_error(MALLOC_FAIL, "seed in store_seed_scheme_sqlite");
	init_seed_scheme(seed);
	seed->def_scheme_id = 
	   (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(seed->name, CONF_S, "%s",
		 sqlite3_column_text(state, 1));
	seed->lvm = (short int) sqlite3_column_int(state, 2);
	list = base->sscheme;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = seed;
	} else {
		base->sscheme = seed;
	}
}

void
store_server_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_server_t *server, *list;

	if (!(server = malloc(sizeof(cbc_server_t))))
		report_error(MALLOC_FAIL, "server in store_server_sqlite");
	init_cbc_server(server);
	server->server_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(server->vendor, CONF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(server->make, CONF_S, "%s", sqlite3_column_text(state, 2));
	snprintf(server->model, CONF_S, "%s", sqlite3_column_text(state, 3));
	snprintf(server->uuid, CONF_S, "%s", sqlite3_column_text(state, 4));
	server->cust_id = (unsigned long int) sqlite3_column_int64(state, 5);
	server->vm_server_id = (unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(server->name, MAC_S, "%s", sqlite3_column_text(state, 7));
	list = base->server;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = server;
	} else {
		base->server = server;
	}
}

void
store_varient_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_varient_t *vari, *list;

	if (!(vari = malloc(sizeof(cbc_varient_t))))
		report_error(MALLOC_FAIL, "vari in store_varient_sqlite");
	init_varient(vari);
	vari->varient_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(vari->varient, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vari->valias, MAC_S, "%s", sqlite3_column_text(state, 2));
	list = base->varient;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vari;
	} else {
		base->varient = vari;
	}
}

void
store_vmhost_sqlite(sqlite3_stmt *state, cbc_t *base)
{
	cbc_vm_server_hosts *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cbc_vm_server_hosts))))
		report_error(MALLOC_FAIL, "vmhost in store_vmhost_mysql");
	init_vm_hosts(vmhost);
	vmhost->vm_s_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(vmhost->vm_server, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vmhost->type, HOST_S, "%s", sqlite3_column_text(state, 2));
	vmhost->server_id = (unsigned long int) sqlite3_column_int64(state, 3);
	list = base->vmhost;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	}
}

#endif
