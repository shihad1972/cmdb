/* cmdb_cbc.h */

#ifndef __CMDB_CBS_H__
#define __CMDB_CBS_H__

enum {			/* Display codes; use NONE from action codes */
	SERVER = 1,
	CUSTOMER = 2,
	CONTACT = 3
};

typedef struct cbc_comm_line_t { /* Hold parsed command line args */
	short int action;
	short int type;
	char config[CONF_S];
	char name[CONF_S];
	char id[CONF_S];
} cbc_comm_line_t;

typedef struct cbc_config_t { /* Hold CMDB configuration values */
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char socket[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
} cbc_config_t;


int
parse_cbc_config_file(cbc_config_t *dc, char *config);

void
init_cbc_config_values(cbc_config_t *dc);

#endif