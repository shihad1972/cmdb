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
		add_package(cmc, cpcl);
	else if (cpcl->action == RM_CONFIG)
		remove_package(cmc, cpcl);
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
	if ((strncmp(cpl->os, "NULL", COMM_S) == 0) &&
	    (strncmp(cpl->alias, "NULL", COMM_S) == 0))
		return NO_OS_COMM;
	return NONE;
}

int
add_package(cbc_config_s *cmc, cbcpack_comm_line_s *cpl)
{
	int retval = NONE, osnum = NONE;
	cbc_s *base;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add package");
	init_cbc_struct(base);
	if ((retval = run_multiple_query(cmc, base, BUILD_OS | VARIENT)) != 0) {
		printf("Unable to run os and varient query\n");
		free(base);
		return retval;
	}
	if ((osnum = get_os_list_count(cpl, base)) != 0) {
		printf("We have %d os\n", osnum);
	} else {
		printf("Unknown OS: ");
		if (strncmp(cpl->os, "NULL", COMM_S) != 0)
			printf("%s ", cpl->os);
		if (strncmp(cpl->alias, "NULL", COMM_S) != 0)
			printf("%s ", cpl->alias);
		if (strncmp(cpl->version, "NULL", COMM_S) != 0) 
			printf("%s ", cpl->version);
		if (strncmp(cpl->ver_alias, "NULL", COMM_S) != 0)
			printf("%s ", cpl->ver_alias);
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			printf("%s ", cpl->arch);
		printf("\n");
		return OS_NOT_FOUND;
	}
	printf("Adding package %s\n", cpl->package);
	clean_cbc_struct(base);
	return retval;
}

int
get_os_list_count(cbcpack_comm_line_s *cpl, cbc_s *cbc)
{
	int retval = NONE, type;
	cbc_build_os_s *bos = cbc->bos;

	/* See what we are looking for */
	if ((strncmp(cpl->version, "NULL", COMM_S) != 0) ||
	    (strncmp(cpl->ver_alias, "NULL", COMM_S) != 0)) {
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			type = BOTH;
		else
			type = VER;
	} else {
		if (strncmp(cpl->arch, "NULL", COMM_S) != 0)
			type = ARCH;
		else
			type = NONE;
	}
	/* 
	 * Count number of build operating systems we will be inserting into db
	 * Then we can create an array that size.
	 */
	if (type == NONE) {
		while (bos) {
			if ((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			    (strncmp(cpl->alias, bos->alias, MAC_S) == 0))
				retval++;
			bos = bos->next;
		}
	} else if (type == ARCH) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    (strncmp(cpl->arch, bos->arch, RANGE_S) == 0))
				retval++;
			bos = bos->next;
		}
	} else if (type == VER) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    ((strncmp(cpl->version, bos->version, MAC_S) == 0) ||
			     (strncmp(cpl->ver_alias, bos->ver_alias, MAC_S) == 0)))
				retval++;
			bos = bos->next;
		}
	} else if (type == BOTH) {
		while (bos) {
			if (((strncmp(cpl->os, bos->os, MAC_S) == 0) ||
			     (strncmp(cpl->alias, bos->alias, MAC_S) == 0)) &&
			    ((strncmp(cpl->version, bos->version, MAC_S) == 0) ||
			     (strncmp(cpl->ver_alias, bos->ver_alias, MAC_S) == 0)) &&
			    (strncmp(cpl->arch, bos->arch, RANGE_S) == 0))
				retval++;
			bos = bos->next;
		}
	}
	return retval;
}

int
remove_package(cbc_config_s *cmc, cbcpack_comm_line_s *cpl)
{
	int retval = NONE;

	printf("Removing package %s\n", cpl->package);
	return retval;
}

