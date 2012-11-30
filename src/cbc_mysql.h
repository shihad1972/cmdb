/* cmdb_mysql.h: Header file for cmdb mysql functions */

#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "mysqlfunc.h"


void
cbc_mysql_init(cbc_config_t *dc, MYSQL *cbc_mysql);