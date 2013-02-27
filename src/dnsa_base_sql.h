 /*
 *  dnsa: DNS administration
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
 *  dnsa_base_sql.h
 *
 *  This file contains the SQL statement list for the dnsa program
 */

#ifndef __DNSA_BASE_SQL_H
# define __DNSA_BASE_SQL_H
# include "../config.h"

extern const char *sql_select[];
extern const char *sql_insert[];
extern const char *sql_search[];
extern const char *sql_extended_search[];
extern const char *sql_update[];
extern const unsigned int select_fields[];
extern const unsigned int insert_fields[];
extern const unsigned int search_fields[];
extern const unsigned int search_args[];
extern const unsigned int extended_search_fields[];
extern const unsigned int extended_search_args[];
extern const unsigned int ext_search_field_type[][3];
extern const unsigned int ext_search_arg_type[][1];
extern const unsigned int update_args[];
extern const unsigned int update_arg_type[][2];

# ifdef HAVE_MYSQL
extern const int mysql_inserts[][13];
# endif /* HAVE_MYSQL */

enum {			/* SELECT statements to use in multiple */
	ZONE = 1,
	REV_ZONE = 2,
	RECORD = 4,
	REV_RECORD = 8,
	ALL_A_RECORD = 16,
	DUPLICATE_A_RECORD = 32,
	PREFERRED_A = 64
};

enum {			/* SELECT and INSERT indexes */
	ZONES = 0,
	REV_ZONES,
	RECORDS,
	REV_RECORDS,
	ALL_A_RECORDS,
	DUPLICATE_A_RECORDS,
	PREFERRED_AS
};

enum {			/* Extended searches */
	RECORDS_ON_DEST_AND_ID = 0
};

enum {			/* Search indexes and queries */
	ZONE_ID_ON_NAME = 0,
	REV_ZONE_ID_ON_NET_RANGE,
	REV_ZONE_PREFIX
};

enum {			/* Update indexes */
	ZONE_VALID_YES = 0,
	ZONE_UPDATED_YES,
	ZONE_UPDATED_NO,
	ZONE_SERIAL,
	REV_ZONE_VALID_YES
};

int
run_query(dnsa_config_t *config, dnsa_t *base, int type);
int
run_multiple_query(dnsa_config_t *config, dnsa_t *base, int type);
int
get_query(int type, const char **query, unsigned int *fields);
void
get_search(int type, size_t *fields, size_t *args, void **input, void **ouput, dnsa_t *base);
int
run_search(dnsa_config_t *config, dnsa_t *base, int type);
int
run_extended_search(dnsa_config_t *config, dbdata_t *base, int type);
int
run_insert(dnsa_config_t *config, dnsa_t *base, int type);
int
run_update(dnsa_config_t *config, dbdata_t *data, int type);

# ifdef HAVE_MYSQL
#  include <mysql.h>

void
cmdb_mysql_init(dnsa_config_t *dc, MYSQL *cmdb_mysql);
int
run_query_mysql(dnsa_config_t *config, dnsa_t *base, int type);
int
run_multiple_query_mysql(dnsa_config_t *config, dnsa_t *base, int type);
int
run_insert_mysql(dnsa_config_t *config, dnsa_t *base, int type);
int
run_search_mysql(dnsa_config_t *config, dnsa_t *base, int type);
int
run_extended_search_mysql(dnsa_config_t *config, dbdata_t *base, int type);
int
run_update_mysql(dnsa_config_t *config, dbdata_t *data, int type);
int
setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, dnsa_t *base);
int
setup_bind_ext_mysql_args(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_t *base);
int
setup_bind_ext_mysql_fields(MYSQL_BIND *mybind, unsigned int i, int j, int type, dbdata_t *base);
int
setup_insert_mysql_bind_buffer(int type, void **input, dnsa_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_record(void **input, dnsa_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_zone(void **input, dnsa_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_rev_zone(void **input, dnsa_t *base, unsigned int i);
/*
void
setup_insert_mysql_bind_buff_server(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_customer(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_contact(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_service(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_hardware(void **buffer, cmdb_t *base, unsigned int i); */
void
store_result_mysql(MYSQL_ROW row, dnsa_t *base, int type, unsigned int fields);
void
store_zone_mysql(MYSQL_ROW row, dnsa_t *base);
void
store_record_mysql(MYSQL_ROW row, dnsa_t *base);
void
store_rev_zone_mysql(MYSQL_ROW row, dnsa_t *base);
void
store_rev_record_mysql(MYSQL_ROW row, dnsa_t *base);
void
store_preferred_a_mysql(MYSQL_ROW row, dnsa_t *dnsa);
void
store_duplicate_a_record_mysql(MYSQL_ROW row, dnsa_t *dnsa);
/*
void
store_customer_mysql(MYSQL_ROW row, cmdb_t *base);
void
store_contact_mysql(MYSQL_ROW row, cmdb_t *base);
void
store_service_mysql(MYSQL_ROW row, cmdb_t *base);
void
store_service_type_mysql(MYSQL_ROW row, cmdb_t *base);
void
store_hardware_mysql(MYSQL_ROW row, cmdb_t *base);
void
store_hardware_type_mysql(MYSQL_ROW row, cmdb_t *base);
void
store_vm_hosts_mysql(MYSQL_ROW row, cmdb_t *base); */

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
run_query_sqlite(dnsa_config_t *config, dnsa_t *base, int type);
int
run_multiple_query_sqlite(dnsa_config_t *config, dnsa_t *base, int type);
int
run_insert_sqlite(dnsa_config_t *config, dnsa_t *base, int type);
int
run_search_sqlite(dnsa_config_t *config, dnsa_t *base, int type);
int
run_extended_search_sqlite(dnsa_config_t *config, dbdata_t *base, int type);
int
run_update_sqlite(dnsa_config_t *config, dbdata_t *data, int type);
int
setup_insert_sqlite_bind(sqlite3_stmt *state, dnsa_t *base, int type);
int
setup_bind_extended_sqlite(sqlite3_stmt *state, dbdata_t *list, int type, int i);
int
get_extended_results_sqlite(sqlite3_stmt *state, dbdata_t *list, int type, int i);
void
store_result_sqlite(sqlite3_stmt *state, dnsa_t *base, int type, unsigned int fields);
void
store_zone_sqlite(sqlite3_stmt *state, dnsa_t *base);
void
store_rev_zone_sqlite(sqlite3_stmt *state, dnsa_t *base);
void
store_record_sqlite(sqlite3_stmt *state, dnsa_t *base);
void
store_rev_record_sqlite(sqlite3_stmt *state, dnsa_t *base);
void
store_preferred_a_sqlite(sqlite3_stmt *state, dnsa_t *base);
void
store_duplicate_a_record_sqlite(sqlite3_stmt *state, dnsa_t *base);
int
setup_bind_sqlite_records(sqlite3_stmt *state, record_row_t *record);
int
setup_bind_sqlite_zones(sqlite3_stmt *state, zone_info_t *zone);
int
setup_bind_sqlite_rev_zones(sqlite3_stmt *state, rev_zone_info_t *zone);
/*
void
store_server_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_customer_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_contact_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_service_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_service_type_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_hardware_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_hardware_type_sqlite(sqlite3_stmt *state, cmdb_t *base);
void
store_vm_hosts_sqlite(sqlite3_stmt *state, cmdb_t *base);
int
setup_bind_sqlite_server(sqlite3_stmt *state, cmdb_server_t *server);
int
setup_bind_sqlite_customer(sqlite3_stmt *state, cmdb_customer_t *customer);
int
setup_bind_sqlite_contact(sqlite3_stmt *state, cmdb_contact_t *cont);
int
setup_bind_sqlite_service(sqlite3_stmt *state, cmdb_service_t *service);
int
setup_bind_sqlite_hardware(sqlite3_stmt *state, cmdb_hardware_t *hard); */

# endif /* HAVE_SQLITE3 */

#endif /* __DNSA_BASE_SQL_H */

