/* cmdb_dnsa.h: DNSA header file */

#ifndef __CMDB_DNSA_H__
#define __CMDB_DNSA_H__

enum {			/* zone types; use NONE from action codes */
	FORWARD_ZONE = 1,
	REVERSE_ZONE = 2
};

typedef struct comm_line_t { /* Hold parsed command line args */
	short int action;
	short int type;
	unsigned long int prefix;
	char domain[CONF_S];
	char config[CONF_S];
	char host[RBUFF_S];
	char dest[RBUFF_S];
	char rtype[RANGE_S];
} comm_line_t;

typedef struct dnsa_config_t { /* Hold DNSA configuration values */
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char dir[CONF_S];
	char bind[CONF_S];
	char dnsa[CONF_S];
	char rev[CONF_S];
	char rndc[CONF_S];
	char chkz[CONF_S];
	char chkc[CONF_S];
	char socket[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
} dnsa_config_t;

typedef struct record_row_t { /* Hold dns record */
	int id;
	int pri;
	int zone;
	char dest[RBUFF_S];
	char host[RBUFF_S];
	char type[RBUFF_S];
	char valid[RBUFF_S];
	struct record_row_t *next;
} record_row_t;

/* Get command line args and pass them. Put actions into the struct */
int
parse_dnsa_command_line(int argc, char **argv, comm_line_t *comm);
/* Grab config values from file */
int
parse_dnsa_config_file(dnsa_config_t *dc, char *config);
/*initialise configuration struct */
void
init_config_values(dnsa_config_t *dc);
void
parse_dnsa_config_error(int error);
/* Validate command line input */
int
validate_comm_line(comm_line_t *comm);

#endif
