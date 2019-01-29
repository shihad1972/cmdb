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
	CONFIG_LEN = 256,
	DOMAIN_LEN = 256
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
        AILSA_HELP = 100,
        AILSA_VERSION = 101
};

enum {                  // Error codes
        AILSA_NO_ACTION = 1,
        AILSA_NO_DATA = 200,
        AILSA_NO_CONNECT = 201
};

// mkvm data types

typedef struct ailsa_mkvm_s {
        char *name;
        char *pool;
        char *uri;
        char *storxml;
	char *network;
        unsigned long int size;
        short int action;
} ailsa_mkvm_s;

// library version info

void
show_ailsacmdb_version();

// library error message function(s)

void
ailsa_show_error(int retval);

// memory functions

void
ailsa_clean_mkvm(void *vm);
void *
ailsa_calloc(size_t len, const char *msg);

// the rest ...
/* 
void
ailsa_chomp(char *line);
void
ailsa_munch(char *line);
int
ailsa_add_trailing_slash(char *member);
int
ailsa_add_trailing_dot(char *member);
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
