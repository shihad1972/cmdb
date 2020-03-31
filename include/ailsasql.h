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
	HARDWARE_TYPES_ALL
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
	SERVICE_TYPE_ID_ON_DETAILS
};

enum {			// SQL INSERT QUERIES
	INSERT_CONTACTS = 0,
	INSERT_SERVER,
	INSERT_SERVICE_TYPE
};

typedef struct ailsa_sql_single_s {
	const char *string;
	short int type;
	short int length;
} ailsa_sql_single_s;

typedef struct ailsa_sql_query_s {
	const char *query;
	short int number;
	short int fields[9];
} ailsa_sql_query_s;

extern const struct ailsa_sql_single_s server_table[];
extern size_t server_fields;

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *results);
int
ailsa_argument_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *args, AILLIST *results);
int
ailsa_insert_query(ailsa_cmdb_s *cmdb, unsigned int query_no, AILLIST *insert);

# ifdef HAVE_MYSQL
#  include <mysql.h>
void
ailsa_mysql_init(ailsa_cmdb_s *dc, MYSQL *cbc_mysql);
int
ailsa_mysql_query_with_checks(MYSQL *mycmdb, const char *query);
int
ailsa_run_mysql_stmt(MYSQL *cmdb, MYSQL_BIND *my_bind, const char *query);
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
ailsa_setup_rw_sqlite(const char *query, const char *file, sqlite3 **cmdb, sqlite3_stmt **stmt);
void
ailsa_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt);
# endif // HAVE_SQLITE3
#endif
