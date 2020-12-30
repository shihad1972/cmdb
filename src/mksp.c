/*
 *
 *  mksp : make storage pool
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
 *  mksp.c
 *
 *  main and command line parsing functions for mksp
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
parse_mksp_command_line(int argc, char *argv[], ailsa_mkvm_s *vm);

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_mkvm_s *vm = ailsa_calloc(sizeof(ailsa_mkvm_s), "vm in main");

	parse_mkvm_config(vm);
	if ((retval = parse_mksp_command_line(argc, argv, vm)) != 0)
		goto cleanup;
	switch (vm->action) {
	case AILSA_ADD:
		retval = mksp_create_storage_pool(vm);
		break;
	case AILSA_HELP:
		display_mksp_usage();
		break;
	case AILSA_VERSION:
		display_version(argv[0]);
		break;
	}
	cleanup:
		ailsa_show_error(retval);
		if (retval > 0)
			display_mksp_usage();
		ailsa_clean_mkvm((void *)vm);
		return retval;
}

static int
parse_mksp_command_line(int argc, char *argv[], ailsa_mkvm_s *vm)
{
	int retval = 0;
	int opt;
	const char *optstr = "aghvyn:l:p:u:";
#ifdef HAVE_GOTOPT_H
	int index;
	struct option opts[] = {
		{"name",		required_argument,	NULL,	'n'},
		{"volume-group",	required_argument,	NULL,	'l'},
		{"uri",			required_argument,	NULL,	'u'},
		{"path",		required_argument,	NULL,	'p'},
		{"add",			no_argument,		NULL,	'a'},
		{"lvm",			no_argument,		NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"directory",		no_argument,		NULL,	'y'},
		{"version",		no_argument,		NULL,	'v'}
	};
	while ((opt = getopt_long(argc, argv, optstr, opts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		switch(opt) {
		case 'n':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "name trimmed to 255 characters`n");
			if (!(vm->name))
				vm->name = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->name, CONFIG_LEN, "%s", optarg);
			break;
		case 'l':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "volume group name trimmed to 255 characters\n");
			if (!(vm->logvol))
				vm->logvol = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->logvol, CONFIG_LEN, "%s", optarg);
			break;
		case 'u':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "uri trimmed to 255 characters\n");
			if (!(vm->uri))
				vm->uri = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->uri, CONFIG_LEN, "%s", optarg);
			break;
		case 'p':
			if (strlen(optarg) >= CONFIG_LEN)
				ailsa_syslog(LOG_INFO, "path trimmed to 255 characters\n");
			if (!(vm->path))
				vm->path = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->path, CONFIG_LEN, "%s", optarg);
			break;
		case 'g':
			vm->sptype = AILSA_LOGVOL;
			break;
		case 'y':
			vm->sptype = AILSA_DIRECTORY;
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
			ailsa_syslog(LOG_ERR, "Unknown option %c\n", opt);
			return 1;
			break;
		}
	}
	if (vm->action == 0)
		retval = AILSA_NO_ACTION;
	if (vm->sptype == 0)
		retval = AILSA_NO_TYPE;
	if (vm->action == AILSA_ADD) {
		if (!(vm->name))
			retval = AILSA_NO_NAME;
		else if (strlen(vm->name) == 0)
			retval = AILSA_NO_NAME;
	}
	if ((vm->action == AILSA_HELP) || (vm->action == AILSA_VERSION))
		return 0;
	if (vm->sptype == AILSA_LOGVOL) {
		if (!(vm->logvol)) 
			retval = AILSA_NO_LOGVOL;
		else if (strlen(vm->logvol) == 0)
			retval = AILSA_NO_LOGVOL;
	}
	if (vm->sptype == AILSA_DIRECTORY) {
		if (!(vm->path))
			retval = AILSA_NO_DIRECTORY;
		else if (strlen(vm->path) == 0)
			retval = AILSA_NO_DIRECTORY;
	}
	return retval;
}
