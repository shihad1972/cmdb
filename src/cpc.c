/* 
 *
 *  cpc: Create Preseed Configuration
 *  Copyright (C) 2014 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cpc.c
 * 
 *  Main source file for cpc
 * 
 *  Part of the cpc program
 * 
 * 
 */
#define _GNU_SOURCE
#include <config.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cbc_data.h"
#include "checks.h"

typedef struct cpc_config_s {
	char *disk;
	char *domain;
	char *interface;
	char *file;
	char *kbd;
	char *locale;
	char *mirror;
	char *name;
	char *ntp_server;
	char *packages;
	char *pinstall;
	char *proxy;
	char *rpass;
	char *suite;
	char *tzone;
	char *ugroups;
	char *uid;
	char *uname;
	char *upass;
	char *url;
	char *user;
	short int add_root;
	short int add_user;
	short int encrypt_rpass;
	short int encrypt_upass;
	short int post;
	short int ntp;
	short int recommends;
	short int utc;
	short int action;
} cpc_config_s;

static int
parse_cpc_comm_line(int argc, char *argv[], cpc_config_s *cl);

static int
parse_cpc_config_file(cpc_config_s *cpc);

static int
parse_cpc_environment(cpc_config_s *cpc);

static void
fill_default_cpc_config_values(cpc_config_s *cpc);

static void
add_header(string_len_s *preseed);

static void
add_locale(string_len_s *pre, cpc_config_s *cpc);

static void
add_network(string_len_s *pre, cpc_config_s *cpc);

static void
add_mirror(string_len_s *pre, cpc_config_s *cpc);

static void
add_account(string_len_s *pre, cpc_config_s *cpc);

static void
add_root_account(string_len_s *pre, cpc_config_s *cpc);

static void
add_no_root_account(string_len_s *pre);

static void
add_user_account(string_len_s *pre, cpc_config_s *cpc);

static void
add_clock_and_ntp(string_len_s *pre, cpc_config_s *cpc);

static void
add_partitions(string_len_s *pre, cpc_config_s *cpc);
/*
static void
add_no_recommends(string_len_s *pre, cpc_config_s *cpc); */

static void
add_apt(string_len_s *pre, cpc_config_s *cpc);

static void
add_final(string_len_s *pre, cpc_config_s *cpc);

static void
build_preseed(cpc_config_s *cpc);

static void
init_cpc_config(cpc_config_s *cpc);

static void
fill_default_cpc_config_values(cpc_config_s *cpc);

static void
clean_cpc_config(cpc_config_s *cpc);

static void
replace_space(char *packages);

int
main (int argc, char *argv[])
{
	int retval = NONE;
	cpc_config_s *cpc = NULL;

	if (!(cpc = malloc(sizeof(cpc_config_s))))
		report_error(MALLOC_FAIL, "cpc in main");
	init_cpc_config(cpc);
	fill_default_cpc_config_values(cpc);
	if ((retval = parse_cpc_config_file(cpc)) != 0)
		fprintf(stderr, "Problem parsing config file. Continuing\n");
	if ((retval = parse_cpc_environment(cpc)) != 0)
		fprintf(stderr, "Problem getting environment variables. Continuing\n");
	if ((retval = parse_cpc_comm_line(argc, argv, cpc)) != 0) {
		clean_cpc_config(cpc);
		display_cpc_usage();
		exit(1);
	}
	if (cpc->action) {
		if (cpc->action == HELP)
			display_cpc_usage();
		else if (cpc->action == VERS)
			display_version(argv[0]);
	} else {
		build_preseed(cpc);
	}
	clean_cpc_config(cpc);
	return retval;
}

static int
parse_cpc_comm_line(int argc, char *argv[], cpc_config_s *cl)
{
	const char *optstr = "d:e:f:hi:k:l:m:n:p:s:t:u:vy:";
	int opt, retval;
	retval = 0;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"domain",		required_argument,	NULL,	'd'},
		{"ntp-server",		required_argument,	NULL,	'e'},
		{"file",		required_argument,	NULL,	'f'},
		{"help",		no_argument,		NULL,	'h'},
		{"interface",		required_argument,	NULL,	'i'},
		{"disk",		required_argument,	NULL,	'k'},
		{"locale",		required_argument,	NULL,	'l'},
		{"mirror",		required_argument,	NULL,	'm'},
		{"name",		required_argument,	NULL,	'n'},
		{"packages",		required_argument,	NULL,	'p'},
		{"suite",		required_argument,	NULL,	's'},
		{"timezone",		required_argument,	NULL,	't'},
		{"url",			required_argument,	NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"keyboard",		required_argument,	NULL,	'y'},
		{NULL,			0,			NULL,	0}
	};

	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'd') {
			snprintf(cl->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'e') {
			snprintf(cl->ntp_server, RBUFF_S, "%s", optarg);
		} else if (opt == 'f') {
			snprintf(cl->file, RBUFF_S, "%s", optarg);
		} else if (opt == 'i') {
			snprintf(cl->interface, RBUFF_S, "%s", optarg);
		} else if (opt == 'y') {
			snprintf(cl->kbd, RBUFF_S, "%s", optarg);
		} else if (opt == 'l') {
			snprintf(cl->locale, RBUFF_S, "%s", optarg);
		} else if (opt == 'm') {
			snprintf(cl->mirror, RBUFF_S, "%s", optarg);
		} else if (opt == 'n') {
			snprintf(cl->name, RBUFF_S, "%s", optarg);
		} else if (opt == 'p') {
			snprintf(cl->packages, RBUFF_S, "%s", optarg);
			replace_space(cl->packages);
		} else if (opt == 's') {
			snprintf(cl->suite, RBUFF_S, "%s", optarg);
		} else if (opt == 't') {
			snprintf(cl->tzone, RBUFF_S, "%s", optarg);
		} else if (opt == 'u') {
			snprintf(cl->url, RBUFF_S, "%s", optarg);
		} else if (opt == 'k') {
			snprintf(cl->disk, RBUFF_S, "/dev/%s\n", optarg);
		} else if (opt == 'h') {
			cl->action = HELP;
		} else if (opt == 'v') {
			cl->action = VERS;
		} else {
			fprintf(stderr, "Unknown option %c\n", opt);
			retval = DISPLAY_USAGE;
		}
	}
	return retval;
}

static int
parse_cpc_config_file(cpc_config_s *cpc)
{
	char *file, *home, buff[CONF_S];
	int retval = 0;
	FILE *config;
#ifndef CPC_GET_CONFIG_FILE
# define CPC_GET_CONFIG_FILE(CONFIG, conf) { \
  while (fgets(buff, CONF_S, config)) \
    sscanf(buff, CONFIG, cpc->conf); \
  rewind(config); \
 }
#endif

	home = getenv("HOME");
	if (asprintf(&file, "%s/.cpc.conf", home) < 0)
		return CONF_ERR;
	if (!(config = fopen(file, "r"))) 
		return retval;
	CPC_GET_CONFIG_FILE("CPC_DISK=%s", disk);
	CPC_GET_CONFIG_FILE("CPC_DOMAIN=%s", domain);
	CPC_GET_CONFIG_FILE("CPC_INTERFACE=%s", interface);
	CPC_GET_CONFIG_FILE("CPC_KBD=%s", kbd);
	CPC_GET_CONFIG_FILE("CPC_LOCALE=%s", locale);
	CPC_GET_CONFIG_FILE("CPC_MIRROR=%s", mirror);
	CPC_GET_CONFIG_FILE("CPC_NAME=%s", name);
	CPC_GET_CONFIG_FILE("CPC_NTPSERVER=%s", ntp_server);
	CPC_GET_CONFIG_FILE("CPC_PACKAGES=%s", packages);
	replace_space(cpc->packages);
	CPC_GET_CONFIG_FILE("CPC_SUITE=%s", suite);
	CPC_GET_CONFIG_FILE("CPC_TZONE=%s", tzone);
	CPC_GET_CONFIG_FILE("CPC_URL=%s", url);
/* Cannot account for idiocy = should NOT put passwords in clear text */
	CPC_GET_CONFIG_FILE("CPC_RPASS=%s", rpass);
	CPC_GET_CONFIG_FILE("CPC_UPASS=%s", upass);
	fclose(config);
	free(file);
	return retval;
#undef CPC_GET_CONFIG_FILE
}

static int
parse_cpc_environment(cpc_config_s *cpc)
{
	char *envar;
	int retval = 0;

#ifndef CPC_GET_ENVIRON
# define CPC_GET_ENVIRON(environ, vari) { \
	if ((envar = getenv(environ)) != NULL) \
		snprintf(cpc->vari, RBUFF_S, "%s", envar); \
} 
#endif
	CPC_GET_ENVIRON("CPC_DISK", disk)
	CPC_GET_ENVIRON("CPC_DOMAIN", domain)
	CPC_GET_ENVIRON("CPC_INTERFACE", interface)
	CPC_GET_ENVIRON("CPC_KBD", kbd)
	CPC_GET_ENVIRON("CPC_LOCALE", locale)
	CPC_GET_ENVIRON("CPC_MIRROR", mirror)
	CPC_GET_ENVIRON("CPC_NAME", name)
	CPC_GET_ENVIRON("CPC_NTPSERVER", ntp_server)
	CPC_GET_ENVIRON("CPC_PACKAGES", packages)
	replace_space(cpc->packages);
	CPC_GET_ENVIRON("CPC_SUITE", suite)
	CPC_GET_ENVIRON("CPC_TZONE", tzone)
/* Probablay even worse than having them in a config file :( */
	CPC_GET_ENVIRON("CPC_RPASS", rpass)
	CPC_GET_ENVIRON("CPC_UPASS", upass)
	return retval;
#undef CPC_GET_ENVIRON
}

static void
fill_default_cpc_config_values(cpc_config_s *cpc)
{
	uid_t uid;
	struct passwd *user;

	uid = getuid();
	if (!(user = getpwuid(uid))) {
		fprintf(stderr, "getpwuid: %s\n", strerror(errno));
		snprintf(cpc->uname, RBUFF_S, "debian");
		snprintf(cpc->user, RBUFF_S, "Debian User");
		snprintf(cpc->uid, RBUFF_S, "1000");
	} else {
		snprintf(cpc->uname, RBUFF_S, "%s", user->pw_name);
		snprintf(cpc->user, RBUFF_S, "%s", user->pw_gecos);
		snprintf(cpc->uid, RBUFF_S, "%d", uid);
	}
	sprintf(cpc->disk, "/dev/vda");
	sprintf(cpc->domain, "mydomain.lan");
	sprintf(cpc->name, "debian");
	sprintf(cpc->interface, "auto");
	sprintf(cpc->kbd, "gb");
	sprintf(cpc->locale, "en_GB");
	sprintf(cpc->mirror, "mirror.ox.ac.uk");
	sprintf(cpc->ntp_server, "0.uk.pool.ntp.org");
	sprintf(cpc->url, "/debian");
	sprintf(cpc->rpass, "hackmebaby");
	sprintf(cpc->suite, "stable");
	sprintf(cpc->tzone, "UTC");
	sprintf(cpc->upass, "r00tm3");
/* This should really be in the config NOT HERE! */
	sprintf(cpc->packages, "openssh-server less locate");
	cpc->add_root = cpc->add_user = cpc->utc = cpc->ntp = 1;
}

static void
build_preseed(cpc_config_s *cpc)
{
	string_len_s *output;

	if (!(output = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "output in output_preseed");
	init_string_len(output);
	add_header(output);
	add_locale(output, cpc);
	add_network(output, cpc);
	add_mirror(output, cpc);
	add_account(output, cpc);
	add_clock_and_ntp(output, cpc);
	add_partitions(output, cpc);
	add_apt(output, cpc);
	add_final(output, cpc);
	printf("%s\n", output->string);
	clean_string_len(output);
}

static void
add_header(string_len_s *pre)
{
	snprintf(pre->string, BUFF_S, "\
### Preseed config\n\
## Created by cpc\n\
## Inspired by https://www.debian.org/releases/wheezy/example-preseed.txt\n\
\n");
	pre->size = strlen(pre->string);
}

static void
add_locale(string_len_s *pre, cpc_config_s *cpc)
{
	snprintf(pre->string + pre->size, RBUFF_S, "\
### Locale config\n\
d-i console-setup/ask_detect boolean false\n\
d-i debian-installer/locale string %s\n\
d-i keyboard-configuration/xkb-keymap select %s\n\
\n", cpc->locale, cpc->kbd);
	pre->size = strlen(pre->string);
}

static void
add_network(string_len_s *pre, cpc_config_s *cpc)
{
	char *buffer;
	size_t size;

	if ((asprintf(&buffer, "\
### Network config\n\
d-i netcfg/enable boolean true\n\
d-i netcfg/choose_interface select %s\n\
d-i netcfg/disable_dhcp boolean false\n\
d-i netcfg/get_hostname string %s\n\
d-i netcfg/get_domain string %s\n\
d-i netcfg/wireless_wep string\n\
d-i hw-detect/load_firmware boolean true\n\
\n", cpc->interface, cpc->name, cpc->domain)) == -1)
		report_error(MALLOC_FAIL, "buffer in add_network");
	size = strlen(buffer);
	if ((pre->size + size) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	pre->size += size;
	free(buffer);
}

static void
add_mirror(string_len_s *pre, cpc_config_s *cpc)
{
	char *buffer, *proxy = NULL;
	size_t size, psize = 0, tsize;

	if ((asprintf(&buffer, "\
### Mirror configuration\n\
d-i mirror/country string manual\n\
d-i mirror/http/hostname string %s\n\
d-i mirror/http/directory string %s\n\
d-i mirror/suite string %s\n\
", cpc->mirror, cpc->url, cpc->suite)) == -1) 
		report_error(MALLOC_FAIL, "buffer in add_mirror");
	size = strlen(buffer);
	if (strlen(cpc->proxy) > 0) {
		if ((asprintf(&proxy, "\
d-i mirror/http/proxy string %s\n\n\
", cpc->proxy)) == -1)
			report_error(MALLOC_FAIL, "proxy in add_mirror");
		psize = strlen(proxy);
	} else {
		if ((asprintf(&proxy, "\
d-i mirror/http/proxy string\n\n\
")) == -1)
			report_error(MALLOC_FAIL, "proxy in add_mirror");
		psize = strlen(proxy);
	}
	tsize = pre->size + size + psize;
	if ((tsize) >= pre->len)
		resize_string_buff(pre);
	tsize = size + psize;
	snprintf(pre->string + pre->size, tsize + 1, "%s%s", buffer, proxy);
	pre->size += tsize;
	free(buffer);
	free(proxy);
}

static void
add_account(string_len_s *pre, cpc_config_s *cpc)
{
	if (cpc->add_root > 0) {
		add_root_account(pre, cpc);
	} else {
		if (cpc->add_user == 0) {
			clean_cpc_config(cpc);
			clean_string_len(pre);
			fprintf(stderr, "\
You must create either a root account or user account!\n");
			exit(1);
		}
		add_no_root_account(pre);
	}
	if (cpc->add_user > 0)
		add_user_account(pre, cpc);
}

static void
add_root_account(string_len_s *pre, cpc_config_s *cpc)
{
	char *buffer;
	size_t size;
	if (cpc->encrypt_rpass > 0) {
		if ((asprintf(&buffer, "\
### Root account\n\
d-i passwd/root-password-crypted password %s\n", cpc->rpass)) == -1)
			report_error(MALLOC_FAIL, "buffer in add_root_account");
	} else {
		if ((asprintf(&buffer, "\
### Root account\n\
d-i passwd/root-password password %s\n\
d-i passwd/root-password-again password %s\n\
\n", cpc->rpass, cpc->rpass)) == -1)
			report_error(MALLOC_FAIL, "buffer in add_root_account");
	}
	size = strlen(buffer);
	if ((pre->size + size) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	pre->size += size;
	free(buffer);
}

static void
add_no_root_account(string_len_s *pre)
{
	char *buffer;
	size_t size;

	if ((asprintf(&buffer, "\
### No root account config\n\
d-i passwd/root-login boolean false\n\
\n")) == -1)
		report_error(MALLOC_FAIL, "buffer in add_no_root_account");
	size = strlen(buffer);
	if ((pre->size + size) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	pre->size += size;
	free(buffer);
}

static void
add_user_account(string_len_s *pre, cpc_config_s *cpc)
{
	char *buffer = NULL, *pass = NULL, *uid = NULL, *groups = NULL;
	size_t size;

	if ((asprintf(&buffer, "\
### User config\n\
d-i passwd/user-fullname string %s\n\
d-i passwd/username string %s\n\
", cpc->user, cpc->uname)) == -1)
		report_error(MALLOC_FAIL, "buffer in add_user_account");
	if (cpc->encrypt_upass > 1) {
		if ((asprintf(&pass, "\
d-i passwd/user-password-crypted password %s\n\
", cpc->upass)) == -1) 
			report_error(MALLOC_FAIL, "pass in add_user_account");
	} else {
		if ((asprintf(&pass, "\
d-i passwd/user-password password %s\n\
d-i passwd/user-password-again password %s\n\
", cpc->upass, cpc->upass)) == -1)
			report_error(MALLOC_FAIL, "pass in add_user_account");
	}
	if (strlen(cpc->uid) > 0)
		if ((asprintf(&uid, "\
d-i passwd/user-uid string %s\n\
", cpc->uid)) == -1) 
			report_error(MALLOC_FAIL, "uid in add_user_account");
	if (strlen(cpc->ugroups) > 0) {
		if ((asprintf(&groups, "\
d-i passwd/user-default-groups string %s\n\
", cpc->ugroups)) == -1)
			report_error(MALLOC_FAIL, "ugroups in add_user_account");
	}
	size = strlen(buffer);
	size += strlen(pass);
	if ((pre->size + size) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s%s", buffer, pass);
	pre->size += size;
	if (uid) {
		size = strlen(uid);
		if ((pre->size + size) >= pre->len)
			resize_string_buff(pre);
		snprintf(pre->string + pre->size, size + 1, "%s", uid);
		pre->size += size;
	}
	if (groups) {
		size = strlen(groups);
		if ((pre->size + size) >= pre->len)
			resize_string_buff(pre);
		snprintf(pre->string + pre->size, size + 1, "%s", groups);
		pre->size += size;
	}
	if (groups)
		free(groups);
	if(uid)
		free(uid);
	free(pass);
	free(buffer);
}

static void
add_clock_and_ntp(string_len_s *pre, cpc_config_s *cpc)
{
	char *ntp = NULL, *tzone, *utc;
	size_t nsize, tsize, usize, size;

	nsize = tsize = usize = 0;
	if (cpc->utc > 0) {
		if ((asprintf(&utc, "\n\
### Clock, timezone and optionally ntp setup\n\
d-i clock-setup/utc boolean true\n\
")) == -1)
			report_error(MALLOC_FAIL, "utc in add_clock_and_ntp");
	} else {
		if ((asprintf(&utc, "\n\
### Clock, timezone and optionally ntp setup\n\
d-i clock-setup/utc boolean false\n\
")) == -1)
			report_error(MALLOC_FAIL, "utc in add_clock_and_ntp");
	}
	usize = strlen(utc);
	if ((asprintf(&tzone, "\
d-i time/zone string %s\n\
", cpc->tzone)) == -1)
		report_error(MALLOC_FAIL, "tzone in add_clock_and_ntp");
	tsize = strlen(tzone);
	if (cpc->ntp > 0) {
		if ((asprintf(&ntp, "\
d-i clock-setup/ntp boolean true\n\
d-i clock-setup/ntp-server string %s\n\
\n", cpc->ntp_server)) == -1)
			report_error(MALLOC_FAIL, "ntp in add_clock_and_ntp");
	} else {
		if ((asprintf(&ntp, "\
d-i clock-setup/ntp boolean false\n\
\n")) == -1)
			report_error(MALLOC_FAIL, "ntp in add_clock_and_ntp");
	}
	nsize = strlen(ntp);
	size = nsize + tsize + usize;
	if ((pre->size + size) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s%s%s", utc, tzone, ntp);
	pre->size += size;
	free(ntp);
	free(tzone);
	free(utc);
}

static void
add_partitions(string_len_s *pre, cpc_config_s *cpc)
{
/* Starting with the most simeple partition structure we can get away with
   Later we can add configuration file options for separate partions
   and even LVM / crypto. But for now, all in one big one :) */

	char *part;
	size_t psize;

	if ((asprintf(&part, "\
### Partition setup\n\
d-i partman-auto/disk string %s\n\
d-i partman-auto/method string regular\n\
d-i partman-lvm/device_remove_lvm boolean true\n\
d-i partman-md/device_remove_md boolean true\n\
d-i partman-auto/choose_recipe select atomic\n\
d-i partman-partitioning/confirm_write_new_label boolean true\n\
d-i partman/choose_partition select finish\n\
d-i partman/confirm boolean true\n\
d-i partman/confirm_nooverwrite boolean true\n\
d-i partman/mount_style select uuid\n\
d-i grub-installer/only_debian boolean true\n\
\n", cpc->disk)) == -1)
		report_error(MALLOC_FAIL, "part in add_partitions");
	psize = strlen(part);
	if ((pre->size + psize) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, psize + 1, "%s", part);
	pre->size += psize;
	free(part);
}
/*
static void
add_no_recommends(string_len_s *pre, cpc_config_s *cpc)
{
	char *rec;
	size_t rsize;
	if (cpc->recommends == 0)
		return;
	if ((asprintf(&rec, "\
d-i base-installer/install-recommends boolean false\n\
\n")) == -1)
		report_error(MALLOC_FAIL, "rec in add_no_recommends");
	rsize = strlen(rec);
	if ((pre->size + rsize) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, rsize + 1, "%s", rec);
	pre->size += rsize;
	free(rec);
} */

static void
add_apt(string_len_s *pre, cpc_config_s *cpc)
{
	char *apt, *pack;
	size_t asize = 0, psize = 0;

	if ((asprintf(&apt, "\
### Apt setup\n\
# You can choose to install non-free and contrib software.\n\
d-i apt-setup/non-free boolean true\n\
d-i apt-setup/contrib boolean true\n\
d-i apt-setup/services-select multiselect security, updates\n\
d-i apt-setup/security_host string security.debian.org\n\
\n\
### Package selection\n\
tasksel tasksel/first multiselect standard\n\
popularity-contest popularity-contest/participate boolean false\n\
")) == -1)
		report_error(MALLOC_FAIL, "apt in add_apt");
	asize = strlen(apt);
	if (strlen(cpc->packages) > 0) {
		if ((asprintf(&pack, "\
d-i pkgsel/include string %s\n\
\n", cpc->packages)) == -1)
			report_error(MALLOC_FAIL, "pack in add_apt");
		psize = strlen(pack);
	}
	if ((pre->size + asize) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, asize + 1, "%s", apt);
	pre->size += asize;
	free(apt);
	if (psize > 0) {
		if ((pre->size + psize) >= pre->len)
			resize_string_buff(pre);
		snprintf(pre->string + pre->size, psize + 1, "%s", pack);
		pre->size += psize;
		free(pack);
	}
}

static void
add_final(string_len_s *pre, cpc_config_s *cpc)
{
	char *final, *post;
	size_t fsize, psize;

	if ((asprintf(&final, "\
### Finish off the install\n\
d-i finish-install/reboot_in_progress note\n\
")) == -1)
		report_error(MALLOC_FAIL, "final in add_final");
	fsize = strlen(final);
	if ((pre->size + fsize) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, fsize + 1, "%s", final);
	pre->size += fsize;
	free(final);
	if (cpc->post > 0) {
		if ((asprintf(&post, "\
d-i preseed/late_command string %s\n\
\n", cpc->pinstall)) == -1) 
			report_error(MALLOC_FAIL, "post in add_final");
		psize = strlen(post);
		if ((pre->size + psize) >= pre->len)
			resize_string_buff(pre);
		snprintf(pre->string + pre->size, psize + 1, "%s", post);
		pre->size += psize;
		free(post);
	}
}

static void
init_cpc_config(cpc_config_s *cpc)
{
	memset(cpc, 0, sizeof(cpc_config_s));
	if (!(cpc->disk = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->disk init");
	if (!(cpc->domain = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->domain init");
	if (!(cpc->interface = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->interface init");
	if (!(cpc->file = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->file init");
	if (!(cpc->kbd = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->kdb init");
	if (!(cpc->locale = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->locale init");
	if (!(cpc->mirror = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->mirror init");
	if (!(cpc->name = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->name init");
	if (!(cpc->ntp_server = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->ntp_server init");
	if (!(cpc->packages = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->packages init");
	if (!(cpc->pinstall = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->pinstall init");
	if (!(cpc->proxy = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->proxy init");
	if (!(cpc->rpass = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->rpass init");
	if (!(cpc->suite = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->suite init");
	if (!(cpc->tzone = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->tzone init");
	if (!(cpc->ugroups = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->ugroups init");
	if (!(cpc->uid = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->uid init");
	if (!(cpc->uname = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->uname init");
	if (!(cpc->upass = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->upass init");
	if (!(cpc->url = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->url init");
	if (!(cpc->user = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->user init");
}

static void
clean_cpc_config(cpc_config_s *cpc)
{
	if (cpc) {
		if (cpc->disk)
			free(cpc->disk);
		if (cpc->domain)
			free(cpc->domain);
		if (cpc->interface)
			free(cpc->interface);
		if (cpc->file)
			free(cpc->file);
		if (cpc->kbd)
			free(cpc->kbd);
		if (cpc->locale)
			free(cpc->locale);
		if (cpc->mirror)
			free(cpc->mirror);
		if (cpc->name)
			free(cpc->name);
		if (cpc->ntp_server)
			free(cpc->ntp_server);
		if (cpc->packages)
			free(cpc->packages);
		if (cpc->pinstall)
			free(cpc->pinstall);
		if (cpc->proxy)
			free(cpc->proxy);
		if (cpc->rpass)
			free(cpc->rpass);
		if (cpc->suite)
			free(cpc->suite);
		if (cpc->tzone)
			free(cpc->tzone);
		if (cpc->ugroups)
			free(cpc->ugroups);
		if (cpc->uid)
			free(cpc->uid);
		if (cpc->uname)
			free(cpc->uname);
		if (cpc->upass)
			free(cpc->upass);
		if (cpc->url)
			free(cpc->url);
		if (cpc->user)
			free(cpc->user);
	} else {
		return;
	}
	free(cpc);
}

static void
replace_space(char *packages)
{
	char *s = NULL;

	while ((s = strchr(packages, ',')) != NULL)
		*s = ' ';
}
