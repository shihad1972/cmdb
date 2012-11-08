/* mysqlfunc.h:
 *
 * Header file for the mysql functions
 */

#include <mysql.h>
#include "dnsa.h"

#ifndef __MYSQL_FUNC_H
#define __MYSQL_FUNC_H

/* Initialise a MYSQL connection */
void
dnsa_mysql_init(dnsa_config_t *dc, MYSQL *dnsa_mysql);
/* Run a mysql query and initialise the result set */
void
dnsa_mysql_query(MYSQL *dnsa_mysql, const char *query);
#endif