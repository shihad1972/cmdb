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
	SERVER_NAME_COID = 0
};
typedef struct ailsa_sql_single_s {
	const char *string;
	short int type;
	short int length;
} ailsa_sql_single_s;

typedef struct ailsa_sql_basic_s {
	const char *query;
	short int number;
	short int fields[3];
} ailsa_sql_basic_s;

extern const struct ailsa_sql_single_s server[];
extern size_t server_fields;

int
ailsa_basic_query(ailsa_cmdb_s *cmdb, short int query_no, AILLIST *results);

# ifdef HAVE_MYSQL
#  include <mysql.h>
void
cmdb_mysql_init(ailsa_cmdb_s *dc, MYSQL *cbc_mysql);

# endif // HAVE_MYSQL

// SQL functions.
/*
void
ailsa_clean_dbv(void *dbv);
int
ailsa_init_ss(AILSS *data);
void
ailsa_clean_ss_data(void *data);
void
ailsa_clean_ss(AILSS *data);
int
ailsa_simple_select(ailsa_cmdb_s *config, AILSS *query, AILLIST *results);
*/
#endif
