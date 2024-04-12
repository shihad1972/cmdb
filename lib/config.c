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

void
parse_mkvm_config(ailsa_mkvm_s *vm)
{
	parse_system_mkvm_config(vm);
	parse_user_mkvm_config(vm);
}

void
parse_cmdb_config(ailsa_cmdb_s *cmdb)
{
	if (!(cmdb->toplevelos))
		cmdb->toplevelos = ailsa_calloc(CONFIG_LEN, "cmdb->toplevelos in parse_cmdb_config");
	if (!(cmdb->tmpdir))
		cmdb->tmpdir = ailsa_calloc(CONFIG_LEN, "cmdb->tmpdir in parse_cmdb_config");
	if (!(cmdb->pxe))
		cmdb->pxe = ailsa_calloc(CONFIG_LEN, "cmdb->pxe in parse_cmdb_config");
	snprintf(cmdb->toplevelos, CONFIG_LEN, "%s/cmdb", LOCALSTATEDIR);
	snprintf(cmdb->tmpdir, CONFIG_LEN, "%s/cmdb/tmp", LOCALSTATEDIR);
	snprintf(cmdb->pxe, CONFIG_LEN, "pxelinux.cfg");
	parse_system_cmdb_config(cmdb);
	parse_user_cmdb_config(cmdb);
}

static void
parse_system_mkvm_config(ailsa_mkvm_s *vm)
{
	char path[FILE_LEN];
	FILE *conf = NULL;

	memset(path, 0, FILE_LEN);
	snprintf(path, FILE_LEN, "%s/cmdb/mkvm.conf", SYSCONFDIR);
	if (!(conf = fopen(path, "r"))) {
#ifdef DEBUG
		ailsa_syslog(LOG_DEBUG, "Cannot open file %s\n", path);
#endif
		goto cleanup;
	}
	fseek(conf, 0, SEEK_END);
	if (ftell(conf) > 0) {
		fseek(conf, 0, SEEK_SET);
		parse_mkvm_config_values(vm, conf);
	}
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
			ailsa_syslog(LOG_DEBUG, "Cannot open file %s\n", *uconf);
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
			ailsa_syslog(LOG_ERR, "Output path to config file truncated! Longer than 255 bytes\n");
			goto cleanup;
		}
		if (!(conf = fopen(wpath, "r"))) {
#ifdef DEBUG
			ailsa_syslog(LOG_DEBUG, "Cannot open file %s\n", wpath);
#endif
			goto cleanup;
		}
	}
	fseek(conf, 0, SEEK_END);
	if (ftell(conf) > 0) {
		fseek(conf, 0, SEEK_SET);
		parse_mkvm_config_values(vm, conf);
	}
	
	cleanup:
		if (conf)
			fclose(conf);
#ifdef HAVE_WORDEXP
		wordfree(&p);
#endif // HAVE_WORDEXP
}

/* Grab config values from conffile file that uses NAME=value as configuration
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

	memset(buff, 0, CONFIG_LEN);
	memset(temp, 0, CONFIG_LEN);
	GET_CONFIG_OPTION("URI=%s", vm->uri);
	GET_CONFIG_OPTION("POOL=%s", vm->pool);
	GET_CONFIG_OPTION("NAME=%s", vm->name);
	GET_CONFIG_OPTION("INTERFACE=%s", vm->netdev);
	GET_CONFIG_OPTION("NETWORK=%s", vm->network);
	GET_CONFIG_INT("RAM=%lu", vm->ram);
	GET_CONFIG_INT("CPUS=%lu", vm->cpus);
	GET_CONFIG_INT("STORAGE=%lu", vm->size);
}

static void
parse_cmdb_config_values(ailsa_cmdb_s *cmdb, FILE *conf)
{
	char buff[CONFIG_LEN], temp[CONFIG_LEN];
	char *tmp;
	int at = '@';
	memset(buff, 0, CONFIG_LEN);
	memset(temp, 0, CONFIG_LEN);

	GET_CONFIG_OPTION("TMPDIR=%s", cmdb->tmpdir);
	GET_CONFIG_OPTION("TOPLEVELOS=%s", cmdb->toplevelos);
	GET_CONFIG_OPTION("PXE=%s", cmdb->pxe);
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
	GET_CONFIG_OPTION("TFTPDIR=%s", cmdb->tftpdir);
	GET_CONFIG_OPTION("DHCPCONF=%s", cmdb->dhcpconf);
	GET_CONFIG_INT("PORT=%u", cmdb->port);
	GET_CONFIG_INT("REFRESH=%lu", cmdb->refresh);
	GET_CONFIG_INT("RETRY=%lu", cmdb->retry);
	GET_CONFIG_INT("EXPIRE=%lu", cmdb->expire);
	GET_CONFIG_INT("TTL=%lu", cmdb->ttl);
	GET_CONFIG_INT("CLIFLAG=%lu", cmdb->cliflag);
	if ((tmp = strchr(cmdb->hostmaster, at)))
		*tmp = '.';
	if (cmdb->hostmaster)
		if (ailsa_add_trailing_dot(cmdb->hostmaster) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add . to end of hostmaster");
	if (cmdb->dir)
		if (ailsa_add_trailing_slash(cmdb->dir) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of DIR");
	if (cmdb->pxe)
		if (ailsa_add_trailing_slash(cmdb->pxe) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of PXE");
	if (cmdb->bind)
		if (ailsa_add_trailing_slash(cmdb->bind) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of BIND");
	if (cmdb->tmpdir)
		if (ailsa_add_trailing_slash(cmdb->tmpdir) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of TMPDIR");
	if (cmdb->tftpdir)
		if (ailsa_add_trailing_slash(cmdb->tftpdir) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of FTFPDIR");
	if (cmdb->toplevelos)
		if (ailsa_add_trailing_slash(cmdb->toplevelos) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of TOPLEVELOS");
	if(cmdb->dhcpconf)
		if (ailsa_add_trailing_slash(cmdb->dhcpconf) != 0)
			ailsa_syslog(LOG_ERR, "Cannot add / to the end of DHCPCONF");
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
	printf("\t-d: add to cmdb\n");
	printf("Options\n");
	printf("\t-u <uri>: Connection URI for libvirtd\n");
	printf("\t-n <name>: Supply VM name\n");
	printf("\t-p <pool>: Provide the storage pool name\n");
	printf("\t-g <size>: Size (in GB) of disk (default's to 10GB)\n");
	printf("\t-c <cpus>: No of CPU's the vm should have (default's to 1)\n");
	printf("\t-r <ram>: Amount of RAM (in MB) the vm should have (default's to 256MB)\n");
	printf("\t-C <coid>: COID of customer in CMDB\n");
	printf("Mutually exclusive Options\n");
	printf("\t[ -k <network>: Name of the network | -b <bridge-device>: Name of the bridge ]\n");
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


static void
parse_system_cmdb_config(ailsa_cmdb_s *cmdb)
{
	char path[FILE_LEN];
	FILE *conf = NULL;

	memset(path, 0, FILE_LEN);
	snprintf(path, FILE_LEN, "%s/cmdb/cmdb.conf", SYSCONFDIR);
	if ((conf = fopen(path, "r"))) {
		parse_cmdb_config_values(cmdb, conf);
		fclose(conf);
	} else {
		ailsa_syslog(LOG_DAEMON, "Cannot open %s for config\n", path);
		exit (1);
	}
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
			ailsa_syslog(LOG_DEBUG, "Cannot open file %s", *uconf);
#endif
		}
	}
#endif // HAVE_WORDEXP
	if (!(conf)) {
		char wpath[CONFIG_LEN];
		int len;
		home = getenv("HOME");	// Need to sanatise this input.
		if ((len = snprintf(wpath, CONFIG_LEN, "%s/.cmdb.conf", home)) >= CONFIG_LEN) {
			ailsa_syslog(LOG_INFO, "Output path to config file truncated! Longer than 255 bytes\n");
			goto cleanup;
		}
		if (!(conf = fopen(wpath, "r"))) {
#ifdef DEBUG
			ailsa_syslog(LOG_DEBUG, "Cannot open file %s", wpath);
#endif
			if ((len = snprintf(wpath, CONFIG_LEN, "%s/.dnsa.conf", home)) >= CONFIG_LEN) {
				ailsa_syslog(LOG_INFO, "Output path to config file truncated! Longer than 255 bytes\n");
				goto cleanup;
			}
			if (!(conf = fopen(wpath, "r"))) {
#ifdef DEBUG
				ailsa_syslog(LOG_DEBUG, "Cannot open file %s\n", wpath);
#endif
				goto cleanup;
			}
		}
	}
	fseek(conf, 0, SEEK_END);
	if (ftell(conf) > 0) {
		fseek(conf, 0, SEEK_SET);
		parse_cmdb_config_values(cmdb, conf);
	}
	cleanup:
		if (conf)
			fclose(conf);
#ifdef HAVE_WORDEXP
		wordfree(&p);
#endif // HAVE_WORDEXP
}

#undef GET_CONFIG_OPTION
#undef GET_CONFIG_INT
