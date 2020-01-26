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
# include <time.h>
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
	MAC_LEN = 32,
	HOST_LEN = 64,
	CONFIG_LEN = 256,
	DOMAIN_LEN = 256,
	BUFFER_LEN = 1024,
	FILE_LEN = 4096
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
# define AILSAVERSION "0.1"

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
	TIMEZONE_REGEX
};

enum {			// Client commands
	CHECKIN = 1,
	HOST = 2,
	DATA = 3,
	UPDATE = 4,
	CLOSE = 5
};

enum {                  // Action Codes
	AILSA_ADD = 1,
	AILSA_CMDB_ADD = 50,
	AILSA_HELP = 100,
	AILSA_VERSION = 101
};

enum {			// SQL Data types
	AILSA_DB_TEXT = 1,
	AILSA_DB_LINT = 2,
	AILSA_DB_SINT = 3,
	AILSA_DB_TINY = 4
};

enum {                  // Error codes
	AILSA_NO_ACTION = 1,
	AILSA_NO_TYPE = 2,
	AILSA_NO_NAME = 3,
	AILSA_NO_LOGVOL = 4,
	AILSA_NO_DIRECTORY = 5,
	AILSA_NO_POOL = 6,
	AILSA_NO_NETWORK = 7,
	AILSA_NO_DATA = 200,
	AILSA_NO_CONNECT = 201,
	AILSA_NO_QUERY = 300,
	AILSA_NO_DBTYPE = 301,
	AILSA_INVALID_DBTYPE = 302
};

// Linked List data types

typedef struct ailsa_element_s {
	struct	ailsa_element_s *prev;
	struct	ailsa_element_s *next;
	void	*data;
} AILELEM;

typedef struct ailsa_list_s {
	size_t 	total;
	int 	(*cmd)(const void *key1, const void *key2);
	void 	(*destroy)(void *data);
	void 	*head;
	void 	*tail;
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

typedef union ailsa_dbdata_u {
	char *string;
	unsigned long int number;
	short int small;
	char tiny;
} AILDATAU;

typedef struct ailsa_dbdata_s {
	AILDATAU *fields;
	AILDATAU *args;
} AILDATA;

typedef struct ailsa_db_s {
	char *query;
	AILLIST *data;
	char type;
} AILSADB;

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
	unsigned long int size;
	unsigned long int ram;
	unsigned long int cpus;
	unsigned long int sptype;
	short int action;
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
} ailsa_data_u;

typedef struct ailsa_data_s {
	union ailsa_data_u *data;
	unsigned int type;
} ailsa_data_s;

// SQL types

typedef struct ailsa_db_value_s {
	char *name;
	unsigned int type;
} AILDBV;

typedef struct ailsa_simple_select_s {
	AILLIST	*fields;
	AILLIST	*args;
	char *query;
} AILSS;

// library version info

void
show_ailsacmdb_version();

// library error message function(s)

void
ailsa_show_error(int retval);

// AIL_ data functions;

// Linked List
void
ailsa_list_init(AILLIST *list, void (*destory)(void *data));

void
ailsa_list_destroy(AILLIST *list);

int
ailsa_list_ins_next(AILLIST *list, AILELEM *element, void *data);

int
ailsa_list_ins_prev(AILLIST *list, AILELEM *element, void *data);

int
ailsa_list_remove(AILLIST *list, AILELEM *element, void **data);

// Hash Table

unsigned int
ailsa_hash(const void *key);

// memory functions

void
ailsa_clean_mkvm(void *vm);
void
ailsa_clean_cmdb(void *cmdb);
void
ailsa_init_data(ailsa_data_s *data);
void
ailsa_clean_data(void *data);
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

// List and hash functions

void
ailsa_list_init(AILLIST *list, void (*destory)(void *data));
void
ailsa_list_destroy(AILLIST *list);
int
ailsa_list_ins_next(AILLIST *list, AILELEM *element, void *data);
int
ailsa_list_ins_prev(AILLIST *list, AILELEM *element, void *data);
int
ailsa_list_remove(AILLIST *list, AILELEM *element, void **data);
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

// Path and various string functions

int
ailsa_add_trailing_dot(char *string);

int
ailsa_add_trailing_slash(char *string);

void
ailsa_chomp(char *line);

void
ailsa_munch(char *line);

// Memory functions

void *
ailsa_calloc(size_t len, const char *msg);

// Logging functions

void
ailsa_start_syslog(const char *prog);

void
ailsa_syslog(int priority, const char *msg, ...);

// Config parse and free functions

void
cmdbd_parse_config(const char *file, void *data, size_t len);

void
cmdbd_clean_config(ailsa_cmdb_s *cmdbd);

void
cmdbc_clean_config(struct cmdbc_config *cmdbc);

void
cmdbd_print_config(ailsa_cmdb_s *conf);

// Networking Functions

int
ailsa_tcp_socket_bind(const char *node, const char *service);

int
ailsa_tcp_socket(const char *node, const char *service);
 
int
ailsa_accept_tcp_connection(int sock);

int
ailsa_get_fqdn(char *host, char *fqdn, char *ip);

int
ailsa_do_client_send(int s, struct cmdb_client_config *c);

int
ailsa_handle_send_error(int error);

int
ailsa_handle_recv_error(int error);

int
ailsa_send_response(int client, char *buf);

int
ailsa_do_close(int client, char *buf);

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

// Various struct data functions

// struct data init functions

int
ailsa_init_client_info(struct client_info *ci);

// Struct data clean functions to be used with AILLIST destroy()

void
ailsa_clean_hard(void *hard);

void
ailsa_clean_iface(void *iface);

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

// SQL functions.

void
ailsa_clean_dbv(void *dbv);
int
ailsa_init_ss(AILSS *data);
void
ailsa_clean_ss_data(void *data);
void
ailsa_clean_ss(AILSS *data);
int
ailsa_simple_select(ailsa_cmdb_s *config, AILSS *query, AILLIST *results);

// the rest ...
void
ailsa_chomp(char *line);
void
ailsa_munch(char *line);
int
ailsa_add_trailing_slash(char *member);
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
add_trailing_slash(char *member);
int
add_trailing_dot(char *member);
int
write_file(char *filename, char *output);
void
convert_time(char *timestamp, unsigned long int *store);
char *
get_uname(unsigned long int uid);
const char *
sqlite3_errstr(int error);

#endif // __AILSACMDB_H__
