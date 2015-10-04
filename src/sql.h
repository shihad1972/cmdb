
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
 *  sql.h
 *  various definitions for SQL queries.
 */

#ifndef __HAVE_SQL_H_
# define __HAVE_SQL_H_
# define THIS_SQL_MAX 524288

enum {
	CMDB = 0,
	DNSA = 1,
	CBC = 2
};

const unsigned int sql_tables[] = { 8, 9, 20 };

const unsigned int delete_args[] = {
// cbc delete args 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};
const unsigned int update_args[] = {
// cbc update args
	2, 2, 2, 2, 2, 3, 3, 3, 4, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5,
	5, 6, 3, 3, 3, 3, 3, 2, 2, 2
};

const unsigned int search_args[] = {
// cbc search args
	1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, 1, 1, // 22
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 1, // 22
	1, 1, 1, 1, 3, 2, 2, 1, 2, 1, 1, 1, 2, 1, 1, 3, 2, 2, 1, 1, 1, 1, // 22
	3, 1, 2, 1, 4, 2, 3, 1, 1, 1, 1, 1, 1, 1
};
const unsigned int search_fields[] = {
// cbc search fields
	5, 5, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 1, 1, 1, 10,
	10, 7, 2, 6, 1, 5, 3, 4, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 3, 11, 1, 2,
	2, 6, 1, 2, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 4, 1, 4, 4, 1, 2, 1,
	1, 1, 3, 1, 1, 1, 1, 1, 1, 2, 3, 5, 2, 1
};

/*
 * SQL SELECT Statements and associated data definitions
 *
 * These statements will be used by the sql_select function. We need to know
 * the number of fields (returned data) and their types. There are no
 * arguments to these queries. They will return the full table.
 */

const unsigned int select_queries[] = { 8, 6, 17 };	// No of queries

const unsigned int table_fields[] = {
// cmdb table fields
	9, 11, 3, 9, 3, 10, 12, 8,
// dnsa table fields
	11, 9, 13, 9, 23, 18, //9, 4, 11,
// cbc table fields;
	13, 13, 10, 11, 7, 12, 4, 12, 8, 8, 7, 6, 8, 9, 6, 10, 7
};

const char *sql_table_list[] = {
// cmdb tables
	"contacts",
	"customer",
	"hard_type",
	"hardware",
	"service_type",
	"services",
	"server",
	"vm_server_hosts",
// dnsa tables
	"glue_zones",
	"preferred_a",
	"records",
	"rev_records",
	"rev_zones",
	"zones",
// cbc tables
	"build",
	"build_domain",
	"build_ip",
	"build_os",
	"build_type",
	"default_part",
	"disk_dev",
	"locale",
	"packages",
	"part_options",
	"seed_schemes",
	"system_package_args",
	"system_package_conf",
	"system_packages",
	"system_scrips",
	"system_scripts_args",
	"varient"
};
const char *sql_columns[] = {
// cmdb table columns
	"cont_id", "name", "phone", "email", "cust_id", "cuser", "muser", "ctime", "mtime",
	"cust_id", "name", "address", "city", "county", "postcode", "coid", "cuser", "muser", "ctime", "mtime",
	"hard_type_id", "type", "class",
	"hard_id", "detail", "device", "server_id", "hard_type_id", "cuser", "muser", "ctime", "mtime",
	"service_type_id", "service", "detail",
	"service_id", "server_id", "cust_id", "service_type_id", "detail", "url", "cuser", "muser", "ctime", "mtime",
	"server_id", "vendor", "make", "model", "uuid", "cust_id", "vm_server_id", "name", "cuser", "muser", "ctime", "mtime",
	"vm_server_id", "vm_server", "type", "server_id", "cuser", "muser", "ctime", "mtime",
// dnsa table columns
	"id", "name", "zone_id", "pri_dns", "sec_dns", "pri_ns", "sec_ns", "cuser", "muser", "ctime", "mtime",
	"prefa_id", "ip", "ip_addr", "record_id", "fqdn", "cuser", "muser", "ctime", "mtime",
	"id", "zone", "host", "type", "protocol", "service", "pri", "destination", "valid", "cuser", "muser", "ctime", "mtime",
	"rev_record_id", "rev_zone", "host", "destination", "valid", "cuser", "muser", "ctime", "mtime",
	"rev_zone_id", "net_range", "prefix", "net_start", "net_finish", "start_ip", "finish_ip", "pri_dns", 
	  "sec_dns", "serial", "refresh", "retry", "expire", "ttl", "valid", "owner", "updated", "type",
	  "master", "cuser", "muser", "ctime", "mtime",
	"id", "name", "pri_dns", "sec_dns", "serial", "refresh", "retry", "expire", "ttl", "valid", "owner",
	  "updated", "type", "master", "cuser", "muser", "ctime", "mtime",
// cbc table colums
	"build_id", "mac_addr", "varient_id", "net_inst_int", "server_id", "os_id", "ip_id", "locale_id",
	  "def_scheme_id", "cuser", "muser", "ctime", "mtime",
	"bd_id", "start_ip", "end_ip", "netmask", "gateway", "ns", "domain", "ntp_server", "config_ntp",
	  "cuser", "muser", "ctime", "mtime",
	"ip_id", "ip", "hostname", "domainname", "bd_id", "server_id", "cuser", "muser", "ctime", "mtime",
	"os_id", "os", "os_version", "alias", "ver_alias", "arch", "bt_id", "cuser", "muser", "ctime", "mtime",
	"bt_id", "alias", "build_type", "arg", "url", "mirror", "boot_line",
	"def_part_id", "minimum", "maximum", "priority", "mount_point", "filesystem", "def_scheme_id",
	  "logical_volume", "cuser", "muser", "ctime", "mtime",
	"disk_id", "server_id", "device", "lvm",
	"locale_id", "locale", "country", "language", "keymap", "os_id", "bt_id", "timezone", "cuser", "muser", "ctime", "mtime",
	"pack_id", "package", "varient_id", "os_id", "cuser", "muser", "ctime", "mtime",
	"part_options_id", "def_part_id", "def_scheme_id", "poption", "cuser", "muser", "ctime", "mtime",
	"def_scheme_id", "scheme_name", "lvm", "cuser", "muser", "ctime", "mtime",
	"syspack_id", "name", "cuser", "muser", "ctime", "mtime",
	"syspack_arg_id", "syspack_id", "field", "type", "cuser", "muser", "ctime", "mtime",
	"syspack_conf_id", "syspack_arg_id", "syspack_id", "bd_id", "arg", "cuser", "muser", "ctime", "mtime",
	"systscr_id", "name", "cuser", "muser", "ctime", "mtime",
	"systscr_arg_id", "systscr_id", "bd_id", "bt_id", "arg", "no", "cuser", "muser", "ctime", "mtime",
	"varient_id", "varient", "valias", "cuser", "muser", "ctime", "mtime"
};

// hangover dnsa queries. Will need to make them searches..
/*const char *sql_select[] = {
"\
SELECT name, host, destination, r.id, zone FROM records r, zones z\
 WHERE z.id = r.zone AND r.type = 'A' ORDER BY destination","\
SELECT destination, COUNT(*) c FROM records\
 WHERE type = 'A' GROUP BY destination HAVING c > 1","\
SELECT id, zone, pri, destination FROM records WHERE TYPE = 'CNAME'"
};*/

/*
 * SQL INSERT statements and associated data types
 *
 * These will be used by the sql_inert function. We need to know the
 * args (data to be inserted) and their types
 *
 */
unsigned int short_inserts[] = {
	4, 2, 4, 18, 20
};

const char *sql_insert[] = {
// cbc inserts
/* Start at 0 */ "\
INSERT INTO boot_line (os, os_ver, bt_id, boot_line) VALUES (?, ?,\
 ?, ?, ?)","\
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
 (?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO disk_dev (server_id, device, lvm) VALUES (?, ?, ?)","\
INSERT INTO locale (locale, country, language, keymap, os_id,\
 bt_id, timezone, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)","\
INSERT INTO packages (package, varient_id, os_id, cuser, muser) VALUES \
 (?, ?, ?, ?, ?)","\
INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, \
 def_scheme_id, logical_volume, cuser, muser) \
 VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
/* 10 */"\
INSERT INTO seed_schemes (scheme_name, lvm, cuser, muser) VALUES (?, ?, ?, ?)","\
INSERT INTO server (vendor, make, model, uuid, cust_id, vm_server_id, name, \
cuser, muser)  VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)","\
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

const char *sql_search[] = { 
// cbc searches
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
};

#endif // __HAVE_SQL_H_

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
UPDATE build_domain SET muser = ? WHERE bd_id = ?"
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
DELETE FROM part_options WHERE part_options_id = ?"
};

