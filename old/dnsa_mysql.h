/* dnsa_mysql.h: Header file for DNSA mysql specific functions */

#include <mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"


/* Initialise a MYSQL connection for dnsa */
void
dnsa_mysql_init(dnsa_config_t *dc, MYSQL *cmdb_mysql);