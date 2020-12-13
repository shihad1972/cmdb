/*
 *
 *  mknet : make network
 *  Copyright (C) 2018 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  mknet.c
 *
 *  main and command line parsing functions for mknet
 *
 */

#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#ifdef HAVE_REGEX_H
# include <regex.h>
#endif // HAVE_REGEX_H
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#include <ailsacmdb.h>
#include <ailsasql.h>
#include "virtual.h"

const char *prog = "mknet";

static int
parse_mknet_command_line(int argc, char *argv[], ailsa_mkvm_s *vm);

static void
display_usage(void);

int
main(int argc, char *argv[])
{
        int retval = 0;
        ailsa_mkvm_s *vm = ailsa_calloc(sizeof(ailsa_mkvm_s), "vm in main");
	ailsa_cmdb_s *cmdb = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmdb in main");

	parse_mkvm_config(vm);
	parse_cmdb_config(cmdb);
	if ((retval = parse_mknet_command_line(argc, argv, vm)) != 0) {
                display_usage();
		goto cleanup;
        }
        switch(vm->action) {
        case CMDB_LIST:
                retval = ailsa_list_networks(vm);
                break;
        case AILSA_DISPLAY_USAGE:
                display_usage();
                break;
        case CMDB_ADD:
                retval = ailsa_add_network(cmdb, vm);
                break;
        case AILSA_VERSION:
                ailsa_syslog(LOG_INFO, "%s: version %s", prog, VERSION);
                break;
        }

        cleanup:
                ailsa_clean_mkvm(vm);
		ailsa_clean_cmdb(cmdb);
                return retval;
}

static int
parse_mknet_command_line(int argc, char *argv[], ailsa_mkvm_s *vm)
{
        int retval, opt;
        const char *optstr = "adhlvn:u:";

        retval = 0;
#ifdef HAVE_GOTOPT_H
        int index;
        struct option opts[] = {
                {"uri",		required_argument,	NULL,	'u'},
		{"network",	required_argument,	NULL,	'k'},
                {"add",         no_argument,            NULL,   'a'},
                {"cmdb",        no_argument,            NULL,   'd'},
                {"list",        no_argument,            NULL,   'l'},
                {"help",        no_argument,            NULL,   'h'},
                {"version",     no_argument,            NULL,   'v'},
                {NULL,		0,			NULL,	0}
        };
        while ((opt = getopt_long(argc, argv, optstr, opts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
        {
                switch(opt) {
                case 'a':
                        vm->action = CMDB_ADD;
                        break;
                case 'l':
                        vm->action = CMDB_LIST;
                        break;
                case 'h':
                        vm->action = AILSA_DISPLAY_USAGE;
                        break;
                case 'v':
                        vm->action = AILSA_VERSION;
                        break;
                case 'n':
                        if (strlen(optarg) >= CONFIG_LEN)
                                ailsa_syslog(LOG_ERR, "Network trimmed to 255 characters");
                        if (vm->network)
                                snprintf(vm->network, CONFIG_LEN, "%s", optarg);
                        else
                                vm->network = strndup(optarg, CONFIG_LEN);
                        break;
                case 'u':
                        if (strlen(optarg) >= CONFIG_LEN)
                                ailsa_syslog(LOG_ERR, "URI Trimmed to 255 characters");
                        if (vm->uri)
                                snprintf(vm->uri, CONFIG_LEN, "%s", optarg);
                        else
                                vm->uri = strndup(optarg, CONFIG_LEN);
                        break;
		case 'd':
			vm->cmdb = AILSA_CMDB_ADD;
			break;
                default:
			ailsa_syslog(LOG_ERR, "Unknown option %c\n", opt);
			return 1;
			break;
                }
        }
        if (vm->action == 0)
		retval = AILSA_NO_ACTION;
        if (!(vm->uri))
                retval = AILSA_NO_URI;
        return retval;
}

static void
display_usage(void)
{

        printf("%s: make virtual network %s\n", prog, VERSION);
        printf("Usage:\t");
	printf("%s <action> <options>\n", prog);
	printf("Actions\n");
	printf("\t-l: list\n");
        printf("Options\n");
	printf("\t-u <uri>: Connection URI for libvirtd\n");
        printf("\t-n <network>: Name of the libvirt network\n");
}
