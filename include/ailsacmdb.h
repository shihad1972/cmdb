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
# endif /* NI_MAXHOST so I do not have to use __GNU_SOURCE. grrrr */
/*
** base64 returnable errors
**
** Error codes returned to the operating system.
**
*/
#define B64_SYNTAX_ERROR        1
#define B64_FILE_ERROR          2
#define B64_FILE_IO_ERROR       3
#define B64_ERROR_OUT_CLOSE     4
#define B64_LINE_SIZE_TO_MIN    5
#define B64_SYNTAX_TOOMANYARGS  6

// Temporary
#define BASEDIR "/var/lib/cmdb/data/"
#define AILSAVERSION "0.1"

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


void
show_ailsacmdb_version();

int
ailsa_add_trailing_dot(char *string);

int
ailsa_add_trailing_slash(char *string);

void
ailsa_chomp(char *line);

void
ailsa_munch(char *line);

void *
ailsa_malloc(size_t len, const char *msg);

void
ailsa_start_syslog(const char *prog);

void
cmdbd_parse_config(const char *file, void *data, size_t len);

void
cmdbd_clean_config(struct cmdbd_config *cmdbd);

void
cmdbc_clean_config(struct cmdbc_config *cmdbc);

// Networking Functions

int
ailsa_tcp_socket_bind(const char *node, const char *service);

int
ailsa_tcp_socket(const char *node, const char *service);

/* 
int
ailsa_accept_tcp_connection(int sock); */

int
ailsa_accept_client(int sock);

int
ailsa_get_fqdn(char *host, char *fqdn, char *ip);

int
ailsa_do_client_send(int s, struct cmdb_client_config *c);

// File / Directory IO helper functions

int
check_directory_access(const char *dir, int create);

int
ailsa_write_file(const char *name, void *data, size_t len);

int
ailsa_read_file(const char *name, void *data, size_t len);

int
ailsa_append_file(const char *name, void *data, size_t len);
 
#endif // __AILSACMDB_H__
