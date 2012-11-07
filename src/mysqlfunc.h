/* mysqlfunc.h:
 *
 * Header file for the mysql functions
 */

#include <mysql.h>
#include "dnsa.h"

#ifndef __MYSQL_FUNC_H
#define __MYSQL_FUNC_H

void
dnsa_mysql_init(dnsa_config_t *dc, MYSQL *dnsa_mysql);
#endif