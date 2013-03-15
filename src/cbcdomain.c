/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	cbc_config_t *cmc;
	cbcdomain_comm_line_s *cdcl;

	if (!(cmc = malloc(sizeof(cbc_config_t))))
		report_error(MALLOC_FAIL, "cmc in cbcdomain main");
	if (!(cdcl = malloc(sizeof(cbcdomain_comm_line_s))))
		report_error(MALLOC_FAIL, "cdcl in cbcdomain main");
	init_cbcdomain_config(cmc, cdcl);
	if ((retval = parse_cbcdomain_comm_line(argc, argv, cdcl)) != 0) {
		free(cdcl);
		free(cmc);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cdcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cdcl->action == DISPLAY_CONFIG)
		retval = display_cbc_build_domain(cmc, cdcl);
	else if (cdcl->action == LIST_CONFIG)
		retval = list_cbc_build_domain(cmc);
	else if (cdcl->action == ADD_CONFIG)
		retval = add_cbc_build_domain(cmc, cdcl);
	else
		printf("Unknown Action type\n");

	free(cdcl);
	free(cmc);
	exit(retval);
}

void
init_cbcdomain_comm_line(cbcdomain_comm_line_s *cdcl)
{
	snprintf(cdcl->domain, COMM_S, "NULL");
	snprintf(cdcl->basedn, COMM_S, "NULL");
	snprintf(cdcl->binddn, COMM_S, "NULL");
	snprintf(cdcl->ldapserver, COMM_S, "NULL");
	snprintf(cdcl->logserver, COMM_S, "NULL");
	snprintf(cdcl->nfsdomain, COMM_S, "NULL");
	snprintf(cdcl->smtpserver, COMM_S, "NULL");
	snprintf(cdcl->xymonserver, COMM_S, "NULL");
	cdcl->action = cdcl->confldap = cdcl->ldapssl = cdcl->conflog = 0;
	cdcl->confsmtp = cdcl->confxymon = 0;
	cdcl->start_ip = cdcl->end_ip = cdcl->netmask = cdcl->gateway = 0;
	cdcl->ns = 0;
}

void
init_cbcdomain_config(cbc_config_t *cmc, cbcdomain_comm_line_s *cdcl)
{
	init_cbc_config_values(cmc);
	init_cbcdomain_comm_line(cdcl);
}

int
parse_cbcdomain_comm_line(int argc, char *argv[], cbcdomain_comm_line_s *cdl)
{
	int opt, retval;

	retval = NONE;
	while ((opt = getopt(argc, argv, "ab:de:f:g:i:k:lmn:prs:x:")) != -1) {
		if (opt == 'a') {
			cdl->action = ADD_CONFIG;
		} else if (opt == 'b') {
			snprintf(cdl->basedn, NAME_S, "%s", optarg);
		} else if (opt == 'd') {
			cdl->action = DISPLAY_CONFIG;
		} else if (opt == 'e') {
			snprintf(cdl->smtpserver, HOST_S, "%s", optarg);
		} else if (opt == 'f') {
			snprintf(cdl->nfsdomain, CONF_S, "%s", optarg);
		} else if (opt == 'g') {
			snprintf(cdl->logserver, HOST_S, "%s", optarg);
		} else if (opt == 'i') {
			snprintf(cdl->binddn, NAME_S, "%s", optarg);
		} else if (opt == 'k') {
			retval = split_network_args(cdl, optarg);
		} else if (opt == 'l') {
			cdl->action = LIST_CONFIG;
		} else if (opt == 'm') {
			cdl->action = MOD_CONFIG;
		} else if (opt == 'n') {
			snprintf(cdl->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'p') {
			cdl->ldapssl = TRUE;
		} else if (opt == 'r') {
			cdl->action = RM_CONFIG;
		} else if (opt == 's') {
			snprintf(cdl->ldapserver, HOST_S, "%s", optarg);
		} else if (opt == 'x') {
			snprintf(cdl->xymonserver, HOST_S, "%s", optarg);
		}
	}
	if (cdl->action == 0)
		return NO_ACTION;
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

	ip = optarg;
	for (i = 0; i < 4; i++) {
		tmp = strchr(network, delim);
		*tmp = '\0';
		tmp++;
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
