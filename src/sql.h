
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
# define THIS_SQL_MAX 2097152	// 2MegaBytes

/* 
 * We are defining 5 different types of query per program:
 * SELECT (all columns and rows in 1 table)
 * SELECT (searching based on data; we return the number of rows we find)
 * INSERT (insert full new row into table)
 * UPDATE (update row(s) based on data; we return no of rows affected)
 * DELETE (delete row(s) based on id)
 */

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

const char *update_tables[] = {
// cmdb update tables
	"server",
	"customer",
	"server",
	"server",
	"server",
	"server",
	"server",
// dnsa update tables
	"zones",
	"zones",
	"zones",
	"zones",
	"rev_zones",
	"rev_zones",
	"zones",
// cbc update tables
	"build_domain",
	"build",
	"build",
	"build",
	"build",
	"build",
	"build",
	"build",
	"build_domain",
	"varient",
	"seed_schemes",
	"build_domain"
};

const unsigned int update_fields[] = {
// cmdb update fields
	1, 1, 1, 1, 1, 1, 1,
// dnsa update fields
	1, 1, 0, 2, 1, 2, 0,
// cbc update fields
	1, 1, 1, 1, 2, 2, 2, 3, 2, 1, 1, 1
};

const unsigned int static_update_fields[] = {
// cmdb static fields
	0, 0, 0, 0, 0, 0, 0,
// dnsa static fields
	2, 2, 1, 0, 2, 0, 1,
// cbc static fields
	0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
};

const char *update_field_columns[][5] = {
// cmdb update field columns
	{ "muser", NULL, NULL, NULL, NULL },
	{ "muser", NULL, NULL, NULL, NULL },
	{ "uuid", NULL, NULL, NULL, NULL },
	{ "make", NULL, NULL, NULL, NULL },
	{ "model", NULL, NULL, NULL, NULL },
	{ "vendor", NULL, NULL, NULL, NULL },
	{ "cust_id", NULL, NULL, NULL, NULL },
// dnsa update field columns
	{ "muser", NULL, NULL, NULL, NULL },
	{ "id", NULL, NULL, NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL },
	{ "serial", "muser", NULL, NULL, NULL },
	{ "muser", NULL, NULL, NULL, NULL },
	{ "serial", "muser", NULL, NULL, NULL },
	{ NULL, NULL, NULL, NULL, NULL },
// cbc update field columns
	{ "ntp_server", NULL, NULL, NULL, NULL },
	{ "varient_id", NULL, NULL, NULL, NULL },
	{ "os_id", NULL, NULL, NULL, NULL },
	{ "def_scheme_id", NULL, NULL, NULL, NULL },
	{ "varient_id", "os_id", NULL, NULL, NULL },
	{ "varient_id", "def_scheme_id", NULL, NULL, NULL },
	{ "os_id", "def_scheme_id", NULL, NULL, NULL },
	{ "varient_id", "os_id", "def_scheme_id", NULL, NULL },
	{ "config_ntp", "ntp_server", "muser", NULL, NULL },
	{ "muser", NULL, NULL, NULL, NULL },
	{ "muser", NULL, NULL, NULL, NULL },
	{ "muser", NULL, NULL, NULL, NULL }
};

const char *static_update_columns[][2] = {
// cmdb static columns
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
// dnsa static columns
	{ "valid", "updated" },
	{ "valid", "updated" },
	{ "updated", NULL },
	{ NULL, NULL },
	{ "valid", "updated" },
	{ NULL, NULL },
	{ "valid", NULL },
// cbc static columns
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ "config_ntp", NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL }
};

const char *static_update_values[][2] = {
// cmdb static values
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
// dnsa static values
	{ "yes", "no" },
	{ "unknown", "yes" },
	{ "no", NULL },
	{ NULL, NULL },
	{ "yes", "no" },
	{ NULL, NULL },
	{ "no", NULL },
// cbc static values
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ "1", NULL },
	{ NULL, NULL },
	{ NULL, NULL },
	{ NULL, NULL }
};

/*
 * SQL Searches
 *
 * First, the Name of search. The program is important as we
 * will use it as an index to find the tables and columns
 */

enum {
// cmdb searches
        SERVER_ID_ON_NAME = 0,
        CUST_ID_ON_COID,
        SERV_TYPE_ID_ON_SERVICE,
        HARD_TYPE_ID_ON_HCLASS,
        VM_ID_ON_NAME,
        HCLASS_ON_HARD_TYPE_ID,
        CUST_ID_ON_NAME,
        CONTACT_ID_ON_COID_NAME,
        SERVICE_ID_ON_URL,
        SERVICE_ID_ON_SERVICE,
        SERVICE_ID_ON_URL_SERVICE,
        SERVICE_ID_ON_SERVER_ID,
        SERVICE_ID_ON_CUST_ID,
        SERVICE_ID_ON_SERVER_ID_SERVICE,
        SERVICE_ID_ON_CUST_ID_SERVICE,
// dnsa searches
        ZONE_ID_ON_NAME,
        REV_ZONE_ID_ON_NET_RANGE,
        REV_ZONE_PREFIX,
        RECORDS_ON_DEST_AND_ID,
        RECORDS_ON_ZONE,
        DEST_IN_RANGE,
        RECORD_ID_ON_IP_DEST_DOM,
        FWD_ZONE_ID_ON_NAME,
        BUILD_DOM_ON_SERVER_ID,
// cbc searches
        LDAP_CONFIG_ON_DOM,
        LDAP_CONFIG_ON_ID,
        BUILD_DOMAIN_COUNT,
        BUILD_OS_ON_NAME,
        OS_ALIAS_ON_OS,
        BUILD_TYPE_ID_ON_ALIAS,
        OS_ID_ON_NAME,
        OS_ID_ON_ALIAS,
        BUILD_ID_ON_OS_ID,
        SERVERS_USING_BUILD_OS,
        VARIENT_ID_ON_VARIENT,
        VARIENT_ID_ON_VALIAS,
        OS_ID_ON_NAME_SHORT,
        OS_ID_ON_ALIAS_SHORT,
        OS_ID_ON_NAME_AND_VERSION,
        OS_VARIENT_ID_ON_PACKAGE,
        SERVERS_WITH_BUILD,
        DHCP_DETAILS,
        SERVER_ID_ON_UUID,
        SERVER_ID_ON_SNAME,
        SERVER_NAME_ON_ID,
        TFTP_DETAILS,
        NET_BUILD_DETAILS,
        BUILD_MIRROR,
        BASIC_PART,
        FULL_PART,
        BUILD_PACKAGES,
        LDAP_CONFIG,
        XYMON_CONFIG,
        SMTP_CONFIG,
        IP_ON_BD_ID,
        NETWORK_CARD,
        HARD_DISK_DEV,
        BUILD_IP_ON_SERVER_ID,
        BUILD_ID_ON_SERVER_ID,
        OS_ID_ON_NAME_VER_ALIAS,
        OS_ID_ON_ALIAS_VER_ALIAS,
        DEF_SCHEME_ID_ON_SCH_NAME,
        BD_ID_ON_DOMAIN,
        CONFIG_LDAP_BUILD_DOM,
        KICK_BASE,
        KICK_NET_DETAILS,
        BUILD_TYPE_URL,
        NTP_CONFIG,
        LOG_CONFIG,
        ALL_CONFIG,
        NFS_DOMAIN,
        BUILD_DOM_SERVERS,
        PACK_ID_ON_DETAILS,
        DEFP_ID_ON_SCHEME_PART,
        IP_ID_ON_HOST_DOMAIN,
        IP_ID_ON_IP,
        MAC_ON_SERVER_ID_DEV,
        LOCALE_ID_ON_OS_ID,
        IP_ID_ON_SERVER_ID,
        BUILD_DOM_IP_RANGE,
        DISK_DEV_ON_SERVER_ID_DEV,
        LVM_ON_DEF_SCHEME_ID,
        SYSPACK_ID_ON_NAME,
        SYSP_INFO_SYS_AND_BD_ID,
        SPARG_ON_SPID_AND_FIELD,
        SYSP_INFO_ARG_AND_BD_ID,
        SYSP_INFO_ON_BD_ID,
        BDOM_NAME_ON_SERVER_ID,
        NAME_DOM_ON_SERVER_ID,
        BD_ID_ON_SERVER_ID,
        SYS_PACK_CONF_ID,
        SCR_ID_ON_NAME,
        SCRIPT_CONFIG,
        BUILD_TYPE_ON_ALIAS,
        SCR_ARG_ID,
        PART_OPT_ON_SCHEME_ID,
        PART_OPT_ID,
        DEF_SCHEME_ID_FROM_BUILD,
        SCHEME_NAME_ON_SERVER_ID,
        PACKAGE_OS_ID_ON_VID,
        OS_DETAIL_ON_BT_ID,
        LOCALE_DETAILS_ON_OS_ID,
        PACKAGE_VID_ON_OS_ID,
        BOOT_FILES_MIRROR_DETAILS
};

const unsigned int sql_searches[] = {
	15, 12, 80
};

/*
 * We want to be able to modify SQL statements.
 * Here we define the modifiers and which queries have them.
 *
 * The sql_modifiers[][] array contains the program search index
 * for easy calculation.
 */
enum {
	DISTINCT = 1,
	COUNT = 2
};

const unsigned int sql_modifiers[][3] = {
	{ 1, 5, DISTINCT },
	{ 1, 10, COUNT }
};

/*
 * These arrays describe the SQL query.
 * We have the fields we are searching on, the arguments the query
 * will return, and the number of tables we need to join to perform
 * the query.
 */

const unsigned int search_fields[] = {
// cmdb search fields
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
// dnsa search fields
	1, 1, 1, 3, 5, 1, 1, 1, 1,
// cbc search fields
	5, 5, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 3, 1, 1,
	1, 10, 10, 7, 2, 6, 1, 5, 3, 4, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1,
	3, 11, 1, 2, 2, 6, 1, 2, 1, 1, 1, 1, 1, 1, 1, 3, 1, 1, 1, 4,
	1, 4, 4, 1, 2, 1, 1, 1, 3, 1, 1, 1, 1, 1, 1, 2, 3, 5, 2, 1
};

const unsigned int search_args[] = {
// cmdb search args
	1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 2,
// dnsa search args
	1, 1, 1, 1, 1, 2, 3, 1, 1,
// cbc search args
	1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, 1, 1, 1, 2, 1, 0, 1, 1, 1, // 20
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3, 1, 1, 1, // 20
	1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 2, 1, 2, 1, 1, 1, 2, 1, 1, 3, // 20
	2, 2, 1, 1, 1, 1, 3, 1, 2, 1, 4, 2, 3, 1, 1, 1, 1, 1, 1, 1  // 20
};

const unsigned int search_table_count[] = {
// cmdb search table number
	1, 1, 1, 1, 1, 1, 1, 2, 1, 2, 2, 1, 1, 2, 2,
// dnsa search table number
	1, 1, 1, 2, 1, 1, 1, 1, 1,
// cbc search table number
	1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4, 2, 2,
	1, 4, 5, 5, 3, 2, 2, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1
};

/*
 * This is a maxrix of all the search fields in a query. The index
 * is based on which program we are dealing with, so for the higher
 * number programs, you have to add the number of tables in all 
 * the previous programs to the number in the matrix to get the
 * correct table number. 
 *
 * The matrix is based on { table, column }
 */

const unsigned int search_field_columns[][11][2] = {
// cmdb search field columns
	{ { 6, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 4, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 2, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 7, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 2, 2 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 1, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 },
	  { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }
};

const unsigned int search_arg_columns[][5][2] = {
	{ { 0, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 1, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 4, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 2, 2 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 7, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 2, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 1, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 0, 1 }, { 1, 6 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 5 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 4, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 5 }, { 4, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 2 }, { 0, 0 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 1 }, { 4, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 } },
	{ { 5, 2 }, { 4, 1 }, { 0, 0 }, { 0, 0 }, { 0, 0 } }
};

const unsigned int search_join_columns[][4][4] = {
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 4, 1, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 5, 3, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 5, 3, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 5, 3, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
	{ { 5, 3, 4, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } },
};

#endif // __HAVE_SQL_H_

