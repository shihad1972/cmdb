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
 *  cbcvarient.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcvarient program
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
#include "cbcvarient.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	char error[URL_S];
	int retval = NONE;
	cbc_config_s *cmc;
	cbcvari_comm_line_s *cvcl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcvarient main");
	if (!(cvcl = malloc(sizeof(cbcvari_comm_line_s))))
		report_error(MALLOC_FAIL, "cvcl in cbcvarient main");
	init_cbcvari_config(cmc, cvcl);
	if ((retval = parse_cbcvarient_comm_line(argc, argv, cvcl)) != 0) {
		free(cmc);
		free(cvcl);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	if (strncmp(cvcl->varient, "NULL", COMM_S) != 0)
		snprintf(error, URL_S, "name %s", cvcl->varient);
	else if (strncmp(cvcl->valias, "NULL", COMM_S) != 0)
		snprintf(error, URL_S, "alias %s", cvcl->valias);
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cvcl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cvcl->action == LIST_CONFIG)
		printf("List varient\n");
	else if (cvcl->action == DISPLAY_CONFIG)
		printf("Display varient\n");
	else if (cvcl->action == ADD_CONFIG)
		printf("Add varient\n");
	else if (cvcl->action == RM_CONFIG)
		printf("Remove varient\n");
	else
		printf("Unknown action type\n");

	free(cmc);
	free(cvcl);
	exit (retval);
}

void
init_cbcvari_config(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	init_cbc_config_values(cmc);
	init_cbcvari_comm_line(cvl);
}

void
init_cbcvari_comm_line(cbcvari_comm_line_s *cvl)
{
	cvl->action = 0;
	cvl->id = 0;
	cvl->os_id = 0;
	snprintf(cvl->alias, MAC_S, "NULL");
	snprintf(cvl->arch, RANGE_S, "NULL");
	snprintf(cvl->os, MAC_S, "NULL");
	snprintf(cvl->ver_alias, MAC_S, "NULL");
	snprintf(cvl->version, MAC_S, "NULL");
	snprintf(cvl->varient, HOST_S, "NULL");
	snprintf(cvl->valias, MAC_S, "NULL");
}

int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl)
{
	int opt;

	while ((opt = getopt(argc, argv, "ade:i:k:ln:o:rs:t:x:")) != -1) {
		if (opt == 'a')
			cvl->action = ADD_CONFIG;
		else if (opt == 'd')
			cvl->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			cvl->action = LIST_CONFIG;
		else if (opt == 'r')
			cvl->action = RM_CONFIG;
		else if (opt == 'e')
			snprintf(cvl->ver_alias, MAC_S, "%s", optarg);
		else if (opt == 'i')
			cvl->id = strtoul(optarg, NULL, 10);
		else if (opt == 'k')
			snprintf(cvl->valias, MAC_S, "%s", optarg);
		else if (opt == 'n')
			snprintf(cvl->os, MAC_S, "%s", optarg);
		else if (opt == 'o')
			snprintf(cvl->version, MAC_S, "%s", optarg);
		else if (opt == 's')
			snprintf(cvl->alias, MAC_S, "%s", optarg);
		else if (opt == 't')
			snprintf(cvl->arch, RANGE_S, "%s", optarg);
		else if (opt == 'x')
			snprintf(cvl->varient, HOST_S, "%s", optarg);
	}
	if (argc == 1)
		return DISPLAY_USAGE;
	if (cvl->action == 0 && argc != 1) {
		printf("No action provided\n");
		return NO_ACTION;
	}
	if (cvl->action != LIST_CONFIG &&
		((strncmp(cvl->varient, "NULL", COMM_S) == 0) ||
		(strncmp(cvl->valias, "NULL", COMM_S) == 0))) {
		fprintf(stderr, "No varient name or alias was provided\n");
		return DISPLAY_USAGE;
	}
	return NONE;
}
