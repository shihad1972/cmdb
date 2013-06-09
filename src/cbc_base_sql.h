/*
 *  cbc: Create Build config
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc_base_sql.h
 *
 *  This file is the header file for the SQL functions for the cbc program
 */


#ifndef __CBC_BASE_SQL_H
# define __CBC_BASE_SQL_H
# include "../config.h"
# include "base_sql.h"

extern const char *cbc_sql_select[];
extern const char *cbc_sql_insert[];
extern const char *cbc_sql_search[];
extern const char *cbc_sql_update[];
extern const char *cbc_sql_delete[];
extern const char *cbc_sql_update[];
extern const char *cbc_sql_delete[];
extern const char *cbc_sql_search[];

extern const unsigned int cbc_select_fields[];
extern const unsigned int cbc_insert_fields[];
extern const unsigned int cbc_search_args[];
extern const unsigned int cbc_search_fields[];
extern const unsigned int cbc_update_args[];
extern const unsigned int cbc_update_fields[];
extern const unsigned int cbc_delete_args[];
extern const unsigned int cbc_delete_fields[];
extern const unsigned int cbc_update_args[];
extern const unsigned int cbc_delete_args[];
extern const unsigned int cbc_search_args[];
extern const unsigned int cbc_search_fields[];

extern const unsigned int cbc_update_types[][2];
extern const unsigned int cbc_delete_types[][2];
extern const unsigned int cbc_search_arg_types[][3];
extern const unsigned int cbc_search_field_types[][9];


enum {			/* cbc delete SQL statements */
	BDOM_DEL_DOMAIN = 0,
	BDOM_DEL_DOM_ID = 1,
	BOS_DEL_BOS_ID = 2,
	VARI_DEL_VARI_ID = 3,
	PACK_DEL_PACK_ID = 4
};

enum {			/* cbc search SQL statements */
	LDAP_CONFIG_ON_DOM = 0,
	LDAP_CONFIG_ON_ID = 1,
	BUILD_DOMAIN_COUNT = 2,
	BUILD_OS_ON_NAME = 3,
	OS_ALIAS_ON_OS = 4,
	BUILD_TYPE_ID_ON_ALIAS = 5,
	OS_ID_ON_NAME = 6,
	OS_ID_ON_ALIAS = 7,
	BUILD_ID_ON_OS_ID = 8,
	SERVERS_USING_BUILD_OS = 9,
	VARIENT_ID_ON_VARIENT = 10,
	VARIENT_ID_ON_VALIAS = 11,
	OS_ID_ON_NAME_SHORT = 12,
	OS_ID_ON_ALIAS_SHORT = 13,
	OS_ID_ON_NAME_AND_VERSION = 14,
	OS_VARIENT_ID_ON_PACKAGE = 15,
	SERVERS_WITH_BUILD = 16,
	DHCP_DETAILS = 17,
	SERVER_ID_ON_UUID = 18,
	SERVER_ID_ON_SNAME = 19,
	SERVER_NAME_ON_ID = 20,
	TFTP_DETAILS = 21,
	NET_BUILD_DETAILS = 22,
	BUILD_MIRROR = 23,
	BASIC_PART = 24,
	FULL_PART = 25,
	BUILD_PACKAGES = 26,
	LDAP_CONFIG = 27,
	XYMON_CONFIG = 28,
	SMTP_CONFIG = 29
};

# ifdef HAVE_MYSQL
extern const int mysql_inserts[][24];
# endif /* HAVE_MYSQL */

int
cbc_run_query(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_multiple_query(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_insert(cbc_config_s *config, cbc_s *base, int type);

int
cbc_get_query(int type, const char **query, unsigned int *fields);

void
cbc_init_initial_dbdata(dbdata_s **list, unsigned int type);

int
cbc_run_search(cbc_config_s *ccs, dbdata_s *base, int type);

int
cbc_run_delete(cbc_config_s *ccs, dbdata_s *base, int type);

# ifdef HAVE_MYSQL
#  include <mysql.h>
void
cbc_mysql_init(cbc_config_s *dc, MYSQL *cmdb_mysql);

int
cbc_run_query_mysql(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_insert_mysql(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_multiple_query_mysql(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_delete_mysql(cbc_config_s *ccs, dbdata_s *base, int type);

int
cbc_run_search_mysql(cbc_config_s *ccs, dbdata_s *base, int type);

void
cbc_store_result_mysql(MYSQL_ROW row, cbc_s *base, int type, unsigned int fields);

int
cbc_set_search_args_mysql(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base);

int
cbc_set_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base);

int
cbc_setup_insert_mysql_bind(MYSQL_BIND *mybind, unsigned int i, int type, cbc_s *base);

int
cbc_setup_insert_mysql_buffer(int type, void **buffer, cbc_s *base, unsigned int i);

void
cbc_store_boot_line_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_build_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_build_domain_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_build_ip_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_build_os_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_build_type_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_disk_dev_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_locale_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_package_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_dpart_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_spart_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_seed_scheme_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_server_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_varient_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_store_vmhost_mysql(MYSQL_ROW row, cbc_s *base);

void
cbc_setup_bind_mysql_build_domain(void **buffer, cbc_s *base, unsigned int i);

void
cbc_setup_bind_mysql_build_os(void **buffer, cbc_s *base, unsigned int i);

void
cbc_setup_bind_mysql_build_varient(void **buffer, cbc_s *base, unsigned int i);

void
cbc_setup_bind_mysql_build_part_scheme(void **buffer, cbc_s *base, unsigned int i);

void
cbc_setup_bind_mysql_build_def_part(void **buffer, cbc_s *base, unsigned int i);

void
cbc_setup_bind_mysql_build_package(void **buffer, cbc_s *base, unsigned int i);

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
cbc_run_query_sqlite(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_insert_sqlite(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_multiple_query_sqlite(cbc_config_s *config, cbc_s *base, int type);

int
cbc_run_delete_sqlite(cbc_config_s *ccs, dbdata_s *base, int type);

int
cbc_run_search_sqlite(cbc_config_s *ccs, dbdata_s *base, int type);

int
set_cbc_search_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);

int
get_cbc_search_res_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);

void
cbc_store_result_sqlite(sqlite3_stmt *state, cbc_s *base, int type, unsigned int fields);

int
cbc_setup_insert_sqlite_bind(sqlite3_stmt *state, cbc_s *base, int type);

void
cbc_store_boot_line_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_build_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_build_domain_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_build_ip_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_build_os_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_build_type_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_disk_dev_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_locale_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_package_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_dpart_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_spart_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_seed_scheme_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_server_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_varient_sqlite(sqlite3_stmt *state, cbc_s *base);

void
cbc_store_vmhost_sqlite(sqlite3_stmt *state, cbc_s *base);

int
cbc_setup_bind_sqlite_build_domain(sqlite3_stmt *state, cbc_build_domain_s *bdom);

int
cbc_setup_bind_sqlite_build_os(sqlite3_stmt *state, cbc_build_os_s *bos);

int
cbc_setup_bind_sqlite_build_varient(sqlite3_stmt *state, cbc_varient_s *vari);

int
cbc_setup_bind_sqlite_build_part_scheme(sqlite3_stmt *state, cbc_seed_scheme_s *seed);

int
cbc_setup_bind_sqlite_build_part(sqlite3_stmt *state, cbc_pre_part_s *part);

int
cbc_setup_bind_sqlite_build_pack(sqlite3_stmt *state, cbc_package_s *pack);

# endif /* HAVE_SQLITE3 */
#endif /* __CBC_BASE_SQL_H */
