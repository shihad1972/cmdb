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
 *  cbcos.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbcos program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcos.h"

int
main (int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	int retval = NONE;
	cbc_config_s *cmc;
	cbcos_comm_line_s *cocl;

	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbcos main");
	if (!(cocl = malloc(sizeof(cbcos_comm_line_s))))
		report_error(MALLOC_FAIL, "cocl in cbcos main");
	init_cbcos_config(cmc, cocl);
	if ((retval = parse_cbcos_comm_line(argc, argv, cocl)) != 0) {
		free(cocl);
		free(cmc);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	if ((retval = parse_cbc_config_file(cmc, config)) != 0) {
		free(cocl);
		free(cmc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (cocl->action == LIST_CONFIG)
		retval = list_cbc_build_os(cmc);
	else if (cocl->action == DISPLAY_CONFIG)
		retval = display_cbc_build_os(cmc, cocl);
	else
		printf("Unknown action type\n");
	free(cmc);
	if (retval != 0)
		report_error(retval, cocl->os);
	free(cocl);
	exit(retval);
}

void
init_cbcos_config(cbc_config_s *cmc, cbcos_comm_line_s *col)
{
	init_cbc_config_values(cmc);
	init_cbcos_comm_line(col);
}

void
init_cbcos_comm_line(cbcos_comm_line_s *col)
{
	col->action = 0;
	col->id = 0;
	snprintf(col->alias, MAC_S, "NULL");
	snprintf(col->arch, RANGE_S, "NULL");
	snprintf(col->os, MAC_S, "NULL");
	snprintf(col->ver_alias, MAC_S, "NULL");
	snprintf(col->version, MAC_S, "NULL");
}

int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col)
{
	int opt;

	while ((opt = getopt(argc, argv, "ade:i:ln:o:rs:t:")) != -1) {
		if (opt == 'a')
			col->action = ADD_CONFIG;
		else if (opt == 'd')
			col->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			col->action = LIST_CONFIG;
		else if (opt == 'r')
			col->action = RM_CONFIG;
		else if (opt == 'e')
			snprintf(col->ver_alias, MAC_S, "%s", optarg);
		else if (opt == 'i')
			col->id = strtoul(optarg, NULL, 10);
		else if (opt == 'n')
			snprintf(col->os, MAC_S, "%s", optarg);
		else if (opt == 'o')
			snprintf(col->version, MAC_S, "%s", optarg);
		else if (opt == 's')
			snprintf(col->alias, MAC_S, "%s", optarg);
		else if (opt == 't')
			snprintf(col->arch, RANGE_S, "%s", optarg);
	}
	if (col->action == 0 && argc != 1) {
		printf("No action provided\n");
		return NO_ACTION;
	}
	if (col->action == ADD_CONFIG && (
		strncmp(col->version, "NULL", COMM_S) == 0 ||
		strncmp(col->os, "NULL", COMM_S) == 0 ||
		strncmp(col->arch, "NULL", COMM_S) == 0)) {
			printf("Some details were not provided\n");
			return DISPLAY_USAGE;
	}
	if (col->action != LIST_CONFIG && (strncmp(col->os, "NULL", COMM_S) == 0)) {
		printf("No OS name was provided\n");
		return DISPLAY_USAGE;
	}
	return NONE;
}

int
list_cbc_build_os(cbc_config_s *cmc)
{
	char *oalias, *talias;
	int retval = 0;
	cbc_s *base;
	cbc_build_os_s *os;
	cbc_build_sype_t *type;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_os");
	init_cbc_struct(base);
	if ((retval = run_multiple_query(cmc, base, BUILD_OS | BUILD_TYPE)) != 0) {
		clean_cbc_struct(base);
		return MY_QUERY_FAIL;
	}
	type = base->btype;
	printf("Operating Systems\n");
	while (type) {
		os = base->bos;
		talias = type->alias;
		oalias = os->alias;
		while (os) {
			if (strncmp(talias, oalias, MAC_S) != 0) {
				os = os->next;
				oalias = os->alias;
			} else {
				printf("%s\n", os->os);
				os = os->next;
				break;
			}
		}	
		type = type->next;
	}
	clean_cbc_struct(base);
	return retval;
}

int
display_cbc_build_os(cbc_config_s *cmc, cbcos_comm_line_s *col)
{
	char *name = col->os;
	int retval = 0, i = 0;
	cbc_s *base;
	cbc_build_os_s *os;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_os");
	init_cbc_struct(base);
	if ((retval = run_query(cmc, base, BUILD_OS)) != 0) {
		clean_cbc_struct(base);
		return MY_QUERY_FAIL;
	}
	os = base->bos;
	printf("Operating System %s\n", name);
	printf("Version\tVersion alias\tArchitecture\n");
	while (os) {
		if (strncmp(os->os, name, MAC_S) == 0) {
			i++;
			if (strncmp(os->ver_alias, "none", COMM_S) == 0) {
				printf("%s\tnone\t\t%s\n",
				     os->version, os->arch);
			} else {
				if (strlen(os->ver_alias) < 8)
					printf("%s\t%s\t\t%s\n",
					     os->version, os->ver_alias, os->arch);
				else
					printf("%s\t%s\t%s\n",
					     os->version, os->ver_alias, os->arch);
			}
		}
		os = os->next;
	}
	if (i == 0)
		retval =  OS_NOT_FOUND;
	clean_cbc_struct(base);
	return retval;
}
