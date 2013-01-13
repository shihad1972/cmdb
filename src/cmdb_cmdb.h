/* cmdb_cmdb.h */

#ifndef __CMDB_CMDB_H__
#define __CMDB_CMDB_H__
#include "mysql/mysql.h"

enum {			/* Display codes; use NONE from action codes */
	SERVER = 1,
	CUSTOMER = 2,
	CONTACT = 3
};

typedef struct cmdb_comm_line_t { /* Hold parsed command line args */
	short int action;
	short int type;
	char config[CONF_S];
	char name[CONF_S];
	char id[CONF_S];
} cmdb_comm_line_t;

typedef struct cmdb_config_t { /* Hold CMDB configuration values */
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char socket[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
} cmdb_config_t;

typedef struct cmdb_vm_host_t {
	char name[CONF_S];
	char type[CONF_S];
	unsigned long int id;
	struct cmdb_vm_host_t *next;
} cmdb_vm_host_t;

typedef struct cmdb_server_t {
	char vendor[CONF_S];
	char make[CONF_S];
	char model[CONF_S];
	char uuid[CONF_S];
	char name[MAC_S];
	unsigned long int cust_id;
	unsigned long int server_id;
	unsigned long int vm_server_id;
	struct cmdb_server_t *next;
} cmdb_server_t;

typedef struct cmdb_customer_t {
	char name[HOST_S];
	char address[NAME_S];
	char city[HOST_S];
	char county[MAC_S];
	char postcode[RANGE_S];
	char coid[RANGE_S];
	unsigned long int cust_id;
	struct cmdb_customer_t *next;
} cmdb_customer_t;

typedef struct cmdb_hard_type_t {
	char type[HOST_S];
	char hclass[HOST_S];
	unsigned long int ht_id;
	struct cmdb_hard_type_t *next;
} cmdb_hard_type_t;

typedef struct cmdb_hardware_t {
	char detail[HOST_S];
	char device[MAC_S];
	unsigned long int hard_id;
	unsigned long int server_id;
	unsigned long int ht_id;
	struct cmdb_hardware_t *next;
} cmdb_hardware_t;

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_t *comm);
int
parse_cmdb_config_file(cmdb_config_t *dc, char *config);
void
init_cmdb_config_values(cmdb_config_t *dc);
int
display_server_info (char *name, char *uuid, cmdb_config_t *config);
void
display_all_servers(cmdb_config_t *config);
int
display_server_info_on_uuid(char *coid, cmdb_config_t *config);
int
display_server_info_on_name(char *name, cmdb_config_t *config);
int
display_customer_info(char *server, char *uuid, cmdb_config_t *config);
int
display_customer_info_on_coid(char *coid, cmdb_config_t *config);
int
display_customer_info_on_name(char *name, cmdb_config_t *config);
void
display_all_customers(cmdb_config_t *config);
/* Routines to display server data from database. */
void
display_server_from_name(char **server_info);
void
display_server_from_uuid(char **server_info);
/* Routines to display customer data from database. */
void
display_customer_from_name(char **cust_info);
void
display_customer_from_coid(char **cust_info);
int
add_server_to_database(cmdb_config_t *config);
void
get_full_server_config(cmdb_server_t *server);
void
print_server_details(cmdb_server_t *server);
void
print_hardware_details(cmdb_hardware_t *hard);
/* Linked list functions for virtual machine hosts */
cmdb_vm_host_t
*vm_host_create(void);
cmdb_vm_host_t
*get_vm_host(cmdb_config_t *config);
void
fill_vm_host_node(cmdb_vm_host_t *head, MYSQL_ROW myrow);
/* Linked list fucntions for customers */
cmdb_customer_t
*create_customer_node(void);
unsigned long int
get_customer_for_server(cmdb_config_t *config);
cmdb_customer_t 
*add_customer_node(cmdb_customer_t *head, MYSQL_ROW myrow);
/* linked list functions for hardware and hardware types */
cmdb_hard_type_t
*hard_type_node_create(void);
cmdb_hardware_t
*hard_node_create(void);
void
add_hardware_types(cmdb_config_t *config, cmdb_hard_type_t *hthead);
void
fill_hardware_types(cmdb_hard_type_t *head, MYSQL_ROW row);
cmdb_hard_type_t
*get_network_device_id(cmdb_hard_type_t *head);
cmdb_hard_type_t
*get_disk_device_id(cmdb_hard_type_t *head);
int
get_network_device(cmdb_hardware_t *head);
int
get_disk_device(cmdb_hardware_t *head);
/* Server functions */
int
get_server_hardware(cmdb_config_t *config, cmdb_hardware_t *head, unsigned long int id);
#endif