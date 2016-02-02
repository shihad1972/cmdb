/* 
 *
 *  ckc: create kickstart config
 *  Copyright (C) 2015  Iain M Conochie <iain-AT-thargoid.co.uk> 
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
 *  ckc.c
 *
 *  Main source file for ckc program
 */
#define _GNU_SOURCE
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <getopt.h>
#include "cmdb.h"

typedef struct ckc_config_s {
	char *country;
	char *disk;
	char *domain;
	char *file;
	char *host;
	char *ip;
	char *language;
	char *packages;
	char *timezone;
	char *url;
	int action;
} ckc_config_s;

typedef struct ckc_package_s {
	char *package;
	struct ckc_package_s *next;
} ckc_package_s;

static int
parse_ckc_command_line(ckc_config_s *ckc, int argc, char *argv[]);

static void
ckc_build_package_list(ckc_package_s **list, ckc_config_s *ckc);

static void
clean_ckc_config(ckc_config_s *ckc);

static void
clean_ckc_package(ckc_package_s *pack);

static char *
ckc_get_ip_member(char *ip, int count);

static int
ckc_action(ckc_config_s *ckc, char *argv[]);

static void
ckc_split_ip(char *netinf[], char *ip);

static void
output_kickstart(ckc_config_s *ckc);

int
main(int argc, char *argv[])
{
	int retval;
	ckc_config_s *ckc;

	retval = 0;
	ckc = cmdb_malloc(sizeof (ckc_config_s), "ckc in main()");
	if (argc > 1)
		if (parse_ckc_command_line(ckc, argc, argv) != 0)
			fprintf(stderr, "Problem with command line options\n");
	if (ckc->action > 0) {
		retval = ckc_action(ckc, argv);
		goto cleanup;
	}
	output_kickstart(ckc);

	cleanup:
		clean_ckc_config(ckc);
		return retval;
}

static int
parse_ckc_command_line(ckc_config_s *ckc, int argc, char *argv[])
{
	int opt, index, retval;
	struct option long_options[] = {
		{"help",	no_argument,		0,	'h'},
		{"version",	no_argument,		0,	'v'},
		{"domain",	required_argument,	0,	'd'},
		{"file",	required_argument,	0,	'f'},
		{"ip",		required_argument,	0,	'i'},
		{"disk",	required_argument,	0,	'k'},
		{"language",	required_argument,	0,	'l'},
		{"hostname",	required_argument,	0,	'n'},
		{"packages",	required_argument,	0,	'p'},
		{"timezone",	required_argument,	0,	't'},
		{"url",		required_argument,	0,	'u'},
		{"country",	required_argument,	0,	'y'},
		{0,0,0,0}
	};

	retval = 0;
	while ((opt = getopt_long(argc, argv, "d:f:hi:k:l:n:p:t:u:vy:", long_options, &index)) != -1) {
		if (opt == 'd') {
			if (!(ckc->domain = strndup(optarg, RBUFF_S - 1)))
				report_error(MALLOC_FAIL, "ckc->domain in parse_ckc_command_line");
		} else if (opt == 'f') {
			if (!(ckc->file = strndup(optarg, RBUFF_S - 1)))
				report_error(MALLOC_FAIL, "ckc->file in parse_ckc_command_line");
		} else if (opt == 'i') {
			if (!(ckc->ip = strndup(optarg, TBUFF_S - 1)))
				report_error(MALLOC_FAIL, "ckc->ip in parse_ckc_command_line");
		} else if (opt == 'k') {
			if (!(ckc->disk = strndup(optarg, RBUFF_S - 1)))
				report_error(MALLOC_FAIL, "ckc->disk in parse_ckc_command_line");
		} else if (opt == 'l') {
			if (!(ckc->language = strndup(optarg, MAC_S - 1)))
				report_error(MALLOC_FAIL, "ckc->language in parse_ckc_command_line");
		} else if (opt == 'n') {
			if (!(ckc->host = strndup(optarg, HOST_S - 1)))
				report_error(MALLOC_FAIL, "ckc->host in parse_ckc_command_line");
		} else if (opt == 'p') {
			if (!(ckc->packages = strndup(optarg, RBUFF_S - 1)))
				report_error(MALLOC_FAIL, "ckc->packages in parse_ckc_command_line");
		} else if (opt == 't') {
			if (!(ckc->timezone = strndup(optarg, MAC_S - 1)))
				report_error(MALLOC_FAIL, "ckc->timezone in parse_ckc_command_line");
		} else if (opt == 'u') {
			if (!(ckc->url = strndup(optarg, TBUFF_S - 1)))
				report_error(MALLOC_FAIL, "ckc->url in parse_ckc_command_line");
		} else if (opt == 'y') {
			if (!(ckc->country = strndup(optarg, RANGE_S - 1)))
				report_error(MALLOC_FAIL, "ckc->country in parse_ckc_command_line");
		} else if (opt == 'h') {
			ckc->action = HELP;
		} else if (opt == 'v') {
			ckc->action = VERS;
		} else {
			fprintf(stderr, "Unknown option %c\n", opt);
			retval = 1;
		}
	}
	return retval;
}

static void
ckc_build_package_list(ckc_package_s **list, ckc_config_s *ckc)
{
	char *package, *tmp, *line;
	size_t len;
	ckc_package_s *pack, *new;

	if (ckc->packages)
		package = strndup(ckc->packages, RBUFF_S);
	else
		return;
	line = package;
	len = strlen(package);
	if ((tmp = strchr(line, ','))) {
		*tmp = '\0';
		tmp++;
	}
	new = cmdb_malloc(sizeof(ckc_package_s), "new in ckc_build_package_list");
	*list = new;
	new->package = strndup(line, strlen(line));
	line = tmp;
	if (line) {
		while ((tmp = strchr(line, ','))) {
			*tmp = '\0';
			tmp++;
			new = cmdb_malloc(sizeof(ckc_package_s), "new in ckc_build_package_list");
			new->package = strndup(line, strlen(line));
			pack = *list;
			while (pack->next)
				pack = pack->next;
			pack->next = new;
			line = tmp;
		}
		new = cmdb_malloc(sizeof(ckc_package_s), "new in ckc_build_package_list");
		new->package = strndup(line, strlen(line));
		pack = *list;
		while (pack->next)
			pack = pack->next;
		pack->next = new;
	}
	cmdb_free(package, len);
}

static void
clean_ckc_config(ckc_config_s *ckc)
{
	if (!(ckc))
		return;
	if (ckc->country)
		cmdb_free(ckc->country, strlen(ckc->country));
	if (ckc->domain)
		cmdb_free(ckc->domain, strlen(ckc->domain));
	if (ckc->file)
		cmdb_free(ckc->file, strlen(ckc->file));
	if (ckc->ip)
		cmdb_free(ckc->ip, strlen(ckc->ip));
	if (ckc->host)
		cmdb_free(ckc->host, strlen(ckc->host));
	if (ckc->disk)
		cmdb_free(ckc->disk, strlen(ckc->disk));
	if (ckc->language)
		cmdb_free(ckc->language, strlen(ckc->language));
	if (ckc->packages)
		cmdb_free(ckc->packages, strlen(ckc->packages));
	if (ckc->timezone)
		cmdb_free(ckc->timezone, strlen(ckc->timezone));
	if (ckc->url)
		cmdb_free(ckc->url, strlen(ckc->url));
	cmdb_free(ckc, sizeof(ckc_config_s));
}

static void
clean_ckc_package(ckc_package_s *pack)
{
	ckc_package_s *list, *next;

	if (pack)
		list = pack;
	else
		return;
	while (list) {
		next = list->next;
		if (list->package)
			cmdb_free(list->package, strlen(list->package));
		cmdb_free(list, sizeof(ckc_package_s));
		list = next;
	}
}

static int
ckc_action(ckc_config_s *ckc, char *argv[])
{
	if (ckc->action == HELP) {
		display_ckc_usage();
		return HELP;
	} else if (ckc->action == VERS) {
		display_version(argv[0]);
		return 0;
	} else {
		fprintf(stderr, "Unknown action: %d\n", ckc->action);
		return WRONG_ACTION;
	}
	return 0;
}

static char *
ckc_get_ip_member(char *ip, int no)
{
	char *ret, *tmp;

	if (!(ret = strndup(ip, RANGE_S)))
		report_error(MALLOC_FAIL, "ret in ckc_get_ip_member");
	if ((no == 2) || (no == 3)) {
		tmp = strrchr(ret, '.');
		tmp++;
		*tmp = '1';
		tmp++;
		*tmp = '\0';
	} else if (no == 1) {
		snprintf(ret, RANGE_S, "255.255.255.0");
	} else {
		ret = NULL;
	}
	return ret;
}

static void
ckc_split_ip(char *net[], char *ip)
{
	int i = 0;
	char *tmp, *info, *member;

	if (!(info = strndup(ip, TBUFF_S)))
		report_error(MALLOC_FAIL, "info in ckc_split_ip");
	member = info;
	while (i < 4) {
		if (member) {
			if ((tmp = strchr(member, ','))) {
				*tmp = '\0';
				tmp++;
			}
		}
		if (member)
			net[i] = strndup(member, RANGE_S - 1);
		else if (!(tmp))
			net[i] = ckc_get_ip_member(net[0], i);
		i++;
		member = tmp;
	}
	cmdb_free(info, strlen(info));
}

static void
output_kickstart(ckc_config_s *ckc)
{
	int i;
	char *netinfo[4];
	FILE *output;
	ckc_package_s *pack, *list;

	pack = NULL;
	if (ckc->file) {
		if (!(output = fopen(ckc->file, "w"))) {
			fprintf(stderr, "Cannot write to file %s. Using stdout\n", ckc->file);
			output = stdout;
		}
	} else {
		output = stdout;
	}
	if (ckc->ip)
		ckc_split_ip(netinfo, ckc->ip);
	else
		memset(netinfo, 0, sizeof(char *) * 4);
	if (!(ckc->host))
		ckc->host = strndup("kickstart", RANGE_S);
	if (!(ckc->domain))
		ckc->domain = strndup("lan", COMM_S);
	if (ckc->packages)
		ckc_build_package_list(&pack, ckc);
	fprintf(output, "\
# Kickstart file create by ckc %s\n\
\n\
auth --useshadow --enablesha512\n\
text\n\
firewall --disabled\n\
firstboot --disable\n", VERSION);
	if (ckc->disk)
		fprintf(output, "\
bootloader --location=mbr --driveorder=/dev/%s\n", ckc->disk);
	else
		fprintf(output, "\
bootloader --location=mbr --driveorder=/dev/sda\n");
	if (ckc->country)
		fprintf(output, "\
keyboard %s\n", ckc->country);
	else
		fprintf(output, "\
keyboard uk\n");
	if (ckc->language)
		fprintf(output, "\
lang %s\n", ckc->language);
	else
		fprintf(output, "\
lang en_GB\n");
	fprintf(output, "\
logging --level=info\n\
reboot\n");
	fprintf(output, "\
rootpw --iscrypted $6$YuyiUAiz$8w/kg1ZGEnp0YqHTPuz2WpveT0OaYG6Vw89P.CYRAox7CaiaQE49xFclS07BgBHoGaDK4lcJEZIMs8ilgqV84.\
\n");
	fprintf(output, "\
selinux --disabled\n\
skipx\n\
install\n");
	if (ckc->timezone)
		fprintf(output, "\
timezone %s\n", ckc->timezone);
	else
		fprintf(output, "\
timezone Europe/London\n");
	fprintf(output, "\
\n\
# Partition Information\n\
zerombr\n\
clearpart --all --initlabel\n");
	fprintf(output, "\
part /boot --asprimary --fstype \"ext4\" --size=512\n\
part swap --asprimary --fstype \"swap\" --size=1024\n\
part / --asprimary --fstype \"ext4\" --size=10 --grow\n");
	if (ckc->url)
		fprintf(output, "\
url --url=%s\n", ckc->url);
	else
		fprintf(output, "\
# Default URL\n\
url --url=http://mirrors.melbourne.co.uk/centos/6/os/i386\n");
	if (ckc->ip)
		fprintf(output, "\
network --bootproto=static --device=eth0 --ip %s --netmask %s --gateway %s\
 --nameserver %s --hostname %s.%s --onboot=on\n",
 netinfo[0], netinfo[1], netinfo[2], netinfo[3], ckc->host, ckc->domain);
	else
		fprintf(output, "\
network --bootproto=dhcp --hostname %s.%s\n", ckc->host, ckc->domain);
	fprintf(output, "\
\n\
%%packages\n\
\n\
@base\n\
openssh-server\n\
less\n\
ntp\n\
sudo\n\
gpm\n\
logwatch\n\
sysstat\n\
postfix\n");
	list = pack;
	while (list) {
		fprintf(output, "%s\n", list->package);
		list = list->next;
	}
	fprintf(output, "\
\n\
%%end\n\
\n");
	if (output != stdout)
		if (fclose(output) != 0)
			fprintf(stderr, "\
Error in closing %s: %s\n", ckc->file, strerror(errno));
	for (i = 0; i < 4; i++) {
		if (netinfo[i])
			cmdb_free(netinfo[i], strlen(netinfo[i]));
	}
	clean_ckc_package(pack);
}

