/* 
 *
 *  cbc: Create Build Config
 *  Copyright (C) 2013 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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

#include <config.h>
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
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

/**
 * These SQL searches require the cbc_s struct. Each search will fill one of
 * the structs pointed to within cbc_s.
 * The stucts within cbc_s will be malloc'ed by the database store function so
 * only cbc_s needs to be malloc'ed and initialised.
 * These searches return multiple members.
 * Helper functions need to be created for each search to populate the member
 * of cbc_s used.
 */
const char *cbc_sql_select[] = { "\
SELECT boot_id, os, os_ver, bt_id, boot_line FROM boot_line","\
SELECT build_id, mac_addr, varient_id, net_inst_int, server_id, os_id,\
 ip_id, locale_id, def_scheme_id, cuser, muser, ctime, mtime FROM build","\
SELECT bd_id, start_ip, end_ip, netmask, gateway, ns, domain,\
 ntp_server, config_ntp, cuser, muser, ctime, mtime FROM build_domain","\
SELECT ip_id, ip, hostname, domainname, bd_id, server_id, cuser, muser, \
 ctime, mtime FROM build_ip","\
SELECT os_id, os, os_version, alias, ver_alias, arch, bt_id, cuser, muser, \
 ctime, mtime FROM build_os ORDER BY alias, os_version","\
SELECT bt_id, alias, build_type, arg, url, mirror, boot_line FROM build_type\
 ORDER BY alias","\
SELECT disk_id, server_id, device, lvm FROM disk_dev","\
SELECT locale_id, locale, country, language, keymap, timezone, name,\
 cuser, muser, ctime, mtime FROM locale","\
SELECT pack_id, package, varient_id, os_id, cuser, muser, ctime, mtime FROM \
 packages","\
SELECT def_part_id, minimum, maximum, priority, mount_point, filesystem,\
 def_scheme_id, logical_volume, cuser, muser, ctime, mtime FROM default_part","\
SELECT def_scheme_id, scheme_name, lvm, cuser, muser, ctime, mtime FROM \
 seed_schemes","\
SELECT server_id, vendor, make, model, uuid, cust_id, vm_server_id, name, \
 cuser, muser, ctime, mtime FROM server","\
SELECT varient_id, varient, valias, cuser, muser, ctime, mtime FROM varient","\
SELECT vm_server_id, vm_server, type, server_id, cuser, muser, ctime, mtime \
 FROM vm_server_hosts","\
SELECT syspack_id, name, cuser, muser, ctime, mtime FROM system_packages","\
SELECT syspack_arg_id, syspack_id, field, type, cuser, muser, ctime, mtime \
 FROM system_package_args","\
SELECT syspack_conf_id, syspack_arg_id, syspack_id, bd_id, arg, cuser, \
 muser, ctime, mtime FROM system_package_conf","\
SELECT systscr_id, name, cuser, muser, ctime, mtime FROM system_scripts","\
SELECT systscr_arg_id, systscr_id, bd_id, bt_id, arg, no, cuser, muser, ctime, \
 mtime FROM system_scripts_args ORDER BY bd_id, bt_id, systscr_id, no","\
SELECT part_options_id, def_part_id, def_scheme_id, poption, cuser, muser, \
 ctime, mtime FROM part_options ORDER BY def_scheme_id, def_part_id"
};

const char *cbc_sql_insert[] = { "\
INSERT INTO boot_line (os, os_ver, bt_id, boot_line) VALUES (?, ?,\
 ?, ?)","\
INSERT INTO build (mac_addr, varient_id, net_inst_int, server_id, \
 os_id, ip_id, locale_id, def_scheme_id, cuser, muser) VALUES \
(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_domain (start_ip, end_ip, netmask, gateway, ns,\
 domain, ntp_server, config_ntp, cuser, muser) VALUES (\
 ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_ip (ip, hostname, domainname, bd_id, server_id, cuser, \
 muser) VALUES  (?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_os (os, os_version, alias, ver_alias, arch,\
 bt_id, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO build_type (alias, build_type, arg, url, mirror, boot_line) VALUES\
 (?, ?, ?, ?, ?, ?)","\
INSERT INTO disk_dev (server_id, device, lvm) VALUES (?, ?, ?)","\
INSERT INTO locale (locale, country, language, keymap, name, \
 timezone, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO packages (package, varient_id, os_id, cuser, muser) VALUES \
 (?, ?, ?, ?, ?)","\
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, \
 def_scheme_id, logical_volume, cuser, muser) \
 VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO seed_schemes (scheme_name, lvm, cuser, muser) VALUES (?, ?, ?, ?)","\
INSERT INTO server (vendor, make, model, uuid, cust_id, vm_server_id, name, \
cuser, muser)  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO varient (varient, valias, cuser, muser) VALUES (?, ?, ?, ?)","\
INSERT INTO vm_server_hosts (vm_server, type, server_id, cuser, muser) VALUES\
 (?, ?, ?, ?, ?)","\
INSERT INTO system_packages (name, cuser, muser) VALUES (?, ?, ?)","\
INSERT INTO system_package_args (syspack_id, field, type, cuser, muser) \
 VALUES (?, ?, ?, ?, ?)","\
INSERT INTO system_package_conf (syspack_arg_id, syspack_id, bd_id, arg, \
 cuser, muser) VALUES (?, ?, ?, ?, ?, ?)","\
INSERT INTO system_scripts (name, cuser, muser) VALUES (?, ?, ?)","\
INSERT INTO system_scripts_args(systscr_id, bd_id, bt_id, arg, no, cuser, \
 muser) VALUES (?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO part_options(def_part_id, def_scheme_id, poption, cuser, muser) \
 VALUES (?, ?, ?, ?, ?)"
};

const char *cbc_sql_update[] = { "\
UPDATE build_domain SET ntp_server = ? WHERE domain = ?","\
UPDATE build_domain SET ntp_server = ? WHERE bd_id = ?","\
UPDATE build SET varient_id = ? WHERE server_id = ?","\
UPDATE build SET os_id = ? WHERE server_id = ?","\
UPDATE build SET def_scheme_id = ? WHERE server_id = ?","\
UPDATE build SET varient_id = ?, os_id = ? WHERE server_id = ?","\
UPDATE build SET varient_id = ?, def_scheme_id = ? WHERE server_id = ?","\
UPDATE build SET os_id = ?, def_scheme_id = ? WHERE server_id = ?","\
UPDATE build SET varient_id = ?, os_id = ?, def_scheme_id = ? WHERE server_id\
  = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_bind = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_server = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_ssl = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_bind = ?, muser = ? \
  WHERE bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_server = ?, \
  muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_ssl = ?, muser = ? \
  WHERE  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_bind = ?, ldap_server = ?, \
  muser = ? WHERE  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_bind = ?, ldap_ssl = ?, \
  muser = ? WHERE  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_server = ?, ldap_ssl = ?, \
  muser = ? WHERE  bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_bind = ?, \
  ldap_server = ?, muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_server = ?, \
  ldap_ssl = ?, muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_bind = ?, \
  ldap_ssl = ?, muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_bind = ?, ldap_server = ?, \
  ldap_ssl = ?, muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET config_ldap = 1, ldap_dn = ?, ldap_bind = ?, \
  ldap_server = ?, ldap_ssl = ?, muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET nfs_domain = ?, muser = ? WHERE bd_id = ?","\
UPDATE build_domain SET config_ntp = 1, ntp_server = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE build_domain SET config_email = 1, smtp_server = ?, muser = ? \
  WHERE bd_id = ?","\
UPDATE build_domain SET config_log = 1, log_server = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE build_domain SET config_xymon = 1, xymon_server = ?, muser = ? WHERE \
  bd_id = ?","\
UPDATE varient SET muser = ? WHERE varient_id = ?","\
UPDATE seed_schemes SET muser = ? WHERE def_scheme_id = ?","\
UPDATE build_domain SET muser = ? WHERE bd_id = ?","\
UPDATE default_locale SET locale_id = ?"
};

const char *cbc_sql_delete[] = { "\
DELETE FROM build_domain WHERE domain = ?","\
DELETE FROM build_domain WHERE bd_id = ?","\
DELETE FROM build_os WHERE os_id = ?","\
DELETE FROM varient WHERE varient_id = ?","\
DELETE FROM packages WHERE pack_id = ?","\
DELETE FROM build_ip WHERE server_id = ?","\
DELETE FROM build WHERE server_id = ?","\
DELETE FROM disk_dev WHERE server_id = ?","\
DELETE FROM seed_schemes WHERE def_scheme_id = ?","\
DELETE FROM default_part WHERE def_part_id = ?","\
DELETE FROM default_part WHERE def_scheme_id = ?","\
DELETE FROM system_packages WHERE syspack_id = ?","\
DELETE FROM system_package_args WHERE syspack_arg_id = ?","\
DELETE FROM system_package_conf WHERE syspack_conf_id = ?","\
DELETE FROM system_scripts WHERE systscr_id = ?","\
DELETE FROM system_scripts_args WHERE systscr_arg_id = ?","\
DELETE FROM part_options WHERE part_options_id = ?","\
DELETE FROM locale WHERE locale_id = ?"
};

const char *cbc_sql_search[] = {
/* Start at 0 */ "\
SELECT config_ldap, ldap_ssl, ldap_server, ldap_dn, ldap_bind FROM\
 build_domain WHERE domain = ?","\
SELECT config_ldap, ldap_ssl, ldap_server, ldap_dn, ldap_bind FROM\
 build_domain WHERE bd_id = ?","\
SELECT COUNT(*) c FROM build_domain WHERE domain = ?","\
SELECT alias, ver_alias, os_version, arch FROM build_os WHERE os = ?","\
SELECT DISTINCT alias FROM build_os WHERE os = ?","\
SELECT bt_id FROM build_type WHERE alias = ?","\
SELECT os_id FROM build_os WHERE os = ? AND os_version = ? AND arch = ?","\
SELECT os_id FROM build_os WHERE alias = ? AND os_version = ? AND arch = ?","\
SELECT build_id FROM build WHERE os_id = ?","\
SELECT name FROM server s, build b WHERE b.os_id = ?\
 AND b.server_id = s.server_id"
/* 10 */,"\
SELECT varient_id FROM varient WHERE varient = ?","\
SELECT varient_id FROM varient WHERE valias = ?","\
SELECT os_id FROM build_os WHERE os = ?","\
SELECT os_id FROM build_os WHERE alias = ?","\
SELECT os_id FROM build_os WHERE os = ? AND version = ?","\
SELECT os_id, varient_id FROM packages WHERE package = ?","\
SELECT name from server s, build b WHERE s.server_id = b.server_id","\
SELECT b.mac_addr, bi.ip, bd.domain FROM server s \
  LEFT JOIN build b ON b.server_id = s.server_id \
  LEFT JOIN build_ip bi ON b.ip_id = bi.ip_id \
  LEFT JOIN build_domain bd ON bi.bd_id = bd.bd_id WHERE s.server_id = ?","\
SELECT server_id FROM server WHERE uuid = ?","\
SELECT server_id FROM server WHERE name = ?"
/* 20 */,"\
SELECT name FROM server WHERE server_id = ?","\
SELECT bt.boot_line, bo.alias, bo.os_version, l.country, l.locale, l.keymap, \
  bt.arg, bt.url, bo.arch, b.net_inst_int FROM build_type bt \
  LEFT JOIN build_os bo ON bo.alias=bt.alias \
  LEFT JOIN build b ON b.os_id = bo.os_id \
  LEFT JOIN locale l ON l.locale_id = b.locale_id WHERE b.server_id = ?","\
SELECT l.locale, l.keymap, b.net_inst_int, bi.ip, bd.ns, \
  bd.netmask, bd.gateway, bi.hostname, bd.domain, l.language FROM build b \
  LEFT JOIN build_ip bi ON b.ip_id = bi.ip_id \
  LEFT JOIN build_os bo ON b.os_id = bo.os_id \
  LEFT JOIN build_domain bd ON bd.bd_id = bi.bd_id \
  LEFT JOIN locale l ON b.locale_id = l.locale_id WHERE b.server_id = ?","\
SELECT mirror, bo.ver_alias, bo.alias, l.country, bd.config_ntp, bd.ntp_server\
  , bo.arch FROM build_type bt LEFT JOIN build_os bo ON bo.alias = bt.alias \
  LEFT JOIN build b ON b.os_id = bo.os_id \
  LEFT JOIN locale l ON l.locale_id = b.locale_id \
  LEFT JOIN build_ip bi ON b.ip_id = bi.ip_id \
  LEFT JOIN build_domain bd ON bi.bd_id = bd.bd_id WHERE b.server_id = ?","\
SELECT d.device, ss.lvm FROM disk_dev d LEFT JOIN build b on b.server_id = d.server_id \
  LEFT JOIN seed_schemes ss ON ss.def_scheme_id = b.def_scheme_id WHERE d.server_id = ?","\
SELECT priority, minimum, maximum, filesystem, logical_volume, mount_point \
  FROM default_part dp LEFT JOIN build b ON b.def_scheme_id=dp.def_scheme_id \
  WHERE b.server_id = ?","\
SELECT package FROM packages p \
  LEFT JOIN build b ON b.varient_id = p.varient_id \
  AND b.os_id = p.os_id WHERE server_id = ?","\
SELECT bd.config_ldap, bd.ldap_server, bd.ldap_ssl, bd.ldap_dn, bd.ldap_bind \
  FROM build_domain bd LEFT JOIN build_ip bi on bi.bd_id = bd.bd_id \
  LEFT JOIN build b ON b.ip_id = bi.ip_id \
  WHERE b.server_id = ?","\
SELECT bd.config_xymon, bd.xymon_server, bd.domain FROM build_domain bd \
  LEFT JOIN build_ip bi on bi.bd_id = bd.bd_id \
  LEFT JOIN build b ON b.ip_id = bi.ip_id \
  WHERE b.server_id = ?","\
SELECT bd.config_email, bd.smtp_server, bd.domain, bi.ip FROM build_domain bd \
  LEFT JOIN build_ip bi on bi.bd_id = bd.bd_id \
  LEFT JOIN build b ON b.ip_id = bi.ip_id \
  WHERE b.server_id = ?"
/* 30 */,"\
SELECT ip FROM build_ip WHERE bd_id = ?"
/* This hard codes the network device to be hard_type_id 1
 * and disk device to be 2 */,"\
SELECT detail, device FROM hardware WHERE server_id = ? AND hard_type_id = 1 \
  ORDER BY device","\
SELECT device FROM hardware WHERE server_id = ? AND hard_type_id = 2 \
  ORDER BY device","\
SELECT ip FROM build_ip WHERE server_id = ?","\
SELECT build_id FROM build WHERE server_id = ?","\
SELECT os_id FROM build_os WHERE os = ? AND ver_alias = ? AND arch = ?","\
SELECT os_id FROM build_os WHERE alias = ? AND ver_alias = ? AND arch = ?","\
SELECT def_scheme_id FROM seed_schemes WHERE scheme_name = ?","\
SELECT bd_id FROM build_domain WHERE domain = ?","\
SELECT config_ldap FROM build_domain WHERE domain = ?"
/* 40 */,"\
SELECT l.keymap, l.locale, l.timezone FROM build b \
  LEFT JOIN locale l ON b.locale_id = l.locale_id WHERE b.server_id = ?","\
SELECT bt.mirror, bt.alias, bo.arch, bo.os_version, b.net_inst_int, bi.ip, \
  bd.netmask, bd.gateway, bd.ns, bi.hostname, bi.domainname FROM build b \
  LEFT JOIN build_ip bi ON bi.ip_id = b.ip_id LEFT JOIN build_domain bd ON \
  bi.bd_id = bd.bd_id LEFT JOIN build_os bo ON b.os_id = bo.os_id LEFT JOIN \
  build_type bt ON bo.bt_id = bt.bt_id WHERE b.server_id = ?","\
SELECT url FROM build_type bt LEFT JOIN build_os bo ON bt.bt_id = bo.bt_id \
  LEFT JOIN build b ON b.os_id = bo.os_id WHERE b.server_id = ?","\
SELECT bd.config_ntp, bd.ntp_server FROM build_domain bd \
  LEFT JOIN build_ip bi ON bd.bd_id = bi.bd_id WHERE bi.server_id =?","\
SELECT bd.config_log, bd.log_server FROM build_domain bd \
  LEFT JOIN build_ip bi ON bd.bd_id = bi.bd_id WHERE bi.server_id =?","\
SELECT config_ntp, config_ldap, ldap_ssl, config_log, config_xymon, config_email \
  FROM build_domain bd LEFT JOIN build_ip bi ON bi.bd_id = bd.bd_id WHERE bi.server_id = ?","\
SELECT nfs_domain FROM build_domain bd NATURAL JOIN build_ip bi WHERE \
  bi.server_id = ?","\
SELECT s.name, bi.ip FROM build_ip bi LEFT JOIN server s ON \
  s.server_id = bi.server_id WHERE bi.bd_id = ? ORDER BY bi.ip","\
SELECT pack_id FROM packages WHERE package = ? AND varient_id = ? \
AND os_id = ?","\
SELECT def_part_id FROM default_part dp LEFT JOIN seed_schemes ss ON \
  dp.def_scheme_id = ss.def_scheme_id WHERE ss.scheme_name = ? AND \
  dp.mount_point = ?"
/* 50 */ ,"\
SELECT ip_id FROM build_ip WHERE hostname = ? AND domainname = ?","\
SELECT ip_id FROM build_ip WHERE ip = ?","\
SELECT detail FROM hardware where server_id = ? and device = ?","\
SELECT locale_id FROM locale WHERE os_id = ?","\
SELECT ip_id FROM build_ip WHERE server_id = ?","\
SELECT bd_id, start_ip, end_ip FROM build_domain WHERE domain = ?","\
SELECT hard_id FROM hardware WHERE server_id = ? and device = ?","\
SELECT lvm FROM seed_schemes WHERE def_scheme_id = ?","\
SELECT syspack_id FROM system_packages WHERE name = ?","\
SELECT sp.name, spa.field, spa.type, spc.arg FROM system_package_args spa \
  LEFT JOIN system_package_conf spc ON spa.syspack_arg_id = spc.syspack_arg_id \
  LEFT JOIN system_packages sp ON sp.syspack_id = spc.syspack_id \
  WHERE spc.bd_id = ?  AND spc.syspack_id = ? AND spc.syspack_arg_id = ? \
  ORDER BY sp.name, spa.field"
/* 60 */,"\
SELECT syspack_arg_id FROM system_package_args WHERE \
  syspack_id = ?  AND field = ?","\
SELECT sp.name, spa.field, spa.type, spc.arg FROM system_package_args spa \
  LEFT JOIN system_package_conf spc ON spa.syspack_arg_id = spc.syspack_arg_id \
  LEFT JOIN system_packages sp ON sp.syspack_id = spc.syspack_id \
  WHERE spc.bd_id = ? AND spc.syspack_id = ? ORDER BY sp.name, spa.field","\
SELECT sp.name, spa.field, spa.type, spc.arg FROM system_package_args spa \
  LEFT JOIN system_package_conf spc ON spa.syspack_arg_id = spc.syspack_arg_id \
  LEFT JOIN system_packages sp ON sp.syspack_id = spc.syspack_id \
  WHERE spc.bd_id = ? ORDER BY sp.name, spa.field","\
SELECT bd.domain FROM build_domain bd \
  LEFT JOIN build_ip ip ON ip.bd_id = bd.bd_id WHERE ip.server_id = ?","\
SELECT s.name, bd.domain from server s \
  LEFT JOIN build_ip ip ON ip.server_id = s.server_id \
  LEFT JOIN build_domain bd ON ip.bd_id = bd.bd_id where s.server_id = ?","\
SELECT bd.bd_id FROM build_domain bd \
  LEFT JOIN build_ip ip ON ip.bd_id = bd.bd_id WHERE ip.server_id = ?","\
SELECT syspack_conf_id FROM system_package_conf spc \
  JOIN system_packages sp ON sp.syspack_id = spc.syspack_id \
  JOIN system_package_args spa ON spa.syspack_arg_id = spc.syspack_arg_id \
  JOIN build_domain bd ON bd.bd_id = spc.bd_id \
  WHERE bd.domain = ? AND sp.name = ? AND spa.field =?","\
SELECT systscr_id FROM system_scripts WHERE name = ?","\
SELECT ss.name, sa.arg, sa.no FROM system_scripts ss\
  JOIN system_scripts_args sa ON ss.systscr_id = sa.systscr_id\
  JOIN build_type bt ON sa.bt_id = bt.bt_id \
  WHERE bd_id = ? AND bt.alias = ? ORDER BY ss.name, sa.no","\
SELECT build_type FROM build_type WHERE alias = ?"
/* 70 */,"\
SELECT systscr_arg_id from system_scripts_args WHERE bd_id = ? AND bt_id = ?\
  AND systscr_id = ? AND no = ?","\
SELECT poption FROM part_options WHERE def_part_id = ? AND def_scheme_id = ?","\
SELECT part_options_id FROM part_options WHERE def_part_id = ? AND \
  def_scheme_id = ? and poption = ?","\
SELECT def_scheme_id FROM build WHERE server_id = ?","\
SELECT scheme_name FROM seed_schemes ss LEFT JOIN build b ON \
  ss.def_scheme_id = b.def_scheme_id WHERE b.server_id = ?","\
SELECT package, os_id FROM packages WHERE varient_id = ?","\
SELECT os_id, ctime, arch FROM build_os WHERE bt_id = ?","\
SELECT locale, country, language, keymap, timezone FROM locale WHERE os_id = ?","\
SELECT package, varient_id FROM packages WHERE os_id = ?","\
SELECT mirror from build_type where alias = ?"
/* 80 */,"\
SELECT locale_id FROM locale WHERE name = ?","\
SELECT locale_id FROM default_locale WHERE locale_id > 0","\
SELECT bo.os, bo.os_version FROM build_os bo LEFT JOIN build b\
 ON bo.os_id = b.os_id WHERE b.server_id = ?"
};

const unsigned int cbc_select_fields[] = {
	5, 13, 13, 10, 11, 7, 4, 11, 8, 12, 7, 12, 7, 8, 6, 8, 9, 6, 10, 8
};

const unsigned int cbc_insert_fields[] = {
	4, 10, 10, 7, 8, 6, 3, 8, 5, 9, 4, 9, 4, 5, 3, 5, 6, 3, 7, 5
};

const unsigned int cbc_update_args[] = {
	2, 2, 2, 2, 2, 3, 3, 3, 4, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5,
	5, 6, 3, 3, 3, 3, 3, 2, 2, 2, 1
};
const unsigned int cbc_delete_args[] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const unsigned int cbc_search_args[] = {
	1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, // 22
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, // 22
	1, 1, 1, 1, 3, 2, 2, 1, 2, 1, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 1, 1, // 22
	3, 1, 2, 1, 4, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1
};
const unsigned int cbc_search_fields[] = {
	5, 5, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 10,
	10, 7, 2, 6, 1, 5, 3, 4, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3, 11, 1, 2,
	2, 6, 1, 2, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 4, 1, 4, 4, 1, 2, 1,
	1, 1, 3, 1, 1, 1, 1, 1, 1, 2, 3, 5, 2, 1, 1, 1, 2
};

const int cbc_inserts[][24] = {
	{ DBTEXT, DBTEXT, DBINT, DBTEXT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBINT, DBTEXT, DBINT, DBINT, DBINT, DBINT, DBINT, DBINT,
	  DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBINT, DBINT, DBINT, DBINT, DBINT, DBTEXT, DBTEXT, DBSHORT,
	  DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBINT, DBTEXT, DBTEXT, DBINT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, DBINT, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBINT, DBTEXT, DBSHORT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBINT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBINT, DBINT, DBINT, DBTEXT, DBTEXT, DBINT, DBTEXT, DBINT, DBINT, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBSHORT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, DBTEXT, DBINT, DBINT,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBTEXT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0 },
	{ DBINT, DBTEXT, DBTEXT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBINT, DBINT, DBINT, DBTEXT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBTEXT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0 },
	{ DBINT, DBINT, DBINT, DBTEXT, DBINT, DBINT, DBINT, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ DBINT, DBINT, DBTEXT, DBINT, DBINT, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	  0, 0, 0, 0, 0, 0, 0, 0 }
};

const unsigned int cbc_update_types[][6] = {
	{ DBTEXT, DBTEXT, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBSHORT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBSHORT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBSHORT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBSHORT, DBINT, DBINT, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, NONE } ,
	{ DBTEXT, DBTEXT, DBSHORT, DBINT, DBINT, NONE } ,
	{ DBTEXT, DBTEXT, DBSHORT, DBINT, DBINT, NONE } ,
	{ DBTEXT, DBTEXT, DBSHORT, DBINT, DBINT, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBSHORT, DBINT, DBINT } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE }
};
const unsigned int cbc_delete_types[][2] = {
	{ DBTEXT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE } ,
	{ DBINT, NONE }
};
const unsigned int cbc_search_arg_types[][4] = {
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBINT, NONE } ,
	{ DBTEXT, DBTEXT, NONE, NONE } ,
	{ DBTEXT, DBTEXT, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, DBTEXT, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBINT, DBTEXT, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, NONE } ,
	{ DBINT, DBTEXT, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, NONE },
	{ DBTEXT, NONE, NONE, NONE },
	{ DBINT, DBTEXT, NONE, NONE },
	{ DBTEXT, NONE, NONE, NONE },
	{ DBINT, DBINT, DBINT, DBINT },
	{ DBINT, DBINT, NONE, NONE },
	{ DBINT, DBINT, DBTEXT, NONE },
	{ DBINT, NONE, NONE, NONE },
	{ DBINT, NONE, NONE, NONE },
	{ DBINT, NONE, NONE, NONE },
	{ DBINT, NONE, NONE, NONE },
	{ DBINT, NONE, NONE, NONE },
	{ DBINT, NONE, NONE, NONE },
	{ DBTEXT, NONE, NONE, NONE },
	{ DBTEXT, NONE, NONE, NONE },
	{ NONE, NONE, NONE, NONE },
	{ DBINT, NONE, NONE, NONE }
};
const unsigned int cbc_search_field_types[][11] = {
	{ DBSHORT, DBSHORT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBSHORT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, DBINT, DBINT, DBTEXT, DBTEXT, DBTEXT, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBSHORT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBSHORT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBTEXT, DBSHORT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBTEXT, DBTEXT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBINT, DBINT, DBINT, DBINT, DBTEXT, DBTEXT } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, DBSHORT, DBSHORT, DBSHORT, DBSHORT, DBSHORT, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBINT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBSHORT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, DBTEXT, DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE },
	{ DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBINT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE } ,
	{ DBTEXT, DBTEXT, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE, NONE }
};

int
cbc_run_query(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	int retval = 0;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cbc_run_query");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cbc_run_query_mysql(config, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cbc_run_query_sqlite(config, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cbc_run_multiple_query(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	int retval = 0;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cbc_run_multiple_query");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cbc_run_multiple_query_mysql(config, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cbc_run_multiple_query_sqlite(config, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cbc_run_insert(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	int retval = 0;
	if ((strncmp(config->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cbc_run_insert");
#ifdef HAVE_MYSQL
	else if ((strncmp(config->dbtype, "mysql", RANGE_S) == 0))
		retval = cbc_run_insert_mysql(config, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(config->dbtype, "sqlite", RANGE_S) == 0))
		retval = cbc_run_insert_sqlite(config, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, config->dbtype);
	return retval;
}

int
cbc_run_delete(ailsa_cmdb_s *ccs, dbdata_s *base, int type)
{
	int retval = 0;
	if ((strncmp(ccs->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cbc_run_delete");
#ifdef HAVE_MYSQL
	else if ((strncmp(ccs->dbtype, "mysql", RANGE_S) == 0))
		retval = cbc_run_delete_mysql(ccs, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(ccs->dbtype, "sqlite", RANGE_S) == 0))
		retval = cbc_run_delete_sqlite(ccs, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, ccs->dbtype);
	return retval;
}

int
cbc_run_search(ailsa_cmdb_s *ccs, dbdata_s *base, int type)
{
	int retval = 0;
	if ((strncmp(ccs->dbtype, "none", RANGE_S) == 0))
		report_error(NO_DB_TYPE, "cbc_run_search");
#ifdef HAVE_MYSQL
	else if ((strncmp(ccs->dbtype, "mysql", RANGE_S) == 0))
		retval = cbc_run_search_mysql(ccs, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if ((strncmp(ccs->dbtype, "sqlite", RANGE_S) == 0))
		retval = cbc_run_search_sqlite(ccs, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, ccs->dbtype);
	return retval;
}

int
cbc_run_update(ailsa_cmdb_s *ccs, dbdata_s *base, int type)
{
	int retval = 0;
	if (strncmp(ccs->dbtype, "none", COMM_S) == 0)
		report_error(NO_DB_TYPE, "cbc_run_update");
#ifdef HAVE_MYSQL
	else if (strncmp(ccs->dbtype, "mysql", COMM_S) == 0)
		retval = cbc_run_update_mysql(ccs, base, type);
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
	else if (strncmp(ccs->dbtype, "sqlite", COMM_S) == 0)
		retval = cbc_run_update_sqlite(ccs, base, type);
#endif /* HAVE_SQLITE3 */
	else
		report_error(DB_TYPE_INVALID, ccs->dbtype);
	return retval;
}
/*
int
cbc_set_default(ailsa_cmdb_s *ccs, char *table, unsigned long int id)
{
	int retval = 0;
	if (strncmp(ccs->dbtype, "none", COMM_S) == 0)
		report_error(NO_DB_TYPE, "cbc_set_default");
#ifdef HAVE_MYSQL
	else if (strncmp(ccs->dbtype, "mysql", COMM_S) == 0)
		retval = cbc_set_default_mysql(ccs, table, id);
#endif // HAVE_MYSQL
	else
		report_error(DB_TYPE_INVALID, ccs->dbtype);
	return retval;
} */

int
cbc_get_query(int type, const char **query, unsigned int *fields)
{
	int retval = 0;

	if (type == BOOT_LINE) {
		*query = cbc_sql_select[BOOT_LINES];
		*fields = cbc_select_fields[BOOT_LINES];
	} else if (type == BUILD) {
		*query = cbc_sql_select[BUILDS];
		*fields = cbc_select_fields[BUILDS];
	} else if (type == BUILD_DOMAIN) {
		*query = cbc_sql_select[BUILD_DOMAINS];
		*fields = cbc_select_fields[BUILD_DOMAINS];
	} else if (type == BUILD_IP) {
		*query = cbc_sql_select[BUILD_IPS];
		*fields = cbc_select_fields[BUILD_IPS];
	} else if (type == BUILD_OS) {
		*query = cbc_sql_select[BUILD_OSS];
		*fields = cbc_select_fields[BUILD_OSS];
	} else if (type == BUILD_TYPE) {
		*query = cbc_sql_select[BUILD_TYPES];
		*fields = cbc_select_fields[BUILD_TYPES];
	} else if (type == DISK_DEV) {
		*query = cbc_sql_select[DISK_DEVS];
		*fields = cbc_select_fields[DISK_DEVS];
	} else if (type == LOCALE) {
		*query = cbc_sql_select[LOCALES];
		*fields = cbc_select_fields[LOCALES];
	} else if (type == BPACKAGE) {
		*query = cbc_sql_select[BPACKAGES];
		*fields = cbc_select_fields[BPACKAGES];
	} else if (type == DPART) {
		*query = cbc_sql_select[DPARTS];
		*fields = cbc_select_fields[DPARTS];
	} else if (type == SSCHEME) {
		*query = cbc_sql_select[SSCHEMES];
		*fields = cbc_select_fields[SSCHEMES];
	} else if (type == CSERVER) {
		*query = cbc_sql_select[CSERVERS];
		*fields = cbc_select_fields[CSERVERS];
	} else if (type == VARIENT) {
		*query = cbc_sql_select[VARIENTS];
		*fields = cbc_select_fields[VARIENTS];
	} else if (type == VMHOST) {
		*query = cbc_sql_select[VMHOSTS];
		*fields = cbc_select_fields[VMHOSTS];
	} else if (type == SYSPACK) {
		*query = cbc_sql_select[SYSPACKS];
		*fields = cbc_select_fields[SYSPACKS];
	} else if (type == SYSARG) {
		*query = cbc_sql_select[SYSARGS];
		*fields = cbc_select_fields[SYSARGS];
	} else if (type == SYSCONF) {
		*query = cbc_sql_select[SYSCONFS];
		*fields = cbc_select_fields[SYSCONFS];
	} else if (type == SCRIPT) {
		*query = cbc_sql_select[SCRIPTS];
		*fields = cbc_select_fields[SCRIPTS];
	} else if (type == SCRIPTA) {
		*query = cbc_sql_select[SCRIPTAS];
		*fields = cbc_select_fields[SCRIPTAS];
	} else if (type == PARTOPT) {
		*query = cbc_sql_select[PARTOPTS];
		*fields = cbc_select_fields[PARTOPTS];
	} else {
		retval = UNKNOWN_QUERY;
	}
	return retval;
}

#ifdef HAVE_MYSQL

void
cbc_mysql_init(ailsa_cmdb_s *dc, MYSQL *cbc_mysql)
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
cbc_run_query_mysql(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	MYSQL cbc;
	MYSQL_RES *cbc_res;
	MYSQL_ROW cbc_row;
	my_ulonglong cbc_rows;
	const char *query;
	int retval = 0;
	unsigned int fields;

	cbc_mysql_init(config, &cbc);
	if ((retval = cbc_get_query(type, &query, &fields)) != 0) {
		report_error(retval, "cbc_run_query_mysql");
	}
	if ((retval = cmdb_mysql_query_with_checks(&cbc, query)) != 0) {
		report_error(MY_QUERY_FAIL, mysql_error(&cbc));
	}
	if (!(cbc_res = mysql_store_result(&cbc))) {
		cmdb_mysql_cleanup(&cbc);
		report_error(MY_STORE_FAIL, mysql_error(&cbc));
	}
	fields = mysql_num_fields(cbc_res);
	if (((cbc_rows = mysql_num_rows(cbc_res)) == 0)) {
		cmdb_mysql_cleanup_full(&cbc, cbc_res);
		return NO_RECORDS;
	}
	while ((cbc_row = mysql_fetch_row(cbc_res)))
		cbc_store_result_mysql(cbc_row, base, type, fields);
	cmdb_mysql_cleanup_full(&cbc, cbc_res);
	return NONE;
}

int
cbc_run_insert_mysql(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	MYSQL cbc;
	MYSQL_BIND my_bind[cbc_insert_fields[type]];
	const char *query;
	int retval = 0;
	unsigned int i;

	memset(my_bind, 0, sizeof(my_bind));
	query = cbc_sql_insert[type];
	for (i = 0; i < cbc_insert_fields[type]; i++)
		if ((retval = cbc_setup_insert_mysql_bind(&my_bind[i], i, type, base)) != 0)
			report_error(retval, query);
	cbc_mysql_init(config, &cbc);
	cmdb_run_mysql_stmt(&cbc, my_bind, query);
	cmdb_mysql_cleanup(&cbc);
	return retval;
}

int
cbc_run_multiple_query_mysql(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	int retval;
	retval = NONE;
	if (type & BOOT_LINE)
		if ((retval = cbc_run_query_mysql(config, base, BOOT_LINE)) != 0)
			return retval;
	if (type & BUILD)
		if ((retval = cbc_run_query_mysql(config, base, BUILD)) != 0)
			return retval;
	if (type & BUILD_DOMAIN)
		if ((retval = cbc_run_query_mysql(config, base, BUILD_DOMAIN)) != 0)
			return retval;
	if (type & BUILD_IP)
		if ((retval = cbc_run_query_mysql(config, base, BUILD_IP)) != 0)
			return retval;
	if (type & BUILD_OS)
		if ((retval = cbc_run_query_mysql(config, base, BUILD_OS)) != 0)
			return retval;
	if (type & BUILD_TYPE)
		if ((retval = cbc_run_query_mysql(config, base, BUILD_TYPE)) != 0)
			return retval;
	if (type & DISK_DEV)
		if ((retval = cbc_run_query_mysql(config, base, DISK_DEV)) != 0)
			return retval;
	if (type & LOCALE)
		if ((retval = cbc_run_query_mysql(config, base, LOCALE)) != 0)
			return retval;
	if (type & BPACKAGE)
		if ((retval = cbc_run_query_mysql(config, base, BPACKAGE)) != 0)
			return retval;
	if (type & DPART)
		if ((retval = cbc_run_query_mysql(config, base, DPART)) != 0)
			return retval;
	if (type & SSCHEME)
		if ((retval = cbc_run_query_mysql(config, base, SSCHEME)) != 0)
			return retval;
	if (type & CSERVER)
		if ((retval = cbc_run_query_mysql(config, base, CSERVER)) != 0)
			return retval;
	if (type & VARIENT)
		if ((retval = cbc_run_query_mysql(config, base, VARIENT)) != 0)
			return retval;
	if (type & VMHOST)
		if ((retval = cbc_run_query_mysql(config, base, VMHOST)) != 0)
			return retval;
	if (type & SYSPACK)
		if ((retval = cbc_run_query_mysql(config, base, SYSPACK)) != 0)
			return retval;
	if (type & SYSARG)
		if ((retval = cbc_run_query_mysql(config, base, SYSARG)) != 0)
			return retval;
	if (type & SYSCONF)
		if ((retval = cbc_run_query_mysql(config, base, SYSCONF)) != 0)
			return retval;
	if (type & SCRIPT)
		if ((retval = cbc_run_query_mysql(config, base, SCRIPT)) != 0)
			return retval;
	if (type & SCRIPTA)
		if ((retval = cbc_run_query_mysql(config, base, SCRIPTA)) != 0)
			return retval;
	if (type & PARTOPT)
		if ((retval = cbc_run_query_mysql(config, base, PARTOPT)) != 0)
			return retval;
	return retval;
}

int
cbc_run_delete_mysql(ailsa_cmdb_s *ccs, dbdata_s *data, int type)
{
	MYSQL cbc;
	MYSQL_BIND mybind[cbc_delete_args[type]];
	const char *query = cbc_sql_delete[type];
	int retval = 0;
	unsigned int i, dbtype;
	dbdata_s *list = data;

	memset(mybind, 0, sizeof(mybind));
	for (i = 0; i < cbc_delete_args[type]; i++) {
		dbtype = cbc_delete_types[type][i];
		cmdb_set_bind_mysql(&(mybind[i]), dbtype, &(list->args));
		list = list->next;
	}
	cbc_mysql_init(ccs, &cbc);
	retval = cmdb_run_mysql_stmt(&cbc, mybind, query);
	cmdb_mysql_cleanup(&cbc);
	return retval;
}
/*
int
cbc_set_default_mysql(ailsa_cmdb_s *ccs, char *table, unsigned long int id)
{
	MYSQL cbc;
	MYSQL_STMT *cbc_stmt;
	int retval = 0;

	return retval;
} */

int
cbc_run_search_mysql(ailsa_cmdb_s *ccs, dbdata_s *data, int type)
{
	MYSQL cbc;
	MYSQL_STMT *cbc_stmt;
	unsigned int i, dbtype;
	const char *query = cbc_sql_search[type];
	int retval = 0, j = 0;
	dbdata_s *list;
	unsigned int a = cbc_search_args[type];
	unsigned int f = cbc_search_fields[type];
	MYSQL_BIND args[a];
	MYSQL_BIND fields[f];
	my_ulonglong numrows;

	memset(args, 0, sizeof(args));
	memset(fields, 0, sizeof(fields));
	list = data;
	for (i = 0; i < cbc_search_args[type]; i++) {
		dbtype = cbc_search_arg_types[type][i];
		cmdb_set_bind_mysql(&args[i], dbtype, &(list->args));
		list = list->next;
	}
	list = data;
	for (i = 0; i < cbc_search_fields[type]; i++)
		if ((retval = cbc_set_search_fields_mysql(&fields[i], i, j, type, list)) != 0)
			report_error(DB_WRONG_TYPE, query);
	cbc_mysql_init(ccs, &cbc);
	if (!(cbc_stmt = mysql_stmt_init(&cbc)))
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_prepare(cbc_stmt, query, strlen(query))) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_bind_param(cbc_stmt, &args[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_bind_result(cbc_stmt, &fields[0])) != 0)
		report_error(MY_BIND_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_execute(cbc_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	if ((retval = mysql_stmt_store_result(cbc_stmt)) != 0)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	numrows = mysql_stmt_num_rows(cbc_stmt);
	while ((retval = mysql_stmt_fetch(cbc_stmt)) == 0) {
		j++;
		if ((int)numrows > j) {
			for (i = 0; i < cbc_search_fields[type]; i++)
				if ((retval = cbc_set_search_fields_mysql(&fields[i], i, j, type, list)) != 0)
					report_error(DB_WRONG_TYPE, query);
		}
		if ((retval = mysql_stmt_bind_result(cbc_stmt, &fields[0])) != 0)
			report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	}
	if (retval != MYSQL_NO_DATA)
		report_error(MY_STATEMENT_FAIL, mysql_stmt_error(cbc_stmt));
	mysql_stmt_free_result(cbc_stmt);
	mysql_stmt_close(cbc_stmt);
	cmdb_mysql_cleanup(&cbc);
	return j;
}

int
cbc_run_update_mysql(ailsa_cmdb_s *ccs, dbdata_s *data, int type)
{
	MYSQL cbc;
	MYSQL_BIND args[cbc_update_args[type]];
	const char *query = cbc_sql_update[type];
	int retval = NONE;
	dbdata_s *list = data;
	unsigned int i, dbtype;

	memset(args, 0, sizeof(args));
	for (i = 0; i < cbc_update_args[type]; i++) {
		dbtype = cbc_update_types[type][i];
		cmdb_set_bind_mysql(&(args[i]), dbtype, &(list->args));
		list = list->next;
	}
	cbc_mysql_init(ccs, &cbc);
	retval = cmdb_run_mysql_stmt(&cbc, args, query);
	cmdb_mysql_cleanup(&cbc);
	return retval;
}

int
cbc_set_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base)
{
	int retval = 0, j, n, q;
	static int m = 0, stype = 0;
	unsigned int dbtype;
	dbdata_s *list, *new;
	list = base;

	/* Check if this is a new query. */
	if (stype == 0)
		stype = type;
	else if (stype != type) {
		stype = type;
		m = 0;
	}
	/* Check if this is the first row returned. If not we need to create
	 * a dbdata_s to hold the returned data.
	 * However, if the args list is longer than fields list, then check
	 * if we have spare capacity. */
	n = (int)cbc_search_args[type];
	q = (int)cbc_search_fields[type];
	if ((k > 0) && (((k + 1) * q) > n)) {
		if (!(new = malloc(sizeof(dbdata_s))))
			report_error(MALLOC_FAIL, "new in cbc_set_search_fields_mysql");
		init_dbdata_struct(new);
		while (list->next) {
			list = list->next;
		}
		list->next = new;
		list = base;
	}
	/* Attempt to not have the list drop off the end when moving along
	 * the list below */
	if ((k == 0) && ((unsigned)m >= cbc_search_fields[type]))
		m = 0;
	/* M is the number of dbdata_s in the linked list. Cannot check
	 * list->next as this would not work for the first row */
	for (j = 0; j < m; j++)
		list = list->next;
	/* This function is not working if I call the same query on different data
	 * I need to rewrite this. This is just an ugly hack :( */
	if (!(list))
		list = base;
	dbtype = cbc_search_field_types[type][i];
	cmdb_set_bind_mysql(mybind, dbtype, &(list->fields));
	m++;

	return retval;
}

void
cbc_store_result_mysql(MYSQL_ROW row, cbc_s *base, int type, unsigned int fields)
{
	unsigned int required;
/*	if (type == BOOT_LINE) {
		required = cbc_select_fields[BOOT_LINES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_boot_line_mysql(row, base);
	} else*/ if (type == BUILD) {
		required = cbc_select_fields[BUILDS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_mysql(row, base);
	} else if (type == BUILD_DOMAIN) {
		required = cbc_select_fields[BUILD_DOMAINS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_domain_mysql(row, base);
	} else if (type == BUILD_IP) {
		required = cbc_select_fields[BUILD_IPS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_ip_mysql(row, base);
	} else if (type == BUILD_OS) {
		required = cbc_select_fields[BUILD_OSS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_os_mysql(row, base);
	} else if (type == BUILD_TYPE) {
		required = cbc_select_fields[BUILD_TYPES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_type_mysql(row, base);
	} else if (type == DISK_DEV) {
		required = cbc_select_fields[DISK_DEVS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_disk_dev_mysql(row, base);
	} else if (type == LOCALE) {
		required = cbc_select_fields[LOCALES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_locale_mysql(row, base);
	} else if (type == BPACKAGE) {
		required = cbc_select_fields[BPACKAGES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_package_mysql(row, base);
	} else if (type == DPART) {
		required = cbc_select_fields[DPARTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_dpart_mysql(row, base);
	} else if (type == SSCHEME) {
		required = cbc_select_fields[SSCHEMES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_seed_scheme_mysql(row, base);
	} else if (type == CSERVER) {
		required = cbc_select_fields[CSERVERS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_server_mysql(row, base);
	} else if (type == VARIENT) {
		required = cbc_select_fields[VARIENTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_varient_mysql(row, base);
	} else if (type == VMHOST) {
		required = cbc_select_fields[VMHOSTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_vmhost_mysql(row, base);
	} else if (type == SYSPACK) {
		required = cbc_select_fields[SYSPACKS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_syspack_mysql(row, base);
	} else if (type == SYSARG) {
		required = cbc_select_fields[SYSARGS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_sysarg_mysql(row, base);
	} else if (type == SYSCONF) {
		required = cbc_select_fields[SYSCONFS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_sysconf_mysql(row, base);
	} else if (type == SCRIPT) {
		required = cbc_select_fields[SCRIPTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_script_mysql(row, base);
	} else if (type == SCRIPTA) {
		required = cbc_select_fields[SCRIPTAS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_scripta_mysql(row, base);
	} else if (type == PARTOPT) {
		required = cbc_select_fields[PARTOPTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_partopt_mysql(row, base);
	} else {
		report_error(UNKNOWN_STRUCT_DB_TABLE, "cbc_store_result_mysql");
	}
}

int
cbc_setup_insert_mysql_bind(MYSQL_BIND *mybind, unsigned int i, int type, cbc_s *base)
{
	int retval = NONE;
	void *buffer;

	mybind->is_null = 0;
	mybind->length = 0;
	cbc_setup_insert_mysql_buffer(type, &buffer, base, i);
	mybind->buffer = buffer;
	if (cbc_inserts[type][i] == DBINT) {
		mybind->buffer_type = MYSQL_TYPE_LONG;
		mybind->is_unsigned = 1;
		mybind->buffer_length = sizeof(unsigned long int);
	} else if (cbc_inserts[type][i] == DBTEXT) {
		mybind->buffer_type = MYSQL_TYPE_STRING;
		mybind->is_unsigned = 0;
		mybind->buffer_length = strlen(buffer);
	} else if (cbc_inserts[type][i] == DBSHORT) {
		mybind->buffer_type = MYSQL_TYPE_SHORT;
		mybind->is_unsigned = 0;
		mybind->buffer_length = sizeof(short int);
	} else {
		return DB_WRONG_TYPE;
	}
	return retval;
}

void
cbc_setup_insert_mysql_buffer(int type, void **buffer, cbc_s *base, unsigned int i)
{
	if (type == BUILD_DOMAINS)
		cbc_setup_bind_mysql_build_domain(buffer, base, i);
	else if (type == BUILD_OSS)
		cbc_setup_bind_mysql_build_os(buffer, base, i);
	else if (type == VARIENTS)
		cbc_setup_bind_mysql_build_varient(buffer, base, i);
	else if (type == SSCHEMES)
		cbc_setup_bind_mysql_build_part_scheme(buffer, base, i);
	else if (type == DPARTS)
		cbc_setup_bind_mysql_build_def_part(buffer, base, i);
	else if (type == BPACKAGES)
		cbc_setup_bind_mysql_build_package(buffer, base, i);
	else if (type == BUILD_IPS)
		cbc_setup_bind_mysql_build_ip(buffer, base, i);
	else if (type == BUILDS)
		cbc_setup_bind_mysql_build(buffer, base, i);
	else if (type == DISK_DEVS)
		cbc_setup_bind_mysql_build_disk(buffer, base, i);
	else if (type == LOCALES)
		cbc_setup_bind_mysql_locale(buffer, base, i);
	else if (type == SYSPACKS)
		cbc_setup_bind_mysql_syspack(buffer, base, i);
	else if (type == SYSARGS)
		cbc_setup_bind_mysql_sysarg(buffer, base, i);
	else if (type == SYSCONFS)
		cbc_setup_bind_mysql_sysconf(buffer, base, i);
	else if (type == SCRIPTS)
		cbc_setup_bind_mysql_script(buffer, base, i);
	else if (type == SCRIPTAS)
		cbc_setup_bind_mysql_scripta(buffer, base, i);
	else if (type == PARTOPTS)
		cbc_setup_bind_mysql_partopts(buffer, base, i);
	else if (type == CSERVERS)
		cbc_setup_bind_mysql_servers(buffer, base, i);
	else
		report_error(UNKNOWN_STRUCT_DB_TABLE, "cbc_setup_insert_mysql_buffer");
}
/*
void
cbc_store_boot_line_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_boot_line_s *boot, *list;

	if (!(boot = malloc(sizeof(cbc_boot_line_s))))
		report_error(MALLOC_FAIL, "boot in cbc_store_boot_line_mysql");
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
} */

void
cbc_store_build_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_build_s *build, *list;

	if (!(build = malloc(sizeof(cbc_build_s))))
		report_error(MALLOC_FAIL, "build in cbc_store_build_mysql");
	init_build_struct(build);
	build->build_id = strtoul(row[0], NULL, 10);
	snprintf(build->mac_addr, MAC_S, "%s", row[1]);
	build->varient_id = strtoul(row[2], NULL, 10);
	snprintf(build->net_int, RANGE_S, "%s", row[3]);
	build->server_id = strtoul(row[4], NULL, 10);
	build->os_id = strtoul(row[5], NULL, 10);
	build->ip_id = strtoul(row[6], NULL, 10);
	build->locale_id = strtoul(row[7], NULL, 10);
	build->def_scheme_id = strtoul(row[8], NULL, 10);
	build->cuser = strtoul(row[9], NULL, 10);
	build->muser = strtoul(row[10], NULL, 10);
	convert_time(row[11], &(build->ctime));
	convert_time(row[12], &(build->mtime));
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
cbc_store_build_domain_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_build_domain_s *dom, *list;

	if (!(dom = malloc(sizeof(cbc_build_domain_s))))
		report_error(MALLOC_FAIL, "dom in cbc_store_build_domain_mysql");
	init_build_domain(dom);
	dom->bd_id = strtoul(row[0], NULL, 10);
	dom->start_ip = strtoul(row[1], NULL, 10);
	dom->end_ip = strtoul(row[2], NULL, 10);
	dom->netmask = strtoul(row[3], NULL, 10);
	dom->gateway = strtoul(row[4], NULL, 10);
	dom->ns = strtoul(row[5], NULL, 10);
	snprintf(dom->domain, RBUFF_S, "%s", row[6]);
	if (strncmp("0", row[8], CH_S) == 0) {
		dom->config_ntp = 0;
	} else {
		snprintf(dom->ntp_server, RBUFF_S, "%s", row[7]);
		dom->config_ntp = 1;
	}
	dom->cuser = strtoul(row[9], NULL, 10);
	dom->muser = strtoul(row[10], NULL, 10);
	convert_time(row[11], &(dom->ctime));
	convert_time(row[12], &(dom->mtime));
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
cbc_store_build_ip_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_build_ip_s *ip, *list;

	if (!(ip = malloc(sizeof(cbc_build_ip_s))))
		report_error(MALLOC_FAIL, "ip in cbc_store_build_ip_mysql");
	init_build_ip(ip);
	ip->ip_id = strtoul(row[0], NULL, 10);
	ip->ip = strtoul(row[1], NULL, 10);
	snprintf(ip->host, HOST_S, "%s", row[2]);
	snprintf(ip->domain, RBUFF_S, "%s", row[3]);
	ip->bd_id = strtoul(row[4], NULL, 10);
	ip->server_id = strtoul(row[5], NULL, 10);
	ip->cuser = strtoul(row[6], NULL, 10);
	ip->muser = strtoul(row[7], NULL, 10);
	convert_time(row[8], &(ip->ctime));
	convert_time(row[9], &(ip->mtime));
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
cbc_store_build_os_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_build_os_s *os, *list;

	if (!(os = malloc(sizeof(cbc_build_os_s))))
		report_error(MALLOC_FAIL, "os in cbc_store_build_os_mysql");
	init_build_os(os);
	os->os_id = strtoul(row[0], NULL, 10);
	snprintf(os->os, MAC_S, "%s", row[1]);
	snprintf(os->version, MAC_S, "%s", row[2]);
	snprintf(os->alias, MAC_S, "%s", row[3]);
	snprintf(os->ver_alias, MAC_S, "%s", row[4]);
	snprintf(os->arch, RANGE_S, "%s", row[5]);
	os->bt_id = strtoul(row[6], NULL, 10);
	os->cuser = strtoul(row[7], NULL, 10);
	os->muser = strtoul(row[8], NULL, 10);
	convert_time(row[9], &(os->ctime));
	convert_time(row[10], &(os->mtime));
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
cbc_store_build_type_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_build_type_s *type, *list;

	if (!(type = malloc(sizeof(cbc_build_type_s))))
		report_error(MALLOC_FAIL, "type in cbc_store_build_type_mysql");
	init_build_type(type);
	type->bt_id = strtoul(row[0], NULL, 10);
	snprintf(type->alias, MAC_S, "%s", row[1]);
	snprintf(type->build_type, MAC_S, "%s", row[2]);
	snprintf(type->arg, RANGE_S, "%s", row[3]);
	snprintf(type->url, RBUFF_S, "%s", row[4]);
	snprintf(type->mirror, RBUFF_S, "%s", row[5]);
	snprintf(type->boot_line, URL_S, "%s", row[6]);
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
cbc_store_disk_dev_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_disk_dev_s *disk, *list;

	if (!(disk = malloc(sizeof(cbc_disk_dev_s))))
		report_error(MALLOC_FAIL, "disk in cbc_store_disk_dev_mysql");
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
cbc_store_locale_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_locale_s *loc, *list;

	if (!(loc = malloc(sizeof(cbc_locale_s))))
		report_error(MALLOC_FAIL, "loc in cbc_store_locale_mysql");
	init_locale(loc);
	loc->locale_id = strtoul(row[0], NULL, 10);
	snprintf(loc->locale, MAC_S, "%s", row[1]);
	snprintf(loc->country, RANGE_S, "%s", row[2]);
	snprintf(loc->language, RANGE_S, "%s", row[3]);
	snprintf(loc->keymap, RANGE_S, "%s", row[4]);
	snprintf(loc->timezone, HOST_S, "%s", row[5]);
	snprintf(loc->name, HOST_S, "%s", row[6]);
	loc->cuser = strtoul(row[7], NULL, 10);
	loc->muser = strtoul(row[8], NULL, 10);
	convert_time(row[9], &(loc->ctime));
	convert_time(row[10], &(loc->mtime));
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
cbc_store_package_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_package_s *pack, *list;

	if (!(pack = malloc(sizeof(cbc_package_s))))
		report_error(MALLOC_FAIL, "pack in cbc_store_package_mysql");
	init_package(pack);
	pack->pack_id = strtoul(row[0], NULL, 10);
	snprintf(pack->package, HOST_S, "%s", row[1]);
	pack->vari_id = strtoul(row[2], NULL, 10);
	pack->os_id = strtoul(row[3], NULL, 10);
	pack->cuser = strtoul(row[4], NULL, 10);
	pack->muser = strtoul(row[5], NULL, 10);
	convert_time(row[6], &(pack->ctime));
	convert_time(row[7], &(pack->mtime));
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
cbc_store_dpart_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_pre_part_s *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_s))))
		report_error(MALLOC_FAIL, "part in cbc_store_dpart_mysql");
	init_pre_part(part);
	part->id.def_part_id = strtoul(row[0], NULL, 10);
	part->min= strtoul(row[1], NULL, 10);
	part->max = strtoul(row[2], NULL, 10);
	part->pri = strtoul(row[3], NULL, 10);
	snprintf(part->mount, HOST_S, "%s", row[4]);
	snprintf(part->fs, RANGE_S, "%s", row[5]);
	part->link_id.def_scheme_id = strtoul(row[6], NULL, 10);
	snprintf(part->log_vol, MAC_S, "%s", row[7]);
	part->cuser = strtoul(row[8], NULL, 10);
	part->muser = strtoul(row[9], NULL, 10);
	convert_time(row[10], &(part->ctime));
	convert_time(row[11], &(part->mtime));
	list = base->dpart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->dpart = part;
	}
}
/*
void
cbc_store_spart_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_pre_part_s *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_s))))
		report_error(MALLOC_FAIL, "part in cbc_store_dpart_mysql");
	init_pre_part(part);
	part->id.part_id = strtoul(row[0], NULL, 10);
	part->min= strtoul(row[1], NULL, 10);
	part->max = strtoul(row[2], NULL, 10);
	part->pri = strtoul(row[3], NULL, 10);
	snprintf(part->mount, HOST_S, "%s", row[4]);
	snprintf(part->fs, RANGE_S, "%s", row[5]);
	part->link_id.server_id = strtoul(row[6], NULL, 10);
	snprintf(part->log_vol, MAC_S, "%s", row[7]);
	part->cuser = strtoul(row[8], NULL, 10);
	part->muser = strtoul(row[9], NULL, 10);
	convert_time(row[10], &(part->ctime));
	convert_time(row[11], &(part->mtime));
	list = base->spart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->spart = part;
	}
} */

void
cbc_store_seed_scheme_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_seed_scheme_s *seed, *list;

	if (!(seed = malloc(sizeof(cbc_seed_scheme_s))))
		report_error(MALLOC_FAIL, "seed in cbc_store_seed_scheme_mysql");
	init_seed_scheme(seed);
	seed->def_scheme_id = strtoul(row[0], NULL, 10);
	snprintf(seed->name, CONF_S, "%s", row[1]);
	if (strncmp(row[2], "0", CH_S) == 0)
		seed->lvm = 0;
	else
		seed->lvm = 1;
	seed->cuser = strtoul(row[3], NULL, 10);
	seed->muser = strtoul(row[4], NULL, 10);
	convert_time(row[5], &(seed->ctime));
	convert_time(row[6], &(seed->mtime));
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
cbc_store_server_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_server_s *server, *list;

	if (!(server = malloc(sizeof(cbc_server_s))))
		report_error(MALLOC_FAIL, "server in cbc_store_server_mysql");
	init_cbc_server(server);
	server->server_id = strtoul(row[0], NULL, 10);
	server->cust_id = strtoul(row[5], NULL, 10);
	server->vm_server_id = strtoul(row[6], NULL, 10);
	snprintf(server->name, HOST_S, "%s", row[7]);
	server->cuser = strtoul(row[8], NULL, 10);
	server->muser = strtoul(row[9], NULL, 10);
	convert_time(row[10], &(server->ctime));
	convert_time(row[11], &(server->mtime));
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
cbc_store_varient_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_varient_s *vari, *list;

	if (!(vari = malloc(sizeof(cbc_varient_s))))
		report_error(MALLOC_FAIL, "vari in cbc_store_varient_mysql");
	init_varient(vari);
	vari->varient_id = strtoul(row[0], NULL, 10);
	snprintf(vari->varient, HOST_S, "%s", row[1]);
	snprintf(vari->valias, MAC_S, "%s", row[2]);
	vari->cuser = strtoul(row[3], NULL, 10);
	vari->muser = strtoul(row[4], NULL, 10);
	convert_time(row[5], &(vari->ctime));
	convert_time(row[6], &(vari->mtime));
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
cbc_store_vmhost_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_vm_server_hosts_s *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cbc_vm_server_hosts_s))))
		report_error(MALLOC_FAIL, "vmhost in cbc_store_vmhost_mysql");
	init_vm_hosts(vmhost);
	vmhost->vm_s_id = strtoul(row[0], NULL, 10);
	snprintf(vmhost->vm_server, RBUFF_S, "%s", row[1]);
	snprintf(vmhost->type, MAC_S, "%s", row[2]);
	vmhost->server_id = strtoul(row[4], NULL, 10);
	vmhost->cuser = strtoul(row[5], NULL, 10);
	vmhost->muser = strtoul(row[6], NULL, 10);
	convert_time(row[7], &(vmhost->ctime));
	convert_time(row[8], &(vmhost->mtime));
	list = base->vmhost;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	}
}

void
cbc_store_syspack_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_syspack_s *spack, *list;

	if (!(spack = malloc(sizeof(cbc_syspack_s))))
		report_error(MALLOC_FAIL, "spack in cbc_store_syspack_mysql");
	init_cbc_syspack(spack);
	spack->syspack_id = strtoul(row[0], NULL, 10);
	snprintf(spack->name, URL_S, "%s", row[1]);
	spack->cuser = strtoul(row[2], NULL, 10);
	spack->muser = strtoul(row[3], NULL, 10);
	convert_time(row[4], &(spack->ctime));
	convert_time(row[5], &(spack->mtime));
	list = base->syspack;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = spack;
	} else {
		base->syspack = spack;
	}
}

void
cbc_store_sysarg_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_syspack_arg_s *spack, *list;

	if (!(spack = malloc(sizeof(cbc_syspack_arg_s))))
		report_error(MALLOC_FAIL, "spack in cbc_store_sysarg_mysql");
	init_cbc_syspack_arg(spack);
	spack->syspack_arg_id = strtoul(row[0], NULL, 10);
	spack->syspack_id = strtoul(row[1], NULL, 10);
	snprintf(spack->field, URL_S, "%s", row[2]);
	snprintf(spack->type, MAC_S, "%s", row[3]);
	spack->cuser = strtoul(row[4], NULL, 10);
	spack->muser = strtoul(row[5], NULL, 10);
	convert_time(row[6], &(spack->ctime));
	convert_time(row[7], &(spack->mtime));
	list = base->sysarg;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = spack;
	} else {
		base->sysarg = spack;
	}
}

void
cbc_store_sysconf_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_syspack_conf_s *spack, *list;

	if (!(spack = malloc(sizeof(cbc_syspack_conf_s))))
		report_error(MALLOC_FAIL, "spack in cbc_store_sysconf_mysql");
	init_cbc_syspack_conf(spack);
	spack->syspack_conf_id = strtoul(row[0], NULL, 10);
	spack->syspack_arg_id = strtoul(row[1], NULL, 10);
	spack->syspack_id = strtoul(row[2], NULL, 10);
	spack->bd_id = strtoul(row[3], NULL, 10);
	snprintf(spack->arg, RBUFF_S, "%s", row[4]);
	spack->cuser = strtoul(row[5], NULL, 10);
	spack->muser = strtoul(row[6], NULL, 10);
	convert_time(row[7], &(spack->ctime));
	convert_time(row[8], &(spack->mtime));
	list = base->sysconf;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = spack;
	} else {
		base->sysconf = spack;
	}
}

void
cbc_store_script_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_script_s *scr, *list;

	initialise_cbc_scripts(&scr);
	scr->systscr_id = strtoul(row[0], NULL, 10);
	snprintf(scr->name, CONF_S, "%s", row[1]);
	scr->cuser = strtoul(row[2], NULL, 10);
	scr->muser = strtoul(row[3], NULL, 10);
	convert_time(row[4], &(scr->ctime));
	convert_time(row[5], &(scr->mtime));
	list = base->scripts;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = scr;
	} else {
		base->scripts = scr;
	}
}

void
cbc_store_scripta_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_script_arg_s *arg, *list;

	initialise_cbc_script_args(&arg);
	arg->systscr_arg_id = strtoul(row[0], NULL, 10);
	arg->systscr_id = strtoul(row[1], NULL, 10);
	arg->bd_id = strtoul(row[2], NULL, 10);
	arg->bt_id = strtoul(row[3], NULL, 10);
	snprintf(arg->arg, CONF_S, "%s", row[4]);
	arg->no = strtoul(row[5], NULL, 10);
	arg->cuser = strtoul(row[6], NULL, 10);
	arg->muser = strtoul(row[7], NULL, 10);
	convert_time(row[8], &(arg->ctime));
	convert_time(row[9], &(arg->mtime));
	list = base->script_arg;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = arg;
	} else {
		base->script_arg = arg;
	}
}

void
cbc_store_partopt_mysql(MYSQL_ROW row, cbc_s *base)
{
	cbc_part_opt_s *opt, *list;

	initialise_cbc_part_opt(&opt);
	opt->part_options_id = strtoul(row[0], NULL, 10);
	opt->def_part_id = strtoul(row[1], NULL, 10);
	opt->def_scheme_id = strtoul(row[2], NULL, 10);
	opt->option = cmdb_malloc(CONF_S, "cbc_store_partopt_mysql");
	snprintf(opt->option, CONF_S, "%s", row[3]);
	opt->cuser = strtoul(row[4], NULL, 10);
	opt->muser = strtoul(row[5], NULL, 10);
	convert_time(row[6], &(opt->ctime));
	convert_time(row[7], &(opt->mtime));
	list = base->part_opt;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = opt;
	} else {
		base->part_opt = opt;
	}
}

void
cbc_setup_bind_mysql_build_domain(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->bdom->start_ip);
	else if (i == 1)
		*buffer = &(base->bdom->end_ip);
	else if (i == 2)
		*buffer = &(base->bdom->netmask);
	else if (i == 3)
		*buffer = &(base->bdom->gateway);
	else if (i == 4)
		*buffer = &(base->bdom->ns);
	else if (i == 5)
		*buffer = &(base->bdom->domain);
	else if (i == 6)
		*buffer = &(base->bdom->ntp_server);
	else if (i == 7)
		*buffer = &(base->bdom->config_ntp);
	else if (i == 8)
		*buffer = &(base->bdom->cuser);
	else if (i == 9)
		*buffer = &(base->bdom->muser);
}

void
cbc_setup_bind_mysql_build_os(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->bos->os);
	else if (i == 1)
		*buffer = &(base->bos->version);
	else if (i == 2)
		*buffer = &(base->bos->alias);
	else if (i == 3)
		*buffer = &(base->bos->ver_alias);
	else if (i == 4)
		*buffer = &(base->bos->arch);
	else if (i == 5)
		*buffer = &(base->bos->bt_id);
	else if (i == 6)
		*buffer = &(base->bos->cuser);
	else if (i == 7)
		*buffer = &(base->bos->muser);
}

void
cbc_setup_bind_mysql_build_varient(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->varient->varient);
	else if (i == 1)
		*buffer = &(base->varient->valias);
	else if (i == 2)
		*buffer = &(base->varient->cuser);
	else if (i == 3)
		*buffer = &(base->varient->muser);
}

void
cbc_setup_bind_mysql_build_part_scheme(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->sscheme->name);
	else if (i == 1)
		*buffer = &(base->sscheme->lvm);
	else if (i == 2)
		*buffer = &(base->sscheme->cuser);
	else if (i == 3)
		*buffer = &(base->sscheme->muser);
}

void
cbc_setup_bind_mysql_build_def_part(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->dpart->min);
	else if (i == 1)
		*buffer = &(base->dpart->max);
	else if (i == 2)
		*buffer = &(base->dpart->pri);
	else if (i == 3)
		*buffer = &(base->dpart->mount);
	else if (i == 4)
		*buffer = &(base->dpart->fs);
	else if (i == 5)
		*buffer = &(base->dpart->link_id.def_scheme_id);
	else if (i == 6)
		*buffer = &(base->dpart->log_vol);
	else if (i == 7)
		*buffer = &(base->dpart->cuser);
	else if (i == 8)
		*buffer = &(base->dpart->muser);
}

void
cbc_setup_bind_mysql_locale(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->locale->locale);
	else if (i == 1)
		*buffer = &(base->locale->country);
	else if (i == 2)
		*buffer = &(base->locale->language);
	else if (i == 3)
		*buffer = &(base->locale->keymap);
	else if (i == 4)
		*buffer = &(base->locale->name);
	else if (i == 5)
		*buffer = &(base->locale->timezone);
	else if (i == 6)
		*buffer = &(base->locale->cuser);
	else if (i == 7)
		*buffer = &(base->locale->muser);
}

void
cbc_setup_bind_mysql_build_package(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->package->package);
	else if (i == 1)
		*buffer = &(base->package->vari_id);
	else if (i == 2)
		*buffer = &(base->package->os_id);
	else if (i == 3)
		*buffer = &(base->package->cuser);
	else if (i == 4)
		*buffer = &(base->package->muser);
}

void
cbc_setup_bind_mysql_build_ip(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->bip->ip);
	else if (i == 1)
		*buffer = &(base->bip->host);
	else if (i == 2)
		*buffer = &(base->bip->domain);
	else if (i == 3)
		*buffer = &(base->bip->bd_id);
	else if (i == 4)
		*buffer = &(base->bip->server_id);
	else if (i == 5)
		*buffer = &(base->bip->cuser);
	else if (i == 6)
		*buffer = &(base->bip->muser);
}

void
cbc_setup_bind_mysql_build(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->build->mac_addr);
	else if (i == 1)
		*buffer = &(base->build->varient_id);
	else if (i == 2)
		*buffer = &(base->build->net_int);
	else if (i == 3)
		*buffer = &(base->build->server_id);
	else if (i == 4)
		*buffer = &(base->build->os_id);
	else if (i == 5)
		*buffer = &(base->build->ip_id);
	else if (i == 6)
		*buffer = &(base->build->locale_id);
	else if (i == 7)
		*buffer = &(base->build->def_scheme_id);
	else if (i == 8)
		*buffer = &(base->build->cuser);
	else if (i == 9)
		*buffer = &(base->build->muser);
}

void
cbc_setup_bind_mysql_build_disk(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->diskd->server_id);
	else if (i == 1)
		*buffer = &(base->diskd->device);
	else if (i == 2)
		*buffer = &(base->diskd->lvm);
}

void
cbc_setup_bind_mysql_syspack(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->syspack->name);
	else if (i == 1)
		*buffer = &(base->syspack->cuser);
	else if (i == 2)
		*buffer = &(base->syspack->muser);
}

void
cbc_setup_bind_mysql_sysarg(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->sysarg->syspack_id);
	else if (i == 1)
		*buffer = &(base->sysarg->field);
	else if (i == 2)
		*buffer = &(base->sysarg->type);
	else if (i == 3)
		*buffer = &(base->sysarg->cuser);
	else if (i == 4)
		*buffer = &(base->sysarg->muser);
}

void
cbc_setup_bind_mysql_sysconf(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->sysconf->syspack_arg_id);
	else if (i == 1)
		*buffer = &(base->sysconf->syspack_id);
	else if (i == 2)
		*buffer = &(base->sysconf->bd_id);
	else if (i == 3)
		*buffer = &(base->sysconf->arg);
	else if (i == 4)
		*buffer = &(base->sysconf->cuser);
	else if (i == 5)
		*buffer = &(base->sysconf->muser);
}

void
cbc_setup_bind_mysql_script(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->scripts->name);
	else if (i == 1)
		*buffer = &(base->scripts->cuser);
	else if (i == 2)
		*buffer = &(base->scripts->muser);
}

void
cbc_setup_bind_mysql_scripta(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->script_arg->systscr_id);
	else if (i == 1)
		*buffer = &(base->script_arg->bd_id);
	else if (i == 2)
		*buffer = &(base->script_arg->bt_id);
	else if (i == 3)
		*buffer = &(base->script_arg->arg);
	else if (i == 4)
		*buffer = &(base->script_arg->no);
	else if (i == 5)
		*buffer = &(base->script_arg->cuser);
	else if (i == 6)
		*buffer = &(base->script_arg->muser);
}

void
cbc_setup_bind_mysql_partopts(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->part_opt->def_part_id);
	else if (i == 1)
		*buffer = &(base->part_opt->def_scheme_id);
	else if (i == 2)
		*buffer = base->part_opt->option; // This is a pointer anyway
	else if (i == 3)
		*buffer = &(base->part_opt->cuser);
	else if (i == 4)
		*buffer = &(base->part_opt->muser);
}

void
cbc_setup_bind_mysql_servers(void **buffer, cbc_s *base, unsigned int i)
{
	if (i == 0)
		*buffer = &(base->server->name);
	else if (i == 5)
		*buffer = &(base->server->cust_id);
	else if (i == 6)
		*buffer = &(base->server->vm_server_id);
	else if (i == 7)
		*buffer = &(base->server->cuser);
	else if (i == 8)
		*buffer = &(base->server->muser);
	else if (i == 9)
		*buffer = &(base->server->ctime);
	else if (i == 10)
		*buffer = &(base->server->mtime);
}

#endif /* HAVE_MYSQL */

#ifdef HAVE_SQLITE3

int
cbc_run_query_sqlite(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	const char *query, *file;
	int retval = 0, i = 0;
	unsigned int fields;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	file = config->file;
	if ((retval = cbc_get_query(type, &query, &fields)) != 0) {
		report_error(retval, "cbc_run_query_sqlite");
	}
	cmdb_setup_ro_sqlite(query, file, &cbc, &state);
	fields = (unsigned int) sqlite3_column_count(state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW) {
		i++;
		cbc_store_result_sqlite(state, base, type, fields);
	}
	cmdb_sqlite_cleanup(cbc, state);
	if (i == 0)
		return NO_RECORDS;
	else
		return NONE;
}

int
cbc_run_insert_sqlite(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	const char *query, *file = NULL;
	int retval;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	retval = 0;
	query = cbc_sql_insert[type];
	if (config)
		file = config->file;
	else
		report_error(CBC_NO_DATA, "config in cbc_run_insert_sqlite");
	cmdb_setup_rw_sqlite(query, file, &cbc, &state);
	if ((retval = cbc_setup_insert_sqlite_bind(state, base, type)) != 0) {
		printf("Error: %s\n", sqlite3_errmsg(cbc));
		sqlite3_close(cbc);
		return retval;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cbc));
		cmdb_sqlite_cleanup(cbc, state);
		retval = SQLITE_INSERT_FAILED;
		return retval;
	}
	cmdb_sqlite_cleanup(cbc, state);
	return NONE;
}

int
cbc_run_multiple_query_sqlite(ailsa_cmdb_s *config, cbc_s *base, int type)
{
	int retval;
	retval = NONE;
	if (type & BOOT_LINE)
		if ((retval = cbc_run_query_sqlite(config, base, BOOT_LINE)) != 0)
			return retval;
	if (type & BUILD)
		if ((retval = cbc_run_query_sqlite(config, base, BUILD)) != 0)
			return retval;
	if (type & BUILD_DOMAIN)
		if ((retval = cbc_run_query_sqlite(config, base, BUILD_DOMAIN)) != 0)
			return retval;
	if (type & BUILD_IP)
		if ((retval = cbc_run_query_sqlite(config, base, BUILD_IP)) != 0)
			return retval;
	if (type & BUILD_OS)
		if ((retval = cbc_run_query_sqlite(config, base, BUILD_OS)) != 0)
			return retval;
	if (type & BUILD_TYPE)
		if ((retval = cbc_run_query_sqlite(config, base, BUILD_TYPE)) != 0)
			return retval;
	if (type & DISK_DEV)
		if ((retval = cbc_run_query_sqlite(config, base, DISK_DEV)) != 0)
			return retval;
	if (type & LOCALE)
		if ((retval = cbc_run_query_sqlite(config, base, LOCALE)) != 0)
			return retval;
	if (type & BPACKAGE)
		if ((retval = cbc_run_query_sqlite(config, base, BPACKAGE)) != 0)
			return retval;
	if (type & DPART)
		if ((retval = cbc_run_query_sqlite(config, base, DPART)) != 0)
			return retval;
	if (type & SSCHEME)
		if ((retval = cbc_run_query_sqlite(config, base, SSCHEME)) != 0)
			return retval;
	if (type & CSERVER)
		if ((retval = cbc_run_query_sqlite(config, base, CSERVER)) != 0)
			return retval;
	if (type & VARIENT)
		if ((retval = cbc_run_query_sqlite(config, base, VARIENT)) != 0)
			return retval;
	if (type & VMHOST)
		if ((retval = cbc_run_query_sqlite(config, base, VMHOST)) != 0)
			return retval;
	if (type & SYSPACK)
		if ((retval = cbc_run_query_sqlite(config, base, SYSPACK)) != 0)
			return retval;
	if (type & SYSARG)
		if ((retval = cbc_run_query_sqlite(config, base, SYSARG)) != 0)
			return retval;
	if (type & SYSCONF)
		if ((retval = cbc_run_query_sqlite(config, base, SYSCONF)) != 0)
			return retval;
	if (type & SCRIPT)
		if ((retval = cbc_run_query_sqlite(config, base, SCRIPT)) != 0)
			return retval;
	if (type & SCRIPTA)
		if ((retval = cbc_run_query_sqlite(config, base, SCRIPTA)) != 0)
			return retval;
	if (type & PARTOPT)
		if ((retval = cbc_run_query_sqlite(config, base, PARTOPT)) != 0)
			return retval;
	return retval;
}

int
cbc_run_delete_sqlite(ailsa_cmdb_s *ccs, dbdata_s *data, int type)
{
	const char *query = cbc_sql_delete[type], *file = NULL;
	int retval = 0;
	unsigned int i;
	dbdata_s *list = NULL;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	if (ccs)
		file = ccs->file;
	else
		report_error(CBC_NO_DATA, "ccs in cbc_run_delete_sqlite");
	if (data)
		list = data;
	else
		report_error(CBC_NO_DATA, "data in cbc_run_delete_sqlite");
	cmdb_setup_rw_sqlite(query, file, &cbc, &state);
	for (i = 1; i <= cbc_delete_args[type]; i++) {
		if (!list)
			break;
		if (cbc_delete_types[type][i - 1] == DBINT) {
			if ((sqlite3_bind_int64(state, (int)i, (sqlite3_int64)list->args.number)) > 0) {
				fprintf(stderr, "Cannot bind arg %ud\n", i);
				return retval;
			}
		} else if (cbc_delete_types[type][i - 1] == DBTEXT) {
			if ((sqlite3_bind_text(state, (int)i, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
				fprintf(stderr, "Cannot bind arg %ud\n", i);
				return retval;
			}
		} else
			return WRONG_TYPE;
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cbc));
		cmdb_sqlite_cleanup(cbc, state);
		return NONE;
	}
	retval = sqlite3_changes(cbc);
	cmdb_sqlite_cleanup(cbc, state);
	return retval;
}

int
cbc_run_search_sqlite(ailsa_cmdb_s *ccs, dbdata_s *data, int type)
{
	const char *query = cbc_sql_search[type], *file = NULL;
	int retval = 0, i;
	dbdata_s *list = NULL;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	if (data)
		list = data;
	else
		report_error(CBC_NO_DATA, "data in cbc_run_search_sqlite");
	if (ccs)
		file = ccs->file;
	else
		report_error(CBC_NO_DATA, "css in cbc_run_search_sqlite");
	cmdb_setup_ro_sqlite(query, file, &cbc, &state);
	for (i = 0; (unsigned long)i < cbc_search_args[type]; i++) {
		set_cbc_search_sqlite(state, list, type, i);
		if (list->next)
			list = list->next;
	}
	list = data;
	i = 0;
	while ((retval = sqlite3_step(state)) == SQLITE_ROW) {
		get_cbc_search_res_sqlite(state, list, type, i);
		i++;
	}
	cmdb_sqlite_cleanup(cbc, state);
	return i;
}

int
cbc_run_update_sqlite(ailsa_cmdb_s *ccs, dbdata_s *data, int type)
{
	const char *query = cbc_sql_update[type], *file = ccs->file;
	int retval = NONE, i;
	dbdata_s *list = data;
	sqlite3 *cbc;
	sqlite3_stmt *state;

	cmdb_setup_rw_sqlite(query, file, &cbc, &state);
	for (i = 0; (unsigned long)i < cbc_update_args[type]; i++) {
		set_cbc_update_sqlite(state, list, type, i);
		list = list->next;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		printf("Recieved error: %s\n", sqlite3_errmsg(cbc));
		cmdb_sqlite_cleanup(cbc, state);
		return NONE;
	}
	i = sqlite3_changes(cbc);
	cmdb_sqlite_cleanup(cbc, state);
	return i;
}


int
set_cbc_search_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = 0;

	if (cbc_search_arg_types[type][i] == DBTEXT) {
		if ((retval = sqlite3_bind_text(
state, i + 1, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0)
			report_error(MY_BIND_FAIL, sqlite3_errstr(retval));
	} else if (cbc_search_arg_types[type][i] == DBINT) {
		if ((retval = sqlite3_bind_int64(
state, i + 1, (sqlite3_int64)list->args.number)) > 0)
			report_error(MY_BIND_FAIL, sqlite3_errstr(retval));
	} else if (cbc_search_arg_types[type][i] == DBSHORT) {
		if ((retval = sqlite3_bind_int(state, i + 1, list->args.small)) > 0)
			report_error(MY_BIND_FAIL, sqlite3_errstr(retval));
	} else {
		report_error(DB_WRONG_TYPE, cbc_sql_search[type]);
	}
	return retval;
}

int
set_cbc_update_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = 0;

	if (cbc_update_types[type][i] == DBTEXT) {
		if ((retval = sqlite3_bind_text(
state, i + 1, list->args.text, (int)strlen(list->args.text), SQLITE_STATIC)) > 0) {
			report_error(MY_BIND_FAIL, sqlite3_errstr(retval));
		}
	} else if (cbc_update_types[type][i] == DBINT) {
		if ((retval = sqlite3_bind_int64(
state, i + 1, (sqlite3_int64)list->args.number)) > 0) {
			report_error(MY_BIND_FAIL, sqlite3_errstr(retval));
		}
	} else if (cbc_update_types[type][i] == DBSHORT) {
		if ((retval = sqlite3_bind_int(state, i + 1, list->args.small)) > 0) {
			report_error(MY_BIND_FAIL, sqlite3_errstr(retval));
		}
	} else {
		report_error(DB_WRONG_TYPE, cbc_sql_update[type]);
	}
	return retval;
}

int
get_cbc_search_res_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i)
{
	int retval = 0, j, k;
	unsigned int u;
	dbdata_s *data;

	if (i > 0) {
		for (k = 1; k <= i; k++) {
			for (u = 1; u <= cbc_search_fields[type]; u++)
				if ((u != cbc_search_fields[type]) || ( k != i))
					list = list->next;
		}
		for (j = 0; (unsigned)j < cbc_search_fields[type]; j++) {
			if (!(data = malloc(sizeof(dbdata_s))))
				report_error(MALLOC_FAIL, "dbdata_s in get_cbc_search_res_sqlite");
			init_dbdata_struct(data);
			if (cbc_search_field_types[type][j] == DBTEXT)
				snprintf(data->fields.text, RBUFF_S, "%s", sqlite3_column_text(state, j));
			else if (cbc_search_field_types[type][j] == DBINT)
				data->fields.number = (unsigned long int)sqlite3_column_int64(state, j);
			else if (cbc_search_field_types[type][j] == DBSHORT)
				data->fields.small = (short int)sqlite3_column_int(state, j);
			else
				report_error(DB_WRONG_TYPE, cbc_sql_search[type]);
			if (list->next)
				clean_dbdata_struct(list->next);
			list->next = data;
			list = list->next;
		}
	} else {
		for (j = 0; (unsigned)j < cbc_search_fields[type]; j++) {
			if (cbc_search_field_types[type][j] == DBTEXT)
				snprintf(list->fields.text, RBUFF_S, "%s", sqlite3_column_text(state, j));
			else if (cbc_search_field_types[type][j] == DBINT)
				list->fields.number = (unsigned long int)sqlite3_column_int64(state, j);
			else if (cbc_search_field_types[type][j] == DBSHORT)
				list->fields.small = (short int)sqlite3_column_int(state, j);
			else
				report_error(DB_WRONG_TYPE, cbc_sql_search[type]);
			list = list->next;
		}
	}
	return retval;
}

void
cbc_store_result_sqlite(sqlite3_stmt *state, cbc_s *base, int type, unsigned int fields)
{
	unsigned int required;
/*	if (type == BOOT_LINE) {
		required = cbc_select_fields[BOOT_LINES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_boot_line_sqlite(state, base);
	} else */ if (type == BUILD) {
		required = cbc_select_fields[BUILDS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_sqlite(state, base);
	} else if (type == BUILD_DOMAIN) {
		required = cbc_select_fields[BUILD_DOMAINS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_domain_sqlite(state, base);
	} else if (type == BUILD_IP) {
		required = cbc_select_fields[BUILD_IPS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_ip_sqlite(state, base);
	} else if (type == BUILD_OS) {
		required = cbc_select_fields[BUILD_OSS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_os_sqlite(state, base);
	} else if (type == BUILD_TYPE) {
		required = cbc_select_fields[BUILD_TYPES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_build_type_sqlite(state, base);
	} else if (type == DISK_DEV) {
		required = cbc_select_fields[DISK_DEVS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_disk_dev_sqlite(state, base);
	} else if (type == LOCALE) {
		required = cbc_select_fields[LOCALES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_locale_sqlite(state, base);
	} else if (type == BPACKAGE) {
		required = cbc_select_fields[BPACKAGES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_package_sqlite(state, base);
	} else if (type == DPART) {
		required = cbc_select_fields[DPARTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_dpart_sqlite(state, base);
	} else if (type == SSCHEME) {
		required = cbc_select_fields[SSCHEMES];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_seed_scheme_sqlite(state, base);
	} else if (type == CSERVER) {
		required = cbc_select_fields[CSERVERS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_server_sqlite(state, base);
	} else if (type == VARIENT) {
		required = cbc_select_fields[VARIENTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_varient_sqlite(state, base);
	} else if (type == VMHOST) {
		required = cbc_select_fields[VMHOSTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_vmhost_sqlite(state, base);
	} else if (type == SYSPACK) {
		required = cbc_select_fields[SYSPACKS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_syspack_sqlite(state, base);
	} else if (type == SYSARG) {
		required = cbc_select_fields[SYSARGS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_sysarg_sqlite(state, base);
	} else if (type == SYSCONF) {
		required = cbc_select_fields[SYSCONFS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_sysconf_sqlite(state, base);
	} else if (type == SCRIPT) {
		required = cbc_select_fields[SCRIPTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_script_sqlite(state, base);
	} else if (type == SCRIPTA) {
		required = cbc_select_fields[SCRIPTAS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_scripta_sqlite(state, base);
	} else if (type == PARTOPT) {
		required = cbc_select_fields[PARTOPTS];
		if (fields != required)
			cbc_query_mismatch(fields, required, type);
		cbc_store_partopt_sqlite(state, base);
	} else {
		report_error(UNKNOWN_STRUCT_DB_TABLE, "cbc_store_result_sqlite");
	}
}

int
cbc_setup_insert_sqlite_bind(sqlite3_stmt *state, cbc_s *base, int type)
{
	int retval = NONE;

	if (type == BUILD_DOMAINS)
		retval = cbc_setup_bind_sqlite_build_domain(state, base->bdom);
	else if (type == BUILD_OSS)
		retval = cbc_setup_bind_sqlite_build_os(state, base->bos);
	else if (type == VARIENTS)
		retval = cbc_setup_bind_sqlite_build_varient(state, base->varient);
	else if (type == SSCHEMES)
		retval = cbc_setup_bind_sqlite_build_part_scheme(state, base->sscheme);
	else if (type == DPARTS)
		retval = cbc_setup_bind_sqlite_build_part(state, base->dpart);
	else if (type == BPACKAGES)
		retval = cbc_setup_bind_sqlite_build_pack(state, base->package);
	else if (type == BUILD_IPS)
		retval = cbc_setup_bind_sqlite_build_ip(state, base->bip);
	else if (type == BUILDS)
		retval = cbc_setup_bind_sqlite_build(state, base->build);
	else if (type == DISK_DEVS)
		retval = cbc_setup_bind_sqlite_build_disk(state, base->diskd);
	else if (type == LOCALES)
		retval = cbc_setup_bind_sqlite_locale(state, base->locale);
	else if (type == SYSPACKS)
		retval = cbc_setup_bind_sqlite_syspack(state, base->syspack);
	else if (type == SYSARGS)
		retval = cbc_setup_bind_sqlite_sysarg(state, base->sysarg);
	else if (type == SYSCONFS)
		retval = cbc_setup_bind_sqlite_sysconf(state, base->sysconf);
	else if (type == SCRIPTS)
		retval = cbc_setup_bind_sqlite_script(state, base->scripts);
	else if (type == SCRIPTAS)
		retval = cbc_setup_bind_sqlite_scripta(state, base->script_arg);
	else if (type == PARTOPTS)
		retval = cbc_setup_bind_sqlite_partopt(state, base->part_opt);
	else if (type == CSERVERS)
		retval = cbc_setup_bind_sqlite_server(state, base->server);
	else
		report_error(UNKNOWN_STRUCT_DB_TABLE, "cbc_run_insert_sqlite");
	return retval;
}
/*
void
cbc_store_boot_line_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	cbc_boot_line_s *boot, *list;

	if (!(boot = malloc(sizeof(cbc_boot_line_s))))
		report_error(MALLOC_FAIL, "boot in cbc_store_boot_line_sqlite");
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
} */

void
cbc_store_build_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_build_s *build, *list;

	if (!(build = malloc(sizeof(cbc_build_s))))
		report_error(MALLOC_FAIL, "build in cbc_store_build_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_build_sqlite");
	init_build_struct(build);
	build->build_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(build->mac_addr, MAC_S, "%s", sqlite3_column_text(state, 1));
	build->varient_id = (unsigned long int) sqlite3_column_int64(state, 2);
	snprintf(build->net_int, RANGE_S, "%s", sqlite3_column_text(state, 3));
	build->server_id = (unsigned long int) sqlite3_column_int64(state, 4);
	build->os_id = (unsigned long int) sqlite3_column_int64(state, 5);
	build->ip_id = (unsigned long int) sqlite3_column_int64(state, 6);
	build->locale_id = (unsigned long int) sqlite3_column_int64(state, 7);
	build->def_scheme_id = (unsigned long int) sqlite3_column_int64(state, 8);
	build->cuser = (unsigned long int) sqlite3_column_int64(state, 9);
	build->muser = (unsigned long int) sqlite3_column_int64(state, 10);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 11));
	convert_time(stime, &(build->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 12));
	convert_time(stime, &(build->mtime));
	list = base->build;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = build;
	} else {
		base->build = build;
	}
	free(stime);
}

void
cbc_store_build_domain_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_build_domain_s *dom, *list;

	if (!(dom = malloc(sizeof(cbc_build_domain_s))))
		report_error(MALLOC_FAIL, "dom in cbc_store_build_domain_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_build_domain_sqlite");
	init_build_domain(dom);
	dom->bd_id = (unsigned long int) sqlite3_column_int64(state, 0);
	dom->start_ip = (unsigned long int) sqlite3_column_int64(state, 1);
	dom->end_ip = (unsigned long int) sqlite3_column_int64(state, 2);
	dom->netmask = (unsigned long int) sqlite3_column_int64(state, 3);
	dom->gateway = (unsigned long int) sqlite3_column_int64(state, 4);
	dom->ns = (unsigned long int) sqlite3_column_int64(state, 5);
	snprintf(dom->domain, RBUFF_S, "%s", sqlite3_column_text(state, 6));
	if ((dom->config_ntp = (short int) sqlite3_column_int(state, 8)) != 0)
		snprintf(dom->ntp_server, HOST_S, "%s",
		 sqlite3_column_text(state, 7));
	dom->cuser = (unsigned long int) sqlite3_column_int64(state, 9);
	dom->muser = (unsigned long int) sqlite3_column_int64(state, 10);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 11));
	convert_time(stime, &(dom->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 12));
	convert_time(stime, &(dom->mtime));
	list = base->bdom;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = dom;
	} else {
		base->bdom = dom;
	}
	free(stime);
}

void
cbc_store_build_ip_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_build_ip_s *ip, *list;

	if (!(ip = malloc(sizeof(cbc_build_ip_s))))
		report_error(MALLOC_FAIL, "ip in cbc_store_build_ip_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_build_ip_sqlite");
	init_build_ip(ip);
	ip->ip_id = (unsigned long int) sqlite3_column_int64(state, 0);
	ip->ip = (unsigned long int) sqlite3_column_int64(state, 1);
	snprintf(ip->host, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(ip->domain, RBUFF_S, "%s", sqlite3_column_text(state, 3));
	ip->bd_id = (unsigned long int) sqlite3_column_int64(state, 4);
	ip->server_id = (unsigned long int) sqlite3_column_int64(state, 5);
	ip->cuser = (unsigned long int) sqlite3_column_int64(state, 6);
	ip->muser = (unsigned long int) sqlite3_column_int64(state, 7);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 8));
	convert_time(stime, &(ip->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 9));
	convert_time(stime, &(ip->mtime));
	list = base->bip;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = ip;
	} else {
		base->bip = ip;
	}
	free(stime);
}

void
cbc_store_build_os_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_build_os_s *os, *list;

	if (!(os = malloc(sizeof(cbc_build_os_s))))
		report_error(MALLOC_FAIL, "os in cbc_store_build_os_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_build_os_sqlite");
	init_build_os(os);
	os->os_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(os->os, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(os->version, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(os->alias, MAC_S, "%s", sqlite3_column_text(state, 3));
	snprintf(os->ver_alias, MAC_S, "%s", sqlite3_column_text(state, 4));
	snprintf(os->arch, RANGE_S, "%s", sqlite3_column_text(state, 5));
	os->bt_id = (unsigned long int) sqlite3_column_int64(state, 6);
	os->cuser = (unsigned long int) sqlite3_column_int64(state, 7);
	os->muser = (unsigned long int) sqlite3_column_int64(state, 8);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 9));
	convert_time(stime, &(os->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 10));
	convert_time(stime, &(os->mtime));
	list = base->bos;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = os;
	} else {
		base->bos = os;
	}
	free(stime);
}

void
cbc_store_build_type_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	cbc_build_type_s *type, *list;

	if (!(type = malloc(sizeof(cbc_build_type_s))))
		report_error(MALLOC_FAIL, "type in cbc_store_build_type_sqlite");
	init_build_type(type);
	type->bt_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(type->alias, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(type->build_type, MAC_S, "%s", sqlite3_column_text(state, 2));
	snprintf(type->arg, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(type->url, CONF_S, "%s", sqlite3_column_text(state, 4));
	snprintf(type->mirror, RBUFF_S, "%s", sqlite3_column_text(state, 5));
	snprintf(type->boot_line, URL_S, "%s", sqlite3_column_text(state, 6));
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
cbc_store_disk_dev_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	cbc_disk_dev_s *disk, *list;

	if (!(disk = malloc(sizeof(cbc_disk_dev_s))))
		report_error(MALLOC_FAIL, "disk in cbc_store_disk_dev_sqlite");
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
cbc_store_locale_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_locale_s *loc, *list;

	if (!(loc = malloc(sizeof(cbc_locale_s))))
		report_error(MALLOC_FAIL, "loc in cbc_store_locale_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_locale_sqlite");
	init_locale(loc);
	loc->locale_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(loc->locale, MAC_S, "%s", sqlite3_column_text(state, 1));
	snprintf(loc->country, RANGE_S, "%s", sqlite3_column_text(state, 2));
	snprintf(loc->language, RANGE_S, "%s", sqlite3_column_text(state, 3));
	snprintf(loc->keymap, RANGE_S, "%s", sqlite3_column_text(state, 4));
	snprintf(loc->timezone, HOST_S, "%s", sqlite3_column_text(state, 5));
	snprintf(loc->name, HOST_S, "%s", sqlite3_column_text(state, 6));
	loc->cuser = (unsigned long int) sqlite3_column_int64(state, 7);
	loc->muser = (unsigned long int) sqlite3_column_int64(state, 8);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 9));
	convert_time(stime, &(loc->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 10));
	convert_time(stime, &(loc->mtime));
	list = base->locale;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = loc;
	} else {
		base->locale = loc;
	}
	free(stime);
}

void
cbc_store_package_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_package_s *pack, *list;

	if (!(pack = malloc(sizeof(cbc_package_s))))
		report_error(MALLOC_FAIL, "pack in cbc_store_package_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_package_sqlite");
	init_package(pack);
	pack->pack_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(pack->package, HOST_S, "%s", sqlite3_column_text(state, 1));
	pack->vari_id = (unsigned long int) sqlite3_column_int64(state, 2);
	pack->os_id = (unsigned long int) sqlite3_column_int64(state, 3);
	pack->cuser = (unsigned long int) sqlite3_column_int64(state, 4);
	pack->muser = (unsigned long int) sqlite3_column_int64(state, 5);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(pack->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(pack->mtime));
	list = base->package;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = pack;
	} else {
		base->package = pack;
	}
	free(stime);
}

void
cbc_store_dpart_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_pre_part_s *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_s))))
		report_error(MALLOC_FAIL, "part in cbc_store_dpart_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_dpart_sqlite");
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
	part->cuser = (unsigned long int) sqlite3_column_int64(state, 8);
	part->muser = (unsigned long int) sqlite3_column_int64(state, 9);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 10));
	convert_time(stime, &(part->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 11));
	convert_time(stime, &(part->mtime));
	list = base->dpart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->dpart = part;
	}
	free(stime);
}
/*
void
cbc_store_spart_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_pre_part_s *part, *list;

	if (!(part = malloc(sizeof(cbc_pre_part_s))))
		report_error(MALLOC_FAIL, "part in cbc_store_spart_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_spart_sqlite");
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
	part->cuser = (unsigned long int) sqlite3_column_int64(state, 9);
	part->muser = (unsigned long int) sqlite3_column_int64(state, 10);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 11));
	convert_time(stime, &(part->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 12));
	convert_time(stime, &(part->mtime));
	list = base->spart;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = part;
	} else {
		base->spart = part;
	}
	free(stime);
} */

void
cbc_store_seed_scheme_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_seed_scheme_s *seed, *list;

	if (!(seed = malloc(sizeof(cbc_seed_scheme_s))))
		report_error(MALLOC_FAIL, "seed in cbc_store_seed_scheme_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_seed_scheme_sqlite");
	init_seed_scheme(seed);
	seed->def_scheme_id = 
	   (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(seed->name, CONF_S, "%s",
		 sqlite3_column_text(state, 1));
	seed->lvm = (short int) sqlite3_column_int(state, 2);
	seed->cuser = (unsigned long int) sqlite3_column_int64(state, 3);
	seed->muser = (unsigned long int) sqlite3_column_int64(state, 4);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 5));
	convert_time(stime, &(seed->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(seed->mtime));
	list = base->sscheme;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = seed;
	} else {
		base->sscheme = seed;
	}
	free(stime);
}

void
cbc_store_server_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_server_s *server, *list;

	if (!(server = malloc(sizeof(cbc_server_s))))
		report_error(MALLOC_FAIL, "server in cbc_store_server_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_server_sqlite");
	init_cbc_server(server);
	server->server_id = (unsigned long int) sqlite3_column_int64(state, 0);
	server->cust_id = (unsigned long int) sqlite3_column_int64(state, 5);
	server->vm_server_id = (unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(server->name, HOST_S, "%s", sqlite3_column_text(state, 7));
	server->cuser = (unsigned long int) sqlite3_column_int64(state, 8);
	server->muser = (unsigned long int) sqlite3_column_int64(state, 9);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 10));
	convert_time(stime, &(server->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 11));
	convert_time(stime, &(server->mtime));
	list = base->server;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = server;
	} else {
		base->server = server;
	}
	free(stime);
}

void
cbc_store_varient_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_varient_s *vari, *list;

	if (!(vari = malloc(sizeof(cbc_varient_s))))
		report_error(MALLOC_FAIL, "vari in cbc_store_varient_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_varient_sqlite");
	init_varient(vari);
	vari->varient_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(vari->varient, HOST_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vari->valias, MAC_S, "%s", sqlite3_column_text(state, 2));
	vari->cuser = (unsigned long int) sqlite3_column_int64(state, 3);
	vari->muser = (unsigned long int) sqlite3_column_int64(state, 4);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 5));
	convert_time(stime, &(vari->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(vari->mtime));
	list = base->varient;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vari;
	} else {
		base->varient = vari;
	}
	free(stime);
}

void
cbc_store_vmhost_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_vm_server_hosts_s *vmhost, *list;

	if (!(vmhost = malloc(sizeof(cbc_vm_server_hosts_s))))
		report_error(MALLOC_FAIL, "vmhost in cbc_store_vmhost_mysql");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_vmhost_mysql");
	init_vm_hosts(vmhost);
	vmhost->vm_s_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(vmhost->vm_server, RBUFF_S, "%s", sqlite3_column_text(state, 1));
	snprintf(vmhost->type, MAC_S, "%s", sqlite3_column_text(state, 2));
	vmhost->server_id = (unsigned long int) sqlite3_column_int64(state, 3);
	vmhost->cuser = (unsigned long int) sqlite3_column_int64(state, 4);
	vmhost->muser = (unsigned long int) sqlite3_column_int64(state, 5);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(vmhost->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(vmhost->mtime));
	list = base->vmhost;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = vmhost;
	} else {
		base->vmhost = vmhost;
	}
	free(stime);
}

void
cbc_store_syspack_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_syspack_s *spack, *list;

	if (!(spack = malloc(sizeof(cbc_syspack_s))))
		report_error(MALLOC_FAIL, "spack in cbc_store_syspack_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_syspack_sqlite");
	init_cbc_syspack(spack);
	spack->syspack_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(spack->name, URL_S, "%s", sqlite3_column_text(state, 1));
	spack->cuser = (unsigned long int) sqlite3_column_int64(state, 2);
	spack->muser = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 4));
	convert_time(stime, &(spack->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 5));
	convert_time(stime, &(spack->mtime));
	list = base->syspack;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = spack;
	} else {
		base->syspack = spack;
	}
	free(stime);
}

void
cbc_store_sysarg_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_syspack_arg_s *spack, *list;

	if (!(spack = malloc(sizeof(cbc_syspack_arg_s))))
		report_error(MALLOC_FAIL, "spack in cbc_store_sysarg_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_sysarg_sqlite");
	init_cbc_syspack_arg(spack);
	spack->syspack_arg_id = (unsigned long int) sqlite3_column_int64(state, 0);
	spack->syspack_id = (unsigned long int) sqlite3_column_int64(state, 1);
	snprintf(spack->field, URL_S, "%s", sqlite3_column_text(state, 2));
	snprintf(spack->type, MAC_S, "%s", sqlite3_column_text(state, 3));
	spack->cuser = (unsigned long int) sqlite3_column_int64(state, 4);
	spack->muser = (unsigned long int) sqlite3_column_int64(state, 5);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(spack->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(spack->mtime));
	list = base->sysarg;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = spack;
	} else {
		base->sysarg = spack;
	}
	free(stime);
}

void
cbc_store_sysconf_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_syspack_conf_s *spack, *list;

	if (!(spack = malloc(sizeof(cbc_syspack_conf_s))))
		report_error(MALLOC_FAIL, "spack in cbc_store_sysarg_sqlite");
	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_sysarg_sqlite");
	init_cbc_syspack_conf(spack);
	spack->syspack_conf_id = (unsigned long int) sqlite3_column_int64(state, 0);
	spack->syspack_arg_id = (unsigned long int) sqlite3_column_int64(state, 1);
	spack->syspack_id = (unsigned long int) sqlite3_column_int64(state, 2);
	spack->bd_id = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(spack->arg, RBUFF_S, "%s", sqlite3_column_text(state, 4));
	spack->cuser = (unsigned long int) sqlite3_column_int64(state, 5);
	spack->muser = (unsigned long int) sqlite3_column_int64(state, 6);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(spack->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 8));
	convert_time(stime, &(spack->mtime));
	list = base->sysconf;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = spack;
	} else {
		base->sysconf = spack;
	}
	free(stime);
}

void
cbc_store_script_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_script_s *scr, *list;

	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_script_sqlite");
	initialise_cbc_scripts(&scr);
	scr->systscr_id = (unsigned long int) sqlite3_column_int64(state, 0);
	snprintf(scr->name, CONF_S, "%s", sqlite3_column_text(state, 1));
	scr->muser = (unsigned long int) sqlite3_column_int64(state, 2);
	scr->cuser = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 4));
	convert_time(stime, &(scr->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 5));
	convert_time(stime, &(scr->mtime));
	list = base->scripts;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = scr;
	} else {
		base->scripts = scr;
	}
	free(stime);
}

void
cbc_store_scripta_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_script_arg_s *arg, *list;

	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_scripta_sqlite");
	initialise_cbc_script_args(&arg);
	arg->systscr_arg_id = (unsigned long int) sqlite3_column_int64(state, 0);
	arg->systscr_id = (unsigned long int) sqlite3_column_int64(state, 1);
	arg->bd_id = (unsigned long int) sqlite3_column_int64(state, 2);
	arg->bt_id = (unsigned long int) sqlite3_column_int64(state, 3);
	snprintf(arg->arg, CONF_S, "%s", sqlite3_column_text(state, 4));
	arg->no = (unsigned long int) sqlite3_column_int64(state, 5);
	arg->cuser = (unsigned long int) sqlite3_column_int64(state, 6);
	arg->muser = (unsigned long int) sqlite3_column_int64(state, 7);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 8));
	convert_time(stime, &(arg->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 9));
	convert_time(stime, &(arg->mtime));
	list = base->script_arg;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = arg;
	} else {
		base->script_arg = arg;
	}
	free(stime);
}

void
cbc_store_partopt_sqlite(sqlite3_stmt *state, cbc_s *base)
{
	char *stime;
	cbc_part_opt_s *opt, *list;

	if (!(stime = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "stime in cbc_store_partopt_sqlite");
	initialise_cbc_part_opt(&opt);
	opt->part_options_id = (unsigned long int) sqlite3_column_int64(state, 0);
	opt->def_part_id = (unsigned long int) sqlite3_column_int64(state, 1);
	opt->def_scheme_id = (unsigned long int) sqlite3_column_int64(state, 2);
	if (!(opt->option))
		opt->option = cmdb_malloc(CONF_S, "opt->option in cbc_store_partopt_sqlite");
	snprintf(opt->option, CONF_S, "%s", sqlite3_column_text(state, 3));
	opt->cuser = (unsigned long int) sqlite3_column_int64(state, 4);
	opt->muser = (unsigned long int) sqlite3_column_int64(state, 5);
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 6));
	convert_time(stime, &(opt->ctime));
	snprintf(stime, MAC_S, "%s", sqlite3_column_text(state, 7));
	convert_time(stime, &(opt->mtime));
	list = base->part_opt;
	if (list) {
		while (list->next)
			list = list->next;
		list->next = opt;
	} else {
		base->part_opt = opt;
	}
	free(stime);
}

int
cbc_setup_bind_sqlite_build(sqlite3_stmt *state, cbc_build_s *build)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, build->mac_addr, (int)strlen(build->mac_addr), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind mac addr %s\n", build->mac_addr);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)build->varient_id)) > 0) {
		fprintf(stderr, "Cannot bind varient_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, build->net_int, (int)strlen(build->net_int), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind domain %s\n", build->net_int);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)build->server_id)) > 0) {
		fprintf(stderr, "Cannot bind server_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)build->os_id)) > 0) {
		fprintf(stderr, "Cannot bind os_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 6, (sqlite3_int64)build->ip_id)) > 0) {
		fprintf(stderr, "Cannot bind ip_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 7, (sqlite3_int64)build->locale_id)) > 0) {
		fprintf(stderr, "Cannot bind locale_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 8, (sqlite3_int64)build->def_scheme_id)) > 0) {
		fprintf(stderr, "Cannot bind def_scheme_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 9, (sqlite3_int64)build->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 10, (sqlite3_int64)build->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser\n");
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_domain(sqlite3_stmt *state, cbc_build_domain_s *bdom)
{
	int retval;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite3_int64)bdom->start_ip)) > 0) {
		fprintf(stderr, "Cannot bind start_ip\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)bdom->end_ip)) > 0) {
		fprintf(stderr, "Cannot bind end_ip\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)bdom->netmask)) > 0) {
		fprintf(stderr, "Cannot bind netmask\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)bdom->gateway)) > 0) {
		fprintf(stderr, "Cannot bind gateway\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)bdom->ns)) > 0) {
		fprintf(stderr, "Cannot bind ns\n");
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 6, bdom->domain, (int)strlen(bdom->domain), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind domain %s\n", bdom->domain);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 7, bdom->ntp_server, (int)strlen(bdom->ntp_server), SQLITE_STATIC)) > 0){
		fprintf(stderr,
"Cannot bind ntp server %s\n", bdom->ntp_server);
		return retval;
	}
	if ((retval = sqlite3_bind_int(state, 8, bdom->config_ntp)) > 0) {
		fprintf(stderr, "Cannot bind config ntp");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 9, (sqlite3_int64)bdom->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 10, (sqlite3_int64)bdom->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser");
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_ip(sqlite3_stmt *state, cbc_build_ip_s *bip)
{
	int retval;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite3_int64)bip->ip)) > 0) {
		fprintf(stderr, "Cannot bind ip\n");
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, bip->host, (int)strlen(bip->host), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind host %s\n", bip->host);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, bip->domain, (int)strlen(bip->domain), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind domain %s\n", bip->domain);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)bip->bd_id)) > 0) {
		fprintf(stderr, "Cannot bind bd_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)bip->server_id)) > 0) {
		fprintf(stderr, "Cannot bind server_id\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 6, (sqlite3_int64)bip->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser\n");
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 7, (sqlite3_int64)bip->muser)) > 0) {
		fprintf(stderr, "Cannot bind cuser\n");
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_os(sqlite3_stmt *state, cbc_build_os_s *bos)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, bos->os, (int)strlen(bos->os), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind OS %s\n", bos->os);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, bos->version, (int)strlen(bos->version), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind OS version %s\n", bos->version);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, bos->alias, (int)strlen(bos->alias), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind alias %s\n", bos->alias);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, bos->ver_alias, (int)strlen(bos->ver_alias), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind OS ver alias%s\n", bos->ver_alias);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, bos->arch, (int)strlen(bos->arch), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind arch %s\n", bos->arch);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 6, (sqlite3_int64)bos->bt_id)) > 0) {
		fprintf(stderr, "Cannot bind build type id %lu\n", bos->bt_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 7, (sqlite3_int64)bos->cuser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", bos->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(state, 8, (sqlite3_int64)bos->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", bos->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_varient(sqlite3_stmt *state, cbc_varient_s *vari)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, vari->varient, (int)strlen(vari->varient), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind varient %s\n", vari->varient);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, vari->valias, (int)strlen(vari->valias), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind valias %s\n", vari->valias);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)vari->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", vari->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)vari->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", vari->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_part_scheme(sqlite3_stmt *state, cbc_seed_scheme_s *seed)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, seed->name, (int)strlen(seed->name), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind seed name %s\n", seed->name);
		return retval;
	}
	if ((retval = sqlite3_bind_int(state, 2, seed->lvm)) > 0) {
		fprintf(stderr, "Cannot bind LVM value %d\n", seed->lvm);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)seed->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", seed->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)seed->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", seed->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_part(sqlite3_stmt *state, cbc_pre_part_s *part)
{
	int retval;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite3_int64)part->min)) > 0) {
		fprintf(stderr, "Cannot bind min size %lu\n", part->min);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)part->max)) > 0) {
		fprintf(stderr, "Cannot bind max size %lu\n", part->max);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)part->pri)) > 0) {
		fprintf(stderr, "Cannot bind priority %lu\n", part->pri);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, part->mount, (int)strlen(part->mount), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind mount point %s\n", part->mount);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, part->fs, (int)strlen(part->fs), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind file system %s\n", part->fs);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 6, (sqlite3_int64)part->link_id.def_scheme_id)) > 0) {
		fprintf(stderr, "Cannot bind scheme id %lu\n", part->link_id.def_scheme_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 7, part->log_vol, (int)strlen(part->log_vol), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind logical volume %s\n", part->log_vol);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 8, (sqlite3_int64)part->cuser)) > 0) {
		fprintf(stderr, "Cannot bind scheme cuser %lu\n", part->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 9, (sqlite3_int64)part->muser)) > 0) {
		fprintf(stderr, "Cannot bind scheme cuser %lu\n", part->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_pack(sqlite3_stmt *state, cbc_package_s *pack)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, pack->package, (int)strlen(pack->package), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind package name %s\n", pack->package);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)pack->vari_id)) > 0) {
		fprintf(stderr, "Cannot bind varient ID %lu\n", pack->vari_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)pack->os_id)) > 0) {
		fprintf(stderr, "Cannot bind OS ID %lu\n", pack->os_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)pack->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", pack->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)pack->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", pack->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_build_disk(sqlite3_stmt *state, cbc_disk_dev_s *disk)
{
	int retval;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite3_int64)disk->server_id)) > 0) {
		fprintf(stderr, "Cannot bind server_id %lu\n", disk->server_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, disk->device, (int)strlen(disk->device), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind disk device %s\n", disk->device);
		return retval;
	}
	if ((retval = sqlite3_bind_int(state, 3, disk->lvm)) > 0) {
		fprintf(stderr, "Cannot bind LVM\n");
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_locale(sqlite3_stmt *state, cbc_locale_s *loc)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, loc->locale, (int)strlen(loc->locale), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind locale %s\n", loc->locale);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, loc->country, (int)strlen(loc->country), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind country %s\n", loc->country);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, loc->language, (int)strlen(loc->language), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind language %s\n", loc->language);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, loc->keymap, (int)strlen(loc->keymap), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind keymap %s\n", loc->keymap);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 5, loc->timezone, (int)strlen(loc->timezone), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind timezone %s\n", loc->timezone);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 6, loc->name, (int)strlen(loc->timezone), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind name %s\n", loc->timezone);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 7, (sqlite_int64)loc->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", loc->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 8, (sqlite3_int64)loc->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", loc->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_syspack(sqlite3_stmt *state, cbc_syspack_s *spack)
{
	int retval;

	if ((retval = sqlite3_bind_text(
state, 1, spack->name, (int)strlen(spack->name), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind name %s\n", spack->name);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)spack->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", spack->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)spack->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", spack->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_sysarg(sqlite3_stmt *state, cbc_syspack_arg_s *spack)
{
	int retval;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite3_int64)spack->syspack_id)) > 0) {
		fprintf(stderr, "Cannot bind syspack_id %lu\n", spack->syspack_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 2, spack->field, (int)strlen(spack->field), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind field %s\n", spack->field);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, spack->type, (int)strlen(spack->type), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind type %s\n", spack->type);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)spack->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", spack->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)spack->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", spack->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_sysconf(sqlite3_stmt *state, cbc_syspack_conf_s *spack)
{
	int retval;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite3_int64)spack->syspack_arg_id)) > 0) {
		fprintf(stderr, "Cannot bind syspack_arg_id %lu\n",
		 spack->syspack_arg_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)spack->syspack_id)) > 0) {
		fprintf(stderr, "Cannot bind syspack_id %lu\n", spack->syspack_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)spack->bd_id)) > 0) {
		fprintf(stderr, "Cannot bind bd_id %lu\n", spack->bd_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, spack->arg, (int)strlen(spack->arg), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind arg %s\n", spack->arg);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)spack->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", spack->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 6, (sqlite3_int64)spack->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", spack->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_script(sqlite3_stmt *state, cbc_script_s *scr)
{
	int retval = 0;

	if ((retval = sqlite3_bind_text(
state, 1, scr->name, (int)strlen(scr->name), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind anem %s\n", scr->name);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite3_int64)scr->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", scr->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite3_int64)scr->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", scr->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_scripta(sqlite3_stmt *state, cbc_script_arg_s *arg)
{
	int retval = 0;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite_int64)arg->systscr_id)) > 0) {
		fprintf(stderr, "Cannot bind systscr_id %lu\n", arg->systscr_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite_int64)arg->bd_id)) > 0) {
		fprintf(stderr, "Cannot bind bd_id %lu\n", arg->bd_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 3, (sqlite_int64)arg->bt_id)) > 0) {
		fprintf(stderr, "Cannot bind bt_id %lu\n", arg->bt_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 4, arg->arg, (int)strlen(arg->arg), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "Cannot bind arg %s\n", arg->arg);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)arg->no)) > 0) {
		fprintf(stderr, "Cannot bind no %lu\n", arg->no);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 6, (sqlite3_int64)arg->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", arg->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 7, (sqlite3_int64)arg->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", arg->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_partopt(sqlite3_stmt *state, cbc_part_opt_s *opt)
{
	int retval = 0;

	if ((retval = sqlite3_bind_int64(
state, 1, (sqlite_int64)opt->def_part_id)) > 0) {
		fprintf(stderr, "Cannot bind def_part_id %lu\n", opt->def_part_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 2, (sqlite_int64)opt->def_scheme_id)) > 0) {
		fprintf(stderr, "Cannot bind def_scheme_id %lu\n", opt->def_scheme_id);
		return retval;
	}
	if ((retval = sqlite3_bind_text(
state, 3, opt->option, (int)strlen(opt->option), SQLITE_STATIC)) > 0) {
		fprintf(stderr, "cannot bind option %s\n", opt->option);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 4, (sqlite3_int64)opt->cuser)) > 0) {
		fprintf(stderr, "Cannot bind cuser %lu\n", opt->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 5, (sqlite3_int64)opt->muser)) > 0) {
		fprintf(stderr, "Cannot bind muser %lu\n", opt->muser);
		return retval;
	}
	return retval;
}

int
cbc_setup_bind_sqlite_server(sqlite3_stmt *state, cbc_server_s *server)
{
	int retval = NONE;

	if ((retval = sqlite3_bind_text(
state, 1, server->name, (int)strlen(server->name), SQLITE_STATIC)) > 0) {
		printf("Cannot bind %s\n", server->name);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 6, (sqlite3_int64)server->cust_id)) > 0) {
		printf("Cannot bind %lu\n", server->cust_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 7, (sqlite3_int64)server->vm_server_id)) > 0) {
		printf("Cannot bind %lu\n", server->vm_server_id);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 8, (sqlite3_int64)server->cuser)) > 0) {
		printf("Cannot bind %lu\n", server->cuser);
		return retval;
	}
	if ((retval = sqlite3_bind_int64(
state, 9, (sqlite3_int64)server->muser)) > 0) {
		printf("Cannot bind %lu\n", server->muser);
		return retval;
	}
	return retval;
}

#endif
