/*
 *  cmdb suite of programs
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
 *  base_sql.h
 *
 *  This file is the header file for the SQL queries for the cmdb suite of
 *  programs
 */
#ifndef __BASE_SQL_H
# define __BASE_SQL_H
# include <config.h>
# include "cmdb.h"
/* cbc queries */

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
	SSCHEME = 1024,
	CSERVER = 2048,
	VARIENT = 4096,
	VMHOST = 8192,
	SYSPACK = 16384,
	SYSARG = 32768,
	SYSCONF = 65536,
	SCRIPT = 131072,
	SCRIPTA = 262144,
	PARTOPT = 524288
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
	SSCHEMES,
	CSERVERS,
	VARIENTS,
	VMHOSTS,
	SYSPACKS,
	SYSARGS,
	SYSCONFS,
	SCRIPTS,
	SCRIPTAS,
	PARTOPTS
};

/* cmdb queries */


enum {			/* Select statements */
	SERVER = 1,
	CUSTOMER = 2,
	CONTACT = 4,
	SERVICE = 8,
	SERVICE_TYPE = 16,
	HARDWARE = 32,
	HARDWARE_TYPE = 64,
	VM_HOST = 128
};

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
	VM_ID_ON_NAME,
	HCLASS_ON_HARD_TYPE_ID,
	CUST_ID_ON_NAME,
	CONTACT_ID_ON_COID_NAME,
	SERVICE_ID_ON_URL,
	SERVICE_ID_ON_SERVICE,
	SERVICE_ID_ON_URL_SERVICE,
	SERVICE_ID_ON_SERVER_ID,
	SERVICE_ID_ON_CUST_ID,
	SERVICE_ID_ON_SERVER_ID_SERVICE,
	SERVICE_ID_ON_CUST_ID_SERVICE
};

/* dnsa queries */


enum {			/* SELECT statements to use in multiple */
	ZONE = 1,
	REV_ZONE = 2,
	RECORD = 4,
	REV_RECORD = 8,
	ALL_A_RECORD = 16,
	DUPLICATE_A_RECORD = 32,
	PREFERRED_A = 64,
	RECORDS_ON_CNAME_TYPE = 128,
	GLUE = 256
};

enum {			/* SELECT and INSERT indexes */
	ZONES = 0,
	REV_ZONES,
	RECORDS,
	REV_RECORDS,
	ALL_A_RECORDS,
	DUPLICATE_A_RECORDS,
	PREFERRED_AS,
	RECORDS_ON_CNAME_TYPES,
	GLUES
};

enum {			/* Delete indexes that diverge from SELECT */
	REV_RECORDS_ON_REV_ZONE = 7,
	RECORDS_ON_FWD_ZONE = 9
};

enum {			/* Extended searches */
	RECORDS_ON_DEST_AND_ID = 0,
	RECORDS_ON_ZONE,
	DEST_IN_RANGE,
	RECORD_ID_ON_IP_DEST_DOM,
	FWD_ZONE_ID_ON_NAME,
	BUILD_DOM_ON_SERVER_ID
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
	REV_ZONE_VALID_YES,
	REV_ZONE_SERIAL,
	ZONE_VALID_NO
};

# ifdef HAVE_MYSQL
#  include <mysql.h>

void
cmdb_mysql_query(MYSQL *cmdb_mysql, const char *query);

int
cmdb_mysql_query_with_checks(MYSQL *mycmdb, const char *query);

void
cmdb_mysql_clean(MYSQL *cmdb_mysql, char *query);

void
cmdb_mysql_clean_full(MYSQL_RES *cmdb_res, MYSQL *cmdb_mysql, char *query);

void
cmdb_mysql_cleanup(MYSQL *cmdb);

void
cmdb_mysql_cleanup_full(MYSQL *cmdb, MYSQL_RES *res);

int
cmdb_run_mysql_stmt(MYSQL *cmdb, MYSQL_BIND *my_bind, const char *query);

void
cmdb_set_bind_mysql(MYSQL_BIND *mybind, unsigned int i, dbdata_u *data);
# endif /* HAVE_MYSQL */

# ifdef HAVE_SQLITE3
#  include <sqlite3.h>

void
cmdb_setup_ro_sqlite(const char *query, const char *file , sqlite3 **cmdb, sqlite3_stmt **stmt);

void
cmdb_setup_rw_sqlite(const char *query, const char *file , sqlite3 **cmdb, sqlite3_stmt **stmt);

void
cmdb_sqlite_cleanup(sqlite3 *cmdb, sqlite3_stmt *stmt);

# endif /* HAVE_SQLITE3 */
#endif /* __BASE_SQL_H */

