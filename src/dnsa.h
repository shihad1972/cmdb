/* dnsa.h: DNSA header file */

#ifndef __DNSA_H__
#define __DNSA_H__
enum {			/* Buffer Sizes */
	CH_S = 2,
	COMM_S = 8,
	RANGE_S = 16,
	HOST_S = 64,
	CONF_S = 80,
	RBUFF_S = 256,
	TBUFF_S = 512,
	BUFF_S = 1024,
	FILE_S = 4096
};

enum {			/* error codes */
	OK = 0,
	ARGC_INVAL = 1,
	ARGV_INVAL = 2,
	NO_DOMAIN = 3,
	MULTI_DOMAIN = 4,
	NO_DELIM = 5,
	NO_RECORDS = 6,
	WRONG_ACTION = 7,
	WRONG_TYPE = 8,
	DOMAIN_LIST_FAIL = 9,
	MY_INIT_FAIL = 10,
	MY_CONN_FAIL = 11,
	MY_QUERY_FAIL = 12,
	MY_STORE_FAIL = 13,
	FILE_O_FAIL = 20,
	CHKZONE_FAIL = 21,
	MALLOC_FAIL = 30
};

enum {			/* action codes */
	NONE = 0,
	WRITE_ZONE = 1,
	DISPLAY_ZONE = 2,
	CONFIGURE_ZONE = 3,
	LIST_ZONES = 4
};

enum {			/* zone types; use NONE from action codes */
	FORWARD_ZONE = 1,
	REVERSE_ZONE = 2
};

typedef struct comm_line_t { /* Hold parsed command line args */
	short int action;
	short int type;
	char domain[CONF_S];
	char config[CONF_S];
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

/* Get command line args and pass them. Put actions into the struct */
int
parse_command_line(int argc, char **argv, comm_line_t *comm);
/* Grab config values from file */
int
parse_config_file(dnsa_config_t *dc, char *config);
/*initialise configuration struct */
void
init_config_values(dnsa_config_t *dc);
/* Error reporting function */
void 
report_error(int error, const char *errstr);

#endif
