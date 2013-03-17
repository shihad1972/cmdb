/* 
 *
 *  cbc: Create Build Config
 *  Copyright (C) 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc_bdom_sql.h:
 *
 *  Contains function  and data definitions for database access for 
 *  build domain (cbcdomain).
 */

#ifndef __CBC_BDOM_SQL_H__
# define __CBC_BDOM_SQL_H__
# include "../config.h"
# include "cmdb_cbc.h"
# include "base_sql.h"

extern const char *cbcdom_sql_update[];
extern const char *cbcdom_sql_delete[];
extern const char *cbcdom_sql_search[];

extern const unsigned int cbcdom_update_args[];
extern const unsigned int cbcdom_delete_args[];
extern const unsigned int cbcdom_search_args[];
extern const unsigned int cbcdom_search_fields[];

extern const unsigned int cbcdom_update_types[][2];
extern const unsigned int cbcdom_delete_types[][2];
extern const unsigned int cbcdom_search_arg_types[][2];
extern const unsigned int cbcdom_search_field_types[][5];

enum {			/* Build domain delete SQL statements */
	BDOM_DEL_DOMAIN = 0,
	BDOM_DEL_DOM_ID = 1
};

enum {			/* Build domain search SQL statements */
	LDAP_CONFIG_ON_DOM = 0,
	LDAP_CONFIG_ON_ID = 1,
	BUILD_DOMAIN_COUNT = 2
};

int
cbcdom_run_search(cbc_config_s *ccs, dbdata_s *base, int type);

int
cbcdom_run_delete(cbc_config_s *ccs, dbdata_s *base, int type);


# ifdef HAVE_MYSQL
#  include <mysql.h>

int
cbcdom_run_delete_mysql(cbc_config_s *ccs, dbdata_s *base, int type);

int
cbcdom_run_search_mysql(cbc_config_s *ccs, dbdata_s *base, int type);

int
set_dom_search_args_mysql(MYSQL_BIND *mybind, unsigned int i, int type, dbdata_s *base);

int
set_dom_search_fields_mysql(MYSQL_BIND *mybind, unsigned int i, int k, int type, dbdata_s *base);

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

int
cbcdom_run_delete_sqlite(cbc_config_s *ccs, dbdata_s *base, int type);

int
cbcdom_run_search_sqlite(cbc_config_s *ccs, dbdata_s *base, int type);

int
set_cbcdom_search_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);

int
get_cbcdom_search_res_sqlite(sqlite3_stmt *state, dbdata_s *list, int type, int i);

# endif /* HAVE_SQLITE3 */

#endif /* __CBC_BDOM_SQL_H__ */
