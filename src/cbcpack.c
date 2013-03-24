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
 *  cbcpack.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcpack program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcpack.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	char error[URL_S];
	int retval = NONE;
	cbc_config_s *cmc;
	cbcpack_comm_line_s *cpcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcpack main");
	if (!(cpcl = malloc(sizeof(cbcpack_comm_line_s))))
		report_error(MALLOC_FAIL, "cpcl in cbcpack main");
	init_cbcpack_config(cmc, cpcl);
	if ((retval = parse_cbcpack_comm_line(argc, argv, cpcl)) != 0) {
		free(cmc);
		free(cpcl);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	if (strncmp(cpcl->package, "NULL", COMM_S) != 0)
		snprintf(error, HOST_S, "%s", cpcl->package);
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cpcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cpcl->action == LIST_CONFIG)
		printf("Please use cbcvarient to list packages\n");
	else if (cpcl->action == DISPLAY_CONFIG)
		printf("Please use cbcvarient to display packages\n");
	else if (cpcl->action == MOD_CONFIG)
		printf("Cowardly refusal to modify packages\n");
	else if (cpcl->action == ADD_CONFIG)
		printf("Adding package %s\n", cpcl->package);
	else if (cpcl->action == RM_CONFIG)
		printf("Removing package %s\n", cpcl->package);
	else
		printf("Unknown action type\n");
	if (retval != NONE) {
		free(cmc);
		free(cpcl);
		report_error(retval, error);
	}

	free(cmc);
	free(cpcl);
	exit(retval);
}

void
init_cbcpack_config(cbc_config_s *cmc, cbcpack_comm_line_s *cpl)
{
	init_cbc_config_values(cmc);
	init_cbcpack_comm_line(cpl);
}

void
init_cbcpack_comm_line(cbcpack_comm_line_s *cpl)
{
	cpl->action = 0;
	snprintf(cpl->alias, MAC_S, "NULL");
	snprintf(cpl->arch, RANGE_S, "NULL");
	snprintf(cpl->os, MAC_S, "NULL");
	snprintf(cpl->ver_alias, MAC_S, "NULL");
	snprintf(cpl->version, MAC_S, "NULL");
	snprintf(cpl->varient, HOST_S, "NULL");
	snprintf(cpl->valias, MAC_S, "NULL");
	snprintf(cpl->package, HOST_S, "NULL");
}

int
parse_cbcpack_comm_line(int argc, char *argv[], cbcpack_comm_line_s *cpl)
{
	int opt;

	while ((opt = getopt(argc, argv, "ade:k:lmn:o:p:rs:t:x:")) != -1) {
		if (opt == 'a')
			cpl->action = ADD_CONFIG;
		else if (opt == 'd')
			cpl->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			cpl->action = LIST_CONFIG;
		else if (opt == 'm')
			cpl->action = MOD_CONFIG;
		else if (opt == 'r')
			cpl->action = RM_CONFIG;
		else if (opt == 'e')
			snprintf(cpl->ver_alias, MAC_S, "%s", optarg);
		else if (opt == 'k')
			snprintf(cpl->valias, MAC_S, "%s", optarg);
		else if (opt == 'n')
			snprintf(cpl->os, MAC_S, "%s", optarg);
		else if (opt == 'o')
			snprintf(cpl->version, MAC_S, "%s", optarg);
		else if (opt == 'p')
			snprintf(cpl->package, HOST_S, "%s", optarg);
		else if (opt == 's')
			snprintf(cpl->alias, MAC_S, "%s", optarg);
		else if (opt == 't')
			snprintf(cpl->arch, RANGE_S, "%s", optarg);
		else if (opt == 'x')
			snprintf(cpl->varient, HOST_S, "%s", optarg);
		else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cpl->action == 0 && argc != 1)
		return NO_ACTION;
	if (strncmp(cpl->package, "NULL", COMM_S) == 0)
		return NO_PACKAGE;
	if ((strncmp(cpl->valias, "NULL", COMM_S) == 0) &&
	    (strncmp(cpl->varient, "NULL", COMM_S) == 0))
		return NO_VARIENT;
	if ((strncmp(cpl->os, "NULL", COMM_S) == 0) &&
	    (strncmp(cpl->alias, "NULL", COMM_S) == 0))
		return NO_OS_COMM;
	return NONE;
}
