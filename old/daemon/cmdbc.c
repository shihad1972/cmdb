/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdbd.c
 *
 *  Contains main() function for cmdbd program
 *
 */

#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <assert.h>
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif // HAVE_GETOPT_H
#include <errno.h>
// Needed for basename()
#include <libgen.h>
// Linux only - need autoconf test
#include <uuid.h>
//
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ailsacmdb.h>
#include "cmdb.h"
#include "client_info.h"

static struct cmdb_client_config ccc;

static void
parse_command_line(struct cmdbc_config *cm, int argc, char *argv[]);

static int
cmdbc_check_config(struct cmdbc_config *c);

static int
cmdbc_init_client(char *basedir);

static int
get_client_uuid(char *basedir);

int
main(int argc, char *argv[])
{
	char basedir[FILE_S];
	int retval = 0;
	int s;
	struct cmdbc_config *cm = ailsa_calloc(sizeof(struct cmdbc_config), "cm in main");

	snprintf(basedir, FILE_S, "%s/cmdb/client/", LOCALSTATEDIR);
	ailsa_start_syslog(basename(argv[0]));
	if (argc > 1)
		parse_command_line(cm, argc, argv);
	if ((retval = cmdbc_check_config(cm)) != 0) {
		assert(retval == 0);
		goto cleanup;
	}
	if ((retval = cmdbc_init_client(basedir)) != 0)
		goto cleanup;
	if ((s = ailsa_tcp_socket(cm->host, "cmdb")) < 0) {
		goto cleanup;
	}
	if ((retval = ailsa_do_client_send(s, &ccc)) != 0)
		goto cleanup;
	cleanup:
		cmdbc_clean_config(cm);
		my_free(cm);
		return retval;
}

static int
cmdbc_check_config(struct cmdbc_config *c)
{
	int retval = 0;

	if (!c->host) {
		syslog(LOG_ALERT, "No server hostname specified");
		retval = 1;
	}
	if (!c->service)
		c->service = strndup("cmdb", RANGE_S);
	return retval;
}

static void
parse_command_line(struct cmdbc_config *cm, int argc, char *argv[])
{
	const char *optstr = "h:s:";
	int c;
#ifdef HAVE_GETOPT_H
	int index;
	static const struct option longopts[] = {
		{ "host", required_argument,	0, 'h' },
		{ "service", required_argument,	0, 's' },
		{ NULL, 0, 0, 0 }
	};
	while ((c = getopt_long(argc, argv, optstr, longopts, &index)) != -1)
#else
	while ((c = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		switch (c) {
		case 'h':
			cm->host = strndup(optarg, RBUFF_S);
			break;
		case 's':
			cm->service = strndup(optarg, RANGE_S);
			break;
		default:
			syslog(LOG_WARNING, "Unknown option %c", c);
		}
	}
}

static int
cmdbc_init_client(char *basedir)
{
	int retval = 0;
	memset(&ccc, 0, sizeof(struct cmdbc_config));
	if ((retval = check_directory_access(basedir, true)) != 0)
		return retval;
	if ((retval = get_client_uuid(basedir)) != 0)
		return retval;
	ccc.host = ailsa_calloc(NI_MAXHOST, "ccc.host in cmdbc_init_client");
	if ((retval = gethostname(ccc.host, NI_MAXHOST)) != 0) {
		syslog(LOG_ALERT, "gethostname() error: %s", strerror(errno));
		return retval;
	}
	ccc.fqdn = ailsa_calloc(RBUFF_S, "ccc.fqdn in cmdbc_init_clinet");
	ccc.ip = ailsa_calloc(INET6_ADDRSTRLEN, "ccc.ip in cmdbc_init_client");
	retval = ailsa_get_fqdn(ccc.host, ccc.fqdn, ccc.ip);
	if (strnlen(ccc.fqdn, RBUFF_S) == 0)
		my_free(ccc.fqdn);
	if (strnlen(ccc.ip, INET6_ADDRSTRLEN) == 0)
		my_free(ccc.ip);
	return retval;
}

static int
get_client_uuid(char *basedir)
{
	char *file;
	int retval;
	struct stat st;
	size_t len = sizeof(uuid_t);
	uuid_t cli_uuid;

	file = ailsa_calloc(CONF_S, "file in get_client_uuid");
	snprintf(file, CONF_S, "%s%s", basedir, "uuid");
	if ((retval = stat(file, &st)) != 0) {	// no uuid file
		uuid_generate(cli_uuid);
		if ((retval = ailsa_write_file(file, &cli_uuid, len)) != 0)
			return retval;
	} else {
		if ((retval = ailsa_read_file(file, &cli_uuid, len)) != 0)
			return retval;
	}
	ccc.uuid = ailsa_calloc(HOST_S, "ccc.uuid in get_client_uuid");
	uuid_unparse_upper(cli_uuid, ccc.uuid);
	return retval;
}

