/* cmdb.h: DNSA header file */

#ifndef __CMDB_H__
#define __CMDB_H__
enum {			/* Buffer Sizes */
	CH_S = 2,
	COMM_S = 8,
	RANGE_S = 16,
	MAC_S = 32,
	HOST_S = 64,
	CONF_S = 80,
	NAME_S = 128,
	URL_S = 136,
	RBUFF_S = 256,
	TBUFF_S = 512,
	BUFF_S = 1024,
	FILE_S = 4096,
	BUILD_S = 65536
};

enum {			/* dnsa error codes */
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
	NO_ZONE_CONFIGURATION = 22,
	CANNOT_INSERT_ZONE = 23,
	CANNOT_INSERT_RECORD = 24,
	MALLOC_FAIL = 30,
	BUFFER_FULL = 31
};

enum {			/* cmdb error codes: start @ 101 to avoid conflict */
	SERVER_NOT_FOUND = 101,
	MULTIPLE_SERVERS = 102,
	SERVER_ID_NOT_FOUND = 103,
	MULTIPLE_SERVER_IDS = 104,
	SERVER_UUID_NOT_FOUND = 105,
	MULTIPLE_SERVER_UUIDS = 106,
	CUSTOMER_NOT_FOUND = 107,
	MULTIPLE_CUSTOMERS = 108,
	NO_NAME_UUID_ID = 109,
	SERVER_BUILD_NOT_FOUND = 110,
	MULTIPLE_SERVER_BUILDS = 111,
	SERVER_PART_NOT_FOUND = 112,
	SERVER_PACKAGES_NOT_FOUND = 113,
	OS_NOT_FOUND = 114,
	OS_VERSION_NOT_FOUND = 115,
	BUILD_DOMAIN_NOT_FOUND = 116,
	VARIENT_NOT_FOUND = 117,
	NO_VM_HOSTS = 118,
	NO_CUSTOMERS = 119
};

enum {			/* cmdb return codes */
	GENERIC_ERROR = -1,
	NO_NAME = -2,
	NO_ID = -3,
	NO_TYPE = -4,
	NO_ACTION = -5,
	NO_NAME_OR_ID = -6,
	DISPLAY_USAGE = -7,
	NO_DOMAIN_NAME = -8,
	NO_IP_ADDRESS = -9,
	NO_HOST_NAME = -10,
	NO_RECORD_TYPE = -11,
	NO_PREFIX = -12,
	NO_PARTITION_SCHEMES = -13,
	NO_UUID = -14
};

enum {			/* cmdb config file error codes */
	CONF_ERR = 1,
	PORT_ERR = 2,
};

enum {			/* dnsa config file error codes */
	DIR_ERR = 3,
	BIND_ERR = 4
};

enum {			/* cbc config file error codes */
	TMP_ERR = 3,
	TFTP_ERR = 4,
	PXE_ERR = 5,
	OS_ERR = 6,
	PRESEED_ERR = 7,
	KICKSTART_ERR = 8,
	NO_TMP_ERR = 9,
	NO_TFTP_ERR = 10,
	NO_PXE_ERR = 11,
	NO_OS_ERR = 12,
	NO_PRESEED_ERR = 13,
	NO_KICKSTART_ERR = 14,
	MULTI_TMP_ERR = 15,
	MULTI_TFTP_ERR = 16,
	MULTI_PXE_ERR = 17,
	MULTI_OS_ERR = 18,
	MULTI_PRESEED_ERR = 19,
	MULTI_KICKSTART_ERR = 20,
	NO_ERR = 21,
	MULTI_ERR = 22,
	DHCP_ERR = 23,
	NO_DHCP_ERR = 24,
	MULTI_DHCP_ERR = 25
};

enum {			/* cmdb Action codes */
	NONE = 0,
	DISPLAY = 1,
	LIST_OBJ = 2,
	ADD_TO_DB = 3
};
enum {			/* dnsa action codes */
	WRITE_ZONE = 1,
	DISPLAY_ZONE = 2,
	CONFIGURE_ZONE = 3,
	LIST_ZONES = 4,
	ADD_ZONE = 5,
	ADD_RECORD = 6,
	ADD_HOST = 7
};

enum {			/* cbc action codes */
	WRITE_CONFIG = 1,
	DISPLAY_CONFIG = 2,
	ADD_CONFIG = 3,
	CREATE_CONFIG = 4
};

enum {			/* cbc values for build type */
	PRESEED = 1,
	KICKSTART = 2
};

enum {			/* cbc values for special partitions */
	ROOT = 1,
	BOOT = 2,
	SWAP = 3
};

enum {
	FALSE = 0,
	TRUE = 1
};

extern char *optarg;
extern int optind, opterr, optopt;

/* Error reporting function */
void 
report_error(int error, const char *errstr);
/* cmdb comand line error function */
void
display_cmdb_command_line_error(int retval, char *program);
void
display_cmdb_usage(void);
void
display_cbc_usage(void);
void
display_dnsa_usage(void);

void
chomp(char *input);

#endif
