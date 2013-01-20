/* mysqlfunc.h:
 *
 * Header file for the mysql functions
 */

#include <mysql/mysql.h>
#include "cmdb.h"

#ifndef __MYSQL_FUNC_H
#define __MYSQL_FUNC_H

/* Run a mysql query and initialise the result set */

void
cmdb_mysql_query(MYSQL *cmdb_mysql, const char *query);

int
cmdb_mysql_query_with_checks(MYSQL *mycmdb, const char *query);

void
cmdb_mysql_clean(MYSQL *cmdb_mysql, char *query);

void
cmdb_mysql_clean_full(MYSQL_RES *cmdb_res, MYSQL *cmdb_mysql, char *query);
#endif