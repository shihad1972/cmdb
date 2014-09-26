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
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "cmdb_cpc.h"
#include "cbc_data.h"
#include "checks.h"

int
main (int argc, char *argv[])
{
	int retval = NONE;
	cpc_config_s *cpc = '\0';

	if (!(cpc = malloc(sizeof(cpc_config_s))))
		report_error(MALLOC_FAIL, "cpc in main");
	init_cpc_config(cpc);
	fill_default_cpc_config_values(cpc);
	if ((retval = parse_cpc_comm_line(argc, argv, cpc)) != 0) {
		clean_cpc_config(cpc);
		report_error(retval, "cpc command line");
	}
	build_preseed(cpc);
	clean_cpc_config(cpc);
	return retval;
}

int
parse_cpc_comm_line(int argc, char *argv[], cpc_config_s *cl)
{
	int opt, retval = NONE;

	while ((opt = getopt(argc, argv, "d:f:n:")) != -1) {
		if (opt == 'd') {
			snprintf(cl->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'f') {
			snprintf(cl->file, RBUFF_S, "%s", optarg);
		} else if (opt == 'n') {
			snprintf(cl->name, RBUFF_S, "%s", optarg);
		} else {
			fprintf(stderr, "Unknown option %c\n", opt);
			retval = DISPLAY_USAGE;
		}
	}
	return retval;
}

void
fill_default_cpc_config_values(cpc_config_s *cpc)
{
	uid_t uid;
	struct passwd *user;

	uid = getuid();
	if (!(user = getpwuid(uid))) {
		clean_cpc_config(cpc);
		error(0, errno, "getpwuid: ");
		return;
	}
	snprintf(cpc->uname, RBUFF_S, "%s", user->pw_name);
	snprintf(cpc->user, RBUFF_S, "%s", user->pw_gecos);
	snprintf(cpc->uid, RBUFF_S, "%d", uid);
	sprintf(cpc->domain, "mydomain.lan");
	sprintf(cpc->name, "debian");
	sprintf(cpc->interface, "auto");
	sprintf(cpc->kbd, "uk");
	sprintf(cpc->locale, "en_GB");
	sprintf(cpc->mirror, "mirror.ox.ac.uk");
	sprintf(cpc->url, "/debian");
	sprintf(cpc->suite, "stable");
	sprintf(cpc->rpass, "hackmebabay1moretime");
	sprintf(cpc->upass, "r00tm3@ga1n");
	cpc->add_root = cpc->add_user = 1;
}

void
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
	printf("%s\n", output->string);
	clean_string_len(output);
}

void
add_header(string_len_s *pre)
{
	snprintf(pre->string, BUFF_S, "\
### Preseed config\n\
## Created by cpc\n\
## Inspired by https://www.debian.org/releases/wheezy/example-preseed.txt\n\
\n");
	pre->size = strlen(pre->string);
}

void
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

void
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
	if (size + pre->size >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	pre->size += size;
	free(buffer);
}

void
add_mirror(string_len_s *pre, cpc_config_s *cpc)
{
	char *buffer, *proxy = '\0';
	size_t size, psize = 0, tsize;

	if ((asprintf(&buffer, "\
### Mirror configuration\n\
d-i mirror/country string manual\n\
d-i mirror/http/hostname string %s\n\
d-i mirror/http/directory string %s\n\
d-i mirror/suite string %s\n\
\n", cpc->mirror, cpc->url, cpc->suite)) == -1) 
		report_error(MALLOC_FAIL, "buffer in add_mirror");
	size = strlen(buffer);
	if (strlen(cpc->proxy) > 0) {
		if ((asprintf(&proxy, "\
d-i mirror/http/proxy string %s\n\
", cpc->proxy)) == -1)
			report_error(MALLOC_FAIL, "proxy in add_mirror");
		psize = strlen(proxy);
	}
	tsize = pre->size + size + psize;
	if ((tsize) >= pre->len)
		resize_string_buff(pre);
	tsize = size + psize;
	if (proxy)
		snprintf(pre->string + pre->size, tsize + 1, "%s%s", buffer, proxy);
	else
		snprintf(pre->string + pre->size, tsize + 1, "%s", buffer);
	pre->size += tsize;
	free(buffer);
	if (proxy)
		free(proxy);
}

void
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

void
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
	if ((pre->size + size) > pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	pre->size += size;
	free(buffer);
}

void
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
	if ((size + pre->size) >= pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s", buffer);
	pre->size += size;
	free(buffer);
}

void
add_user_account(string_len_s *pre, cpc_config_s *cpc)
{
	char *buffer = '\0', *pass = '\0', *uid = '\0', *groups = '\0';
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
	if ((pre->size + size) > pre->len)
		resize_string_buff(pre);
	snprintf(pre->string + pre->size, size + 1, "%s%s", buffer, pass);
	pre->size += size;
	if (uid) {
		size = strlen(uid);
		if ((pre->size + size) > pre->len)
			resize_string_buff(pre);
		snprintf(pre->string + pre->size, size + 1, "%s", uid);
		pre->size += size;
	}
	if (groups) {
		size = strlen(groups);
		if ((pre->size + size) > pre->len)
			resize_string_buff(pre);
		snprintf(pre->string + pre->size, size + 1, "%s", groups);
		pre->size += size;
	}
	free(groups);
	free(uid);
	free(pass);
	free(buffer);
}

void
init_cpc_config(cpc_config_s *cpc)
{
	memset(cpc, 0, sizeof(cpc_config_s));
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
	if (!(cpc->rpass = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->rpass init");
	if (!(cpc->proxy = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->proxy init");
	if (!(cpc->suite = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->suite init");
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

void
clean_cpc_config(cpc_config_s *cpc)
{
	if (cpc) {
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
		if (cpc->rpass)
			free(cpc->rpass);
		if (cpc->proxy)
			free(cpc->proxy);
		if (cpc->suite)
			free(cpc->suite);
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

