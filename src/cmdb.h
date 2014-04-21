/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  cmdb.h: Main cmdb header file 
 */

#ifndef __CMDB_H__
# define __CMDB_H__
# ifdef HAVE_SQLITE3
#  include <sqlite3.h>
# endif /* HAVE_SQLITE3 */
enum {			/* Buffer Sizes */
	CH_S = 2,
	COMM_S = 8,
	RANGE_S = 16,
	MAC_S = 32,
	HOST_S = 64,
	CONF_S = 128,
	NAME_S = 128,
	URL_S = 256,
	RBUFF_S = 256,
	TBUFF_S = 512,
	BUFF_S = 1024,
	FILE_S = 4096,
	BUILD_S = 65536
};

enum {			/* Database colum types */
	DBTEXT = 1,
	DBINT = 2,
	DBSHORT = 3,
	FLYBY
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
	MY_INSERT_FAIL = 14,
	MY_STATEMENT_FAIL = 15,
	NO_QUERY = 16,
	MY_BIND_FAIL = 17,
	UNKNOWN_QUERY = 18,
	INSERT_NOT_CONFIGURED = 19,
	FILE_O_FAIL = 20,
	DIR_C_FAIL = 21,
	CHKZONE_FAIL = 22,
	NO_ZONE_CONFIGURATION = 23,
	CANNOT_INSERT_ZONE = 24,
	CANNOT_INSERT_RECORD = 25,
	NO_FORWARD_RECORDS = 26,
	CANNOT_ADD_A_RECORD = 27,
	REV_BUILD_FAILED = 28,
	ZONE_ALREADY_EXISTS = 29,
	REFUSE_TO_DELETE_NS_RECORD = 30,
	REFUSE_TO_DELETE_A_RECORD_DEST = 31,
	CANNOT_FIND_RECORD_ID = 32,
	MULTIPLE_RECORDS_FOUND = 33,
	CANNOT_DELETE_RECORD = 34,
	CANNOT_DELETE_ZONE = 35,
	MULTIPLE_ZONE_DELETED = 36,
	NO_MASTER = 37,
	NO_MASTER_NAME = 38,
	NO_GLUE_IP = 39,
	NO_GLUE_NS = 40,
	NO_PARENT_ZONE = 41,
	NO_ZONE_LIST = 42,
	NOT_PRI_OR_SEC_NS = 43,
	NO_GLUE_ZONE = 44,
	NO_PREFIX = 45,
	UNKNOWN_ZONE_TYPE = 46,
	MALLOC_FAIL = 50,
	BUFFER_FULL = 51,
	USER_INPUT_INVALID = 52,
	CANNOT_CONVERT = 53,
	UNKNOWN_STRUCT_DB_TABLE = 54,
	BUFFER_TOO_SMALL = 55,
	CREATE_FILE_FAIL = 60,
	WRONG_PROTO = 61
};

enum {			/* Database Type errors */
	NO_DB_TYPE = 91,
	DB_TYPE_INVALID = 92,
	NO_MYSQL = 93,
	NO_SQLITE = 94
};

enum {			/* cmdb and cbc error codes: start @ 100 to avoid conflict */
	NO_SERVERS = 100,
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
	OS_DOES_NOT_EXIST = 116,
	MULTIPLE_OS = 117,
	MULTIPLE_VARIENTS = 118,
	VARIENT_NOT_FOUND = 119,
	NO_VM_HOSTS = 120,
	NO_CUSTOMERS = 121,
	NO_HARDWARE_TYPES = 122,
	MULTIPLE_HARDWARE_TYPES = 123,
	NO_NETWORK_HARDWARE = 124,
	MULTIPLE_NETWORK_HARDWARE = 125,
	NO_STORAGE_HARDWARE = 126,
	MULTIPLE_STORAGE_HARDWARE = 127,
	SERVER_EXISTS = 128,
	CREATE_BUILD_FAILED = 129,
	NO_LOCALE_FOR_OS = 130,
	NO_DISK_SCHEMES = 131,
	SCHEME_NOT_FOUND = 132,
	MULTIPLE_SCHEMES = 133,
	ID_INVALID = 134,
	NAME_INVALID = 135,
	BUILD_DOMAIN_NOT_FOUND = 136,
	MULTIPLE_BUILD_DOMAINS = 137,
	BOOT_LINE_NOT_FOUND = 138,
	MULTIPLE_BOOT_LINES = 139,
	NO_BUILD_IP = 140,
	BUILD_IP_IN_USE = 141,
	CANNOT_INSERT_IP = 142,
	MULTIPLE_BUILD_IPS = 143,
	CANNOT_FIND_BUILD_IP = 144,
	CANNOT_INSERT_BUILD = 145,
	CANNOT_INSERT_PARTITIONS = 146,
	CANNOT_INSERT_DISK_DEVICE = 147,
	CANNOT_DELETE_BUILD = 148,
	NO_BOOT_PARTITION = 149,
	FIELDS_MISMATCH = 150,
	SQLITE_STATEMENT_FAILED = 151,
	SQLITE_BIND_FAILED = 152,
	SQLITE_INSERT_FAILED = 153,
	DB_INSERT_FAILED = 154,
	DB_UPDATE_FAILED = 155,
	DB_WRONG_TYPE = 156,
	NO_DATA = 157,
	BUILD_DOMAIN_EXISTS = 160,
	BUILD_OS_EXISTS = 161,
	OS_ALIAS_NEEDED = 162,
	BUILD_OS_IN_USE = 163,
	NO_BUILD_PACKAGES = 164,
	NO_PACKAGE = 165,
	NO_VARIENT = 166,
	NO_OS_COMM = 167,
	VARIENT_EXISTS = 168,
	NO_PARTITION_INFO = 169,
	NO_SCHEME_INFO = 170,
	NO_LOG_VOL = 171,
	EXTRA_LOG_VOL = 172,
	LOG_VOL_EXISTS = 173,
	PARTITION_EXISTS = 174,
	TOO_MANY_OS = 175,
	TOO_MANY_VARIENT = 176,
	PACKAGE_EXISTS = 177,
	CANNOT_DELETE_PACKAGE = 178,
	NO_DHCP_B_ERR = 179,
	MULTI_DHCP_B_ERR = 180,
	NO_TFTP_B_ERR = 181,
	MULTI_TFTP_B_ERR = 182,
	NO_NET_BUILD_ERR = 183,
	MULTI_NET_BUILD_ERR = 184,
	NO_BUILD_MIRR_ERR = 185,
	MULTI_BUILD_MIRR_ERR = 186,
	NO_BASIC_DISK = 187,
	MULTI_BASIC_DISK = 188,
	NO_FULL_DISK = 189,
	LOCALE_NOT_FOUND = 190,
	MULTIPLE_LOCALES = 191,
	PARTITIONS_NOT_FOUND = 192,
	BUILD_TYPE_NOT_FOUND = 193,
	BUILD_IN_DATABASE = 194,
	MULTIPLE_PART_NAMES = 195,
	CANNOT_MODIFY_BUILD_DOMAIN = 196,
	LOCALE_NOT_IMPLEMENTED = 197,
	NO_MODIFIERS = 198,
	NO_MOD_BUILD_DOM_NET = 199,
	DID_NOT_MOD_BUILD_DOMAIN = 200,
	MULTI_BUILD_DOM_APP_MOD = 201,
	NO_CONTACT_INFO = 202,
	NO_CONTACT = 203,
	MULTI_CONTACT = 204,
	NO_CONTACT_DATA = 205,
	NO_BUILD_URL = 206,
	NO_BD_CONFIG = 207,
	NO_LOG_CONFIG = 208,
	MULTI_SERVICES = 209,
	NO_SERVICE_DATA = 210,
	NO_SERVICES = 211
};

enum {			/* command line error codes */
	GENERIC_ERROR = -1,
	NO_NAME = 1,
	NO_COID = 2,
	NO_ACTION = 4,
	DISPLAY_USAGE = 8,
	NO_NAME_OR_ID = 16,
	NO_TYPE = 32,
	NO_ID = 64,
	NO_MAKE = 128,
	NO_VENDOR = 256,
	NO_MODEL = 512,
	NO_UUID = 1024,
	NO_ADDRESS = 2048,
	NO_CITY = 4096,
	NO_COUNTY = 8192,
	NO_POSTCODE = 16384,
	NO_PHONE = 32768,
	NO_EMAIL = 65536,
	NO_DETAIL = 131672,
	NO_URL = 262144,
	NO_DEVICE = 524288,
	NO_VHOST = 1048576,
	NO_SERVICE = 2097152,
	NO_CLASS = 4194304,
	NO_SERVICE_URL = 8388608,
	NO_NAME_COID = 16777216,
	NO_DOMAIN_NAME = -8,
	NO_IP_ADDRESS = -9,
	NO_HOST_NAME = -10,
	NO_RECORD_TYPE = -11,
	NO_PARTITION_SCHEMES = -13,
	NO_CONT_NAME = -26,
	DOMAIN_AND_IP_GIVEN = -29,
	NO_OS_SPECIFIED = -30,
	NO_BUILD_DOMAIN = -31,
	NO_BUILD_VARIENT = -32,
	NO_BUILD_PARTITION = -33
};

enum {			/* config file error codes */
	CONF_ERR = 301,
	PORT_ERR = 302,
	DIR_ERR = 303,
	BIND_ERR = 304,
	HOSTM_ERR = 305,
	PRINS_ERR = 306,
	SECNS_ERR = 307,
	TMP_ERR = 308,
	TFTP_ERR = 309,
	PXE_ERR = 310,
	OS_ERR = 311,
	PRESEED_ERR = 312,
	KICKSTART_ERR = 313,
	NO_TMP_ERR = 314,
	NO_TFTP_ERR = 415,
	NO_PXE_ERR = 316,
	NO_OS_ERR = 317,
	NO_PRESEED_ERR = 318,
	NO_KICKSTART_ERR = 319,
	MULTI_TMP_ERR = 320,
	MULTI_TFTP_ERR = 321,
	MULTI_PXE_ERR = 322,
	MULTI_OS_ERR = 323,
	MULTI_PRESEED_ERR = 324,
	MULTI_KICKSTART_ERR = 325,
	NO_ERR = 326,
	MULTI_ERR = 327,
	DHCP_ERR = 328,
	NO_DHCP_ERR = 329,
	MULTI_DHCP_ERR = 330,
	UCONF_ERR = 331,
	UPORT_ERR = 332
};

enum {			/* cmdb Action codes */
	NONE = 0,
	DISPLAY = 1,
	LIST_OBJ = 2,
	ADD_TO_DB = 3,
	RM_FROM_DB = 4,
	CVERSION = 5
};

enum {			/* cbc action codes */
	WRITE_CONFIG = 11,
	DISPLAY_CONFIG = 12,
	ADD_CONFIG = 13,
	CREATE_CONFIG = 14,
	MOD_CONFIG = 15,
	RM_CONFIG = 16,
	LIST_CONFIG = 17,
	QUERY_CONFIG = 18,
	LIST_SERVERS = 19
};

enum {			/* dnsa action codes */
	DISPLAY_ZONE = 21,
	LIST_ZONES = 22,
	COMMIT_ZONES = 23,
	ADD_ZONE = 24,
	ADD_HOST = 25,
	BUILD_REV = 26,
	MULTIPLE_A = 27,
	ADD_PREFER_A = 28,
	DELETE_ZONE = 29,
	DELETE_RECORD = 30,
	DELETE_PREFERRED = 31
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

enum {			/* cbc create build error codes */
	NO_OS = 1,
	NO_OS_VERSION = 2,
	NO_ARCH = 3,
	WRONG_OS_VERSION = 4,
	WRONG_OS_ARCH = 5
};

enum {			/* Codes for server spcifier */
	NAME = 1,
	UUID = 2,
	ID = 3
};

enum {
	FALSE = 0,
	TRUE = 1
};

typedef union dbdata_u {
	char text[256];
	unsigned long int number;
	short int small;
} dbdata_u;

typedef struct dbdata_s {
	union dbdata_u fields;
	union dbdata_u args;
	struct dbdata_s *next;
} dbdata_s;

typedef struct string_len_s {
	char *string;
	size_t len;
	size_t size;
} string_len_s;

typedef struct string_l {
	char *string;
	struct string_l *next;
} string_l;

typedef unsigned long int uli_t;

extern char *optarg;
extern int optind, opterr, optopt;

/* Error reporting function */
void 
report_error(int error, const char *errstr);
void
display_action_error(short int action);
void
display_type_error(short int type);
void
get_error_string(int error, char *errstr);
/* cmdb comand line error function */
void
display_command_line_error(int retval, char *program);
void
display_cmdb_usage(void);
void
display_cbc_usage(void);
void
display_cbcdomain_usage(void);
void
display_cbcos_usage(void);
void
display_cbcvarient_usage(void);
void
display_cbcpack_usage(void);
void
display_cbcpart_usage(void);
void
display_dnsa_usage(void);
/* Database fields query mismatch error functions */
void
cbc_query_mismatch(unsigned int fields, unsigned int required, int query);
void
cmdb_query_mismatch(unsigned int fields, unsigned int required, int query);
void
dnsa_query_mismatch(unsigned int fields, unsigned int required, int query);
/* Miscellaneous  */
void
chomp(char *input);
void
get_config_file_location(char *config);
int
add_trailing_slash(char *member);
int
add_trailing_dot(char *member);
int
write_file(char *filename, char *output);
void
init_dbdata_struct(dbdata_s *data);
void
clean_dbdata_struct(dbdata_s *data);
void
init_string_len(string_len_s *string);
void
clean_string_len(string_len_s *string);
void
init_string_l(string_l *string);
void
clean_string_l(string_l *list);
void
init_initial_string_l(string_l **string, int count);
void
resize_string_buff(string_len_s *build);
# ifdef HAVE_SQLITE3
#  ifndef HAVE_SQLITE3_ERRSTR
const char *
sqlite3_errstr(int error);
#  endif /* HAVE_SQLITE3_ERRSTR */
# endif /* HAVE_SQLITE3 */
#endif
