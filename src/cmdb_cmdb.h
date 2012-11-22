/* cmdb_cmdb.h */

#ifndef __CMDB_CMDB_H
#define __CMDB_CMDB_H

enum {			/* Action codes */
	NONE = 0,
	DISPLAY = 1,
	LIST_OBJ = 2
};

enum {			/* Display codes; use NONE from action codes */
	SERVER = 1,
	CUSTOMER = 2,
	CONTACT = 3
};

enum {			/* Return codes */
	GENERIC_ERROR = -1,
	NO_NAME = -2,
	NO_ID = -3,
	NO_TYPE = -4,
	NO_ACTION = -5,
	NO_NAME_OR_ID = -6
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
parse_command_line(int argc, char **argv, cmdb_comm_line_t *comm);

#endif