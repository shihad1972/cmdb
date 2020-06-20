/*
 *
 *  alisacmdb: Alisatech Configuration Management Database library
 *  Copyright (C) 2015 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  sql.c
 *
 *  Contains the functions to initialise and destroy various data types
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef HAVE_MYSQL
# include <mysql.h>
#endif /* HAVE_MYSQL */
#ifdef HAVE_SQLITE3
# include <sqlite3.h>
#endif /* HAVE_SQLITE3 */

#include <ailsacmdb.h>
#include <ailsasql.h>
#include <cmdb.h>
#include <base_sql.h>

const char *basic_queries[] = {
"SELECT s.name, c.coid FROM server s INNER JOIN customer c on c.cust_id = s.cust_id", // SERVER_NAME_COID
"SELECT coid, name, city FROM customer ORDER BY coid", // COID_NAME_CITY
"SELECT vm_server, type FROM vm_server_hosts", // VM_SERVERS
"SELECT service, detail FROM service_type", // SERVICE_TYPES_ALL
"SELECT type, class FROM hard_type", // HARDWARE_TYPES_ALL
"SELECT DISTINCT bo.os FROM build_os bo JOIN build_type bt ON bt.alias=bo.alias", // BUILD_OS_NAME_TYPE
"SELECT os, os_version, alias, arch, ver_alias, cuser, ctime FROM build_os", // BUILD_OSES
"SELECT varient_id, valias, varient, cuser, muser, ctime, mtime FROM varient", // BUILD_VARIENTS
"SELECT DISTINCT alias FROM build_os",	// BUILD_OS_ALIASES
"SELECT p.package, p.os_id FROM packages p LEFT JOIN varient v ON v.varient_id = p.varient_id \
  WHERE valias = 'base' OR varient = 'base'", // BASE_VARIENT_PACKAGES
"SELECT locale_id FROM default_locale WHERE locale_id > 0", // DEFAULT_LOCALE
"SELECT locale_id, name FROM locale", // LOCALE_NAMES
"SELECT scheme_name, lvm from seed_schemes", // PARTITION_SCHEME_NAMES
"SELECT domain FROM build_domain", // BUILD_DOMAIN_NAMES
"SELECT domain, ns, start_ip, gateway, netmask from build_domain", // BUILD_DOMAIN_NETWORKS
"SELECT name, type, sec_dns, master FROM zones WHERE valid = 'yes'", // FWD_ZONE_CONFIG
"SELECT name FROM system_packages", // SYSTEM_PACKAGE_NAMES
"SELECT name FROM system_scripts", // SYSTEM_SCRIPT_NAMES
"SELECT name, type FROM zones", // ZONE_NAME_TYPES
"SELECT name, valid, serial, type, master FROM zones", // ZONE_INFORMATION
"SELECT net_range, prefix, valid, serial,type, master FROM rev_zones", // REV_ZONE_INFORMATION
"SELECT z.name, g.name, g.pri_ns, g.sec_ns, g.pri_dns, g.sec_dns \
 FROM glue_zones g LEFT JOIN zones z ON z.id = g.zone_id", // GLUE_ZONE_INFORMATION
"SELECT net_range FROM rev_zones WHERE type = 'master'", // REV_ZONES_NET_RANGE
"SELECT type, net_range, pri_dns, sec_dns, master, prefix FROM rev_zones", // REV_ZONE_CONFIG
};

const struct ailsa_sql_query_s argument_queries[] = {
	{ // CONTACT_DETAILS_ON_COID
"SELECT co.name, co.phone, co.email FROM customer cu INNER JOIN contacts co ON co.cust_id = cu.cust_id WHERE cu.coid = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SERVICES_ON_SERVER
"SELECT st.service, s.url, s.detail FROM service_type st \
	LEFT JOIN services s ON st.service_type_id = s.service_type_id \
	LEFT JOIN server se ON s.server_id = se.server_id \
	WHERE se.name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // HARDWARE_ON_SERVER
"SELECT ht.class, h.device, h.detail FROM hardware h \
	LEFT JOIN hard_type ht ON ht.hard_type_id = h.hard_type_id \
	LEFT JOIN server s ON h.server_id = s.server_id \
	WHERE s.name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SERVER_DETAILS_ON_NAME
"SELECT s.vendor, s.make, s.model, s.uuid, c.coid, \
	s.cuser, s.ctime, s.muser, s.mtime FROM server s \
	JOIN customer c ON s.cust_id = c.cust_id \
	WHERE s.name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // VM_HOST_BUILT_SERVERS
"SELECT s.name, v.varient from varient v \
	JOIN build b ON v.varient_id = b.varient_id \
	JOIN server s ON b.server_id = s.server_id \
	JOIN vm_server_hosts vm ON s.vm_server_id = vm.vm_server_id \
	WHERE vm.vm_server = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // CUSTOMER_DETAILS_ON_COID
"SELECT name, address, city, county, postcode, cuser, ctime, muser, mtime FROM customer WHERE coid = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // CUST_ID_ON_COID
"SELECT cust_id FROM customer WHERE coid = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // CONT_ID_ON_CONTACT_DETAILS
"SELECT cont_id FROM contacts WHERE name = ? AND phone = ? and email = ? and cust_id = ?",
	4,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT }
	},
	{ // VM_SERVER_ID_ON_NAME
"SELECT vm_server_id FROM vm_server_hosts WHERE vm_server = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SERVICE_TYPE_ID_ON_DETAILS
"SELECT service_type_id FROM service_type WHERE service = ? AND detail = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // HARDWARE_TYPE_ID_ON_DETAILS
"SELECT hard_type_id FROM hard_type WHERE type = ? AND class = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SERVER_ID_ON_NAME
"SELECT server_id FROM server WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // HARDWARE_TYPE_ID_ON_CLASS
"SELECT hard_type_id FROM hard_type WHERE class = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{  // HARDWARE_ID_ON_DETAILS
"SELECT hard_id FROM hardware WHERE server_id = ? AND hard_type_id = ? AND detail = ? AND device = ?",
	4,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SERVICE_TYPE_ID_ON_SERVICE
"SELECT service_type_id FROM service_type WHERE service = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SERVICE_ID_ON_DETAILS
"SELECT service_id FROM services WHERE server_id = ? AND cust_id = ? AND service_type_id = ? AND detail = ? AND url = ?",
	5,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // BT_ID_ON_ALIAS
"SELECT bt_id FROM build_type WHERE alias = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // CHECK_BUILD_OS
"SELECT os_id FROM build_os WHERE os = ? AND arch = ? AND os_version = ?",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // BUILD_OS_ON_NAME_OR_ALIAS
"SELECT os_id FROM build_os WHERE (os = ? OR alias = ?)",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // BUILD_OS_ON_NAME_VERSION
"SELECT os_id FROM build_os WHERE (os = ? OR alias = ?) AND (os_version = ? OR ver_alias = ?)",
	4,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // BUILD_OS_ON_NAME_ARCH
"SELECT os_id FROM build_os WHERE (os = ? OR ALIAS = ?) AND arch = ?",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // BUILD_OS_ON_ALL
"SELECT os_id FROM build_os WHERE (os = ? OR alias = ?) AND (os_version = ? OR ver_alias = ?) AND arch = ?",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // OS_FROM_BUILD_TYPE_AND_ARCH
"SELECT os_id FROM build_os WHERE bt_id = ? AND arch = ? ORDER BY ctime DESC LIMIT 2",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // PACKAGE_DETAIL_ON_OS_ID
"SELECT package, varient_id FROM packages WHERE os_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // SERVERS_WITH_BUILDS_ON_OS_ID
"SELECT s.name FROM server s LEFT JOIN build b ON b.server_id = s.server_id LEFT JOIN build_os bo ON bo.os_id = b.os_id WHERE bo.os_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // VARIENT_ID_ON_VARIANT_OR_VALIAS
"SELECT varient_id FROM varient WHERE varient = ? OR valias = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // PACKAGE_DETAILS_FOR_VARIENT
"SELECT p.package, o.os, o.os_version, o.arch FROM packages p JOIN build_os o ON p.os_id = o.os_id WHERE p.varient_id = ? ORDER BY o.os, o.os_version, o.arch",
	1,
	{ AILSA_DB_LINT }
	},
	{ // LOCALE_ON_NAME
"SELECT locale_id, locale, country, language, keymap, timezone, cuser, ctime FROM locale WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // PARTITIONS_ON_SCHEME_NAME
"SELECT priority, minimum, maximum, filesystem, logical_volume, mount_point FROM default_part dp LEFT JOIN seed_schemes ss ON ss.def_scheme_id = dp.def_scheme_id WHERE ss.scheme_name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SEED_SCHEME_ON_NAME
"SELECT def_scheme_id, lvm, cuser, ctime from seed_schemes where scheme_name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // PART_OPTIONS_ON_SCHEME_NAME_AND_PARTITION
"SELECT poption FROM part_options po LEFT JOIN default_part dp ON po.def_part_id = dp.def_part_id LEFT JOIN seed_schemes ss ON ss.def_scheme_id = dp.def_scheme_id WHERE scheme_name = ? and mount_point = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // IDENTIFY_PARTITION
"SELECT def_part_id, ss.def_scheme_id FROM seed_schemes ss LEFT JOIN default_part dp ON ss.def_scheme_id = dp.def_scheme_id WHERE scheme_name = ? AND mount_point = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SCHEME_LVM_INFO
"SELECT lvm FROM seed_schemes WHERE scheme_name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SCHEME_ID_ON_NAME
"SELECT def_scheme_id FROM seed_schemes WHERE scheme_name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // PARTITION_ID_ON_SEED_MOUNT
"SELECT def_part_id FROM default_part dp LEFT JOIN seed_schemes ss on dp.def_scheme_id = ss.def_scheme_id WHERE scheme_name = ? AND mount_point = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // IDENTIFY_PART_OPTION
"SELECT part_options_id FROM part_options WHERE def_scheme_id = ? AND def_part_id = ? AND poption = ?",
	3,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // BUILD_DOMAIN_ID_ON_DOMAIN
"SELECT bd_id FROM build_domain WHERE domain = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // BUILD_DOMAIN_OVERLAP
"SELECT bd_id FROM build_domain WHERE (start_ip >= ? AND end_ip <= ?) OR (start_ip <= ? AND end_ip >= ?) OR (start_ip <= ? AND end_ip >= ?)",
	6,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // BUILD_DOMAIN_DETAILS_ON_NAME
"SELECT start_ip, end_ip, netmask, gateway, ns, config_ntp, ntp_server, cuser, ctime, muser, mtime FROM build_domain WHERE domain = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // BUILD_DETAILS_ON_DOMAIN
"SELECT hostname, ip, varient FROM varient v LEFT JOIN build b ON v.varient_id = b.varient_id LEFT JOIN build_ip bi on bi.ip_id = b.ip_id LEFT JOIN build_domain bd ON bi.bd_id = bd.bd_id WHERE bd.domain = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // FWD_ZONE_ID_ON_ZONE_NAME
"SELECT id FROM zones WHERE name = ? AND type = 'master'",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // REV_ZONE_ID_ON_RANGE
"SELECT rev_zone_id FROM rev_zones WHERE net_range = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // NAME_SERVERS_ON_NAME
"SELECT pri_dns, sec_dns FROM zones WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // ZONE_SOA_ON_NAME
"SELECT ttl, serial, refresh, retry, expire, updated FROM zones WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // NS_MX_SRV_RECORDS
"SELECT type, host, protocol, service, pri, destination FROM records WHERE zone = (SELECT id FROM zones WHERE name = ?) AND (type = 'NS' OR type = 'MX' OR type = 'SRV')",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // ZONE_RECORDS_ON_NAME
"SELECT type, host, destination FROM records WHERE type != 'NS' AND type != 'MX' AND type != 'SRV' AND zone = (SELECT id FROM zones WHERE name = ?) ORDER BY type, host",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYS_PACK_DETAILS_ON_DOMAIN
"SELECT name, field, type, arg from system_packages sp LEFT JOIN system_package_conf spc ON sp.syspack_id = spc.syspack_id LEFT JOIN build_domain bd  ON bd.bd_id = spc.bd_id LEFT JOIN system_package_args spa ON spa.syspack_arg_id = spc.syspack_arg_id WHERE domain = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYS_PACK_DETAILS_ON_NAME_DOMAIN
"SELECT name, field, type, arg from system_packages sp LEFT JOIN system_package_conf spc ON sp.syspack_id = spc.syspack_id LEFT JOIN build_domain bd  ON bd.bd_id = spc.bd_id LEFT JOIN system_package_args spa ON spa.syspack_arg_id = spc.syspack_arg_id WHERE domain = ? AND name = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SYS_PACK_DETAILS_MIN
"SELECT name, field, type, arg from system_packages sp LEFT JOIN system_package_conf spc ON sp.syspack_id = spc.syspack_id LEFT JOIN build_domain bd  ON bd.bd_id = spc.bd_id LEFT JOIN system_package_args spa ON spa.syspack_arg_id = spc.syspack_arg_id WHERE domain = ? AND name = ? AND field = ?",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SYS_PACK_ARGS_ON_NAME
"SELECT field, type FROM system_package_args spa LEFT JOIN system_packages sp ON sp.syspack_id = spa.syspack_id WHERE name = ? ORDER BY field",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYSTEM_PACKAGE_ID
"SELECT syspack_id FROM system_packages WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYSTEM_PACKAGE_ARG_ID
"SELECT syspack_arg_id FROM system_package_args spa LEFT JOIN system_packages sp ON sp.syspack_id = spa.syspack_id WHERE sp.name = ? AND spa.field = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SYSTEM_SCRIPTS_ON_NAME
"SELECT domain, alias, no, ssa.arg FROM system_scripts_args ssa LEFT JOIN build_domain bd ON ssa.bd_id = bd.bd_id LEFT JOIN build_type bt ON ssa.bt_id = bt.bt_id LEFT JOIN system_scripts ss ON ssa.systscr_id = ss.systscr_id WHERE name = ? ORDER BY domain, alias, no",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYSTEM_SCRIPTS_ON_NAME_DOMAIN
"SELECT alias, no, ssa.arg FROM system_scripts_args ssa LEFT JOIN build_domain bd ON ssa.bd_id = bd.bd_id LEFT JOIN build_type bt ON ssa.bt_id = bt.bt_id LEFT JOIN system_scripts ss ON ssa.systscr_id = ss.systscr_id WHERE name = ? AND domain = ? ORDER BY alias, no",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // SYSTEM_SCRIPTS_ON_DOMAIN
"SELECT name, alias, no, ssa.arg FROM system_scripts_args ssa LEFT JOIN build_domain bd ON ssa.bd_id = bd.bd_id LEFT JOIN build_type bt ON ssa.bt_id = bt.bt_id LEFT JOIN system_scripts ss ON ssa.systscr_id = ss.systscr_id WHERE domain = ? ORDER BY alias, name, no",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYSTEM_SCRIPT_ID_ON_NAME
"SELECT systscr_id FROM system_scripts WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // SYSTEM_SCRIPT_ARG_ID
"SELECT systscr_arg_id FROM system_scripts_args WHERE arg = ? AND no = ? AND systscr_id = ? AND bd_id = ? AND bt_id = ?",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // GLUE_ZONE_ON_ZONE_NAME
"SELECT g.name, g.pri_ns, g.sec_ns FROM glue_zones g LEFT JOIN zones z ON z.id = g.zone_id WHERE z.name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // DESTINATION_ON_NAME_ZONE
"SELECT r.destination FROM records r LEFT JOIN zones z ON r.zone = z.id WHERE z.name = ? AND r.host = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // ZONE_TYPE_ON_NAME
"SELECT type FROM zones WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{  // ZONE_SERIAL_ON_NAME
"SELECT serial FROM zones WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // REV_ZONE_INFO_ON_RANGE
"SELECT serial, pri_dns, prefix, type FROM rev_zones WHERE net_range = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // REV_RECORDS_ON_ZONE_ID
"SELECT host, destination FROM rev_records WHERE rev_zone = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // REV_SOA_ON_NET_RANGE
"SELECT ttl, pri_dns, sec_dns, serial, refresh, retry, expire, updated FROM rev_zones WHERE net_range = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // REV_RECORDS_ON_NET_RANGE
"SELECT host, destination FROM rev_records WHERE rev_zone = (SELECT rev_zone_id FROM rev_zones WHERE net_range = ?)",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // RECORD_ID_BASE
"SELECT id FROM records WHERE zone = ? AND type = ? AND host = ? AND destination = ?",
	4,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RECORD_ID_MX
"SELECT id FROM records WHERE zone = ? AND type = ? AND host = ? AND destination = ? AND pri = ?",
	5,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT }
	},
	{ // RECORD_ID_SRV
"SELECT id FROM records WHERE zone = ? AND type = ? AND host = ? AND destination = ? AND pri = ? AND protocol = ? AND service = ?",
	7,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RECORD_ON_ZONE_AND_HOST
"SELECT id, type FROM records WHERE zone = ? AND host = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // RECORD_ON_ZONE_AND_HOST_AND_A
"SELECT id FROM records WHERE zone = ? AND host = ? AND type = 'A'",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // RECORD_ID_ON_HOST_ZONE_TYPE
"SELECT id FROM records WHERE zone = ? AND type = ? AND host = ?",
	3,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // RECORD_IN_ON_PRI
"SELECT id FROM records WHERE zone = ? AND type = ? AND host = ? AND pri = ?",
	4,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT }
	},
	{ // RECORD_IN_ON_PROTOCOL
"SELECT id FROM records WHERE zone = ? AND type = ? AND host = ? AND pri = ? AND protocol = ? AND service = ?",
	6,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // DUP_IP_NET_RANGE
"SELECT destination, count(*) c FROM records WHERE type = 'A' AND destination LIKE ? GROUP BY destination HAVING c > 1",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // DUP_IP_A_RECORD
"SELECT destination, count(*) c FROM records WHERE type = 'A' AND destination = ? GROUP BY destination HAVING c > 1",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // A_RECORDS_WITH_IP
"SELECT r.host, z.name FROM records r LEFT JOIN zones z ON r.zone = z.id WHERE destination = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // FQDN_PREF_A_ON_IP
"SELECT fqdn FROM preferred_a WHERE ip = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // NET_START_FINISH_ON_RANGE
"SELECT net_start, net_finish FROM rev_zones WHERE net_range = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // PREFIX_ON_NET_RANGE
"SELECT prefix FROM rev_zones WHERE net_range = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // PREFER_A_INFO_ON_RANGE
"SELECT ip, fqdn, record_id FROM preferred_a WHERE ip LIKE ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // RECORDS_ON_NET_RANGE
"SELECT destination, r.id, host, name FROM records r LEFT JOIN zones z ON z.id = r.zone WHERE destination LIKE ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // REV_RECORD_ID_ON_ZONE_HOST
"SELECT rev_record_id FROM rev_records WHERE rev_zone = ? AND host = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // DESTINATION_ON_RECORD_ID
"SELECT destination, host, name FROM records r LEFT JOIN zones z ON r.zone = z.id WHERE r.id = ?",
	1,
	{ AILSA_DB_LINT }
	},
};

const struct ailsa_sql_query_s insert_queries[] = {
	{ // INSERT_CONTACT
"INSERT INTO contacts (name, phone, email, cust_id, cuser, muser) VALUES (?, ?, ?, ?, ?, ?)",
	6,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SERVER
"INSERT INTO server (name, make, model, vendor, uuid, cust_id, vm_server_id, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
	9,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SERVICE_TYPE
"INSERT INTO service_type (service, detail) VALUES (?, ?)",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // INSERT_HARDWARE_TYPE
"INSERT INTO hard_type (type, class) VALUES (?, ?)",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // INSERT_VM_HOST
"INSERT INTO vm_server_hosts (vm_server, type, server_id, cuser, muser) VALUES (?, ?, ?, ?, ?)",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_HARDWARE
"INSERT INTO hardware (server_id, hard_type_id, detail, device, cuser, muser) VALUES (?, ?, ?, ?, ?, ?)",
	6,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SERVICE
"INSERT INTO services (server_id, cust_id, service_type_id, detail, url, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?)",
	7,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_CUSTOMER
"INSERT INTO customer (name, address, city, county, postcode, coid, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
	8,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_BUILD_OS
"INSERT INTO build_os (os, os_version, alias, ver_alias, arch, bt_id, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
	8,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_BUILD_PACKAGE
"INSERT INTO packages (package, varient_id, os_id, cuser, muser) VALUES (?, ?, ?, ?, ?)",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_VARIENT
"INSERT INTO varient(varient, valias, cuser, muser) VALUES (?, ?, ?, ?)",
	4,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_LOCALE
"INSERT INTO locale(locale, country, language, keymap, timezone, name, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
	8,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SEED_SCHEME
"INSERT INTO seed_schemes (scheme_name, lvm, cuser, muser) VALUES (?, ?, ?, ?)",
	4,
	{ AILSA_DB_TEXT, AILSA_DB_SINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_PARTITION
"INSERT INTO default_part (minimum, maximum, priority, mount_point, filesystem, def_scheme_id, logical_volume, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
	9,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_PART_OPTION
"INSERT INTO part_options (def_scheme_id, def_part_id, poption, cuser, muser) VALUES (?, ?, ?, ?, ?)",
	5,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_BUILD_DOMAIN
"INSERT INTO build_domain (start_ip, end_ip, ns, gateway, netmask, domain, config_ntp, ntp_server, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	10,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_SINT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_FORWARD_ZONE
"INSERT INTO zones (name, pri_dns, sec_dns, refresh, retry, expire, ttl, serial, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	10,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SYSTEM_PACKAGE
"INSERT INTO system_packages (name, cuser, muser) VALUES (?, ?, ?)",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SYSTEM_PACKAGE_ARGS
"INSERT INTO system_package_args (field, type, syspack_id, cuser, muser) VALUES (?, ?, ?, ?, ?)",
	5,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SYSTEM_PACKAGE_CONF
"INSERT INTO system_package_conf (arg, syspack_id, syspack_arg_id, bd_id, cuser, muser) VALUES (?, ?, ?, ?, ?, ?)",
	6,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SYSTEM_SCRIPT
"INSERT INTO system_scripts(name, cuser, muser) VALUES (?, ?, ?)",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_SYSTEM_SCRIPT_ARGS
"INSERT INTO system_scripts_args(arg, no, systscr_id, bd_id, bt_id, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?)",
	7,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_REVERSE_ZONE
"INSERT INTO rev_zones (net_range, prefix, net_start, start_ip, net_finish, finish_ip, pri_dns, sec_dns, serial, refresh, retry, expire, ttl, type, master, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)",
	17,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_RECORD_BASE
"INSERT INTO records (zone, type, host, destination, cuser, muser) VALUES (?, ?, ?, ?, ?, ?)",
	6,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_RECORD_MX
"INSERT INTO records (zone, type, host, destination, pri, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?)",
	7,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_RECORD_SRV
"INSERT INTO records (zone, type, host, destination, pri, protocol, service, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)",
	9,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT },
	},
	{ // INSERT_REVERSE_RECORD
"INSERT INTO rev_records (rev_zone, host, destination, cuser, muser) VALUES (?, ?, ?, ?, ?)",
	5,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_GLUE_ZONE
"INSERT INTO glue_zones (zone_id, name, pri_ns, sec_ns, pri_dns, sec_dns, cuser, muser) VALUES (?, ?, ?, ?, ?, ?, ?, ?)",
	8,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // INSERT_PREF_A
"INSERT INTO preferred_a (ip, ip_addr, record_id, fqdn, cuser, muser) VALUES (?, ?, ?, ?, ?, ?)",
	6,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
};

const struct ailsa_sql_query_s delete_queries[] = {
	{ // DELETE_BUILD_OS
"DELETE FROM build_os WHERE os_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // DELETE_VARIENT
"DELETE FROM varient WHERE varient = ? OR valias = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // DELETE_LOCALE
"DELETE FROM locale WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // DELETE_PART_OPTION
"DELETE FROM part_options WHERE def_scheme_id = ? AND def_part_id = ? and poption = ?",
	3,
	{ AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // DELETE_PARTITION
"DELETE FROM default_part WHERE def_part_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // DELETE_SCHEME_ON_NAME
"DELETE FROM seed_schemes WHERE scheme_name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // DELETE_BUILD_DOMAIN_ON_NAME
"DELETE FROM build_domain WHERE domain = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // DELETE_SYS_PACK_CONF
"DELETE FROM system_package_conf WHERE arg = ? AND syspack_id = ? AND syspack_arg_id = ? AND bd_id = ?",
	4,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // DELETE_SYS_PACK_ARG
"DELETE FROM system_package_args WHERE field = ? AND syspack_id = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_LINT }
	},
	{ // DELETE_SYSTEM_PACKAGE
"DELETE FROM system_packages WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
	{ // DELETE_SYSTEM_SCRIPT_ARG
"DELETE FROM system_scripts_args WHERE systscr_arg_id = ?",
	1,
	{ AILSA_DB_LINT } 
	},
	{ // DELETE_SYSTEM_SCRIPT
"DELETE FROM system_scripts WHERE systscr_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // DELETE_FWD_ZONE
"DELETE FROM zones WHERE id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // DELETE_REV_ZONE
"DELETE FROM rev_zones WHERE rev_zone_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // DELETE_REC_TYPE_HOST
"DELETE FROM records WHERE zone = ? AND type = ? AND host = ?",
	3,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // DELETE_REC_PRI
"DELETE FROM records WHERE zone = ? AND type = ? AND host = ? AND pri = ?",
	4,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT }
	},
	{ // DELETE_REC_PROTO
"DELETE FROM records WHERE zone = ? AND type = ? AND host = ? AND pri = ? AND protocol = ? AND service = ?",
	6,
	{ AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // DELETE_REVERSE_RECORD
"DELETE FROM rev_records WHERE rev_record_id = ?",
	1,
	{ AILSA_DB_LINT }
	},
	{ // DELETE_GLUE_ZONE
"DELETE FROM glue_zones WHERE name = ?",
	1,
	{ AILSA_DB_TEXT }
	},
};

const struct ailsa_sql_query_s update_queries[] = {
	{ // SET_DEFAULT_LOCALE
"UPDATE default_locale SET locale_id = (SELECT locale_id FROM locale WHERE name = ?), muser = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_LINT }
	},
	{ // SET_PART_SCHEME_UPDATED
"UPDATE seed_schemes SET muser = ? WHERE scheme_name = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // UPDATE_BUILD_DOMAIN
"UPDATE build_domain SET ntp_server = ?, muser = ? WHERE domain = ?",
	3,
	{ AILSA_DB_TEXT, AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // FWD_ZONE_VALIDATE
"UPDATE zones SET valid = ? WHERE name = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // FWD_ZONE_SERIAL_UPDATE
"UPDATE zones SET serial = ?, updated = 'no' WHERE name = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // REV_ZONE_VALIDATE
"UPDATE rev_zones SET valid = ? WHERE net_range = ?",
	2,
	{ AILSA_DB_TEXT, AILSA_DB_TEXT }
	},
	{ // REV_ZONE_SERIAL_UPDATE
"UPDATE rev_zones SET serial = ?, updated = 'no' WHERE net_range = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
	{ // SET_FWD_ZONE_UPDATED
"UPDATE zones SET muser = ?, updated = 'yes' WHERE id = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_LINT }
	},
	{ // SET_REV_ZONE_UPDATED
"UPDATE rev_zones SET muser = ?, updated = 'yes' WHERE net_range = ?",
	2,
	{ AILSA_DB_LINT, AILSA_DB_TEXT }
	},
};

static int
cmdb_create_multi(ailsa_sql_multi_s *sql, const struct ailsa_sql_query_s query, size_t total);

static int
cmdb_create_delete_multi(struct ailsa_sql_query_s *sql, const struct ailsa_sql_query_s query, size_t total);

static char *
cmdb_create_value_string(const struct ailsa_sql_query_s query);

static unsigned int *
cmdb_insert_fields_array(unsigned int n, size_t tot, const unsigned int *f);

#ifdef HAVE_MYSQL

static int
ailsa_basic_query_mysql(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results);

int
ailsa_argument_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results);

int
ailsa_delete_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *delete);

int
ailsa_insert_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *insert);

static void
ailsa_store_mysql_row(MYSQL_ROW row, AILLIST *results, unsigned int *fields);

int
ailsa_multiple_query_mysql(ailsa_cmdb_s *cmdb, ailsa_sql_multi_s *sql, AILLIST *insert);

#endif

#ifdef HAVE_SQLITE3

static int
ailsa_basic_query_sqlite(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results);

int
ailsa_argument_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results);

int
ailsa_delete_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *delete);

int
ailsa_insert_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *insert);

int
ailsa_multiple_query_sqlite(ailsa_cmdb_s *cmdb, ailsa_sql_multi_s *sql, AILLIST *insert);

static void
ailsa_store_basic_sqlite(sqlite3_stmt *state, AILLIST *results);

static int
ailsa_bind_arguments_sqlite(sqlite3_stmt *state, AILLIST *args, unsigned int t, const unsigned int *f);

static unsigned int
ailsa_set_my_type(unsigned int type);

static void
ailsa_remove_empty_results_mysql(MYSQL_STMT *stmt, AILLIST *results);

#endif

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *results)
{
	int retval = DB_TYPE_INVALID;
	const char *query = basic_queries[query_no];

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_basic_query_mysql(cmdb, query, results);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_basic_query_sqlite(cmdb, query, results);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;
}

int
ailsa_argument_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *args, AILLIST *results)
{
	int retval = DB_TYPE_INVALID;
	const struct ailsa_sql_query_s argument = argument_queries[query_no];

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_argument_query_mysql(cmdb, argument, args, results);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_argument_query_sqlite(cmdb, argument, args, results);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;	
}

int
ailsa_individual_query(ailsa_cmdb_s *cmdb, const ailsa_sql_query_s *query, AILLIST *args, AILLIST *results)
{
	int retval = DB_TYPE_INVALID;

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_argument_query_mysql(cmdb, *query, args, results);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_argument_query_sqlite(cmdb, *query, args, results);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;
}

int
ailsa_delete_query(ailsa_cmdb_s *cmdb, const ailsa_sql_query_s query, AILLIST *delete)
{
	int retval = DB_TYPE_INVALID;

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_delete_query_mysql(cmdb, query, delete);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_delete_query_sqlite(cmdb, query, delete);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;
}

int
ailsa_update_query(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *update)
{
	int retval = DB_TYPE_INVALID;

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_delete_query_mysql(cmdb, query, update);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_delete_query_sqlite(cmdb, query, update);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;
}

int
ailsa_insert_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *insert)
{
	int retval = DB_TYPE_INVALID;
	const struct ailsa_sql_query_s query = insert_queries[query_no];

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_insert_query_mysql(cmdb, query, insert);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_insert_query_sqlite(cmdb, query, insert);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	return retval;
}

int
ailsa_multiple_query(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *insert)
{
	int retval;
	ailsa_sql_multi_s *sql = ailsa_calloc(sizeof(ailsa_sql_multi_s), "sql in ailsa_multiple_insert_query");

	if ((retval = cmdb_create_multi(sql, query, insert->total)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to create multiple struct");
		goto cleanup;
	}
	retval = DB_TYPE_INVALID;
	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_multiple_query_mysql(cmdb, sql, insert);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_multiple_query_sqlite(cmdb, sql, insert);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);

	cleanup:
		cmdb_clean_ailsa_sql_multi(sql);
		return retval;
}

int
ailsa_multiple_delete(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *del)
{
	int retval;
	struct ailsa_sql_delete_s sql;
	struct ailsa_sql_query_s *s = (ailsa_sql_query_s *)&sql;

	memset(&sql, 0, sizeof(ailsa_sql_delete_s));
	if ((retval = cmdb_create_delete_multi((ailsa_sql_query_s *)&sql, query, del->total)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to create multi delete sql struct");
		return retval;
	}
	retval = DB_TYPE_INVALID;

	if ((strncmp(cmdb->dbtype, "none", RANGE_S) == 0))
		ailsa_syslog(LOG_ERR, "no dbtype set");
#ifdef HAVE_MYSQL
	else if ((strncmp(cmdb->dbtype, "mysql", RANGE_S) == 0))
		retval = ailsa_delete_query_mysql(cmdb, *s, del);
#endif // HAVE_MYSQL
#ifdef HAVE_SQLITE3
	else if ((strncmp(cmdb->dbtype, "sqlite", RANGE_S) == 0))
		retval = ailsa_delete_query_sqlite(cmdb, *s, del);
#endif
	else
		ailsa_syslog(LOG_ERR, "dbtype unavailable: %s", cmdb->dbtype);
	if (sql.query)
		my_free(sql.query);
	return retval;
}

static int
cmdb_create_delete_multi(struct ailsa_sql_query_s *sql, const struct ailsa_sql_query_s query, size_t total)
{
	if (!(sql) || (total == 0))
		return AILSA_NO_DATA;
	char *q, *tmp;
	unsigned int *ptr, field;
	int retval = 0;
	size_t len_q, len_s, i;

	if (query.number != 1) {
		ailsa_syslog(LOG_ERR, "Can only multi delete with a single argument. Query has %zu", total);
		return WRONG_LENGTH_LIST;
	}
	len_q = strlen(query.query);
	len_s = (total * 3) + 3;
	len_s += len_q;
	field = query.fields[0];
	q = ailsa_calloc(len_s, "q in cmdb_create_delete_multi");
	snprintf(q, len_q + 1, "%s", query.query);
	tmp = strrchr(q, '=');
	if (!(tmp)) {
		my_free(q);
		ailsa_syslog(LOG_ERR, "Cannot find = in query string");
		return MY_QUERY_FAIL;
	}
	sprintf(tmp, "IN (" );
	tmp += 4;
	ptr = sql->fields;
	for (i = 0; i < total; i++) {
		sprintf(tmp, "?, ");
		tmp += 3;
		ptr[i] = field;
	}
	tmp -= 2;
	sprintf(tmp, ")");
	sql->query = q;
	sql->number = (unsigned int)total;
	return retval;
}

static int
cmdb_create_multi(ailsa_sql_multi_s *sql, const struct ailsa_sql_query_s query, size_t total)
{
	if (!(sql) || (total == 0))
		return AILSA_NO_DATA;
	int retval = 0;
	unsigned int *u = NULL;
	unsigned int n = query.number;
	const unsigned int *f = query.fields;
	size_t i, len_s, len_q, len_t, tot;
	char *values = NULL, *q = NULL;

	len_q = strlen(query.query);
	values = cmdb_create_value_string(query);
	len_s = strlen(values);
	len_t = total / (size_t)n;
	tot = (len_s * len_t) + len_q;
	u = cmdb_insert_fields_array(n, total, f);
	q = ailsa_calloc(tot, "q in cmdb_create_multi_insert");
	sprintf(q, "%s", query.query);
	for (i = 1; i < len_t; i++) {
		sprintf(q + len_q, "%s", values);
		len_q += len_s;
	}
	sql->query = q;
	sql->fields = u;
	sql->total = (unsigned int)total;
	sql->number = n;
	free(values);
	return retval;
}

static char *
cmdb_create_value_string(const struct ailsa_sql_query_s query)
{
	char *value = NULL;
	unsigned int count = query.number;
	unsigned int i;
	size_t len = (size_t)count * 3;
	len += 4;
	value = ailsa_calloc(len, "value in cmdb_create_value_string");
	sprintf(value, ", (");
	for (i = 0; i < count; i++) {
		len = strlen(value);
		sprintf(value + len, "?, ");
	}
	len = strlen(value);
	sprintf(value + len - 2, ")");
	return value;
}

static unsigned int *
cmdb_insert_fields_array(unsigned int n, size_t tot, const unsigned int *f)
{
	unsigned int *m = ailsa_calloc(sizeof(unsigned int) * tot, "i in cmdb_insert_fields_array");
	unsigned int u = (unsigned int)tot / n;
	unsigned int i, j, k;
	for (i = 0; i < u; i++) {
		for (j = 0; j< n; j++) {
			if (i > 0)
				k = (i * n) + j;
			else
				k = j;
			m[k] = f[j];
		}
	}
	return m;
}

#ifdef HAVE_MYSQL

static int
ailsa_basic_query_mysql(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results)
{
	if (!(cmdb) || !(query) || !(results))
		return AILSA_NO_DATA;

	int retval = 0;
	unsigned int total, i, *fields;
	size_t size;
	MYSQL sql;
	MYSQL_RES *sql_res;
	MYSQL_ROW sql_row;
	MYSQL_FIELD *field;
	my_ulonglong sql_rows;

	ailsa_mysql_init(cmdb, &sql);

	if ((retval = ailsa_mysql_query_with_checks(&sql, query)) != 0) {
		ailsa_syslog(LOG_ERR, "MySQL query failed: %s", mysql_error(&sql));
		exit(MY_QUERY_FAIL);
	}
	if (!(sql_res = mysql_store_result(&sql))) {
		ailsa_mysql_cleanup(&sql);
		ailsa_syslog(LOG_ERR, "MySQL store result failed: %s", mysql_error(&sql));
		exit(MY_STORE_FAIL);
	}

	total = mysql_num_fields(sql_res);
	size = (size_t)(total + 1) * sizeof(unsigned int);
	fields = ailsa_calloc(size, "fields in ailsa_basic_query_mysql");
	for (i = 0; i < total; i++) {
		field = mysql_fetch_field_direct(sql_res, i);
		fields[i + 1] = field->type;
	}
	*fields = i;
	if (((sql_rows = mysql_num_rows(sql_res)) != 0)) {
		while ((sql_row = mysql_fetch_row(sql_res)))
			ailsa_store_mysql_row(sql_row, results, fields);
	}
	ailsa_mysql_cleanup_full(&sql, sql_res);
	my_free(fields);
	return retval;
}

int
ailsa_argument_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results)
{
	if (!(cmdb) || !(args) || !(results))
		return AILSA_NO_DATA;

	int retval = 0;
	const char *query = argument.query;
	MYSQL sql;
	MYSQL_STMT *stmt = NULL;
	MYSQL_BIND *params = NULL;
	MYSQL_BIND *res = NULL;

	ailsa_mysql_init(cmdb, &sql);
	if (!(stmt = mysql_stmt_init(&sql))) {
		ailsa_syslog(LOG_ERR, "Error from mysql: %s", mysql_error(&sql));
		retval = MY_STATEMENT_FAIL;
		goto cleanup;
	}
	if ((retval = mysql_stmt_prepare(stmt, query, strlen(query))) != 0) {
		ailsa_syslog(LOG_ERR, "Error from mysql: %s", mysql_error(&sql));
		goto cleanup;
	}
	if ((retval = ailsa_bind_params_mysql(stmt, &params, argument, args)) != 0)
		goto cleanup;
	if ((retval = ailsa_bind_results_mysql(stmt, &res, results)) != 0)
		goto cleanup;
	if ((retval = mysql_stmt_execute(stmt)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot execute MySQL statement. %s", mysql_stmt_error(stmt));
		goto cleanup;
	}
	if ((retval = mysql_stmt_store_result(stmt)) != 0) {
		ailsa_syslog(LOG_ERR, "Cannot store result of MySQL statement: %s", mysql_stmt_error(stmt));
		goto cleanup;
	}
	while ((retval = mysql_stmt_fetch(stmt)) == 0) {
		if (res)
			my_free(res);
		if ((retval = ailsa_bind_results_mysql(stmt, &res, results)) != 0)
			goto cleanup;
	}
	if (retval != MYSQL_NO_DATA) {
		ailsa_syslog(LOG_ERR, "Cannot fetch data from mysql result set: %s", mysql_stmt_error(stmt));
	} else {
		retval = 0;
		ailsa_remove_empty_results_mysql(stmt, results);
	}
	cleanup:
		if (stmt) {
			mysql_stmt_free_result(stmt);
			mysql_stmt_close(stmt);
		}
		if (params)
			my_free(params);
		if (res)
			my_free(res);
		ailsa_mysql_cleanup(&sql);
		return retval;
}

int
ailsa_delete_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s delete, AILLIST *list)
{
	if (!(cmdb) || !(list))
		return AILSA_NO_DATA;
	int retval;
	MYSQL sql;
	MYSQL_BIND *bind = NULL;
	const char *query = delete.query;

	ailsa_mysql_init(cmdb, &sql);
	if ((retval = ailsa_run_mysql_stmt(&sql, bind, delete, list)) == 0)
		ailsa_syslog(LOG_INFO, "No affected rows for %s", query);
	else if (retval < 0)
		ailsa_syslog(LOG_ERR, "Statement failed: %s", query);
	else
		retval = 0;
	ailsa_mysql_cleanup(&sql);
	return retval;
}

int
ailsa_insert_query_mysql(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s insert, AILLIST *list)
{
	if (!(cmdb) || !(list))
		return AILSA_NO_DATA;
	int retval;
	MYSQL sql;
	MYSQL_BIND *bind = NULL;
	const char *query = insert.query;

	ailsa_mysql_init(cmdb, &sql);
	if ((retval = ailsa_run_mysql_stmt(&sql, bind, insert, list)) == 0)
		ailsa_syslog(LOG_INFO, "No affected rows for %s", query);
	else if (retval < 0)
		ailsa_syslog(LOG_ERR, "Statement failed: %s", query);
	ailsa_mysql_cleanup(&sql);
	return 0;
}

int
ailsa_multiple_query_mysql(ailsa_cmdb_s *cmdb, ailsa_sql_multi_s *insert, AILLIST *list)
{
	if (!(cmdb) || !(insert) || !(list))
		return AILSA_NO_DATA;
	int retval = 0;
	const char *query = insert->query;
	unsigned int t = insert->total;
	unsigned int *f = insert->fields;
	unsigned int rows;
	MYSQL sql;
	MYSQL_STMT *stmt;
	MYSQL_BIND *bind = NULL;

	ailsa_mysql_init(cmdb, &sql);
	if (!(stmt = mysql_stmt_init(&sql))) {
		ailsa_syslog(LOG_ERR, "MySQL stmt failed: %s", mysql_error(&sql));
		goto cleanup;
	}
	if ((retval = mysql_stmt_prepare(stmt, query, strlen(query))) != 0) {
		ailsa_syslog(LOG_ERR, "MySQL stmt failed: %s", mysql_stmt_error(stmt));
		goto cleanup;
	}
	if ((retval = ailsa_bind_parameters_mysql(stmt, &bind, list, t, f)) != 0) {
		ailsa_syslog(LOG_ERR, "MySQL bind failed");
		goto cleanup;
	}
	if ((retval = mysql_stmt_execute(stmt)) != 0) {
		ailsa_syslog(LOG_ERR, "MySQL stmt failed: %s", mysql_stmt_error(stmt));
		goto cleanup;
	}
	rows = (unsigned int)mysql_stmt_affected_rows(stmt);
	t = insert->total / insert->number;
	if (t != rows) {
		ailsa_syslog(LOG_ERR, "Inserted %u rows, should have been %u", rows, t);
	}
	cleanup:
		if (bind)
			my_free(bind);
		if (stmt)
			mysql_stmt_close(stmt);
		ailsa_mysql_cleanup(&sql);
		return retval;
}

static void
ailsa_store_mysql_row(MYSQL_ROW row, AILLIST *results, unsigned int *fields)
{
	if (!(row) || !(results) || (fields == 0))
		return;
	unsigned int i, n, *p;
	int retval = 0;
	ailsa_data_s *tmp;

	p = fields;
	n = fields[0];
	for (i = 1; i <= n; i++) {
		tmp = ailsa_calloc(sizeof(ailsa_data_s), "tmp in ailsa_store_mysql_row");
		ailsa_init_data(tmp);
		switch(p[i]) {
		case MYSQL_TYPE_VAR_STRING:
		case MYSQL_TYPE_VARCHAR:
			if (row[i - 1]) {
				tmp->data->text = strndup(row[i - 1], SQL_TEXT_MAX);
				tmp->type = AILSA_DB_TEXT;
			} else {
				tmp->type = AILSA_DB_NULL;
			}
			break;
		case MYSQL_TYPE_LONG:
		case MYSQL_TYPE_LONGLONG:
			tmp->data->number = strtoul(row[i - 1], NULL, 10);
			tmp->type = AILSA_DB_LINT;
			break;
		case MYSQL_TYPE_TIMESTAMP:
			tmp->data->text = strndup(row[i - 1], SQL_TEXT_MAX);
			tmp->type = AILSA_DB_TEXT;
			break;
		default:
			ailsa_syslog(LOG_ERR, "Unknown mysql type %u", p[i]);
			break;
		}
		if ((retval = ailsa_list_insert(results, tmp) != 0))
			ailsa_syslog(LOG_ERR, "Cannot insert data into list in store_mysql_row");
	}
}

int
ailsa_bind_params_mysql(MYSQL_STMT *stmt, MYSQL_BIND **bind, const struct ailsa_sql_query_s argument, AILLIST *args)
{
	if (!(stmt) || !(args))
		return AILSA_NO_DATA;
	AILELEM *member = args->head;
	MYSQL_BIND *tmp = NULL;
	ailsa_data_s *data;
	int retval = 0;
	unsigned long params = mysql_stmt_param_count(stmt), i;
	if (params > 0)
		 tmp = ailsa_calloc(sizeof(MYSQL_BIND) * (size_t)params, "tmp in ailsa_bind_params_mysql");
	else
		return AILSA_NO_PARAMETERS;
	for (i = 0; i < params; i++) {
		data = member->data;
		if ((retval = ailsa_set_bind_mysql(&(tmp[i]), data, argument.fields[i])) != 0) {
			my_free(tmp);
			return retval;
		}
		member = member->next;
	}
	*bind = tmp;
	if ((retval = mysql_stmt_bind_param(stmt, tmp)) != 0)
		ailsa_syslog(LOG_ERR, "Unable to bind MySQL paramters: %s", mysql_stmt_error(stmt));
	return retval;
}


int
ailsa_bind_results_mysql(MYSQL_STMT *stmt, MYSQL_BIND **bind, AILLIST *results)
{
	if (!(stmt) || !(results))
		return AILSA_NO_DATA;
	MYSQL_BIND *tmp = NULL;
	MYSQL_RES *res = NULL;
	MYSQL_FIELD *field = NULL;
	ailsa_data_s *data;
	int retval = 0;
	unsigned int fields = mysql_stmt_field_count(stmt), i, type;
	if (fields > 0)
		tmp = ailsa_calloc(sizeof(MYSQL_BIND) * (size_t)fields, "tmp in ailsa_bind_results_mysql");
	else
		return AILSA_NO_FIELDS;
	if (!(res = mysql_stmt_result_metadata(stmt))) {
		my_free(tmp);
		goto cleanup;
	}
	for (i = 0; i < fields; i++) {
		data = ailsa_calloc(sizeof(ailsa_data_s), "data in ailsa_bind_results_mysql");
		ailsa_init_data(data);
		if ((retval = ailsa_list_insert(results, data)) != 0)
			return retval;
		field = mysql_fetch_field_direct(res, i);
		type = ailsa_set_my_type(field->type);
		if ((retval = ailsa_set_bind_mysql(&(tmp[i]), data, type)) != 0) {
			my_free(tmp);
			goto cleanup;
		}
	}
	*bind = tmp;
	if ((retval = mysql_stmt_bind_result(stmt, tmp)) != 0)
		ailsa_syslog(LOG_ERR, "Unable to bind MySQL results: %s", mysql_stmt_error(stmt));

	cleanup:
		if (res)
			mysql_free_result(res);
		return retval;
}

int
ailsa_set_bind_mysql(MYSQL_BIND *bind, ailsa_data_s *data, unsigned int fields)
{
	if (!(bind) || !(data))
		return AILSA_NO_DATA;
	size_t len;
	switch(fields) {
	case AILSA_DB_TEXT:
		if (!(data->data->text))
			data->data->text = ailsa_calloc(CONFIG_LEN, "data text in ailsa_set_bind_mysql");
		data->type = AILSA_DB_TEXT;
		bind->buffer_type = MYSQL_TYPE_STRING;
		bind->is_unsigned = 0;
		bind->buffer = data->data->text;
		len = strlen(data->data->text);
		if (len > 0)
			bind->buffer_length = len;
		else
			bind->buffer_length = CONFIG_LEN;
		break;
	case AILSA_DB_LINT:
		data->type = AILSA_DB_LINT;
		bind->buffer_type = MYSQL_TYPE_LONG;
		bind->is_unsigned = 1;
		bind->buffer = &(data->data->number);
		bind->buffer_length = sizeof(unsigned long int);
		break;
	case AILSA_DB_SINT:
		data->type = AILSA_DB_SINT;
		bind->buffer_type = MYSQL_TYPE_SHORT;
		bind->is_unsigned = 0;
		bind->buffer = &(data->data->small);
		bind->buffer_length = sizeof(short int);
		break;
	case AILSA_DB_TIME:
		if (!(data->data->time))
			data->data->time = ailsa_calloc(sizeof(MYSQL_TIME), "data text in ailsa_set_bind_mysql");
		data->type = AILSA_DB_TIME;
		bind->buffer_type = MYSQL_TYPE_TIMESTAMP;
		bind->is_unsigned = 0;
		bind->buffer = data->data->time;
		bind->buffer_length = sizeof(MYSQL_TIME);
		break;
	default:
		ailsa_syslog(LOG_ERR, "Wrong db column type %hi in ailsa_set_bind_mysql", fields);
		return AILSA_INVALID_DBTYPE;
	}
	return 0;
}

static unsigned int
ailsa_set_my_type(unsigned int type)
{
	unsigned int retval;

	switch(type) {
	case MYSQL_TYPE_VAR_STRING:
	case MYSQL_TYPE_VARCHAR:
		retval = AILSA_DB_TEXT;
		break;
	case MYSQL_TYPE_LONG:
	case MYSQL_TYPE_LONGLONG:
		retval = AILSA_DB_LINT;
		break;
	case MYSQL_TYPE_SHORT:
		retval = AILSA_DB_SINT;
		break;
	case MYSQL_TYPE_TIMESTAMP:
		retval = AILSA_DB_TIME;
		break;
	default:
		ailsa_syslog(LOG_ERR, "Unknown MySQL type: %u", type);
		exit(AILSA_INVALID_DBTYPE);
		break;
	}
	return retval;
}

static void
ailsa_remove_empty_results_mysql(MYSQL_STMT *stmt, AILLIST *results)
{
	if (!(stmt) || !(results))
		return;

	unsigned int fields = mysql_stmt_field_count(stmt), i;
	void *data = NULL;
	AILELEM *tail = NULL;

	for (i = 0; i < fields; i++) {
		tail = results->tail;
		ailsa_list_remove(results, tail, &data);
		ailsa_clean_data(data);
	}
}

#endif // HAVE_MYSQL

#ifdef HAVE_SQLITE3
static int
ailsa_basic_query_sqlite(ailsa_cmdb_s *cmdb, const char *query, AILLIST *results)
{
	if (!(cmdb) || !(query) || !(results))
		return AILSA_NO_DATA;
	int retval = 0;
	const char *file = cmdb->file;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;

	ailsa_setup_ro_sqlite(query, file, &sql, &state);
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		ailsa_store_basic_sqlite(state, results);
	ailsa_sqlite_cleanup(sql, state);
	if (retval == SQLITE_DONE)
		retval = 0;
	return retval;
}

int
ailsa_argument_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s argument, AILLIST *args, AILLIST *results)
{
	if (!(cmdb) || !(args) || !(results))
		return AILSA_NO_DATA;
	int retval = 0;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;
	const char *query = argument.query;
	const char *file = cmdb->file;
	unsigned int t = argument.number;
	const unsigned int *f = argument.fields;

	ailsa_setup_ro_sqlite(query, file, &sql, &state);
	if ((retval = ailsa_bind_arguments_sqlite(state, args, t, f)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to bind sqlite arguments: got error %d", retval);
		return retval;
	}
	while ((retval = sqlite3_step(state)) == SQLITE_ROW)
		ailsa_store_basic_sqlite(state, results);
	ailsa_sqlite_cleanup(sql, state);
	if (retval == SQLITE_DONE)
		retval = 0;
	return retval;
}

int
ailsa_delete_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *delete)
{
	if (!(cmdb) || !(delete))
		return AILSA_NO_DATA;
	int retval = 0;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;
	const char *sql_query = query.query;
	const char *file = cmdb->file;
	unsigned int t = query.number;
	const unsigned int *f = query.fields;

	ailsa_setup_rw_sqlite(sql_query, strlen(sql_query), file, &sql, &state);
	if ((retval = ailsa_bind_arguments_sqlite(state, delete, t, f)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to bind sqlite arguments: got error %d", retval);
		goto cleanup;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		ailsa_syslog(LOG_ERR, "Unable to insert into sqlite database: %s", sqlite3_errstr(retval));
		retval = SQLITE_INSERT_FAILED;
		goto cleanup;
	}
	cleanup:
		if (retval == SQLITE_DONE)
			retval = 0;
		ailsa_sqlite_cleanup(sql, state);
		return retval;
}

int
ailsa_insert_query_sqlite(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *insert)
{
	if (!(cmdb) || !(insert))
		return AILSA_NO_DATA;
	int retval = 0;
	sqlite3 *sql = NULL;
	sqlite3_stmt *state = NULL;
	const char *sql_query = query.query;
	const char *file = cmdb->file;
	unsigned int t = query.number;
	const unsigned int *f = query.fields;

	ailsa_setup_rw_sqlite(sql_query, strlen(sql_query), file, &sql, &state);
	if ((retval = ailsa_bind_arguments_sqlite(state, insert, t, f)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to bind sqlite arguments: got error %d", retval);
		goto cleanup;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		ailsa_syslog(LOG_ERR, "Unable to insert into sqlite database: %s", sqlite3_errstr(retval));
		retval = SQLITE_INSERT_FAILED;
		goto cleanup;
	}
	cleanup:
		if (retval == SQLITE_DONE)
			retval = 0;
		ailsa_sqlite_cleanup(sql, state);
		return retval;
}

int
ailsa_multiple_query_sqlite(ailsa_cmdb_s *cmdb, ailsa_sql_multi_s *sql, AILLIST *insert)
{
	if (!(cmdb) || !(sql) || !(insert))
		return AILSA_NO_DATA;
	int retval = 0;
	sqlite3 *lite = NULL;
	sqlite3_stmt *state = NULL;
	const char *sql_query = sql->query;
	const char *file = cmdb->file;

	ailsa_setup_rw_sqlite(sql_query, strlen(sql_query), file, &lite, &state);
	if ((retval = ailsa_bind_arguments_sqlite(state, insert, sql->total, sql->fields)) != 0) {
		ailsa_syslog(LOG_ERR, "Unable to bind sqlite arguments: error %d", retval);
		goto cleanup;
	}
	if ((retval = sqlite3_step(state)) != SQLITE_DONE) {
		ailsa_syslog(LOG_ERR, "Unable to insert into sqlite database: %s", sqlite3_errstr(retval));
		retval = SQLITE_INSERT_FAILED;
		goto cleanup;
	}
	cleanup:
		if (retval == SQLITE_DONE)
			retval = 0;
		ailsa_sqlite_cleanup(lite, state);
		return retval;
}

static void
ailsa_store_basic_sqlite(sqlite3_stmt *state, AILLIST *results)
{
	ailsa_data_s *tmp;
	if (!(state) || !(results))
		return;
	short int fields, i;
	int type, retval;

	fields = (short int)sqlite3_column_count(state);
	retval = 0;
	for (i=0; i<fields; i++) {
		tmp = ailsa_calloc(sizeof(ailsa_data_s), "tmp in ailsa_store_basic_sqlite");
		ailsa_init_data(tmp);
		type = sqlite3_column_type(state, (int)i);
		switch(type) {
		case SQLITE_INTEGER:
			tmp->data->number = (unsigned long int)sqlite3_column_int64(state, (int)i);
			tmp->type = AILSA_DB_LINT;
			break;
	 	case SQLITE_TEXT:  // Can use sqlite3_column_bytes() to get size
			tmp->data->text = strndup((const char *)sqlite3_column_text(state, (int)i), SQL_TEXT_MAX);
			tmp->type = AILSA_DB_TEXT;
			break;
		case SQLITE_FLOAT:
			tmp->data->point = sqlite3_column_double(state, (int)i);
			tmp->type = AILSA_DB_FLOAT;
			break;
		case SQLITE_NULL:
			tmp->type = AILSA_DB_NULL;
			break;
		default:
			ailsa_syslog(LOG_ERR, "Unknown sqlite type %d", type);
			break;
		}
		if ((retval = ailsa_list_insert(results, tmp) != 0))
			ailsa_syslog(LOG_ERR, "Cannot insert data %hi into list in ailsa_store_basic_sqlite", i);
	}
}

static int
ailsa_bind_arguments_sqlite(sqlite3_stmt *state, AILLIST *args, unsigned int t, const unsigned int *f)
{
	if (!(state) || !(args))
		return AILSA_NO_DATA;
	const char *text = NULL;
	int retval = 0;
	int i = 0;
	ailsa_data_s *data;
	AILELEM *tmp = args->head;

	for (i = 0; (unsigned int)i < t; i++) {
		if (tmp) {
			data = tmp->data;
		} else {
			ailsa_syslog(LOG_ERR, "List stopped with %hi of %hi arguments bound for sqlite", i, t);
			retval = CBC_DATA_WRONG_COUNT;
			break;
		}
		switch (f[i]) {
		case AILSA_DB_LINT:
			if ((retval = sqlite3_bind_int64(state, i + 1, (sqlite3_int64)data->data->number)) != 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind integer value in loop %hi", i);
				goto cleanup;
			}
			break;
		case AILSA_DB_TEXT:
			text = data->data->text;
			if ((retval = sqlite3_bind_text(state, i + 1, text, (int)strlen(text), SQLITE_STATIC)) > 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind text value %s", text);
				goto cleanup;
			}
			break;
		case AILSA_DB_FLOAT:
			if ((retval = sqlite3_bind_double(state, i + 1, data->data->point)) != 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind double value %g", data->data->point);
				goto cleanup;
			}
			break;
		case AILSA_DB_SINT:
			if ((retval = sqlite3_bind_int(state, i + 1, data->data->small)) != 0) {
				ailsa_syslog(LOG_ERR, "Unable to bind small value in loop %hi", i);
				goto cleanup;
			}
			break;
		default:
			ailsa_syslog(LOG_ERR, "Unknown SQL type %hi", f[i]);
			retval = AILSA_INVALID_DBTYPE;
			goto cleanup;
		}
		tmp = tmp->next;
	}
	cleanup:
		return retval;
}

#endif
