/* cmdb_mysql.h: Header file for cmdb mysql functions */

#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "mysqlfunc.h"


void
cmdb_mysql_init(cmdb_config_t *dc, MYSQL *cmdb_mysql);