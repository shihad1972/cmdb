/*
 *  ailsacmdb: Ailsatech Configuration Management Database library
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  ailsacmdb.h: Main ailsacmdb library header file
 */

#ifndef __AILSACMDB_H__
# define __AILSACMDB_H__
# include <stdlib.h>
# include <stdint.h>
# include <time.h>
# ifdef HAVE_MYSQL
#  include <mysql.h>
# endif
/** Useful macro to safely avoid double-free memory corruption
 ** Shamelessly stolen from the nagios source. Thanks :) */
# ifndef my_free
#  define my_free(ptr) do { if(ptr) { free(ptr); ptr = NULL; } } while(0)
# endif // my_free

# ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
# endif // HAVE_STDBOOL_H
# ifndef true
#  define true 1
# endif // true
# ifndef false
#  define false 0
# endif // false

# ifndef NI_MAXHOST
#  define NI_MAXHOST 1025
# endif // NI_MAXHOST so I do not have to use __GNU_SOURCE. grrrr 
enum {                  // Buffer lengths
	BYTE_LEN = 8,
	SERVICE_LEN = 16,
	MAC_LEN = 32,
	UUID_LEN = 37,
	HOST_LEN = 64,
	CONFIG_LEN = 256,
	DOMAIN_LEN = 256,
	BUFFER_LEN = 1024,
	FILE_LEN = 4096,
	SQL_TEXT_MAX = 4096
};
/*
** base64 returnable errors
**
** Error codes returned to the operating system.
**
*/
# define B64_SYNTAX_ERROR        1
# define B64_FILE_ERROR          2
# define B64_FILE_IO_ERROR       3
# define B64_ERROR_OUT_CLOSE     4
# define B64_LINE_SIZE_TO_MIN    5
# define B64_SYNTAX_TOOMANYARGS  6

// Temporary
# define BASEDIR "/var/lib/cmdb/data/"
# define AILSAVERSION "0.3"

// Data Definitions

extern const char *regexps[];
enum {			/* regex search codes */
	UUID_REGEX = 0,
	NAME_REGEX,
	ID_REGEX,
	CUSTOMER_REGEX,
	COID_REGEX,
	MAC_REGEX,
	IP_REGEX,
	DOMAIN_REGEX,
	PATH_REGEX,
	LOGVOL_REGEX,
	FS_REGEX,
	ADDRESS_REGEX,
	DEV_REGEX,
	CAPACITY_REGEX,
	OS_VER_REGEX,
	POSTCODE_REGEX,
	URL_REGEX,
	PHONE_REGEX,
	EMAIL_REGEX,
	TXTRR_REGEX,
	CN_REGEX,
	DC_REGEX,
	TIMEZONE_REGEX,
	PACKAGE_FIELD_REGEX,
	RESOURCE_TYPE_REGEX,
	SYSTEM_PACKAGE_ARG_REGEX
};

enum {                  // Action Codes
	NONE = 0,
	CMDB_ADD = 1,
	CMDB_DISPLAY = 2,
	CMDB_LIST = 3,
	CMDB_RM = 4,
	CMDB_MOD = 5,
	CMDB_WRITE = 6,
	CMDB_DEFAULT = 7,
	CMDB_QUERY = 8,
	CMDB_VIEW_DEFAULT = 9,
	CMDB_USAGE = 10,
	DNSA_DISPLAY = 11,
	DNSA_LIST = 12,
	DNSA_COMMIT = 13,
	DNSA_AZONE = 14,
	DNSA_AHOST = 15,
	DNSA_BREV = 16,
	DNSA_DISPLAY_MULTI = 17,
	DNSA_ADD_MULTI = 18,
	DNSA_DZONE = 19,
	DNSA_DREC = 20,
	DNSA_DPREFA = 21,
	DNSA_CNAME = 22,
	DOWNLOAD = 31,
	HELP = 32,
	VERS = 33,
	AILSA_INPUT_INVALID = 34,
	AILSA_ADD = 1,
	AILSA_CMDB_ADD = 50,
	AILSA_HELP = 100,
	AILSA_VERSION = 101,
	AILSA_DISPLAY_USAGE = 102,
};

enum {                  // Error codes
	AILSA_NO_ACTION = 1,
	AILSA_NO_TYPE = 2,
	AILSA_NO_NAME = 3,
	AILSA_NO_LOGVOL = 4,
	AILSA_NO_DIRECTORY = 5,
	AILSA_NO_POOL = 6,
	AILSA_NO_NETWORK = 7,
	AILSA_NO_COID = 8,
	AILSA_NO_USER = 9,
	AILSA_NO_NAME_OR_ID = 10,
	AILSA_NO_NUMBER = 11,
	AILSA_NO_ALIAS = 12,
	AILSA_NO_BUILD_DOMAIN = 13,
	AILSA_NO_ARG = 14,
	AILSA_NO_CONTACT_NAME = 15,
	AILSA_NO_EMAIL_ADDRESS = 16,
	AILSA_NO_PHONE_NUMBER = 17,
	AILSA_NO_VARIENT = 18,
	AILSA_NO_CLASS = 19,
	AILSA_NO_DETAIL = 20,
	AILSA_NO_VHOST_TYPE = 21,
	AILSA_NO_DEVICE = 22,
	AILSA_NO_SERVICE_URL = 23,
	AILSA_NO_COUNTY = 24,
	AILSA_NO_ADDRESS = 25,
	AILSA_NO_CITY = 26,
	AILSA_NO_POSTCODE = 27,
	AILSA_WRONG_TYPE_DISPLAY = 28,
	AILSA_NO_ID_OR_CLASS = 29,
	AILSA_NO_DOMAIN_OR_NAME = 30,
	AILSA_NO_DEVICE_DETAIL = 31,
	AILSA_NO_DOMAIN_NAME = 32,
	AILSA_NO_IP_ADDRESS = 33,
	AILSA_NO_MASTER = 34,
	AILSA_NO_RECORD_TYPE = 35,
	AILSA_NO_GLUE_NS = 36,
	AILSA_NO_PREFIX = 37,
	AILSA_NO_HOST_NAME = 38,
	AILSA_UNKNOWN_ZONE_TYPE = 39,
	AILSA_NO_OPTION = 40,
	AILSA_NO_FILESYSTEM = 41,
	AILSA_NO_URI = 42,
	AILSA_DOMAIN_AND_IP_GIVEN = 51,
	AILSA_WRONG_TYPE = 52,
	AILSA_WRONG_ACTION = 53,
	AILSA_NO_PACKAGE = 54,
	AILSA_NO_MASTER_NAME = 55,
	AILSA_LIST_CLONE_FAILED = 56,
	AILSA_PREFIX_OUT_OF_RANGE = 57,
	AILSA_NO_DATA = 200,
	AILSA_NO_CONNECT = 201,
	AILSA_NO_HOST = 202,
	AILSA_HOST_EXISTS = 203,
	AILSA_NO_TOP_LEVEL_DOMAIN = 204,
	AILSA_NO_PRIORITY = 205,
	AILSA_NO_PROTOCOL = 206,
	AILSA_NO_SERVICE = 207,
	AILSA_RANGE_ERROR = 208,
	AILSA_LIST_NO_DESTROY = 209,
	AILSA_LIST_CANNOT_REMOVE = 210,
	AILSA_NO_PARENT = 211,
	AILSA_CANNOT_GET_PARENT = 212,
	AILSA_TOO_MANY_PARENTS = 213,
	AILSA_NO_MAC_ADDR = 214,
	AILSA_NO_DISK_DEV = 215,
	AILSA_IP_CONVERT_FAILED = 216,
	AILSA_NO_OS = 217,
	AILSA_VARIENT_REPLACE_FAIL = 220,
	AILSA_PARTITION_REPLACE_FAIL = 221,
	AILSA_LOCALE_REPLACE_FAIL = 222,
	AILSA_OS_REPLACE_FAIL = 223,
	AILSA_NO_MIRROR = 224,
	CANNOT_FIND_IDENTITY_ID = 225,
	AILSA_WRONG_LIST_LENGHT = 226,
	AILSA_WRONG_ZONE_TYPE = 227,
	AILSA_BUILD_TYPE_NO_FOUND = 228,
	AILSA_BUILD_OS_IN_USE = 229,
	AILSA_BUFFER_TOO_SMALL = 230,
	AILSA_GETADDR_FAIL = 231,
	AILSA_DNS_LOOKUP_FAIL = 232,
	AILSA_CHKZONE_FAIL = 233,
	AILSA_FILE_ERROR = 233,
	AILSA_CONFIG_ERROR = 234,
	AILSA_DOWNLOAD_FAIL = 234,
	AILSA_NO_MOD_BUILD_DOM_NET = 235,
	AILSA_BUILD_DOMAIN_OVERLAP = 236,
	AILSA_NO_SCHEME = 237,
	AILSA_NO_PARTITION = 238,
	AILSA_NO_VM_HOST = 239,
	AILSA_STRING_FAIL = 240,
	AILSA_XML_DEFINED = 241,
	AILSA_NO_QUERY = 300,
	AILSA_NO_DBTYPE = 301,
	AILSA_INVALID_DBTYPE = 302,
	AILSA_NO_PARAMETERS = 303,
	AILSA_NO_QUERY_NO = 304,
	AILSA_NO_FIELDS = 305,
	AILSA_QUERY_FAIL = 306,
	AILSA_STORE_FAIL = 307,
	AILSA_STATEMENT_FAIL = 308,
	AILSA_SQL_FILE_INIT_FAIL = 309,
	AILSA_SQL_BIND_FAIL = 310,
	AILSA_WRONG_DBTYPE = 311,
	AILSA_MY_CONN_FAIL = 312,
	AILSA_MY_INIT_FAIL = 313,
	UUID_REGEX_ERROR = 400,
	NAME_REGEX_ERROR = 401,
	ID_REGEX_ERROR = 402,
	CUSTOMER_REGEX_ERROR = 403,
	COID_REGEX_ERROR = 404,
	MAC_REGEX_ERROR = 405,
	IP_REGEX_ERROR = 406,
	DOMAIN_REGEX_ERROR = 407,
	FS_REGEX_ERROR = 410,
	RESOURCE_TYPE_REGEX_ERROR = 424,
	RTYPE_INPUT_INVALID = 600,
	ZTYPE_INPUT_INVALID = 601,
	SERVICE_INPUT_INVALID = 602,
	PROTOCOL_INPUT_INVALID = 603,
	DOMAIN_INPUT_INVALID = 604,
	CONFIG_INPUT_INVALID = 605,
	HOST_INPUT_INVALID = 606,
	DEST_INPUT_INVALID = 607,
	MASTER_INPUT_INVALID = 608,
	GLUE_IP_INPUT_INVALID = 609,
	GLUE_NS_INPUT_INVALID = 610,
	UUID_INPUT_INVALID = 611,
	SERVER_NAME_INVALID = 612,
	OS_INVALID = 613,
	OS_VERSION_INVALID = 614,
	PART_SCHEME_INVALID = 615,
	VARIENT_INVALID = 616,
	ARCH_INVALID = 617,
	NET_CARD_INVALID = 618,
	HARD_DISK_INVALID = 619,
	CONFIG_INVALID = 620,
	LOCALE_INVALID = 621,
	PROTOCOL_INVALID = 623,
	VMHOST_INVALID = 624,
	VENDOR_INVALID = 625,
	MAKE_INVALID = 626,
	MODEL_INVALID = 627,
	CUSTOMER_NAME_INVALID = 628,
	ADDRESS_INVALID = 629,
	CITY_INVALID = 630,
	EMAIL_ADDRESS_INVALID = 631,
	DETAIL_INVALID = 632,
	HCLASS_INVALID = 633,
	URL_INVALID = 634,
	DEVICE_INVALID = 635,
	PHONE_NUMBER_INVALID = 636,
	POSTCODE_INVALID = 637,
	COUNTY_INVALID = 638,
	COID_INVALID = 639,
	TYPE_INVALID = 640,
	IP_INVALID = 641,
	NTP_SERVER_INVALID = 642,
	BUILD_DOMAIN_NETWORK_INVALID = 643,
	LANGUAGE_INVALID = 644,
	KEYMAP_INVALID = 645,
	TIMEZONE_INVALID = 646,
	LOCAL_NAME_INVALID = 647,
	COUNTRY_INVALID = 648,
	MIN_INVALID = 649,
	MAX_INVALID = 650,
	PRI_INVALID = 651,
	FILESYSTEM_INVALID = 652,
	LOG_VOL_INVALID = 653,
	FS_PATH_INVALID = 654,
	PART_OPTION_INVALID = 655,
	PACKAGE_FIELD_INVALID = 656,
	PACKAGE_ARG_INVALID = 657,
	PACKAGE_NAME_INVALID = 658,
	PACKAGE_TYPE_INVALID = 659,
	AILSA_SERVER_NOT_FOUND = 700,
	AILSA_CUSTOMER_NOT_FOUND = 701,
	AILSA_ZONE_NOT_FOUND = 702,
	AILSA_BUILD_IP_NOT_FOUND = 703,
	AILSA_VARIENT_NOT_FOUND = 704,
	AILSA_OS_NOT_FOUND = 705,

};

enum {			// zone types; use NONE from action codes
	FORWARD_ZONE = 1,
	REVERSE_ZONE = 2,
	GLUE_ZONE = 3,
	TEST_ZONE = 4
};

enum {			// record types; use NONE from action codes
	A = 1,
	CNAME = 2,
	SRV = 3,
	NS = 4,
	MX = 5,
	TXT = 6
};

// Linked List data types

typedef struct ailsa_element_s {
	struct	ailsa_element_s *prev;
	struct	ailsa_element_s *next;
	void	*data;
} AILELEM;

typedef struct ailsa_list_s {
	size_t 	total;
	int 	(*cmp)(const void *key1, const void *key2);
	AILELEM *(*clone)(AILELEM *e);
	void 	(*destroy)(void *data);
	AILELEM	*head;
	AILELEM	*tail;
} AILLIST;

// Linked list clone types

enum {
	AILSA_BEFORE = 1,
	AILSA_AFTER,
	AILSA_HEAD,
	AILSA_TAIL,
};

// Hash table types

typedef struct ailsa_hash_s {
	unsigned int	buckets;
	unsigned int	(*h)(const void *key);
	int		(*match)(const void *key1, const void *key2);
	void		(*destroy)(void *data);
	unsigned int	size;
	AILLIST		*table;
} AILHASH;

// DB result / insert storage types

enum {			// DB Query Types
	DBSELECT = 0,
	DBINSERT,
	DBUPDATE
};

enum {			// MAC Address generation types
	AILSA_ESX = 1,
	AILSA_KVM = 2
};

enum {			// Storage Pool types
	AILSA_LOGVOL = 1,
	AILSA_DIRECTORY = 2
};

// Various client information data structs

typedef struct CMDBHARD {
	char *name;
	char *type;
	char *detail;
} CMDBHARD;

typedef struct CMDBIFACE {
	char *name;
	char *type;
	char *ip;
	char *nm;
	char *mac;
} CMDBIFACE;

typedef struct CMDBROUTE {
	char *dest;
	char *gw;
	char *nm;
	char *iface;
} CMDBROUTE;

typedef struct CMDBFILE {
	char *name;
	void *data;
} CMDBFILE;

typedef struct CMDBPKG {
	char *name;
	char *version;
} CMDBPKG;

// Struct to hold info on one client

struct client_info {
	char *uuid;
	char *fqdn;
	char *hostname;
	char *os;
	char *distro;
	time_t ctime;
	time_t mtime;
	AILLIST *hard;
	AILLIST *iface;
	AILLIST *route;
	AILLIST *file;
	AILLIST *pkg;
// We could also do with local users and mounted / configured file systems
};

// Structs to hold configuration values

struct cmdbd_config {
	char *dbtype;
	char *db;
	char *file;
	char *user;
	char *pass;
	char *host;
	char *dir;
	char *bind;
	char *dnsa;
	char *rev;
	char *rndc;
	char *chkz;
	char *chkc;
	char *socket;
	char *hostmaster;
	char *prins;
	char *secns;
	char *pridns;
	char *secdns;
	char *toplevelos;
	char *pxe;
	char *tmpdir;
	char *preseed;
	char *tftpdir;
	char *dhcpconf;
	char *kickstart;
	unsigned int port;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	unsigned long int cliflag;

};

// Various data types

typedef struct ailsa_cmdb_s {
	char *dbtype;
	char *db;
	char *file;
	char *user;
	char *pass;
	char *host;
	char *dir;
	char *bind;
	char *dnsa;
	char *rev;
	char *rndc;
	char *chkz;
	char *chkc;
	char *socket;
	char *hostmaster;
	char *prins;
	char *secns;
	char *pridns;
	char *secdns;
	char *toplevelos;
	char *pxe;
	char *tmpdir;
	char *tftpdir;
	char *dhcpconf;
	unsigned int port;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	unsigned long int cliflag;
} ailsa_cmdb_s;

struct cmdbc_config {
	char *host;
	char *service;
};

struct cmdb_client_config {     // Ugly, but it will do for now :(
	char *host;
	char *uuid;
	char *fqdn;
	char *ip;
};

// mkvm data types

typedef struct ailsa_mkvm_s {
	char *name;
	char *pool;
	char *uri;
	char *storxml;
	char *path;
	char *network;
	char *netdev;
	char *vt;		// Volume Type
	char *vtstr;
	char *mac;
	char *logvol;
	char *uuid;
	char *coid;
	char *range;
	char *domain;
	char *interface;
	unsigned long int size;
	unsigned long int ram;
	unsigned long int cpus;
	unsigned long int sptype;
	unsigned long int prefix;
	uint32_t nm;
	uint32_t ip;
	short int action;
	short int cmdb;
} ailsa_mkvm_s;

typedef struct ailsa_string_s {
        char *string;
        size_t len;
        size_t size;
} ailsa_string_s;

typedef union ailsa_data_u {
	char *text;
	unsigned long int number;
	short int small;
	char tiny;
	double point;
#ifdef HAVE_MYSQL
	MYSQL_TIME *time;
#endif
} ailsa_data_u;

typedef struct ailsa_data_s {
	union ailsa_data_u *data;
	unsigned int type;
} ailsa_data_s;

typedef struct ailsa_dhcp_s {
	char *iname;    // Interface Name
	char *dname;    // Domain Name
	char *network;
	char *gateway;
	char *nameserver;
	char *netmask;
	unsigned long int gw, nw, nm, ns, ip;
} ailsa_dhcp_s;

typedef struct ailsa_dhcp_conf_s {
	char *name;
	char *mac;
	char *ip;
	char *domain;
} ailsa_dhcp_conf_s;

typedef struct ailsa_iface_s {
	char *name;
	int flag;
	uint32_t ip, sip, fip, nm, bc, nw;
} ailsa_iface_s;

typedef struct ailsa_tftp_s {
	char *alias;
	char *version;
	char *arch;
	char *build_type;
	char *country;
	char *locale;
	char *keymap;
	char *boot_line;
	char *arg;
	char *url;
	char *net_int;
} ailsa_tftp_s;

typedef struct ailsa_cbcos_s {	// should probably look at ctime here
	char *os;
	char *os_version;
	char *alias;
	char *arch;
	char *ver_alias;
	char *ctime;
	char *mtime;
	unsigned long int cuser;
	unsigned long int muser;
} ailsa_cbcos_s;

typedef struct ailsa_build_s {
	char *locale;
	char *language;
	char *keymap;
	char *country;
	char *net_int;
	char *ip;
	char *ns;
	char *nm;
	char *gw;
	char *ntp;
	char *host;
	char *domain;
	char *mirror;
	char *os;
	char *version;
	char *os_ver;
	char *arch;
	char *url;
	char *fqdn;
	short int do_ntp;
} ailsa_build_s;

typedef struct ailsa_partition_s {
	unsigned long int min;
	unsigned long int max;
	unsigned long int pri;
	char *mount;
	char *fs;
	char *logvol;
} ailsa_partition_s;

typedef struct ailsa_syspack_s {
	char *name;
	char *field;
	char *type;
	char *arg;
	char *newarg;
} ailsa_syspack_s;

typedef struct ailsa_sysscript_s {
	char *name;
	char *arg;
	unsigned long int no;
} ailsa_sysscript_s;

typedef struct ailsa_account_s {
	char *username;
	char *hash;
	char *pass;
	unsigned long int server_id;
	unsigned long int identity_id;
} ailsa_account_s;

typedef struct ailsa_preferred_s {
	char *ip;
	char *fqdn;
	unsigned long int ip_addr;
	unsigned long int prefa_id;
	unsigned long int record_id;
} ailsa_preferred_s;

typedef struct ailsa_record_s {	// Can use for fwd or rev records
	char *host;
	char *dest;
	unsigned long int id;
	unsigned long int zone;
	unsigned long int index;
} ailsa_record_s;

enum {
	CBCSCRIPT = 1,
	CBCSCRARG = 2
};

enum {
	SPACKAGE = 1,
	SPACKARG = 2,
	SPACKCNF = 3
};

typedef struct cbc_syss_s {
	char *name;
	char *arg;
	char *domain;
	char *type;
	short int action;
	short int what;
	unsigned long int no;
} cbc_syss_s;

typedef struct cbc_sysp_s {
	char *arg;
	char *domain;
	char *field;
	char *name;
	char *type;
	short int action;
	short int what;
} cbc_sysp_s;

typedef struct cbcdomain_comm_line_s {
	char *domain;
	char *ntpserver;
	char *config;
	short int action;
	short int confntp;
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int netmask;
	unsigned long int gateway;
	unsigned long int ns;
} cbcdomain_comm_line_s;

typedef struct cbc_dhcp_s { // Info for a dhcp network
        char *iname;    // Interface Name
        char *dname;    // Domain Name
        unsigned long int gw, nw, nm, ns, ip; // 
        struct string_l *dom_search;
        struct cbc_dhcp_s *next;
} cbc_dhcp_s;

typedef struct cbc_iface_s { // Info about interface
        char *name;
        uint32_t ip, sip, fip, nm, bc, nw;
        struct cbc_iface_s *next;
} cbc_iface_s;

// library version info

void
show_ailsacmdb_version();

// library error message function(s)

void
ailsa_show_error(int retval);
const char *
ailsa_strerror(int type);
const char *
ailsa_comm_line_strerror(int error);
void
ailsa_display_validation_error(int error);

// cmdb comand line error function

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
display_cbcpart_usage(void);
void
display_dnsa_usage(void);
void
display_cpc_usage(void);
void
display_ckc_usage(void);
void
display_cbclocale_usage(void);
void
display_cbcsysp_usage(void);
void
display_cbcscript_usage(void);

// AIL_ data functions;

// Linked List
void
ailsa_list_init(AILLIST *list, void (*destory)(void *data));
void
ailsa_list_destroy(AILLIST *list);
void
ailsa_list_clean(AILLIST *list);
int
ailsa_list_ins_next(AILLIST *list, AILELEM *element, void *data);
int
ailsa_list_ins_prev(AILLIST *list, AILELEM *element, void *data);
AILELEM *
ailsa_list_get_element(AILLIST *list, size_t number);
int
ailsa_list_insert(AILLIST *list, void *data);
int
ailsa_list_remove(AILLIST *list, AILELEM *element, void **data);
int
ailsa_list_remove_elements(AILLIST *l, AILELEM *e, size_t len);
int
ailsa_list_pop_element(AILLIST *list, AILELEM *element);
int
ailsa_list_insert_clone(AILLIST *list, AILELEM *copy, AILELEM *ptr, int action, size_t size);
AILELEM *
ailsa_clone_element(AILELEM *e, size_t size);
void
ailsa_clean_element(AILLIST *list, AILELEM *e);
void
ailsa_list_full_clean(AILLIST *l);
AILELEM *
ailsa_move_down_list(AILELEM *element, size_t number);
// Hash Table

unsigned int
ailsa_hash(const void *key);
int
ailsa_hash_init(AILHASH *htbl, unsigned int buckets,
		unsigned int (*h)(const void *key),
		int (*match)(const void *key1, const void *key2),
		void (*destroy)(void *data));
void
ailsa_hash_destroy(AILHASH *htbl);
int
ailsa_hash_insert(AILHASH *htbl, void *data, const char *key);
int
ailsa_hash_remove(AILHASH *htbl, void **data, const char *key);
int
ailsa_hash_lookup(AILHASH *htbl, void **data, const char *key);

// memory functions

void
ailsa_clean_mkvm(void *vm);
void
ailsa_clean_cmdb(void *cmdb);
void
ailsa_init_data(ailsa_data_s *data);
void
ailsa_clean_data(void *data);
void
ailsa_clean_dhcp(void *data);
void
ailsa_clean_cbcos(void *cbcos);
void
ailsa_clean_dhcp_config(void *dhcp);
void
ailsa_clean_tftp(void *tftp);
void
ailsa_clean_build(void *build);
void
ailsa_clean_partition(void *partition);
void
ailsa_clean_syspack(void *sysp);
void
ailsa_clean_sysscript(void *script);
void
ailsa_clean_iface(void *data);
void
clean_cbc_syss_s(cbc_syss_s *scr);
void
ailsa_clean_account(void *acc);
void *
ailsa_calloc(size_t len, const char *msg);
void *
ailsa_realloc(void *r, size_t len, const char *msg);

// ailsa_string_s functions

void
ailsa_init_string(ailsa_string_s *string);
void
ailsa_clean_string(ailsa_string_s *str);
void
ailsa_resize_string(ailsa_string_s *str);
void
ailsa_fill_string(ailsa_string_s *str, const char *s);

// UUID functions
char *
ailsa_gen_uuid_str(void);

// Network functions
int
ailsa_gen_mac(char *mac, int type);
uint32_t
prefix_to_mask_ipv4(unsigned long int prefix);

// Config and command line parsing

void
display_mkvm_usage(void);
void
display_mksp_usage(void);
void
parse_mkvm_config(ailsa_mkvm_s *vm);
void
parse_cmdb_config(ailsa_cmdb_s *cmdb);
int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm);
int
parse_mksp_command_line(int argc, char *argv[], ailsa_mkvm_s *vm);


// Path and various string functions

int
ailsa_add_trailing_dot(char *string);
int
ailsa_add_trailing_slash(char *string);
void
ailsa_chomp(char *line);
void
ailsa_munch(char *line);
void
random_string(char *str, size_t len);

// Logging functions

void
ailsa_start_syslog(const char *prog);
void
ailsa_syslog(int priority, const char *msg, ...);

// Config parse and free functions
void
cmdbd_clean_config(ailsa_cmdb_s *cmdbd);

// File / Directory IO helper functions

int
check_directory_access(const char *dir, int create);
int
ailsa_write_file(const char *name, void *data, size_t len);
int
ailsa_read_file(const char *name, void *data, size_t len);
int
ailsa_append_file(const char *name, void *data, size_t len);

// Input validation
 
int
ailsa_validate_input(char *input, int test);
int
ailsa_validate_string(const char *input, const char *re_test);
const char *
ailsa_regex_error(int error);

// Various data functions

int
ailsa_get_iface_list(AILLIST *list);

void
get_in_addr_string(char *in_addr, char range[], unsigned long int prefix);

int
cbc_fill_partition_details(AILLIST *list, AILLIST *dest);

char *
get_iface_name(const char *name);

// These should probably be moved to ailsasql.h

void
cmdb_add_os_name_or_alias_to_list(char *os, char *alias, AILLIST *list);

void
cmdb_add_os_version_or_alias_to_list(char *ver, char *valias, AILLIST *list);

int
cmdb_add_string_to_list(const char *str, AILLIST *list);

int
cmdb_add_number_to_list(unsigned long int number, AILLIST *list);

int
cmdb_add_short_to_list(short int small, AILLIST *list);

char *
cmdb_get_string_from_data_list(AILLIST *list, size_t n);

// End probably ;)

// Various struct data functions

// struct data init functions

int
ailsa_init_client_info(struct client_info *ci);

AILLIST *
ailsa_db_data_list_init(void);

AILLIST *
ailsa_iface_list_init(void);

AILLIST *
ailsa_dhcp_list_init(void);

AILLIST *
ailsa_cbcos_list_init(void);

AILLIST *
ailsa_dhcp_config_list_init(void);

AILLIST *
ailsa_partition_list_init(void);

AILLIST *
ailsa_syspack_list_init(void);

AILLIST *
ailsa_sysscript_list_init(void);

AILLIST *
ailsa_account_list_init(void);

ailsa_data_s *
ailsa_db_text_data_init(void);

ailsa_data_s *
ailsa_db_lint_data_init(void);

ailsa_data_s *
ailsa_db_sint_data_init(void);

// Struct data clean functions to be used with AILLIST destroy()

void
ailsa_clean_hard(void *hard);
void
ailsa_clean_route(void *route);
void
ailsa_clean_file(void *file);
void
ailsa_clean_pkg(void *pkg);
void
ailsa_clean_mkvm(void *vm);

// client_info clean

void
ailsa_clean_client_info(struct client_info *ci);

// SQL functions

char *
build_sql_query(unsigned int prog, unsigned int no);
char *
build_sql_insert(unsigned int prog, unsigned int no, unsigned int count);
char *
build_sql_update(unsigned int prog, unsigned int no);
char *
build_sql_delete(unsigned int prog, unsigned int no);

// various usage functions

void
display_mkvm_usage(void);
void
display_version(char *prog);

// the rest ...
int
ailsa_add_trailing_dot(char *member);
void
display_version(char *prog);
void
chomp(char *input);
void 
display_type_error(short int type);
int
get_config_file_location(char *config);
char *
cmdb_get_uname(unsigned long int uid);
const char *
sqlite3_errstr(int error);

#endif // __AILSACMDB_H__
