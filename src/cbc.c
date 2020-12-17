/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc.c
 * 
 *  main function for the cbc program
 * 
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#ifdef HAVE_GETOPT_H
# define _GNU_SOURCE
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include "cmdb_cbc.h"
#include "build.h"

static int
query_config();

static int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb);

static int
validate_cbc_comm_line(cbc_comm_line_s *cml);

static void
clean_cbc_comm_line(cbc_comm_line_s *cml);

int
main(int argc, char *argv[])
{
	int retval = NONE;
	ailsa_cmdb_s *cmc = ailsa_calloc(sizeof(ailsa_cmdb_s), "cmc in main");
	cbc_comm_line_s *cml = ailsa_calloc(sizeof(cbc_comm_line_s), "cml in main");

	if ((retval = parse_cbc_command_line(argc, argv, cml)) != 0) {
		ailsa_clean_cmdb(cmc);
		clean_cbc_comm_line(cml);
		display_command_line_error(retval, argv[0]);
	}
	if (cml->action == CMDB_QUERY)
		retval = query_config();
	else
		parse_cmdb_config(cmc);
	if (cml->action == CMDB_DISPLAY)
		retval = display_build_config(cmc, cml);
	else if (cml->action == CMDB_LIST)
		list_build_servers(cmc);
	else if (cml->action == CMDB_WRITE)
		retval = write_build_config(cmc, cml);
	else if (cml->action == CMDB_ADD)
		retval = create_build_config(cmc, cml);
	else if (cml->action == CMDB_MOD)
		retval = modify_build_config(cmc, cml);
	else if (cml->action == CMDB_RM)
		retval = remove_build_config(cmc, cml);
	else if (cml->action == CMDB_DEFAULT)
		retval = view_defaults_for_cbc(cmc, cml);
	else if (cml->action == CMDB_QUERY)
		;
	else
		printf("Case %d not implemented yet\n", cml->action);
	ailsa_clean_cmdb(cmc);
	clean_cbc_comm_line(cml);
	if (retval == AILSA_DISPLAY_USAGE)
		retval = NONE;
	exit(retval);
}

static int
query_config()
{
	int retval = NONE;
#if defined HAVE_MYSQL
# if defined HAVE_SQLITE3
	printf("both\n");
# else
	printf("mysql\n");
# endif
#elif defined HAVE_SQLITE3
	printf("sqlite\n");
#else
	printf("none\n");
#endif
	return retval;
}

static int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb)
{
	const char *optstr = "ab:de:ghj:k:lmn:o:p:qrs:t:uvwx:y";
	int retval, opt;
#ifdef HAVE_GETOPT_H
	int index;
	struct option lopts[] = {
		{"add",			no_argument,		NULL,	'a'},
		{"build-domain",	required_argument,	NULL,	'b'},
		{"domain",		required_argument,	NULL,	'b'},
		{"locale",		required_argument,	NULL,	'e'},
		{"display",		no_argument,		NULL,	'd'},
		{"remove-ip",		no_argument,		NULL,	'g'},
		{"help",		no_argument,		NULL,	'h'},
		{"hard-disk",		required_argument,	NULL,	'j'},
		{"network-card",	required_argument,	NULL,	'k'},
		{"list",		no_argument,		NULL,	'l'},
		{"modify",		no_argument,		NULL,	'm'},
		{"name",		required_argument,	NULL,	'n'},
		{"operating-system",	required_argument,	NULL,	'o'},
		{"partition-scheme",	required_argument,	NULL,	'p'},
		{"os-version",		required_argument,	NULL,	's'},
		{"architecture",	required_argument,	NULL,	't'},
		{"query",		no_argument,		NULL,	'q'},
		{"remove",		no_argument,		NULL,	'r'},
		{"delete",		no_argument,		NULL,	'r'},
		{"view-default",	no_argument,		NULL,	'u'},
		{"version",		no_argument,		NULL,	'v'},
		{"write",		no_argument,		NULL,	'w'},
		{"commit",		no_argument,		NULL,	'w'},
		{"varient",		required_argument,	NULL,	'x'},
		{"gui",			no_argument,		NULL,	'y'},
		{NULL,			0,			NULL,	0}
	};
	while ((opt = getopt_long(argc, argv, optstr, lopts, &index)) != -1)
#else
	while ((opt = getopt(argc, argv, optstr)) != -1)
#endif // HAVE_GETOPT_H
	{
		if (opt == 'n') {
			cb->name = strndup(optarg, CONFIG_LEN);
		} else if (opt == 'u') {
			cb->action = CMDB_DEFAULT;
		} else if (opt == 'a') {
			cb->action = CMDB_ADD;
		} else if (opt == 'd') {
			cb->action = CMDB_DISPLAY;
		} else if (opt == 'g') {
			cb->removeip = true;
		} else if (opt == 'l') {
			cb->action = CMDB_LIST;
		} else if (opt == 'm') {
			cb->action = CMDB_MOD;
		} else if (opt == 'q') {
			cb->action = CMDB_QUERY;
		} else if (opt == 'r') {
			cb->action = CMDB_RM;
		} else if (opt == 'w') {
			cb->action = CMDB_WRITE;
		} else if (opt == 'b') {
			cb->build_domain = strndup(optarg, CONFIG_LEN);
		} else if (opt == 'e') {
			cb->locale = strndup(optarg, CONFIG_LEN);
		} else if (opt == 'k') {
			cb->netcard = strndup(optarg, HOST_LEN);
		} else if (opt == 'j') {
// We do not check for a starting /dev here.
			cb->harddisk = strndup(optarg, HOST_LEN);
		} else if (opt == 'o') {
			cb->os = strndup(optarg, MAC_LEN);
		} else if (opt == 'p') {
			cb->partition = strndup(optarg, CONFIG_LEN);
		} else if (opt == 't') {
			cb->arch = strndup(optarg, SERVICE_LEN);
		} else if (opt == 's') {
			cb->os_version = strndup(optarg, MAC_LEN);
		} else if (opt == 'x') {
			cb->varient = strndup(optarg, CONFIG_LEN);
		} else if (opt == 'y') {
			cb->gui = 1;
		} else if (opt == 'v') {
			cb->action = AILSA_VERSION;
		} else if (opt == 'h') {
			return AILSA_DISPLAY_USAGE;
		} else {
			printf("Unknown option: %c\n", opt);
			retval = AILSA_DISPLAY_USAGE;
		}
	}
	if ((cb->action == NONE) && 
	 (cb->server == NONE) &&
	 (!(cb->action_type)))
		return AILSA_DISPLAY_USAGE;
	else if (cb->action == AILSA_VERSION)
		return AILSA_VERSION;
	else if (cb->action == CMDB_QUERY)
		return NONE;
	else if (cb->action == NONE)
		return AILSA_NO_ACTION;
	else if ((cb->action != CMDB_LIST) && (cb->action != CMDB_DEFAULT) &&
		 (cb->name == 0))
		return AILSA_NO_NAME_OR_ID;
	if (cb->action == CMDB_ADD) {
		if (!(cb->harddisk)) {
			ailsa_syslog(LOG_INFO, "No disk provided. Setting to vda");
			cb->harddisk = strdup("vda");
		}
		if (!(cb->netcard)) {
			ailsa_syslog(LOG_INFO, "No network card provided. Setting to eth0");
			cb->netcard = strdup("eth0");
		}
	}
	retval = validate_cbc_comm_line(cb);
	return retval;
}

void
print_cbc_command_line_values(cbc_comm_line_s *cml)
{
	fprintf(stderr, "########\nCommand line Values\n");
	if (cml->action == CMDB_WRITE)
		fprintf(stderr, "Action: Write configuration file\n");
	else if (cml->action == CMDB_DISPLAY)
		fprintf(stderr, "Action: Display configuration\n");
	else if (cml->action == CMDB_ADD)
		fprintf(stderr, "Action: Add configuration for build\n");
	else if (cml->action == CMDB_WRITE)
		fprintf(stderr, "Action: Write build configuration file\n");
	else
		fprintf(stderr, "Action: Unknown!!\n");
	if (cml->config)
		fprintf(stderr, "Config: %s\n", cml->config);
	if (cml->name)
		fprintf(stderr, "Name: %s\n", cml->name);
	if (cml->uuid)
		fprintf(stderr, "UUID: %s\n", cml->uuid);
	fprintf(stderr, "Server ID: %ld\n", cml->server_id);
	fprintf(stderr, "OS ID: %lu\n", cml->os_id);
	if (cml->os)
		fprintf(stderr, "OS: %s\n", cml->os);
	if (cml->os_version)
		fprintf(stderr, "OS Version: %s\n", cml->os_version);
	if (cml->arch)
		fprintf(stderr, "Architecture: %s\n", cml->arch);
	if (cml->locale)
		fprintf(stderr, "Locale: %s\n", cml->locale);
	if (cml->build_domain)
		fprintf(stderr, "Build Domain: %s\n", cml->build_domain);
	if (cml->action_type)
		fprintf(stderr, "Action Type: %s\n", cml->action_type);
	if (cml->partition)
		fprintf(stderr, "Partition: %s\n", cml->partition);
	if (cml->varient)
		fprintf(stderr, "Varient: %s\n", cml->varient);
	
	fprintf(stderr, "\n");
}

static int
validate_cbc_comm_line(cbc_comm_line_s *cml)
{
	if (cml->uuid)
		if (ailsa_validate_input(cml->uuid, UUID_REGEX) < 0)
			return UUID_INPUT_INVALID;
	if (cml->name)
		if (ailsa_validate_input(cml->name, NAME_REGEX) < 0)
			return SERVER_NAME_INVALID;
	if (cml->os)
		if (ailsa_validate_input(cml->os, NAME_REGEX) < 0)
			return OS_INVALID;
	if (cml->os_version)
		if (ailsa_validate_input(cml->os_version, OS_VER_REGEX) < 0)
			return OS_VERSION_INVALID;
	if (cml->partition)
		if (ailsa_validate_input(cml->partition, NAME_REGEX) < 0)
			return PART_SCHEME_INVALID;
	if (cml->varient)
		if (ailsa_validate_input(cml->varient, NAME_REGEX) < 0)
			return VARIENT_INVALID;
	if (cml->build_domain)
		if (ailsa_validate_input(cml->build_domain, DOMAIN_REGEX) < 0)
			return DOMAIN_INPUT_INVALID;
	if (cml->arch)
		if (ailsa_validate_input(cml->arch, CUSTOMER_REGEX) < 0)
			return ARCH_INVALID;
	if (cml->netcard)
		if (ailsa_validate_input(cml->netcard, DEV_REGEX) < 0)
			return NET_CARD_INVALID;
	if (cml->harddisk)
		if (ailsa_validate_input(cml->harddisk, DEV_REGEX) < 0)
			return HARD_DISK_INVALID;
	if (cml->config)
		if (ailsa_validate_input(cml->config, PATH_REGEX) < 0)
			return CONFIG_INVALID;
	if (cml->locale)
		if (ailsa_validate_input(cml->locale, NAME_REGEX) < 0)
			return LOCALE_INVALID;
	return 0;
}

void
clean_cbc_comm_line(cbc_comm_line_s *cml)
{
	if (cml->build_domain)
		my_free(cml->build_domain);
	if (cml->locale)
		my_free(cml->locale);
	if (cml->harddisk)
		my_free(cml->harddisk);
	if (cml->netcard)
		my_free(cml->netcard);
	if (cml->varient)
		my_free(cml->varient);
	if (cml->arch)
		my_free(cml->arch);
	if (cml->os_version)
		my_free(cml->os_version);
	if (cml->os)
		my_free(cml->os);
	if (cml->action_type)
		my_free(cml->action_type);
	if (cml->uuid)
		my_free(cml->uuid);
	if (cml->config)
		my_free(cml->config);
	if (cml->partition)
		my_free(cml->partition);
	if (cml->name)
		my_free(cml->name);
	my_free(cml);
}
