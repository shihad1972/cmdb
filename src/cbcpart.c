/* 
 *
 *  cbcpart: Create Build Configuration Partition
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
 *  cbcpart.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  Part of the cbcpart program
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
#include "cbcpart.h"

int
main (int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	char error[URL_S];
	int retval = NONE;
	cbc_config_s *cmc;
	cbcpart_comm_line_s *cpcl;
	
	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcpart main");
	if (!(cpcl = malloc(sizeof(cbcpart_comm_line_s))))
		report_error(MALLOC_FAIL, "cpcl in cbcpart main");
	init_cbcpart_config(cmc, cpcl);
	if ((retval = parse_cbcpart_comm_line(argc, argv, cpcl)) != 0) {
		free(cmc);
		free(cpcl);
		display_cmdb_command_line_error(retval, argv[0]);
	}

	exit (retval);
}

void
init_cbcpart_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl)
{
	init_cbc_config_values(cbc);
	init_cbcpart_comm_line(cpl);
}

void
init_cbcpart_comm_line(cbcpart_comm_line_s *cpl)
{
	cpl->action = 0;
	cpl->lvm = 0;
	cpl->type = 0;
	snprintf(cpl->partition, COMM_S, "NULL");
	snprintf(cpl->scheme, COMM_S, "NULL");
}

int
parse_cbcpart_comm_line(int argc, char *argv[], cbcpart_comm_line_s *cpl)
{
	int opt;

	while ((opt = getopt(argc, argv, "adln:prst:")) != -1) {
		if (opt == 'a')
			cpl->action = ADD_CONFIG;
		else if (opt == 'd')
			cpl->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			cpl->action = LIST_CONFIG;
		else if (opt == 'r')
			cpl->action = RM_CONFIG;
		else if (opt == 'l')
			cpl->lvm = TRUE;
		else if (opt == 'n')
			snprintf(cpl->scheme, CONF_S, "%s", optarg);
		else if (opt == 'p')
			cpl->type = PARTITION;
		else if (opt == 's')
			cpl->type = SCHEME;
		else if (opt == 't')
			snprintf(cpl->partition, RBUFF_S, "%s", optarg);
		else {
			printf("Unknown option: %c\n", opt);
			return DISPLAY_USAGE;
		}
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cpl->action == 0 && argc != 1)
		return NO_ACTION;
	if ((cpl->action == ADD_CONFIG || cpl->action == RM_CONFIG) && 
	     cpl->type == 0)
		return NO_TYPE;
	if ((cpl->action == ADD_CONFIG || cpl->action == RM_CONFIG) &&
	    cpl->type == 0)
		return NO_PARTITION_INFO;
	return NONE;
}
