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
#include "cmdb_dnsa.h"

extern const char *dnsa_sql_select[];
extern const char *dnsa_sql_insert[];
extern const char *dnsa_sql_search[];
extern const char *dnsa_sql_extended_search[];
extern const char *dnsa_sql_update[];
extern const char *dnsa_sql_delete[];
extern const unsigned int dnsa_select_fields[];
extern const unsigned int dnsa_insert_fields[];
extern const unsigned int dnsa_search_fields[];
extern const unsigned int dnsa_search_args[];
extern const unsigned int dnsa_extended_search_fields[];
extern const unsigned int dnsa_extended_search_args[];
extern const unsigned int dnsa_ext_search_field_type[][5];
extern const unsigned int dnsa_ext_search_arg_type[][2];
extern const unsigned int dnsa_update_args[];
extern const unsigned int dnsa_update_arg_type[][3];
extern const unsigned int dnsa_delete_args[];
extern const unsigned int dnsa_delete_arg_type[][1];

# ifdef HAVE_MYSQL
extern const int dnsa_mysql_inserts[][15];
# endif /* HAVE_MYSQL */

int
dnsa_run_query(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_multiple_query(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_get_query(int type, const char **query, unsigned int *fields);
void
dnsa_get_search(int type, size_t *fields, size_t *args, void **input, void **ouput, dnsa_s *base);
int
dnsa_run_search(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_extended_search(dnsa_config_s *config, dbdata_s *base, int type);
int
dnsa_run_insert(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_update(dnsa_config_s *config, dbdata_s *data, int type);
int
dnsa_run_delete(dnsa_config_s *config, dbdata_s *data, int type);

# ifdef HAVE_MYSQL
#  include <mysql.h>

void
dnsa_mysql_init(dnsa_config_s *dc, MYSQL *cmdb_mysql);
int
dnsa_run_query_mysql(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_multiple_query_mysql(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_insert_mysql(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_search_mysql(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_extended_search_mysql(dnsa_config_s *config, dbdata_s *base, int type);
int
dnsa_run_update_mysql(dnsa_config_s *config, dbdata_s *data, int type);
int
dnsa_run_delete_mysql(dnsa_config_s *config, dbdata_s *data, int type);
int
dnsa_setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, dnsa_s *base);
void
dnsa_setup_bind_ext_mysql_args(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base);
void
dnsa_setup_bind_ext_mysql_fields(MYSQL_BIND *mybind, unsigned int i, int j, int type, dbdata_s *base);
int
dnsa_setup_insert_mysql_bind_buffer(int type, void **input, dnsa_s *base, unsigned int i);
void
dnsa_setup_insert_mysql_bind_buff_record(void **input, dnsa_s *base, unsigned int i);
void
dnsa_setup_insert_mysql_bind_buff_zone(void **input, dnsa_s *base, unsigned int i);
void
dnsa_setup_insert_mysql_bind_buff_rev_zone(void **input, dnsa_s *base, unsigned int i);
void
dnsa_setup_insert_mysql_bind_buff_rev_records(void **input, dnsa_s *base, unsigned int i);
void
dnsa_setup_insert_mysql_bind_buff_pref_a(void **input, dnsa_s *base, unsigned int i);
void
dnsa_setup_insert_mysql_bind_buff_glue(void **input, dnsa_s *base, unsigned int i);
void
dnsa_store_result_mysql(MYSQL_ROW row, dnsa_s *base, int type, unsigned int fields);
void
dnsa_store_zone_mysql(MYSQL_ROW row, dnsa_s *base);
void
dnsa_store_record_mysql(MYSQL_ROW row, dnsa_s *base);
void
dnsa_store_rev_zone_mysql(MYSQL_ROW row, dnsa_s *base);
void
dnsa_store_rev_record_mysql(MYSQL_ROW row, dnsa_s *base);
void
dnsa_store_all_a_records_mysql(MYSQL_ROW row, dnsa_s *base);
void
dnsa_store_preferred_a_mysql(MYSQL_ROW row, dnsa_s *dnsa);
void
dnsa_store_duplicate_a_record_mysql(MYSQL_ROW row, dnsa_s *dnsa);
void
dnsa_store_glue_mysql(MYSQL_ROW row, dnsa_s *dnsa);

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
dnsa_run_query_sqlite(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_multiple_query_sqlite(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_insert_sqlite(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_search_sqlite(dnsa_config_s *config, dnsa_s *base, int type);
int
dnsa_run_extended_search_sqlite(dnsa_config_s *config, dbdata_s *base, int type);
int
dnsa_run_update_sqlite(dnsa_config_s *config, dbdata_s *data, int type);
int
dnsa_run_delete_sqlite(dnsa_config_s *config, dbdata_s *data, int type);
int
dnsa_setup_insert_sqlite_bind(sqlite3_stmt *state, dnsa_s *base, int type);
int
dnsa_setup_bind_extended_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);
int
dnsa_get_extended_results_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);
void
dnsa_store_result_sqlite(sqlite3_stmt *state, dnsa_s *base, int type, unsigned int fields);
void
dnsa_store_zone_sqlite(sqlite3_stmt *state, dnsa_s *base);
void
dnsa_store_rev_zone_sqlite(sqlite3_stmt *state, dnsa_s *base);
void
dnsa_store_record_sqlite(sqlite3_stmt *state, dnsa_s *base);
void
dnsa_store_rev_record_sqlite(sqlite3_stmt *state, dnsa_s *base);
void
dnsa_store_all_a_records_sqlite(sqlite3_stmt *state, dnsa_s *dnsa);
void
dnsa_store_preferred_a_sqlite(sqlite3_stmt *state, dnsa_s *base);
void
dnsa_store_duplicate_a_record_sqlite(sqlite3_stmt *state, dnsa_s *base);
void
dnsa_store_glue_sqlite(sqlite3_stmt *state, dnsa_s *base);
int
dnsa_setup_bind_sqlite_records(sqlite3_stmt *state, record_row_s *record);
int
dnsa_setup_bind_sqlite_zones(sqlite3_stmt *state, zone_info_s *zone);
int
dnsa_setup_bind_sqlite_rev_zones(sqlite3_stmt *state, rev_zone_info_s *zone);
int
dnsa_setup_bind_sqlite_rev_records(sqlite3_stmt *state, rev_record_row_s *rev);
int
dnsa_setup_bind_sqlite_prefer_a(sqlite3_stmt *state, preferred_a_s *prefer);
int
dnsa_setup_bind_sqlite_glue(sqlite3_stmt *state, glue_zone_info_s *glue);

# endif /* HAVE_SQLITE3 */

#endif /* __DNSA_BASE_SQL_H */

