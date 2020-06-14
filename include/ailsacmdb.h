/*
 *  ailsacmdb: Ailsatech Configuration Management Database library
 *  Copyright (C) 2012 - 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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

enum {			// Client commands
	CHECKIN = 1,
	HOST = 2,
	DATA = 3,
	UPDATE = 4,
	CLOSE = 5
};

enum {                  // Action Codes
	CMDB_ADD = 1,
	CMDB_DISPLAY = 2,
	CMDB_LIST = 3,
	CMDB_RM = 4,
	CMDB_MOD = 5,
	AILSA_ADD = 1,
	AILSA_CMDB_ADD = 50,
	AILSA_HELP = 100,
	AILSA_VERSION = 101
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
	AILSA_NO_DATA = 200,
	AILSA_NO_CONNECT = 201,
	AILSA_NO_QUERY = 300,
	AILSA_NO_DBTYPE = 301,
	AILSA_INVALID_DBTYPE = 302,
	AILSA_NO_PARAMETERS = 303,
	AILSA_NO_QUERY_NO = 304,
	AILSA_NO_FIELDS = 305,
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
	GLUE_NS_INPUT_INVALID = 610
};

enum {			// zone types; use NONE from action codes
	FORWARD_ZONE = 1,
	REVERSE_ZONE = 2,
	GLUE_ZONE = 3
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
	void 	(*destroy)(void *data);
	AILELEM	*head;
	AILELEM	*tail;
} AILLIST;

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
	unsigned long int size;
	unsigned long int ram;
	unsigned long int cpus;
	unsigned long int sptype;
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

typedef struct ailsa_iface_s {
	char *name;
	int flag;
	uint32_t ip, sip, fip, nm, bc, nw;
} ailsa_iface_s;

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
ailsa_clean_iface(void *data);
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
get_error_string(int error, char *errstr);
void
cbc_query_mismatch(unsigned int fields, unsigned int required, int query);
void
cmdb_query_mismatch(unsigned int fields, unsigned int required, int query);
void
dnsa_query_mismatch(unsigned int fields, unsigned int required, int query);
void
chomp(char *input);
void
display_action_error(short int action);
void 
display_type_error(short int type);
void
get_config_file_location(char *config);
int
write_file(char *filename, char *output);
void
convert_time(char *timestamp, unsigned long int *store);
char *
get_uname(unsigned long int uid);
const char *
sqlite3_errstr(int error);

#endif // __AILSACMDB_H__
