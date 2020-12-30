/*
 *
 *  mkvm : make virtual machine
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
 *  mkvm.c
 *
 *  main and command line parsing functions for mkvm
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
#include "virtual.h"

static int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm);

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_mkvm_s *vm = ailsa_calloc(sizeof(ailsa_mkvm_s), "vm in main");
	ailsa_cmdb_s *cmdb = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmdb in main");

	parse_mkvm_config(vm);
	parse_cmdb_config(cmdb);
	if ((retval = parse_mkvm_command_line(argc, argv, vm)) != 0)
		goto cleanup;
	switch (vm->action) {
	case AILSA_ADD:
		retval = mkvm_create_vm(cmdb, vm);
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
		if (retval > 0)
			display_mkvm_usage();
		ailsa_clean_mkvm(vm);
		ailsa_clean_cmdb(cmdb);
		return retval;
}

static int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm)
{
	int retval = 0;
	int opt;
	const char *optstr = "c:dg:n:p:r:u:k:b:ahvC:";

#ifdef HAVE_GOTOPT_H
	int index;
	struct option opts[] = {
		{"cpus",	required_argument,	NULL,	'c'},
		{"storage",	required_argument,	NULL,	'g'},
		{"size",	required_argument,	NULL,	'g'},
		{"name",	required_argument,	NULL,	'n'},
		{"pool",	required_argument,	NULL,	'p'},
		{"ram",		required_argument,	NULL,	'r'},
		{"uri",		required_argument,	NULL,	'u'},
		{"network",	required_argument,	NULL,	'k'},
		{"bridge",	required_argument,	NULL,	'b'},
		{"coid",	required_argument,	NULL,	'C'},
		{"add",		no_argument,		NULL,	'a'},
		{"help",	no_argument,		NULL,	'h'},
		{"version",	no_argument,		NULL,	'v'},
		{"cmdb",	no_argument,		NULL,	'd'}
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
		case 'c':
			vm->cpus = strtoul(optarg, NULL, 10);
			break;
		case 'r':
			vm->ram = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "hostname trimmed to 255 characters\n");
			if (!(vm->name))
				vm->name = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->name, CONFIG_LEN, "%s", optarg);
			break;
		case 'p':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "pool namd trimmed to 255 characters\n");
			if (!(vm->pool))
				vm->pool = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->pool, CONFIG_LEN, "%s", optarg);
			break;
		case 'u':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "uri trimmed to 255 characters\n");
			if (!(vm->uri))
				vm->uri = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->uri, CONFIG_LEN, "%s", optarg);
			break;
		case 'k':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "network trimmed to 255 characters\n");
			if (!(vm->network))
				vm->network = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->network, CONFIG_LEN, "%s", optarg);
			break;
		case 'b':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "netdev trimmed to 255 characters\n");
			if (vm->netdev)
				my_free(vm->netdev);
			vm->netdev = strndup(optarg, CONFIG_LEN);
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
		case 'C':
			vm->coid = strndup(optarg, BYTE_LEN + 1);
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
	if (vm->size == 0)
		vm->size = 10;
	if (vm->action == 0)
		retval = AILSA_NO_ACTION;
	if (vm->cpus == 0)
		vm->cpus = 1;
	if (vm->ram == 0)
		vm->ram = 256;
	if ((vm->action == AILSA_ADD) && !(vm->name))
		retval = AILSA_NO_NAME;
	if (!(vm->network ) && !(vm->netdev))
		retval = AILSA_NO_NETWORK;
	return retval;
}
