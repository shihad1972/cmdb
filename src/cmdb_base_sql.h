 /*
 *  cmdb: Configuration Management Database
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
 *  cmdb_base_sql.h
 *
 *  This file contains the SQL statement list for the cmdb suite of programs
 */

#ifndef __CMDB_BASE_SQL_H
#define __CMDB_BASE_SQL_H
# include "../config.h"
#include "cmdb_cmdb.h"

extern const char *sql_select[];
extern const char *sql_insert[];
extern const char *sql_search[];
extern const char *cmdb_sql_delete[];
extern const char *cmdb_sql_update[];
extern const unsigned int select_fields[];
extern const unsigned int insert_fields[];
extern const unsigned int search_fields[];
extern const unsigned int search_args[];
extern const unsigned int cmdb_search_fields[];
extern const unsigned int cmdb_search_args[];
extern const unsigned int cmdb_search_args_type[][2];
extern const unsigned int cmdb_search_fields_type[][1];
extern const unsigned int cmdb_delete_args[];
extern const unsigned int cmdb_delete_args_type[][1];
extern const unsigned int cmdb_update_args[];
extern const unsigned int cmdb_update_args_type[][5];

# ifdef HAVE_MYSQL
extern const int cmdb_mysql_inserts[8][7];
# endif /* HAVE_MYSQL */

enum {		/* cmdb update SQL statements */
	UP_SERVER_MUSER = 0,
	UP_CUST_MUSER = 1,
	UP_SERVER_UUID = 2,
	UP_SERVER_MAKE = 3,
	UP_SERVER_MODEL = 4,
	UP_SERVER_VENDOR = 5,
	UP_SERVER_COID = 6
};

int
cmdb_run_query(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_multiple_query(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_get_query(int type, const char **query, unsigned int *fields);
int
cmdb_get_search(int type, size_t *fields, size_t *args, void **input, void **ouput, cmdb_s *base);
int
run_search(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_search(cmdb_config_s *cmdb, dbdata_s *data, int type);
int
cmdb_run_insert(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_delete(cmdb_config_s *config, dbdata_s *data, int type);
int
cmdb_run_update(cmdb_config_s *config, dbdata_s *data, int type);
void
show_no_results(int type);

# ifdef HAVE_MYSQL
#  include <mysql.h>

void
cmdb_mysql_init(cmdb_config_s *dc, MYSQL *cmdb_mysql);
int
cmdb_run_query_mysql(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_multiple_query_mysql(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_insert_mysql(cmdb_config_s *config, cmdb_s *base, int type);
int
run_search_mysql(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_delete_mysql(cmdb_config_s *config, dbdata_s *data, int type);
int
cmdb_run_update_mysql(cmdb_config_s *config, dbdata_s *data, int type);
int
cmdb_run_search_mysql(cmdb_config_s *cmdb, dbdata_s *data, int type);
void
cmdb_set_fields_mysql(MYSQL_BIND *mybind, unsigned int i, dbdata_s *base);
void
cmdb_set_search_args_mysql(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base);
void
cmdb_set_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base);
int
setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, cmdb_s *base);
int
setup_insert_mysql_bind_buffer(int type, void **input, cmdb_s *base, unsigned int i);
void
setup_insert_mysql_bind_buff_server(void **buffer, cmdb_s *base, unsigned int i);
void
setup_insert_mysql_bind_buff_customer(void **buffer, cmdb_s *base, unsigned int i);
void
setup_insert_mysql_bind_buff_contact(void **buffer, cmdb_s *base, unsigned int i);
void
setup_insert_mysql_bind_buff_service(void **buffer, cmdb_s *base, unsigned int i);
void
setup_insert_mysql_bind_buff_hardware(void **buffer, cmdb_s *base, unsigned int i);
void
setup_insert_mysql_bind_buff_vmhost(void **buffer, cmdb_s *base, unsigned int i);
void
store_result_mysql(MYSQL_ROW row, cmdb_s *base, int type, unsigned int fields);
void
store_server_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_customer_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_contact_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_service_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_service_type_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_hardware_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_hardware_type_mysql(MYSQL_ROW row, cmdb_s *base);
void
store_vm_hosts_mysql(MYSQL_ROW row, cmdb_s *base);

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
cmdb_run_query_sqlite(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_multiple_query_sqlite(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_insert_sqlite(cmdb_config_s *config, cmdb_s *base, int type);
int
run_search_sqlite(cmdb_config_s *config, cmdb_s *base, int type);
int
cmdb_run_delete_sqlite(cmdb_config_s *config, dbdata_s *data, int type);
int
cmdb_run_update_sqlite(cmdb_config_s *config, dbdata_s *data, int type);
int
cmdb_run_search_sqlite(cmdb_config_s *cmdb, dbdata_s *data, int type);
int
set_cmdb_search_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);
int
set_cmdb_args_sqlite(sqlite3_stmt *state, dbdata_s *list, unsigned int type, int i);
int
get_cmdb_search_res_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);
int
setup_insert_sqlite_bind(sqlite3_stmt *state, cmdb_s *base, int type);
void
store_result_sqlite(sqlite3_stmt *state, cmdb_s *base, int type, unsigned int fields);
void
store_server_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_customer_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_contact_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_service_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_service_type_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_hardware_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_hardware_type_sqlite(sqlite3_stmt *state, cmdb_s *base);
void
store_vm_hosts_sqlite(sqlite3_stmt *state, cmdb_s *base);
int
setup_bind_sqlite_server(sqlite3_stmt *state, cmdb_server_s *server);
int
setup_bind_sqlite_customer(sqlite3_stmt *state, cmdb_customer_s *customer);
int
setup_bind_sqlite_contact(sqlite3_stmt *state, cmdb_contact_s *cont);
int
setup_bind_sqlite_service(sqlite3_stmt *state, cmdb_service_s *service);
int
setup_bind_sqlite_hardware(sqlite3_stmt *state, cmdb_hardware_s *hard);
int
setup_bind_sqlite_vmhost(sqlite3_stmt *state, cmdb_vm_host_s *vmhost);

# endif /* HAVE_SQLITE3 */

#endif /* __CMDB_BASE_SQL_H */

