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

extern const char *sql_select[];
extern const char *sql_insert[];
extern const char *sql_search[];
extern const unsigned int select_fields[];
extern const unsigned int insert_fields[];
extern const unsigned int search_fields[];
extern const unsigned int search_args[];

# ifdef HAVE_MYSQL
extern const int mysql_inserts[8][7];
# endif /* HAVE_MYSQL */

enum {			/* SELECT and INSERT indexes */
	SERVERS = 0,
	CUSTOMERS,
	CONTACTS,
	SERVICES,
	SERVICE_TYPES,
	HARDWARES,
	HARDWARE_TYPES,
	VM_HOSTS
};

enum {			/* Search indexes and queries */
	SERVER_ID_ON_NAME = 0,
	CUST_ID_ON_COID,
	SERV_TYPE_ID_ON_SERVICE,
	HARD_TYPE_ID_ON_HCLASS,
	VM_ID_ON_NAME
};

int
run_query(cmdb_config_t *config, cmdb_t *base, int type);
int
run_multiple_query(cmdb_config_t *config, cmdb_t *base, int type);
int
get_query(int type, const char **query, unsigned int *fields);
void
get_search(int type, size_t *fields, size_t *args, void **input, void **ouput, cmdb_t *base);
int
run_search(cmdb_config_t *config, cmdb_t *base, int type);
int
run_insert(cmdb_config_t *config, cmdb_t *base, int type);

# ifdef HAVE_MYSQL
#  include <mysql.h>

void
cmdb_mysql_init(cmdb_config_t *dc, MYSQL *cmdb_mysql);
int
run_query_mysql(cmdb_config_t *config, cmdb_t *base, int type);
int
run_multiple_query_mysql(cmdb_config_t *config, cmdb_t *base, int type);
int
run_insert_mysql(cmdb_config_t *config, cmdb_t *base, int type);
int
run_search_mysql(cmdb_config_t *config, cmdb_t *base, int type);
int
setup_insert_mysql_bind(MYSQL_BIND *bind, unsigned int i, int type, cmdb_t *base);
int
setup_insert_mysql_bind_buffer(int type, void **input, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_server(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_customer(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_contact(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_service(void **buffer, cmdb_t *base, unsigned int i);
void
setup_insert_mysql_bind_buff_hardware(void **buffer, cmdb_t *base, unsigned int i);
void
store_result_mysql(MYSQL_ROW row, cmdb_t *base, int type, unsigned int fields);
void
store_server_mysql(MYSQL_ROW row, cmdb_t *base);
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
store_vm_hosts_mysql(MYSQL_ROW row, cmdb_t *base);

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
run_query_sqlite(cmdb_config_t *config, cmdb_t *base, int type);
int
run_multiple_query_sqlite(cmdb_config_t *config, cmdb_t *base, int type);
int
run_insert_sqlite(cmdb_config_t *config, cmdb_t *base, int type);
int
run_search_sqlite(cmdb_config_t *config, cmdb_t *base, int type);
int
setup_insert_sqlite_bind(sqlite3_stmt *state, cmdb_t *base, int type);
void
store_result_sqlite(sqlite3_stmt *state, cmdb_t *base, int type, unsigned int fields);
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
setup_bind_sqlite_hardware(sqlite3_stmt *state, cmdb_hardware_t *hard);

# endif /* HAVE_SQLITE3 */

#endif /* __CMDB_BASE_SQL_H */

