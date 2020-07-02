/*
 *  ailsasql: Ailsatech SQL library
 *  Copyright (C) 2019 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  ailsasql.h: Main ailsacmdb library header file
 */

#ifndef __AILSASQL_H__
# define __AILSASQL_H__

enum {			// SQL Data types
	AILSA_DB_NULL = 0,
	AILSA_DB_TEXT = 1,
	AILSA_DB_LINT = 2,
	AILSA_DB_SINT = 3,
	AILSA_DB_TINY = 4,
	AILSA_DB_FLOAT = 5,
	AILSA_DB_TIME = 6
};

enum {			// SQL Tables
	BUILD_TABLE = 0,
	BUILD_DOMAIN_TABLE,
	BUILD_IP_TABLE,
	BUILD_OS_TABLE,
	BUILD_TYPE_TABLE,
	CONTACTS_TABLE,
	CUSTOMER_TABLE,
	DEFAULT_LOCALE_TABLE,
	DEFAULT_PART_TABLE,
	DISK_DEV_TABLE,
	GLUE_ZONES_TABLE,
	HARD_TYPE_TABLE,
	HARDWARE_TABLE,
	LOCALE_TABLE,
	OPTIONS_TABLE,
	PACKAGES_TABLE,
	PART_OPTIONS_TABLE,
	PREFERRED_A_TABLE,
	RECORDS_TABLE,
	REV_RECORDS_TABLE,
	REV_ZONES_TABLE,
	SEED_SCHEMES_TABLE,
	SERVER_TABLE,
	SERVICE_TYPE_TABLE,
	SERVICES_TABLE,
	SYSTEM_PACKAGE_ARGS_TABLE,
	SYSTEM_PACKAGE_CONF_TABLE,
	SYSTEM_PACKAGES_TABLE,
	SYSTEM_SCRIPTS_TABLE,
	SYSTEM_SCRIPTS_ARGS_TABLE,
	USERS_TABLES,
	VARIENT_TABLE,
	VM_SERVER_HOSTS_TABLE,
	ZONES_TABLE
};

enum {			// SQL BASIC QUERIES
	SERVER_NAME_COID = 0,
	COID_NAME_CITY,
	VM_SERVERS,
	SERVICE_TYPES_ALL,
	HARDWARE_TYPES_ALL,
	BUILD_OS_NAME_TYPE,
	BUILD_OSES,
	BUILD_VARIENTS,
	BUILD_OS_ALIASES,
	BASE_VARIENT_PACKAGES,
	DEFAULT_LOCALE,
	LOCALE_NAMES,
	PARTITION_SCHEME_NAMES,
	BUILD_DOMAIN_NAMES,
	BUILD_DOMAIN_NETWORKS,
	FWD_ZONE_CONFIG,
	SYSTEM_PACKAGE_NAMES,
	SYSTEM_SCRIPT_NAMES,
	ZONE_NAME_TYPES,
	ZONE_INFORMATION,
	REV_ZONE_INFORMATION,
	GLUE_ZONE_INFORMATION,
	REV_ZONES_NET_RANGE,
	REV_ZONE_CONFIG,
	ALL_SERVERS_WITH_BUILD,
	DHCP_INFORMATION,
};

enum {			// SQL ARGUMENT QUERIES
	CONTACT_DETAILS_ON_COID = 0,
	SERVICES_ON_SERVER,
	HARDWARE_ON_SERVER,
	SERVER_DETAILS_ON_NAME,
	VM_HOST_BUILT_SERVERS,
	CUSTOMER_DETAILS_ON_COID,
	CUST_ID_ON_COID,
	CONT_ID_ON_CONTACT_DETAILS,
	VM_SERVER_ID_ON_NAME,
	SERVICE_TYPE_ID_ON_DETAILS,
	HARDWARE_TYPE_ID_ON_DETAILS,
	SERVER_ID_ON_NAME,
	HARDWARE_TYPE_ID_ON_CLASS,
	HARDWARE_ID_ON_DETAILS,
	SERVICE_TYPE_ID_ON_SERVICE,
	SERVICE_ID_ON_DETAILS,
	BT_ID_ON_ALIAS,
	CHECK_BUILD_OS,
	BUILD_OS_ON_NAME_OR_ALIAS,
	BUILD_OS_ON_NAME_VERSION,
	BUILD_OS_ON_NAME_ARCH,
	BUILD_OS_ON_ALL,
	OS_FROM_BUILD_TYPE_AND_ARCH,
	PACKAGE_DETAIL_ON_OS_ID,
	SERVERS_WITH_BUILDS_ON_OS_ID,
	VARIENT_ID_ON_VARIANT_OR_VALIAS,
	PACKAGE_DETAILS_FOR_VARIENT,
	LOCALE_ON_NAME,
	PARTITIONS_ON_SCHEME_NAME,
	SEED_SCHEME_ON_NAME,
	PART_OPTIONS_ON_SCHEME_NAME_AND_PARTITION,
	IDENTIFY_PARTITION,
	SCHEME_LVM_INFO,
	SCHEME_ID_ON_NAME,
	PARTITION_ID_ON_SEED_MOUNT,
	IDENTIFY_PART_OPTION,
	BUILD_DOMAIN_ID_ON_DOMAIN,
	BUILD_DOMAIN_OVERLAP,
	BUILD_DOMAIN_DETAILS_ON_NAME,
	BUILD_DETAILS_ON_DOMAIN,
	FWD_ZONE_ID_ON_ZONE_NAME,
	REV_ZONE_ID_ON_RANGE,
	NAME_SERVERS_ON_NAME,
	ZONE_SOA_ON_NAME,
	NS_MX_SRV_RECORDS,
	ZONE_RECORDS_ON_NAME,
	SYS_PACK_DETAILS_ON_DOMAIN,
	SYS_PACK_DETAILS_ON_NAME_DOMAIN,
	SYS_PACK_DETAILS_MIN,
	SYS_PACK_ARGS_ON_NAME,
	SYSTEM_PACKAGE_ID,
	SYSTEM_PACKAGE_ARG_ID,
	SYSTEM_SCRIPTS_ON_NAME,
	SYSTEM_SCRIPTS_ON_NAME_DOMAIN,
	SYSTEM_SCRIPTS_ON_DOMAIN,
	SYSTEM_SCRIPT_ID_ON_NAME,
	SYSTEM_SCRIPT_ARG_ID,
	GLUE_ZONE_ON_ZONE_NAME,
	DESTINATION_ON_NAME_ZONE,
	ZONE_TYPE_ON_NAME,
	ZONE_SERIAL_ON_NAME,
	REV_ZONE_INFO_ON_RANGE,
	REV_RECORDS_ON_ZONE_ID,
	REV_SOA_ON_NET_RANGE,
	REV_RECORDS_ON_NET_RANGE,
	RECORD_ID_BASE,
	RECORD_ID_MX,
	RECORD_ID_SRV,
	RECORD_ON_ZONE_AND_HOST,
	RECORD_ON_ZONE_AND_HOST_AND_A,
	RECORD_ID_ON_HOST_ZONE_TYPE,
	RECORD_IN_ON_PRI,
	RECORD_IN_ON_PROTOCOL,
	DUP_IP_NET_RANGE,
	DUP_IP_A_RECORD,
	A_RECORDS_WITH_IP,
	FQDN_PREF_A_ON_IP,
	NET_START_FINISH_ON_RANGE,
	PREFIX_ON_NET_RANGE,
	PREFER_A_INFO_ON_RANGE,
	RECORDS_ON_NET_RANGE,
	REV_RECORD_ID_ON_ZONE_HOST,
	DESTINATION_ON_RECORD_ID,
	BUILD_IP_ON_SERVER_ID,
	BUILD_DOMAIN_NET_INFO,
	BUILD_IP_ON_BUILD_DOMAIN_ID,
	BUILD_ID_ON_SERVER_NAME,
	MAC_ADDRESS_FOR_BUILD,
	LOCALE_ID_FROM_NAME,
	DISK_ID_ON_SERVER_NAME,
	SEED_SCHEME_LVM_ON_ID,
	IP_ID_ON_SERVER_NAME,
	BUILD_DETAILS_ON_SERVER_NAME,
	BUILD_OS_DETAILS_ON_OS_ID,
	IP_NET_ON_SERVER_ID,
	BUILD_OS_AND_TYPE,
	BUILD_VARIENT_ON_SERVER_ID,
	LOCALE_DETAILS_ON_SERVER_ID,
	BUILD_TIMES_AND_USERS,
	PART_SCHEME_NAME_ON_SERVER_ID,
	PARTITIOINS_ON_SERVER_ID,
	OS_ALIAS_ON_OS_NAME,
	BUILD_OS_DETAILS_ON_SERVER_ID,
	FULL_LOCALE_DETAILS_ON_SERVER_ID,
	TFTP_DETAILS_ON_SERVER_ID,
	PRESEED_BUILD_DETAILS,
	BUILD_TYPE_ON_SERVER_ID,
	BUILD_PARTITIONS_ON_SERVER_ID,
	BUILD_PACKAGES_ON_SERVER_ID,
};

enum {			// SQL INSERT QUERIES
	INSERT_CONTACT = 0,
	INSERT_SERVER,
	INSERT_SERVICE_TYPE,
	INSERT_HARDWARE_TYPE,
	INSERT_VM_HOST,
	INSERT_HARDWARE,
	INSERT_SERVICE,
	INSERT_CUSTOMER,
	INSERT_BUILD_OS,
	INSERT_BUILD_PACKAGE,
	INSERT_VARIENT,
	INSERT_LOCALE,
	INSERT_SEED_SCHEME,
	INSERT_PARTITION,
	INSERT_PART_OPTION,
	INSERT_BUILD_DOMAIN,
	INSERT_FORWARD_ZONE,
	INSERT_SYSTEM_PACKAGE,
	INSERT_SYSTEM_PACKAGE_ARGS,
	INSERT_SYSTEM_PACKAGE_CONF,
	INSERT_SYSTEM_SCRIPT,
	INSERT_SYSTEM_SCRIPT_ARGS,
	INSERT_REVERSE_ZONE,
	INSERT_RECORD_BASE,
	INSERT_RECORD_MX,
	INSERT_RECORD_SRV,
	INSERT_REVERSE_RECORD,
	INSERT_GLUE_ZONE,
	INSERT_PREF_A,
	INSERT_DISK_DEV,
	INSERT_BUILD_IP,
	INSERT_BUILD,
};

enum {			// SQL DELETE QUERIES
	DELETE_BUILD_OS = 0,
	DELETE_VARIENT,
	DELETE_LOCALE,
	DELETE_PART_OPTION,
	DELETE_PARTITION,
	DELETE_SCHEME_ON_NAME,
	DELETE_BUILD_DOMAIN_ON_NAME,
	DELETE_SYS_PACK_CONF,
	DELETE_SYS_PACK_ARG,
	DELETE_SYSTEM_PACKAGE,
	DELETE_SYSTEM_SCRIPT_ARG,
	DELETE_SYSTEM_SCRIPT,
	DELETE_FWD_ZONE,
	DELETE_REV_ZONE,
	DELETE_REC_TYPE_HOST,
	DELETE_REC_PRI,
	DELETE_REC_PROTO,
	DELETE_REVERSE_RECORD,
	DELETE_GLUE_ZONE,
	DELETE_PREF_A,
	DELETE_BUILD_IP,
	DELETE_DISK_DEV,
	DELETE_BUILD_ON_SERVER_ID,
};

enum {			// SQL UPDATE QUERIES
	SET_DEFAULT_LOCALE = 0,
	SET_PART_SCHEME_UPDATED,
	UPDATE_BUILD_DOMAIN,
	FWD_ZONE_VALIDATE,
	FWD_ZONE_SERIAL_UPDATE,
	REV_ZONE_VALIDATE,
	REV_ZONE_SERIAL_UPDATE,
	SET_FWD_ZONE_UPDATED,
	SET_REV_ZONE_UPDATED,
	UPDATE_BUILD,
};

typedef struct ailsa_sql_single_s {
	const char *string;
	short int type;
	short int length;
} ailsa_sql_single_s;

typedef struct ailsa_sql_query_s {
	const char *query;
	unsigned int number;
	unsigned int fields[20];
} ailsa_sql_query_s;

typedef struct ailsa_sql_delete_s {
	char *query;
	unsigned int number;
	unsigned int fields[20];
} ailsa_sql_delete_s;

typedef struct ailsa_sql_multi_s {
	char *query;
	unsigned int number;
	unsigned int total;
	unsigned int *fields;
} ailsa_sql_multi_s;


extern const ailsa_sql_query_s varient_queries[];
extern const ailsa_sql_query_s delete_queries[];
extern const ailsa_sql_query_s update_queries[];
extern const ailsa_sql_query_s insert_queries[];

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *results);

int
ailsa_argument_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *args, AILLIST *results);

int
ailsa_individual_query(ailsa_cmdb_s *cmdb, const ailsa_sql_query_s *query, AILLIST *args, AILLIST *results);

int
ailsa_delete_query(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *remove);

int
ailsa_update_query(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *update);

int
ailsa_insert_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *insert);

int
ailsa_multiple_query(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *insert);

int
ailsa_multiple_delete(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *del);

// Some helper functions

int
cmdb_add_server_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_cust_id_to_list(char *coid, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_service_type_id_to_list(char *type, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_hard_type_id_to_list(char *hclass, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_varient_id_to_list(char *varient, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_build_domain_id_to_list(char *domain, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_build_type_id_to_list(char *alias, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_scheme_id_to_list(char *scheme, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_default_part_id_to_list(char *scheme, char *partition, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_disk_dev_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_os_id_to_list(char **os, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_os_alias_to_list(char *os, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_sys_pack_id_to_list(char *pack, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_sys_pack_arg_id_to_list(char **args, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_system_script_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_zone_id_to_list(char *zone, int type, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_vm_server_id_to_list(char *name, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_build_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_locale_id_to_list(char *locale, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_ip_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_disk_id_to_list(char *server, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_check_for_fwd_zone(ailsa_cmdb_s *cc, char *zone);

int
cmdb_check_for_fwd_record(ailsa_cmdb_s *cc, AILLIST *rec);

int
cmdb_check_for_os(ailsa_cmdb_s *cc, char *os, char *arch, char *version);

int
check_builds_for_os_id(ailsa_cmdb_s *cc, unsigned long int id, AILLIST *list);

int
set_db_row_updated(ailsa_cmdb_s *cc, unsigned int query, char *name, unsigned long int number);

int
dnsa_populate_zone(ailsa_cmdb_s *cbs, char *domain, AILLIST *zone);

// Data manipulation functions

void
cmdb_clean_ailsa_sql_multi(ailsa_sql_multi_s *data);

int
cmdb_replace_data_element(AILLIST *list, AILELEM *element, size_t number);

int
ailsa_get_bdom_list(ailsa_cmdb_s *cbs, AILLIST *list);

int
cmdb_populate_cuser_muser(AILLIST *list);

unsigned long int
generate_zone_serial(void);

int
cmdb_get_port_number(char *proto, char *service, unsigned int *port);

int
cmdb_getaddrinfo(char *name, char *ip, int *type);

unsigned long int
get_net_range(unsigned long int prefix);

int
do_rev_lookup(char *ip, char *host, size_t len);

// Some zone functions

int
cmdb_validate_zone(ailsa_cmdb_s *cbc, int type, char *zone);

int
cmdb_write_fwd_zone_config(ailsa_cmdb_s *cbs);

int
cmdb_write_rev_zone_config(ailsa_cmdb_s *cbs);

int
add_forward_zone(ailsa_cmdb_s *dc, char *domain);

# ifdef HAVE_MYSQL
#  include <mysql.h>
void
ailsa_mysql_init(ailsa_cmdb_s *dc, MYSQL *cbc_mysql);

int
ailsa_mysql_query_with_checks(MYSQL *mycmdb, const char *query);

int
ailsa_run_mysql_stmt(MYSQL *cmdb, MYSQL_BIND *my_bind, const struct ailsa_sql_query_s argu, AILLIST *args);

int
ailsa_bind_params_mysql(MYSQL_STMT *stmt, MYSQL_BIND **bind, const struct ailsa_sql_query_s argu, AILLIST *args);

int
ailsa_bind_results_mysql(MYSQL_STMT *stmt, MYSQL_BIND **bind, AILLIST *results);

int
ailsa_bind_parameters_mysql(MYSQL_STMT *stmt, MYSQL_BIND **bind, AILLIST *list, unsigned int total, unsigned int *f);

int
ailsa_set_bind_mysql(MYSQL_BIND *bind, ailsa_data_s *data, unsigned int fields);

void
ailsa_mysql_cleanup(MYSQL *cmdb);

void
ailsa_mysql_cleanup_full(MYSQL *cmdb, MYSQL_RES *res);

char *
ailsa_convert_mysql_time(MYSQL_TIME *time);

# endif // HAVE_MYSQL

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>
void
ailsa_setup_ro_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt);

void
ailsa_setup_rw_sqlite(const char *query, size_t len, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt);

void
ailsa_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt);

# endif // HAVE_SQLITE3
#endif
