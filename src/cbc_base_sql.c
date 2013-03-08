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
INSERT INTO boot_line (boot_id, os, os_ver, bt_id, boot_line) VALUES (?, ?,\
 ?, ?, ?)","\
INSERT INTO build (build_id, mac_addr, varient_id, net_inst_int, server_id, \
 os_id, boot_id, ip_id, locale_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_domain (bd_id, start_ip, end_ip, netmask, gateway, ns,\
 domain, country, language, keymap, ntp_server, config_ntp, ldap_server,\
 ldap_url, ldap_ssl, ldap_dn, ldap_bind, config_ldap, log_server, config_log,\
 smtp_server, config_email, xymon_server, config_xymon, nfs_domain) VALUES (\
 ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_ip (ip_id, ip, hostname, domainname, bd_id) VALUES (?, ?, ?,\
 ?, ?)","\
INSERT INTO build_os (os_id, os, os_version, alias, ver_alias, arch, boot_id,\
 bt_id) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_type (bt_id, alias, build_type, arg, url, mirror) VALUES (\
 ?, ?, ?, ?, ?, ?)","\
INSERT INTO disk_dev (disk_id, server_id, device, lvm) VALUES (?, ?, ?, ?)","\
INSERT INTO locale (locale_id, locale, country, language, keymap, os_id,\
 bt_id, timezone) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO packages (pack_id, package, varient_id, os_id) VALUES (?, ?, ?, ?)\
","\
INSERT INTO default_part (def_part_id, minimum, maximum, priority,\
mount_point, filesystem, def_scheme_id, logical_volume) VALUES (?, ?, ?, ?, ?\
 ?, ?, ?)","\
INSERT INTO seed_part (part_id, minimum, maximum, priority, mount_point,\
 filesystem, server_id, logical_volume) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO seed_schemes (def_scheme_id, scheme_name, lvm) VALUES (?, ?, ?)","\
INSERT INTO server (server_id, vendor, make, model, uuid, cust_id,\
 vm_server_id, name) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO varient (varient_id, varient, valias) VALUES (?, ?, ?)","\
INSERT INTO vm_server_hosts (vm_server_id, vm_server, type, server_id) VALUES\
 (?, ?, ?, ?)"
};

#ifdef HAVE_MYSQL

const int mysql_inserts[][25] = {
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG,
  MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  MYSQL_TYPE_STRING} ,
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_LONG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG,
  MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_SHORT, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING,
  MYSQL_TYPE_STRING, MYSQL_TYPE_LONG, MYSQL_TYPE_LONG, MYSQL_TYPE_STRING,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
{MYSQL_TYPE_LONG, MYSQL_TYPE_STRING, MYSQL_TYPE_STRING, MYSQL_TYPE_LONG,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

#endif /* HAVE_MYSQL */

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

#endif /* HAVE_MYSQL */
