/* cmdb_mysql.h: Header file for cmdb mysql functions */

#include <mysql.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "mysqlfunc.h"


void
cmdb_mysql_init(cmdb_config_t *dc, MYSQL *cmdb_mysql);
void
fill_vm_host_node(cmdb_vm_host_t *head, MYSQL_ROW myrow);
cmdb_customer_t 
*add_customer_node(cmdb_customer_t *head, MYSQL_ROW myrow);
void
fill_hardware_types(cmdb_hard_type_t *head, MYSQL_ROW row);
int
display_on_uuid_mysql(char *coid, cmdb_config_t *config);
int
display_on_name_mysql(char *name, cmdb_config_t *config);
int
insert_server_into_mysql(cmdb_config_t *config, cmdb_server_t *server);
void
display_all_mysql_servers(cmdb_config_t *config);
int
insert_hardware_into_mysql(cmdb_config_t *config, cmdb_hardware_t *hardware);
cmdb_vm_host_t
*vm_host_create(void);