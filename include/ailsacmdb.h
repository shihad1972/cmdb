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

enum {                  // Buffer lengths
	MAC_LEN = 32,
	HOST_LEN = 64,
	CONFIG_LEN = 256,
	DOMAIN_LEN = 256,
	BUFFER_LEN = 1024,
	FILE_LEN = 4096
};

/** Useful macro to safely avoid double-free memory corruption
 ** Shamelessly stolen from the nagios source. Thanks :) */
# ifndef my_free
#  define my_free(ptr) do { if(ptr) { free(ptr); ptr = NULL; } } while(0)
# endif // my_free

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
	AILSA_NO_DATA = 200,
	AILSA_NO_CONNECT = 201
};

enum {			// MAC Address generation types
	AILSA_ESX = 1,
	AILSA_KVM = 2
};

enum {			// Storage Pool types
	AILSA_LOGVOL = 1,
	AILSA_DIRECTORY = 2
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
	char *tmpdir;
	char *tftpdir;
	char *pxe;
	char *toplevelos;
	char *dhcpconf;
	char *kickstart;
	char *preseed;
	unsigned int port;
	unsigned long int refresh;
	unsigned long int retry;
	unsigned long int expire;
	unsigned long int ttl;
	unsigned long int cliflag;
} ailsa_cmdb_s;

typedef struct ailsa_mkvm_s {
	char *name;
	char *pool;
	char *uri;
	char *storxml;
	char *path;
	char *network;
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

// SQL types

typedef struct ailsa_db_value_s {
	char *name;
	unsigned int type;
} AILDBV;

typedef struct ailsa_simple_select_s {
	AILLIST	*fields;
	char *table;
	char *arg;
	char *value;
} AILSS;

// library version info

void
show_ailsacmdb_version();

// library error message function(s)

void
ailsa_show_error(int retval);

// memory functions

void
ailsa_clean_mkvm(void *vm);
void
ailsa_clean_cmdb(void *cmdb);
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

// SQL functions.
void
ailsa_clean_dbv(void *dbv);
int
ailsa_init_ss(AILSS *data);
void
ailsa_clean_ss_data(void *data);
void
ailsa_clean_ss(AILSS *data);
char *
ailsa_build_simple_sql_query(AILSS *query);

// the rest ...
void
ailsa_chomp(char *line);
void
ailsa_munch(char *line);
int
ailsa_add_trailing_slash(char *member);
int
ailsa_add_trailing_dot(char *member);
/*
void
report_error(int error, const char *errstr);
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
display_cbcsysp_usage(void);
void
display_cbcscript_usage(void);
void
display_cbclocale_usage(void);
void
display_mkvm_usage(void); */
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
/* int
get_ip_from_hostname(dbdata_s *data);
void
init_dbdata_struct(dbdata_s *data);
void
init_multi_dbdata_struct(dbdata_s **list, unsigned int i);
void
clean_dbdata_struct(dbdata_s *list);
void
init_string_len(string_len_s *string);
void
clean_string_len(string_len_s *string);
void
init_string_l(string_l *string);
void
clean_string_l(string_l *list);
void
init_initial_string_l(string_l **string, int count); */
const char *
sqlite3_errstr(int error);

#endif // __AILSACMDB_H__
