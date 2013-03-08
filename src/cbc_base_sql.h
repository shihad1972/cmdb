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

extern const char *sql_select[];
extern const char *sql_insert[];
extern const char *sql_search[];
extern const char *sql_update[];
extern const char *sql_delete[];
extern const unsigned int select_fields[];
extern const unsigned int insert_fields[];
extern const unsigned int search_args[];
extern const unsigned int search_fields[];
extern const unsigned int update_args[];
extern const unsigned int update_fields[];
extern const unsigned int delete_args[];
extern const unsigned int delete_fields[];

# ifdef HAVE_MYSQL
extern const int mysql_inserts[][25];
# endif /* HAVE_MYSQL */

enum {			/* SELECT statements */
	BOOT_LINE = 1,
	BUILD = 2,
	BUILD_DOMAIN = 4,
	BUILD_IP = 8,
	BUILD_OS = 16,
	BUILD_TYPE = 32,
	DISK_DEV = 64,
	LOCALE = 128,
	BPACKAGE = 256,
	DPART = 512,
	SPART = 1024,
	SSCHEME = 2048,
	CSERVER = 4096,
	VARIENT = 8192,
	VMHOST = 16384
};

enum {			/* SELECT and INSERT Indices */
	BOOT_LINES = 0,
	BUILDS,
	BUILD_DOMAINS,
	BUILD_IPS,
	BUILD_OSS,
	BUILD_TYPES,
	DISK_DEVS,
	LOCALES,
	BPACKAGES,
	DPARTS,
	SPARTS,
	SSCHEMES,
	CSERVERS,
	VARIENTS,
	VMHOSTS
};

# ifdef HAVE_MYSQL
#  include <mysql.h>
void
cmdb_mysql_init(cbc_config_t *dc, MYSQL *cmdb_mysql);

# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

# endif /* HAVE_SQLITE3 */
#endif /* __CBC_BASE_SQL_H */