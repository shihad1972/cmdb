/*
 *
 *  mkvm : make virtual machine
 *  Copyright (C) 2018 - 2019  Iain M Conochie <iain-AT-thargoid.co.uk>
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

static void
display_mkvm_usage(void);

static void
parse_mkvm_config(ailsa_mkvm_s *vm);

static void
parse_system_mkvm_config(ailsa_mkvm_s *vm);

static void
parse_user_mkvm_config(ailsa_mkvm_s *vm);

static void
parse_config_values(ailsa_mkvm_s *vm, FILE *conf);

int
main(int argc, char *argv[])
{
	int retval = 0;
	ailsa_mkvm_s *vm = ailsa_calloc(sizeof(ailsa_mkvm_s), "vm in main");

	parse_mkvm_config(vm);
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
		if (retval > 0)
			display_mkvm_usage();
		ailsa_clean_mkvm((void *)vm);
		return retval;
}

static int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm)
{
	int retval = 0;
	int opt;
	const char *optstr = "c:g:n:p:r:u:k:ahv";

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
		case 'c':
			vm->cpus = strtoul(optarg, NULL, 10);
			break;
		case 'r':
			vm->ram = strtoul(optarg, NULL, 10);
			break;
		case 'n':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "hostname trimmed to 255 characters\n");
			if (!(vm->name))
				vm->name = strndup(optarg, DOMAIN_LEN);
			else
				snprintf(vm->name, CONFIG_LEN, "%s", optarg);
			break;
		case 'p':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "pool namd trimmed to 255 characters\n");
			if (!(vm->pool))
				vm->pool = strndup(optarg, DOMAIN_LEN);
			else
				snprintf(vm->pool, CONFIG_LEN, "%s", optarg);
			break;
		case 'u':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "uri trimmed to 255 characters\n");
			if (!(vm->uri))
				vm->uri = strndup(optarg, DOMAIN_LEN);
			else
				snprintf(vm->uri, CONFIG_LEN, "%s", optarg);
			break;
		case 'k':
			if (strlen(optarg) >= DOMAIN_LEN)
				fprintf(stderr, "network trimmed to 255 characters\n");
			if (!(vm->network))
				vm->network = strndup(optarg, DOMAIN_LEN);
			else
				snprintf(vm->network, CONFIG_LEN, "%s", optarg);
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
	if (vm->cpus == 0)
		vm->cpus = 1;
	if (vm->ram == 0)
		vm->ram = 256;
	return retval;
}

static void
parse_mkvm_config(ailsa_mkvm_s *vm)
{
	parse_system_mkvm_config(vm);
	parse_user_mkvm_config(vm);
}


static void
parse_system_mkvm_config(ailsa_mkvm_s *vm)
{
	const char *path = "/etc/cmdb/mkvm.conf";
	FILE *conf = NULL;

	if (!(conf = fopen(path, "r")))
		goto cleanup;
	parse_config_values(vm, conf);
	cleanup:
		if (conf)
			fclose(conf);
}

static void
parse_user_mkvm_config(ailsa_mkvm_s *vm)
{
	int retval = 0;
	FILE *conf = NULL;
	char **uconf = NULL;
#ifdef HAVE_WORDEXP
	const char *upath = "~/.mkvm.conf";
	wordexp_t p;

	if ((retval = wordexp(upath, &p, 0)) == 0) {
		uconf = p.we_wordv;
		if (!(conf = fopen(*uconf, "r")))
			goto cleanup;
	}
#endif // HAVE_WORDEXP
	if (!(conf)) {
		char wpath[CONFIG_LEN];
		int len;
		*uconf = getenv("HOME");	// Need to sanatise this input.
		if ((len = snprintf(wpath, CONFIG_LEN, "%s/.mkvm.conf", *uconf)) >= CONFIG_LEN) {
			fprintf(stderr, "Output to config file truncated! Longer than 255 bytes\n");
			goto cleanup;
		}
		if (!(conf = fopen(wpath, "r")))
			goto cleanup;
	}
	parse_config_values(vm, conf);
	cleanup:
		if (conf)
			fclose(conf);
#ifdef HAVE_WORDEXP
		wordfree(&p);
#endif // HAVE_WORDEXP
}


static void
parse_config_values(ailsa_mkvm_s *vm, FILE *conf)
{

/* Grab config values from confile file that uses NAME=value as configuration
   options */
# ifndef GET_CONFIG_OPTION
#  define GET_CONFIG_OPTION(CONFIG, option) { \
   while (fgets(buff, CONFIG_LEN, conf)) \
     sscanf(buff, CONFIG, temp); \
   rewind(conf); \
   if (!(option) && (*temp)) \
     option = ailsa_calloc(CONFIG_LEN, "option in parse_config_values"); \
   if (*temp) \
     snprintf(option, CONFIG_LEN, "%s", temp);\
   memset(temp, 0, CONFIG_LEN); \
  }
# ifndef GET_CONFIG_INT
#  define GET_CONFIG_INT(CONFIG, option) { \
   while (fgets(buff, CONFIG_LEN, conf)) \
     sscanf(buff, CONFIG, &(option)); \
   rewind(conf); \
  }
# endif
# endif
	char buff[CONFIG_LEN], temp[CONFIG_LEN];

	GET_CONFIG_OPTION("NETWORK=%s", vm->network);
	GET_CONFIG_OPTION("URI=%s", vm->uri);
	GET_CONFIG_OPTION("POOL=%s", vm->pool);
	GET_CONFIG_OPTION("NAME=%s", vm->name);
	GET_CONFIG_INT("RAM=%lu", vm->ram);
	GET_CONFIG_INT("CPUS=%lu", vm->cpus);
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
	printf("\t-u <uri>: Connection URI for libvirtd\n");
        printf("\t-n <name>: Supply VM name\n");
        printf("\t-p <pool>: Provide the storage pool name\n");
        printf("\t-g <size>: Size (in GB) of disk (default's to 10GB)\n");
	printf("\t-k <network>: Name of the network to attach the VM to\n");
	printf("\t-c <cpus>: No of CPU's the vm should have (default's to 1)\n");
	printf("\t-r <ram>: Amount of RAM (in MB) the vm should have (default's to 256MB)\n");
}
