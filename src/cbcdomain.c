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
 *  cbcdomain.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc program
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "checks.h"
#include "cbcdomain.h"
#include "builddomain.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	int retval = NONE;
	cbc_config_s *cmc;
	cbcdomain_comm_line_s *cdcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcdomain main");
	if (!(cdcl = malloc(sizeof(cbcdomain_comm_line_s))))
		report_error(MALLOC_FAIL, "cdcl in cbcdomain main");
	init_cbcdomain_config(cmc, cdcl);
	if ((retval = parse_cbcdomain_comm_line(argc, argv, cdcl)) != 0) {
		free(cdcl);
		free(cmc);
		display_command_line_error(retval, argv[0]);
	}
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cdcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if ((cdcl->action == DISPLAY_CONFIG) ||
		(cdcl->action == LIST_SERVERS))
		retval = display_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == LIST_CONFIG)
		retval = list_cbc_build_domain(cmc);
	else if (cdcl->action == ADD_CONFIG)
		retval = add_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == RM_CONFIG)
		retval = remove_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == MOD_CONFIG)
		retval = modify_cbc_build_domain(cmc, cdcl);
	else
		printf("Unknown Action type\n");

	free(cdcl);
	free(cmc);
	if (retval > 0)
		report_error(retval, argv[0]);
	exit(retval);
}

void
init_cbcdomain_comm_line(cbcdomain_comm_line_s *cdcl)
{
	memset(cdcl, 0, sizeof (cbcdomain_comm_line_s));
	snprintf(cdcl->domain, COMM_S, "NULL");
	snprintf(cdcl->basedn, COMM_S, "NULL");
	snprintf(cdcl->binddn, COMM_S, "NULL");
	snprintf(cdcl->ldapserver, COMM_S, "NULL");
	snprintf(cdcl->logserver, COMM_S, "NULL");
	snprintf(cdcl->nfsdomain, COMM_S, "NULL");
	snprintf(cdcl->ntpserver, COMM_S, "NULL");
	snprintf(cdcl->smtpserver, COMM_S, "NULL");
	snprintf(cdcl->xymonserver, COMM_S, "NULL");
}

void
init_cbcdomain_config(cbc_config_s *cmc, cbcdomain_comm_line_s *cdcl)
{
	init_cbc_config_values(cmc);
	init_cbcdomain_comm_line(cdcl);
}

int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl)
{
	int opt, retval;

	retval = NONE;
	while ((opt = getopt(argc, argv, "ab:de:f:g:i:jk:lmn:prs:t:vx:")) != -1) {
		if (opt == 'a') {
			cdl->action = ADD_CONFIG;
		} else if (opt == 'b') {
			snprintf(cdl->basedn, NAME_S, "%s", optarg);
		} else if (opt == 'd') {
			cdl->action = DISPLAY_CONFIG;
		} else if (opt == 'l') {
			cdl->action = LIST_CONFIG;
		} else if (opt == 'm') {
			cdl->action = MOD_CONFIG;
		} else if (opt == 'r') {
			cdl->action = RM_CONFIG;
		} else if (opt == 'j') {
			cdl->action = LIST_SERVERS;
		} else if (opt == 'e') {
			snprintf(cdl->smtpserver, HOST_S, "%s", optarg);
			cdl->confsmtp = 1;
		} else if (opt == 'f') {
			snprintf(cdl->nfsdomain, CONF_S, "%s", optarg);
		} else if (opt == 'g') {
			snprintf(cdl->logserver, HOST_S, "%s", optarg);
			cdl->conflog = 1;
		} else if (opt == 'i') {
			snprintf(cdl->binddn, NAME_S, "%s", optarg);
		} else if (opt == 'k') {
			retval = split_network_args(cdl, optarg);
		} else if (opt == 'n') {
			snprintf(cdl->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'p') {
			cdl->ldapssl = TRUE;
		} else if (opt == 's') {
			snprintf(cdl->ldapserver, HOST_S, "%s", optarg);
			cdl->confldap = 1;
		} else if (opt == 't') {
			snprintf(cdl->ntpserver, HOST_S, "%s", optarg);
			cdl->confntp = 1;
		} else if (opt == 'x') {
			snprintf(cdl->xymonserver, HOST_S, "%s", optarg);
			cdl->confxymon = 1;
		} else if (opt == 'v') {
			cdl->action = CVERSION;
		} else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cdl->action == CVERSION)
		return CVERSION;
	if (cdl->action == NONE)
		return NO_ACTION;
	if (cdl->action != LIST_CONFIG && strncmp(cdl->domain, "NULL", COMM_S) == 0)
		return NO_DOMAIN_NAME;
	if ((cdl->action == MOD_CONFIG) && ((cdl->start_ip != 0) ||
		                            (cdl->end_ip != 0) ||
		                            (cdl->netmask != 0) ||
		                            (cdl->gateway != 0) ||
		                            (cdl->ns != 0)))
		return NO_MOD_BUILD_DOM_NET;
	if ((cdl->action == MOD_CONFIG) &&
	    (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0) &&
	   ((cdl->conflog > 0) || (cdl->confsmtp > 0) || (cdl->confntp > 0) ||
	    (cdl->confxymon > 0)))
		return MULTI_BUILD_DOM_APP_MOD;
	if ((cdl->action == MOD_CONFIG) && (cdl->conflog > 0) &&
	   ((cdl->confsmtp > 0) || (cdl->confntp > 0) || (cdl->confxymon > 0) ||
	   (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0)))
		return MULTI_BUILD_DOM_APP_MOD;
	if ((cdl->action == MOD_CONFIG) && (cdl->confsmtp > 0) &&
	   ((cdl->conflog > 0) || (cdl->confntp > 0) || (cdl->confxymon > 0) ||
	   (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0)))
		return MULTI_BUILD_DOM_APP_MOD;
	if ((cdl->action == MOD_CONFIG) && (cdl->confntp > 0) &&
	   ((cdl->confsmtp > 0) || (cdl->conflog > 0) || (cdl->confxymon > 0) ||
	   (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0)))
		return MULTI_BUILD_DOM_APP_MOD;
	if ((cdl->action == MOD_CONFIG) && (cdl->confxymon > 0) &&
	   ((cdl->confsmtp > 0) || (cdl->conflog > 0) || (cdl->confntp > 0) ||
	   (strncmp(cdl->nfsdomain, "NULL", COMM_S) != 0)))
		return MULTI_BUILD_DOM_APP_MOD;
	return retval;
}

int
split_network_args(cbcdomain_comm_line_s *cdl, char *netinfo)
{
	unsigned long int ips[4];
	char *ip, *tmp;
	const char *network = netinfo;
	int retval = NONE, delim = ',', i;
	uint32_t ip_addr;

/* This is taken from the calling function */
	ip = optarg;
	for (i = 0; i < 4; i++) {
		if ((tmp = strchr(network, delim))) {
			*tmp = '\0';
			tmp++;
		} else {
			return USER_INPUT_INVALID;
		}
		if (inet_pton(AF_INET, ip, &ip_addr)) {
			ips[i] = (unsigned long int) htonl(ip_addr);
		} else {
			retval = USER_INPUT_INVALID;
			break;
		}
		network = ip = tmp;
	}
	cdl->start_ip = ips[0];
	cdl->end_ip = ips[1];
	cdl->gateway = ips[2];
	cdl->netmask = ips[3];
	if (inet_pton(AF_INET, ip, &ip_addr)) {
		cdl->ns = (unsigned long int) htonl(ip_addr);
	} else {
		retval = USER_INPUT_INVALID;
	}
	return retval;
}
