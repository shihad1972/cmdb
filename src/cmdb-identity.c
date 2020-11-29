/* 
 *
 *  cmdb : Configuration Management Database
 *  Copyright (C) 2016 - 2020 Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  cmdb-identity.c
 *
 *  Contains functions for cmdb-identity program
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <syslog.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#define PROGRAM "cmdb-identity"

typedef struct cmdb_identity_comm_line_s { /* Hold parsed command line args for cmdb-identity */
	char *hash;
	char *name;
	char *pass;
	char *user;
	short int action;
} cmdb_identity_comm_line_s;

static int
parse_command_line(int argc, char *argv[], cmdb_identity_comm_line_s *cml);

static void
display_usage();

static void
clean_cmdb_identity_comm_line(cmdb_identity_comm_line_s *comm);

int
main(int argc, char *argv[])
{
        int retval = 0;
        ailsa_start_syslog(basename(argv[0]));
        cmdb_identity_comm_line_s *cml = ailsa_calloc(sizeof(cmdb_identity_comm_line_s), "cml in main");
        if ((retval = parse_command_line(argc, argv, cml)) != 0) {
                display_usage();
                goto cleanup;
        }

        cleanup:
                clean_cmdb_identity_comm_line(cml);
                return retval;
}

static int
parse_command_line(int argc, char *argv[], cmdb_identity_comm_line_s *cml)
{
        int retval = 0;
        int opt;
        const char *optstr = "adlr";
#ifdef HAVE_GETOPT_H
	int index;
        struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
                {"display",             no_argument,            NULL,   'd'},
                {"list",                no_argument,            NULL,   'l'},
                {"remove",              no_argument,            NULL,   'r'},
                {"delete",              no_argument,            NULL,   'r'},
                {NULL, 0, NULL, 0}
	};
        while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
        {
		if (opt == 'a')
			cml->action = CMDB_ADD;
                else if (opt == 'd')
                        cml->action = CMDB_DISPLAY;
                else if (opt == 'l')
                        cml->action = CMDB_LIST;
                else if (opt == 'r')
                        cml->action = CMDB_RM;
                else
			retval = CMDB_USAGE;
        }
        return retval;
}

static void
clean_cmdb_identity_comm_line(cmdb_identity_comm_line_s *comm)
{
	if (!(comm))
		return;
	if (comm->hash)
		my_free(comm->hash);
	if (comm->user)
		my_free(comm->user);
	if (comm->pass)
		my_free(comm->pass);
	if (comm->name)
		my_free(comm->name);
	my_free(comm);
}

static void
display_usage()
{
        ailsa_syslog(LOG_ERR, "Usage: %s ( -a | -d | -l | -r )", PROGRAM);
}
