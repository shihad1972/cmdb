/* cmdb_cmdb.h */

#ifndef __CMDB_CMDB_H__
#define __CMDB_CMDB_H__

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

typedef struct cmdb_vm_server_hosts_t {
	char name[CONF_S];
	char type[CONF_S];
	unsigned long int id;
	struct cmdb_vm_server_hosts_t *next;
} cmdb_vm_server_hosts_t;

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

#endif