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
	char id[RANGE_S];
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

/* Get command line args and pass them. Put actions into the struct */
int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_t *comm);
/* Grab config values from file */
int
parse_cmdb_config_file(cmdb_config_t *dc, char *config);
/* Initialisa config struct */
void
init_cmdb_config_values(cmdb_config_t *dc);
/*Display a servers build info */
int
display_server_info (char *name, char *uuid, cmdb_config_t *config);

#endif