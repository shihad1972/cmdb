/* 
 * 
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
 *  mysqlfunc.h:
 *
 *  Header file for the mysql functions for the suite of CMDB programs
 *  cmdb, dnsa and cbc
 */

#include <mysql.h>
#include "cmdb.h"

#ifndef __MYSQL_FUNC_H
#define __MYSQL_FUNC_H


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

#endif