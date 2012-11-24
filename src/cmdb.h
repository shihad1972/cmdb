/* cmdb.h: DNSA header file */

#ifndef __CMDB_H__
#define __CMDB_H__
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

enum {			/* DNSA error codes */
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

enum {			/* CMDB error codes: start @ 101 to avoid conflict */
	SERVER_NOT_FOUND = 101,
	MULTIPLE_SERVERS = 102,
	CUSTOMER_NOT_FOUND = 103,
	MULTIPLE_CUSTOMERS = 104
};

enum {			/* CMDB Return codes */
	GENERIC_ERROR = -1,
	NO_NAME = -2,
	NO_ID = -3,
	NO_TYPE = -4,
	NO_ACTION = -5,
	NO_NAME_OR_ID = -6,
	DISPLAY_USAGE = -7
};


enum {			/* CMDB Action codes */
	NONE = 0,
	DISPLAY = 1,
	LIST_OBJ = 2
};

enum {			/* DNSA action codes */
	WRITE_ZONE = 1,
	DISPLAY_ZONE = 2,
	CONFIGURE_ZONE = 3,
	LIST_ZONES = 4
};


/* Error reporting function */
void 
report_error(int error, const char *errstr);
/* cmdb comand line error function */
void
display_cmdb_command_line_error(int retval, char *program);
void
display_cmdb_usage(void);

#endif