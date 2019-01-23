/* 
 *
 *  mkvm : make virtual machine
 *  Copyright (C) 2018  Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  mkvm.c
 *
 *  main and command line parsing functions for mkvm
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include "virtual.h"

static int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm);

static void
display_mkvm_usage(void);

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_mkvm_s *vm = ailsa_calloc(sizeof(ailsa_mkvm_s), "vm in main");

	if ((retval = parse_mkvm_command_line(argc, argv, vm)) != 0)
		goto cleanup;
	switch (vm->action) {
	case AILSA_ADD:
		retval = mkvm_create_vm(vm);
		break;
	case AILSA_HELP:
		display_mkvm_usage();
		break;
	case AILSA_VERSION:
		display_version(argv[0]);
		break;
	}
	cleanup:
		ailsa_show_error(retval);
		ailsa_clean_mkvm((void *)vm);
		return retval;
}

static int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm)
{
	int retval = 0;
	int opt;
	const char *optstr = "n:p:u:ahv";

#ifdef HAVE_GOTOPT_H
	int index;
	struct option opts[] = {
		{"storage",	required_argument,	NULL,	'g'},
		{"size",	required_argument,	NULL,	'g'},
		{"name",	required_argument,	NULL,	'n'},
		{"pool",	required_argument,	NULL,	'p'},
		{"uri",		required_argument,	NULL,	'u'},
		{"add",		no_argument,		NULL,	'a'},
		{"help",	no_argument,		NULL,	'h'},
		{"version",	no_argument,		NULL,	'v'}
	};
	while ((opt = getopt_long(argc, argv, optstr, opts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		switch(opt) {
		case 'g':
			vm->size = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "hostname trimmed to 255 characters\n");
			vm->name = strndup(optarg, DOMAIN_LEN);
			break;
		case 'p':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "pool namd trimmed to 255 characters\n");
			vm->pool = strndup(optarg, DOMAIN_LEN);
			break;
		case 'u':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "uri trimmed to 255 characters\n");
			vm->uri = strndup(optarg, DOMAIN_LEN);
			break;
		case 'a':
			vm->action = AILSA_ADD;
			break;
		case 'h':
			vm->action = AILSA_HELP;
			break;
		case 'v':
			vm->action = AILSA_VERSION;
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", opt);
			retval = 1;
			break;
		}
	}
	if (vm->size == 0)
		vm->size = 10;
	if (vm->action == 0)
		retval = AILSA_NO_ACTION;
	return retval;
}


static void
display_mkvm_usage(void)
{
        const char *prog = "mkvm";

        printf("%s: make virtual machine %s\n", prog, VERSION);
        printf("Usage:\t");
        printf("%s <action> <options>\n", prog);
        printf("Actions\n");
        printf("\t-a: add\n");
        printf("\t-h: help\t-v: version\n");
        printf("Options\n");
        printf("\t-n <name>: Supply VM name\n");
        printf("\t-p <pool>: Provide the storage pool name\n");
        printf("\t-g <size>: Size (in GB) of disk (default's to 10GB)\n");
}
