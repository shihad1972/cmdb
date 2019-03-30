/*
 *
 *  libailsacmdb : Ailsa CMDB library
 *
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
 *  config.c
 *
 *  config and command line parsing functions
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

static void
parse_system_mkvm_config(ailsa_mkvm_s *vm);

static void
parse_user_mkvm_config(ailsa_mkvm_s *vm);

static void
parse_mkvm_config_values(ailsa_mkvm_s *vm, FILE *conf);

static void
parse_system_cmdb_config(ailsa_cmdb_s *cmdb);

static void
parse_user_cmdb_config(ailsa_cmdb_s *cmdb);

static void
parse_cmdb_config_values(ailsa_cmdb_s *cmdb, FILE *conf);

int
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
				fprintf(stderr, "name trimmed to 255 characters`n");
			if (!(vm->name))
				vm->name = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->name, CONFIG_LEN, "%s", optarg);
			break;
		case 'l':
			if (strlen(optarg) >= CONFIG_LEN)
				fprintf(stderr, "volume group name trimmed to 255 characters\n");
			if (!(vm->logvol))
				vm->logvol = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->logvol, CONFIG_LEN, "%s", optarg);
			break;
		case 'u':
			if (strlen(optarg) >= CONFIG_LEN)
				fprintf(stderr, "uri trimmed to 255 characters\n");
			if (!(vm->uri))
				vm->uri = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->uri, CONFIG_LEN, "%s", optarg);
			break;
		case 'p':
			if (strlen(optarg) >= CONFIG_LEN)
				fprintf(stderr, "path trimmed to 255 characters\n");
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
			fprintf(stderr, "Unknown option %c\n", opt);
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

int
parse_mkvm_command_line(int argc, char *argv[], ailsa_mkvm_s *vm)
{
	int retval = 0;
	int opt;
	const char *optstr = "c:g:n:p:r:u:k:ahvC";

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
		{"version",	no_argument,		NULL,	'v'},
		{"cmdb",	no_argument,		NULL,	'C'}
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
				fprintf(stderr, "hostname trimmed to 255 characters\n");
			if (!(vm->name))
				vm->name = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->name, CONFIG_LEN, "%s", optarg);
			break;
		case 'p':
			if (strlen(optarg) >= CONFIG_LEN)
				fprintf(stderr, "pool namd trimmed to 255 characters\n");
			if (!(vm->pool))
				vm->pool = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->pool, CONFIG_LEN, "%s", optarg);
			break;
		case 'u':
			if (strlen(optarg) >= CONFIG_LEN)
				fprintf(stderr, "uri trimmed to 255 characters\n");
			if (!(vm->uri))
				vm->uri = strndup(optarg, CONFIG_LEN);
			else
				snprintf(vm->uri, CONFIG_LEN, "%s", optarg);
			break;
		case 'k':
			if (strlen(optarg) >= CONFIG_LEN)
				fprintf(stderr, "network trimmed to 255 characters\n");
			if (!(vm->network))
				vm->network = strndup(optarg, CONFIG_LEN);
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
		case 'C':
			vm->action = AILSA_CMDB_ADD;
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", opt);
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
	return retval;
}

void
parse_mkvm_config(ailsa_mkvm_s *vm)
{
	parse_system_mkvm_config(vm);
	parse_user_mkvm_config(vm);
}

void
parse_cmdb_config(ailsa_cmdb_s *cmdb)
{
	parse_system_cmdb_config(cmdb);
	parse_user_cmdb_config(cmdb);
}

static void
parse_system_mkvm_config(ailsa_mkvm_s *vm)
{
	const char *path = "/etc/cmdb/mkvm.conf";
	FILE *conf = NULL;

	if (!(conf = fopen(path, "r"))) {
#ifdef DEBUG
		fprintf(stderr, "Cannot open file %s\n", path);
#endif
		goto cleanup;
	}
	parse_mkvm_config_values(vm, conf);
	cleanup:
		if (conf)
			fclose(conf);
}

static void
parse_user_mkvm_config(ailsa_mkvm_s *vm)
{
	FILE *conf = NULL;
	char *home;
#ifdef HAVE_WORDEXP
	int retval = 0;
	char **uconf = NULL;
	const char *upath = "~/.mkvm.conf";
	wordexp_t p;

	if ((retval = wordexp(upath, &p, 0)) == 0) {
		uconf = p.we_wordv;
		if (!(conf = fopen(*uconf, "r"))) {
#ifdef DEBUG
			fprintf(stderr, "Cannot open file %s\n", *uconf);
#endif
			goto cleanup;
		}
	}
#endif // HAVE_WORDEXP
	if (!(conf)) {
		char wpath[CONFIG_LEN];
		int len;
		home = getenv("HOME");	// Need to sanatise this input.
		if ((len = snprintf(wpath, CONFIG_LEN, "%s/.mkvm.conf", home)) >= CONFIG_LEN) {
			fprintf(stderr, "Output to config file truncated! Longer than 255 bytes\n");
			goto cleanup;
		}
		if (!(conf = fopen(wpath, "r"))) {
#ifdef DEBUG
			fprintf(stderr, "Cannot open file %s\n", wpath);
#endif
			goto cleanup;
		}
	}
	parse_mkvm_config_values(vm, conf);
	cleanup:
		if (conf)
			fclose(conf);
#ifdef HAVE_WORDEXP
		wordfree(&p);
#endif // HAVE_WORDEXP
}

/* Grab config values from confile file that uses NAME=value as configuration
   options */
#ifndef GET_CONFIG_OPTION
# define GET_CONFIG_OPTION(CONFIG, option) { \
   while (fgets(buff, CONFIG_LEN, conf)) \
     sscanf(buff, CONFIG, temp); \
   rewind(conf); \
   if (!(option) && (*temp)) \
     option = ailsa_calloc(CONFIG_LEN, "option in parse_mkvm_config_values"); \
   if (*temp) \
     snprintf(option, CONFIG_LEN, "%s", temp);\
   memset(temp, 0, CONFIG_LEN); \
  }
#endif
#ifndef GET_CONFIG_INT
# define GET_CONFIG_INT(CONFIG, option) { \
   while (fgets(buff, CONFIG_LEN, conf)) \
     sscanf(buff, CONFIG, &(option)); \
   rewind(conf); \
  }
#endif

static void
parse_mkvm_config_values(ailsa_mkvm_s *vm, FILE *conf)
{

	char buff[CONFIG_LEN], temp[CONFIG_LEN];

	GET_CONFIG_OPTION("NETWORK=%s", vm->network);
	GET_CONFIG_OPTION("URI=%s", vm->uri);
	GET_CONFIG_OPTION("POOL=%s", vm->pool);
	GET_CONFIG_OPTION("NAME=%s", vm->name);
	GET_CONFIG_INT("RAM=%lu", vm->ram);
	GET_CONFIG_INT("CPUS=%lu", vm->cpus);
	GET_CONFIG_INT("STORAGE=%lu", vm->size);
}

static void
parse_cmdb_config_values(ailsa_cmdb_s *cmdb, FILE *conf)
{
	char buff[CONFIG_LEN], temp[CONFIG_LEN];

	GET_CONFIG_OPTION("DBTYPE=%s", cmdb->dbtype);
	GET_CONFIG_OPTION("DB=%s", cmdb->db);
	GET_CONFIG_OPTION("FILE=%s", cmdb->file);
	GET_CONFIG_OPTION("USER=%s", cmdb->user);
	GET_CONFIG_OPTION("PASS=%s", cmdb->pass);
	GET_CONFIG_OPTION("HOST=%s", cmdb->host);
	GET_CONFIG_OPTION("DIR=%s", cmdb->dir);
	GET_CONFIG_OPTION("BIND=%s", cmdb->bind);
	GET_CONFIG_OPTION("DNSA=%s", cmdb->dnsa);
	GET_CONFIG_OPTION("REV=%s", cmdb->rev);
	GET_CONFIG_OPTION("RNDC=%s", cmdb->rndc);
	GET_CONFIG_OPTION("CHKZ=%s", cmdb->chkz);
	GET_CONFIG_OPTION("CHKC=%s", cmdb->chkc);
	GET_CONFIG_OPTION("SOCKET=%s", cmdb->socket);
	GET_CONFIG_OPTION("HOSTMASTER=%s", cmdb->hostmaster);
	GET_CONFIG_OPTION("PRINS=%s", cmdb->prins);
	GET_CONFIG_OPTION("SECNS=%s", cmdb->secns);
	GET_CONFIG_OPTION("PRIDNS=%s", cmdb->pridns);
	GET_CONFIG_OPTION("SECDNS=%s", cmdb->secdns);
	GET_CONFIG_OPTION("TMPDIR=%s", cmdb->tmpdir);
	GET_CONFIG_OPTION("TFTPDIR=%s", cmdb->tftpdir);
	GET_CONFIG_OPTION("PXE=%s", cmdb->pxe);
	GET_CONFIG_OPTION("TOPLEVELOS=%s", cmdb->toplevelos);
	GET_CONFIG_OPTION("DHCPCONF=%s", cmdb->dhcpconf);
	GET_CONFIG_OPTION("KICKSTART=%s", cmdb->kickstart);
	GET_CONFIG_OPTION("PRESEED=%s", cmdb->preseed);
	GET_CONFIG_INT("PORT=%u", cmdb->port);
	GET_CONFIG_INT("REFRESH=%lu", cmdb->refresh);
	GET_CONFIG_INT("RETRY=%lu", cmdb->retry);
	GET_CONFIG_INT("EXPIRE=%lu", cmdb->expire);
	GET_CONFIG_INT("TTL=%lu", cmdb->ttl);
	GET_CONFIG_INT("CLIFLAG=%lu", cmdb->cliflag);
}

void
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

void
display_mksp_usage(void)
{
	const char *prog = "mksp";

	printf("%s: make storage pool %s\n", prog, VERSION);
	printf("Usage:\t");
	printf("%s <action> <type> <options>\n", prog);
	printf("Actions\n");
	printf("\t-a: add\n");
	printf("\t-h: help\t-v: version\n");
	printf("Type\n");
	printf("\t-g: Volume Group\n");
	printf("\t-y: Directory\n");
	printf("Options\n");
	printf("\t-n <name>: Specify the name of the storage pool\n");
	printf("\t-l <volume-group>: Specify the name of the volume group\n");
	printf("\t-p <path>: Specify the path to the storage directory\n");
}

#ifndef OPEN_CMDB_FILE
# define OPEN_CMDB_FILE { \
	if ((conf = fopen(path, "r"))) { \
		parse_cmdb_config_values(cmdb, conf); \
		goto cleanup; \
	} \
  }
#endif

static void
parse_system_cmdb_config(ailsa_cmdb_s *cmdb)
{
	const char *path = NULL;
	FILE *conf = NULL;

	path = "/etc/cmdb/cmdb.conf";
	OPEN_CMDB_FILE
	path = "/etc/cmdb/dnsa.conf";
	OPEN_CMDB_FILE
	path = "/etc/dnsa/cmdb.conf";
	OPEN_CMDB_FILE
	path = "/etc/dnsa/dnsa.conf";
	OPEN_CMDB_FILE
	cleanup:
		if (conf)
			fclose(conf);
}

static void
parse_user_cmdb_config(ailsa_cmdb_s *cmdb)
{
	FILE *conf = NULL;
	char *home = NULL;
#ifdef HAVE_WORDEXP
	int retval = 0;
	char **uconf = NULL;
	const char *upath = "~/.cmdb.conf";
	wordexp_t p;

	if ((retval = wordexp(upath, &p, 0)) == 0) {
		uconf = p.we_wordv;
		if (!(conf = fopen(*uconf, "r"))) {
#ifdef DEBUG
			fprintf(stderr, "Cannot open file %s\n", *uconf);
#endif
			upath = "~/.dnsa.conf";
			wordexp_t r;
			if ((retval = wordexp(upath, &r, 0)) == 0) {
				uconf = r.we_wordv;
				if (!(conf = fopen(*uconf, "r"))) {
#ifdef DEBUG
					fprintf(stderr, "Cannot open file %s\n", *uconf);
#endif
				}
				wordfree(&r);
			}
		}
	}
#endif // HAVE_WORDEXP
	if (!(conf)) {
		char wpath[CONFIG_LEN];
		int len;
		home = getenv("HOME");	// Need to sanatise this input.
		if ((len = snprintf(wpath, CONFIG_LEN, "%s/.cmdb.conf", home)) >= CONFIG_LEN) {
			fprintf(stderr, "Output to config file truncated! Longer than 255 bytes\n");
			goto cleanup;
		}
		if (!(conf = fopen(wpath, "r"))) {
#ifdef DEBUG
			fprintf(stderr, "Cannot open file %s\n", wpath);
#endif
			if ((len = snprintf(wpath, CONFIG_LEN, "%s/.dnsa.conf", home)) >= CONFIG_LEN) {
				fprintf(stderr, "Output to config file truncated! Longer than 255 bytes\n");
				goto cleanup;
			}
			if (!(conf = fopen(wpath, "r"))) {
#ifdef DEBUG
				fprintf(stderr, "Cannot open file %s\n", wpath);
#endif
				goto cleanup;
			}
		}
	}
	parse_cmdb_config_values(cmdb, conf);
	cleanup:
		if (conf)
			fclose(conf);
#ifdef HAVE_WORDEXP
		wordfree(&p);
#endif // HAVE_WORDEXP
}

#undef GET_CONFIG_OPTION
#undef GET_CONFIG_INT
