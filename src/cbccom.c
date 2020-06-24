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
 *  cbccom.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc program
 * 
 */

#include <config.h>
#include <configmake.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
#ifdef HAVE_WORDEXP_H
# include <wordexp.h>
#endif // HAVE_WORDEXP_H
#ifdef HAVE_GETOPT_H
# include <getopt.h>
#endif // HAVE_GETOPT_H
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#ifdef HAVE_DNSA
# include "cmdb_dnsa.h"
#endif // HAVE_DNSA

static void
validate_cbc_comm_line(cbc_comm_line_s *cml);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb)
{
	const char *optstr = "ab:de:ghi:k:j:lmn:o:p:qrs:t:u:vwx:y";
	int retval, opt, trim;
	retval = NONE;
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
		{"id",			required_argument,	NULL,	'i'},
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
		{"uuid",		required_argument,	NULL,	'u'},
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
			if ((trim = snprintf(cb->name, NAME_S, "%s", optarg)) >= NAME_S)
				fprintf(stderr, "Hostname %s trimmed. 63 chars max!\n", optarg);
			cb->server = NAME;
		} else if (opt == 'i') {
			cb->server_id = strtoul(optarg, NULL, 10);
			cb->server = ID;
		} else if (opt == 'u') {
			snprintf(cb->uuid, CONF_S, "%s", optarg);
			cb->server = UUID;
		} else if (opt == 'a') {
			cb->action = ADD_CONFIG;
		} else if (opt == 'd') {
			cb->action = DISPLAY_CONFIG;
		} else if (opt == 'g') {
			cb->removeip = TRUE;
		} else if (opt == 'l') {
			cb->action = LIST_CONFIG;
		} else if (opt == 'm') {
			cb->action = MOD_CONFIG;
		} else if (opt == 'q') {
			cb->action = QUERY_CONFIG;
		} else if (opt == 'r') {
			cb->action = RM_CONFIG;
		} else if (opt == 'w') {
			cb->action = WRITE_CONFIG;
		} else if (opt == 'b') {
			snprintf(cb->build_domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'e') {
			cb->locale = strndup(optarg, NAME_S);
		} else if (opt == 'k') {
			cb->netcard = strndup(optarg, HOST_S);
		} else if (opt == 'j') {
// We do not check for a starting /dev here.
			cb->harddisk = strndup(optarg, HOST_S);
		} else if (opt == 'o') {
			snprintf(cb->os, MAC_S, "%s", optarg);
		} else if (opt == 'p') {
			snprintf(cb->partition, CONF_S, "%s", optarg);
		} else if (opt == 't') {
			snprintf(cb->arch, RANGE_S, "%s", optarg);
		} else if (opt == 's') {
			snprintf(cb->os_version, MAC_S, "%s", optarg);
		} else if (opt == 'x') {
			snprintf(cb->varient, CONF_S, "%s", optarg);
		} else if (opt == 'y') {
			cb->gui = 1;
		} else if (opt == 'v') {
			cb->action = CVERSION;
		} else if (opt == 'h') {
			return DISPLAY_USAGE;
		} else {
			printf("Unknown option: %c\n", opt);
			retval = DISPLAY_USAGE;
		}
	}
	if ((cb->action == NONE) && 
	 (cb->server == NONE) &&
	 (strncmp(cb->action_type, "NULL", MAC_S) == 0))
		return DISPLAY_USAGE;
	else if (cb->action == CVERSION)
		return CVERSION;
	else if (cb->action == QUERY_CONFIG)
		return NONE;
	else if (cb->action == NONE)
		return NO_ACTION;
	else if ((cb->action != LIST_CONFIG) &&
		 (cb->server == 0))
		return NO_NAME_OR_ID;
	if (cb->action == ADD_CONFIG) {
		if ((strncmp(cb->os, "NULL", COMM_S) == 0) &&
		    (strncmp(cb->arch, "NULL", COMM_S) == 0) &&
		    (strncmp(cb->os_version, "NULL", COMM_S) == 0))
			retval = NO_OS_SPECIFIED;
		else if (strncmp(cb->build_domain, "NULL", COMM_S) == 0)
			retval = NO_BUILD_DOMAIN;
		else if (strncmp(cb->varient, "NULL", COMM_S) == 0)
			retval = NO_BUILD_VARIENT;
		else if (strncmp(cb->partition, "NULL", COMM_S) == 0)
			retval = NO_BUILD_PARTITION;
		if (!(cb->harddisk)) {
			ailsa_syslog(LOG_INFO, "No disk provided. Setting to vda");
			cb->harddisk = strdup("vda");
		}
		if (!(cb->netcard)) {
			ailsa_syslog(LOG_INFO, "No network card provided. Setting to eth0");
			cb->netcard = strdup("eth0");
		}
			
	}
	validate_cbc_comm_line(cb);
	return retval;
}

void
print_cbc_command_line_values(cbc_comm_line_s *cml)
{
	fprintf(stderr, "########\nCommand line Values\n");
	if (cml->action == WRITE_CONFIG)
		fprintf(stderr, "Action: Write configuration file\n");
	else if (cml->action == DISPLAY_CONFIG)
		fprintf(stderr, "Action: Display configuration\n");
	else if (cml->action == ADD_CONFIG)
		fprintf(stderr, "Action: Add configuration for build\n");
	else if (cml->action == CREATE_CONFIG)
		fprintf(stderr, "Action: Create build configuration\n");
	else
		fprintf(stderr, "Action: Unknown!!\n");
	fprintf(stderr, "Config: %s\n", cml->config);
	fprintf(stderr, "Name: %s\n", cml->name);
	fprintf(stderr, "UUID: %s\n", cml->uuid);
	fprintf(stderr, "Server ID: %ld\n", cml->server_id);
	fprintf(stderr, "OS ID: %lu\n", cml->os_id);
	fprintf(stderr, "OS: %s\n", cml->os);
	fprintf(stderr, "OS Version: %s\n", cml->os_version);
	fprintf(stderr, "Architecture: %s\n", cml->arch);
	if (cml->locale)
		fprintf(stderr, "Locale: %s\n", cml->locale);
	fprintf(stderr, "Build Domain: %s\n", cml->build_domain);
	fprintf(stderr, "Action Type: %s\n", cml->action_type);
	fprintf(stderr, "Partition: %s\n", cml->partition);
	fprintf(stderr, "Varient: %s\n", cml->varient);
	
	fprintf(stderr, "\n");
}

static void
validate_cbc_comm_line(cbc_comm_line_s *cml)
{
	if (strncmp(cml->uuid, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->uuid, UUID_REGEX) < 0)
			if (ailsa_validate_input(cml->uuid, FS_REGEX) < 0)
				report_error(USER_INPUT_INVALID, "uuid");
	if (strncmp(cml->name, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->name, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "name");
	if (strncmp(cml->os, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->os, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "os");
	if (strncmp(cml->os_version, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->os_version, OS_VER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "os version");
	if (strncmp(cml->partition, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->partition, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "partition scheme");
	if (strncmp(cml->varient, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->varient, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "varient");
	if (strncmp(cml->build_domain, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->build_domain, DOMAIN_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "build domain");
	if (strncmp(cml->arch, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->arch, CUSTOMER_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "architecture");
	if (cml->netcard)
		if (ailsa_validate_input(cml->netcard, DEV_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "network device");
	if (cml->harddisk)
		if (ailsa_validate_input(cml->harddisk, DEV_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "disk drive device");
	if (strncmp(cml->config, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->config, PATH_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "config file path");
	if (cml->locale)
		if (ailsa_validate_input(cml->locale, NAME_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "locale");
}

void
init_cbc_comm_values(cbc_comm_line_s *cbt)
{
	memset(cbt, 0, sizeof(cbc_comm_line_s));
	snprintf(cbt->name, CONF_S, "NULL");
	snprintf(cbt->uuid, CONF_S, "NULL");
	snprintf(cbt->action_type, MAC_S, "NULL");
	snprintf(cbt->os, CONF_S, "NULL");
	snprintf(cbt->os_version, MAC_S, "NULL");
	snprintf(cbt->partition, CONF_S, "NULL");
	snprintf(cbt->varient, CONF_S, "NULL");
	snprintf(cbt->build_domain, RBUFF_S, "NULL");
	snprintf(cbt->arch, MAC_S, "NULL");
	snprintf(cbt->config, COMM_S, "NULL");
}

void
clean_cbc_comm_line(cbc_comm_line_s *cml)
{
	if (cml->locale)
		my_free(cml->locale);
	if (cml->harddisk)
		my_free(cml->harddisk);
	if (cml->netcard)
		my_free(cml->netcard);
	my_free(cml);
}
