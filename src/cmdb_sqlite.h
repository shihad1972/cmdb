/* cmdb_sqlite.h: Header file for cmdb sqlite functions */

#include <sqlite3.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"

void
display_all_sqlite_servers(cmdb_config_t *config);
int
insert_server_into_sqlite(cmdb_config_t *config, cmdb_server_t *server);
int
insert_hardware_into_sqlite(cmdb_config_t *config, cmdb_hardware_t *hardware);
int
display_on_name_sqlite(char *name, cmdb_config_t *config);
int
display_on_uuid_sqlite(char *uuid, cmdb_config_t *config);