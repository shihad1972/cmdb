/* mysqlfunc.h:
 *
 * Header file for the mysql functions
 */

#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"

#ifndef __MYSQL_FUNC_H
#define __MYSQL_FUNC_H

/* Initialise a MYSQL connection */
void
cmdb_mysql_init(dnsa_config_t *dc, MYSQL *cmdb_mysql);
/* Run a mysql query and initialise the result set */
void
cmdb_mysql_query(MYSQL *cmdb_mysql, const char *query);
#endif