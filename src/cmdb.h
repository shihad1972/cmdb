/* 
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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

enum {			/* Database Type errors */
	NO_DB_TYPE = 1,
	DB_TYPE_INVALID = 2,
	NO_MYSQL = 3,
	NO_SQLITE = 4
};

enum {			/* Database query codes */
	SERVER_QUERY = 0,
	CUSTOMER_QUERY = 1,
	CONTACT_QUERY = 2,
	SERVICE_QUERY = 3,
	SERVICE_TYPE_QUERY = 4,
	HARDWARE_QUERY = 5,
	HARDWARE_TYPE_QUERY = 6
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
	FILE_O_FAIL = 20,
	DIR_C_FAIL = 21,
	CHKZONE_FAIL = 22,
	NO_ZONE_CONFIGURATION = 23,
	CANNOT_INSERT_ZONE = 24,
	CANNOT_INSERT_RECORD = 25,
	NO_FORWARD_RECORDS = 26,
	CANNOT_ADD_A_RECORD = 27,
	REV_BUILD_FAILED = 28,
	MALLOC_FAIL = 40,
	BUFFER_FULL = 41,
	USER_INPUT_INVALID = 42
};

enum {			/* cmdb error codes: start @ 100 to avoid conflict */
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
	CANNOT_OPEN_FILE = 150,
	SQLITE_STATEMENT_FAILED = 151
	
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

enum {			/* Display codes; use NONE from action codes */
	SERVER = 1,
	CUSTOMER = 2,
	CONTACT = 4,
	SERVICE = 8,
	SERVICE_TYPE = 16,
	HARDWARE = 32,
	HARDWARE_TYPE = 64,
	VM_HOST = 128
};

enum {			/* dnsa action codes */
	WRITE_ZONE = 21,
	DISPLAY_ZONE = 22,
	CONFIGURE_ZONE = 23,
	LIST_ZONES = 24,
	ADD_ZONE = 25,
	ADD_RECORD = 26,
	ADD_HOST = 27,
	BUILD_REV = 28
};

enum {			/* cbc action codes */
	WRITE_CONFIG = 11,
	DISPLAY_CONFIG = 12,
	ADD_CONFIG = 13,
	CREATE_CONFIG = 14
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

enum {
	FALSE = 0,
	TRUE = 1
};

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
