/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
			snprintf(cb->locale, NAME_S, "%s", optarg);
		} else if (opt == 'k') {
			snprintf(cb->netcard, HOST_S, "%s", optarg);
		} else if (opt == 'j') {
// We do not check for a starting /dev here.
			snprintf(cb->harddisk, HOST_S, "%s", optarg);
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
	}
#ifdef HAVE_LIBPCRE
	validate_cbc_comm_line(cb);
#endif /* HAVE_LIBPCRE */
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
	fprintf(stderr, "Locale: %s\n", cml->locale);
	fprintf(stderr, "Build Domain: %s\n", cml->build_domain);
	fprintf(stderr, "Action Type: %s\n", cml->action_type);
	fprintf(stderr, "Partition: %s\n", cml->partition);
	fprintf(stderr, "Varient: %s\n", cml->varient);
	
	fprintf(stderr, "\n");
}

#ifdef HAVE_LIBPCRE

void
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
	if (strncmp(cml->netcard, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->netcard, DEV_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "network device");
	if (strncmp(cml->config, "NULL", COMM_S) != 0)
		if (ailsa_validate_input(cml->config, PATH_REGEX) < 0)
			report_error(USER_INPUT_INVALID, "config file path");
}

#endif /* HAVE_LIBPCRE */

int
parse_cbc_config_file(cbc_config_s *cbc, const char *config)
{
	FILE *cnf;
	int retval;
#ifdef HAVE_WORDEXP_H
	char **uconf;
	wordexp_t p;
#endif /* HAVE_WORDEXP_H */

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
	} else {
		read_cbc_config_values(cbc, cnf);
		fclose(cnf);
	}
#ifdef HAVE_WORDEXP
	if ((retval = wordexp("~/.dnsa.conf", &p, 0)) == 0) {
		uconf = p.we_wordv;
		if ((cnf = fopen(*uconf, "r"))) {
			if ((retval = read_cbc_config_values(cbc, cnf)) != 0)
				retval = UPORT_ERR;
			fclose(cnf);
		}
		wordfree(&p);
	}
#endif /* HAVE_WORDEXP */
	if ((retval = add_trailing_slash(cbc->tmpdir)) != 0)
		retval = TMP_ERR;
	if ((retval = add_trailing_slash(cbc->tftpdir)) != 0)
		retval = TFTP_ERR;
	if ((retval = add_trailing_slash(cbc->pxe)) != 0)
		retval = PXE_ERR;
	if ((retval = add_trailing_slash(cbc->toplevelos)) != 0)
		retval = OS_ERR;
	if ((retval = add_trailing_slash(cbc->preseed)) != 0)
		retval = PRESEED_ERR;
	if ((retval = add_trailing_slash(cbc->kickstart)) !=0)
		retval = KICKSTART_ERR;

	return retval;
}

int
read_cbc_config_values(cbc_config_s *cbc, FILE *cnf)
{
	int retval = 0;
	unsigned long int portno;
	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DBTYPE=%s", cbc->dbtype);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PASS=%s", cbc->pass);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "FILE=%s", cbc->file);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "HOST=%s", cbc->host);	
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "USER=%s", cbc->user);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DB=%s", cbc->db);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "SOCKET=%s", cbc->socket);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PORT=%s", port);
	rewind (cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "TMPDIR=%s", cbc->tmpdir);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "TFTPDIR=%s", cbc->tftpdir);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PXE=%s", cbc->pxe);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "TOPLEVELOS=%s", cbc->toplevelos);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "PRESEED=%s", cbc->preseed);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "KICKSTART=%s", cbc->kickstart);
	rewind(cnf);
	while ((fgets(buff, CONF_S, cnf)))
		sscanf(buff, "DHCPCONF=%s", cbc->dhcpconf);
	retval = 0;
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
		return retval;
	} else {
		cbc->port = (unsigned int) portno;
	}
	return retval;
}

void
parse_cbc_config_error(int error)
{
	if (error == PORT_ERR)
		fprintf(stderr, "Port higher than 65535!\n");
	else if (error == TMP_ERR)
		fprintf(stderr, "Cannot add trailing / to TMPDIR: > 79 characters\n");
	else if (error == TFTP_ERR)
		fprintf(stderr, "Cannot add trailing / to TFTPDIR: > 79 characters\n");
	else if (error == PXE_ERR)
		fprintf(stderr, "Cannot add trailing / to PXE: > 79 characters\n");
	else if (error == OS_ERR)
		fprintf(stderr, "Cannot add trailing / to TOPLEVELOS: > 79 characters\n");
	else if (error == PRESEED_ERR)
		fprintf(stderr, "Cannot add trailing / to PRESEED: > 79 characters\n");
	else if (error == KICKSTART_ERR)
		fprintf(stderr, "Cannot add trailing / to KICKSTART: > 79 characters\n");
	else
		fprintf(stderr, "Unkown error code: %d\n", error);
}

void
init_all_config(cbc_config_s *cct, cbc_comm_line_s *cclt)
{
	init_cbc_config_values(cct);
	init_cbc_comm_values(cclt);
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
	snprintf(cbt->netcard, COMM_S, "NULL");
	snprintf(cbt->config, COMM_S, "NULL");
	snprintf(cbt->locale, COMM_S, "NULL");
}

void
init_cbc_config_values(cbc_config_s *cbc)
{
	memset(cbc, 0, sizeof(cbc_config_s));
	sprintf(cbc->db, "cmdb");
	sprintf(cbc->dbtype, "none");
	sprintf(cbc->user, "root");
	sprintf(cbc->host, "localhost");
	sprintf(cbc->pass, "%s", "");
	sprintf(cbc->socket, "%s", "");
	sprintf(cbc->tmpdir, "/tmp/cbc");
	sprintf(cbc->tftpdir, "/tftpboot");
	sprintf(cbc->pxe, "pxelinx.cfg");
	sprintf(cbc->toplevelos, "/build");
	sprintf(cbc->dhcpconf, "/etc/dhcpd/dhcpd.hosts");
	sprintf(cbc->preseed, "preseed");
	sprintf(cbc->kickstart, "kickstart");
	cbc->port = 3306;
	cbc->cliflag = 0;
}

void
print_cbc_config(cbc_config_s *cbc)
{
	fprintf(stderr, "########\nConfig Values\n");
	fprintf(stderr, "DB: %s\n", cbc->db);
	fprintf(stderr, "USER: %s\n", cbc->user);
	fprintf(stderr, "PASS: %s\n", cbc->pass);
	fprintf(stderr, "HOST: %s\n", cbc->host);
	fprintf(stderr, "PORT: %d\n", cbc->port);
	fprintf(stderr, "SOCKET: %s\n", cbc->socket);
	fprintf(stderr, "TMPDIR: %s\n", cbc->tmpdir);
	fprintf(stderr, "TFTPDIR: %s\n", cbc->tftpdir);
	fprintf(stderr, "PXE: %s\n", cbc->pxe);
	fprintf(stderr, "TOPLEVELOS: %s\n", cbc->toplevelos);
	fprintf(stderr, "DHCPCONF: %s\n", cbc->dhcpconf);
	fprintf(stderr, "PRESEED: %s\n", cbc->preseed);
	fprintf(stderr, "KICKSTART: %s\n", cbc->kickstart);
	fprintf(stderr, "\n");
}

