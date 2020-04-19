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
	BASE_VARIENT_PACKAGES
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
	PACKAGE_DETAILS_FOR_VARIENT
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
	INSERT_VARIENT
};

enum {			// SQL DELETE QUERIES
	DELETE_BUILD_OS = 0,
	DELETE_VARIENT
};

typedef struct ailsa_sql_single_s {
	const char *string;
	short int type;
	short int length;
} ailsa_sql_single_s;

typedef struct ailsa_sql_query_s {
	const char *query;
	unsigned int number;
	unsigned int fields[9];
} ailsa_sql_query_s;

typedef struct ailsa_sql_multi_s {
	char *query;
	unsigned int number;
	unsigned int total;
	unsigned int *fields;
} ailsa_sql_multi_s;


extern const ailsa_sql_query_s varient_queries[];
extern const ailsa_sql_query_s delete_queries[];

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *results);

int
ailsa_argument_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *args, AILLIST *results);

int
ailsa_individual_query(ailsa_cmdb_s *cmdb, const ailsa_sql_query_s *query, AILLIST *args, AILLIST *results);

int
ailsa_delete_query(ailsa_cmdb_s *cmdb, const struct ailsa_sql_query_s query, AILLIST *remove);

int
ailsa_insert_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *insert);

int
ailsa_multiple_insert_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *insert);

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
cmdb_add_build_type_id_to_list(char *alias, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_add_os_id_to_list(char *os, char *arch, char *version, ailsa_cmdb_s *cc, AILLIST *list);

int
cmdb_check_for_os(ailsa_cmdb_s *cc, char *os, char *arch, char *version);

int
check_builds_for_os_id(ailsa_cmdb_s *cc, unsigned long int id, AILLIST *list);

// Data manipulation functions

void
cmdb_clean_ailsa_sql_multi(ailsa_sql_multi_s *data);

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
