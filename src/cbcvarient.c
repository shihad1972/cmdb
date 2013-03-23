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
		retval = list_cbc_build_varient(cmc);
	else if (cvcl->action == DISPLAY_CONFIG)
		retval = display_cbc_build_varient(cmc, cvcl);
	else if (cvcl->action == ADD_CONFIG)
		retval = add_cbc_build_varient(cmc, cvcl);
	else if (cvcl->action == RM_CONFIG)
		retval = remove_cbc_build_varient(cmc, cvcl);
	else
		printf("Unknown action type\n");
	if (retval != 0) {
		free(cmc);
		free(cvcl);
		report_error(retval, error);
	}

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

	while ((opt = getopt(argc, argv, "ade:k:ln:o:rs:t:x:")) != -1) {
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
		(strncmp(cvl->varient, "NULL", COMM_S) == 0) &&
		(strncmp(cvl->valias, "NULL", COMM_S) == 0)) {
		fprintf(stderr, "No varient name or alias was provided\n");
		return DISPLAY_USAGE;
	}
	if (cvl->action == ADD_CONFIG &&
		((strncmp(cvl->varient, "NULL", COMM_S) == 0) ||
		 (strncmp(cvl->valias, "NULL", COMM_S) == 0))) {
		fprintf(stderr, "\
You need to supply both a varient name and valias\n");
		return DISPLAY_USAGE;
	}
	return NONE;
}

int
list_cbc_build_varient(cbc_config_s *cmc)
{
	int retval = NONE;
	cbc_s *base = '\0';
	cbc_varient_s *list = '\0';

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_varient");
	init_cbc_struct(base);
	if ((retval = run_query(cmc, base, VARIENT)) != 0) {
		clean_cbc_struct(base);
		return retval;
	}
	if (base->varient) {
		list = base->varient;
	} else {
		printf("No build varients??\n");
		clean_cbc_struct(base);
		return VARIENT_NOT_FOUND;
	}
	printf("Build Varients\n");
	printf("Alias\t\t\tName\n");
	while (list) {
		if (strlen(list->valias) < 8)
			printf("%s\t\t\t%s\n", list->valias, list->varient);
		else if (strlen(list->valias) < 16)
			printf("%s\t\t%s\n", list->valias, list->varient);
		else
			printf("%s\t%s\n", list->valias, list->varient);
		list = list->next;
	}
	clean_cbc_struct(base);
	return retval;
}

int
display_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = NONE;
	char varient[HOST_S];
	unsigned long int id;
	cbc_s *base;
	dbdata_s *data = '\0';

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in display_cbc_build_varient");
	init_cbc_struct(base);
	cbc_init_initial_dbdata(&data, VARIENT_ID_ON_VALIAS);
	if (strncmp(cvl->varient, "NULL", COMM_S) != 0) {
		snprintf(data->args.text, HOST_S, "%s", cvl->varient);
		retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VARIENT);
		if (retval != 1) {
			clean_dbdata_struct(data);
			return VARIENT_NOT_FOUND;
		}
	} else if (strncmp(cvl->valias, "NULL", COMM_S) != 0) {
		snprintf(data->args.text, HOST_S, "%s", cvl->valias);
		retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VALIAS);
		if (retval != 1) {
			clean_dbdata_struct(data);
			return VARIENT_NOT_FOUND;
		}
	}
	snprintf(varient, HOST_S, "%s", data->args.text);
	id = data->fields.number;
	if ((retval = run_multiple_query(cmc, base, BUILD_OS | BPACKAGE)) != 0) {
		clean_dbdata_struct(data);
		clean_cbc_struct(base);
		return MY_QUERY_FAIL;
	}

	clean_cbc_struct(base);
	clean_dbdata_struct(data);
	return retval;
}

int
add_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = NONE;
	cbc_s *base;
	cbc_varient_s *vari;

	if (!(base = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "base in add_cbc_build_varient");
	if (!(vari = malloc(sizeof(cbc_varient_s))))
		report_error(MALLOC_FAIL, "vari in add_cbc_build_varient");
	init_cbc_struct(base);
	init_varient(vari);
	base->varient = vari;
	snprintf(vari->varient, HOST_S, "%s", cvl->varient);
	snprintf(vari->valias, MAC_S, "%s", cvl->valias);
	if ((retval = run_insert(cmc, base, VARIENTS)) != 0) {
		printf("Unable to add varient %s to database\n", cvl->varient);
	} else {
		printf("Varient %s added to database\n", cvl->varient);
		printf("You can now use cbcpackage to define the packages");
		printf(" for this build varient\n");
	}
	clean_cbc_struct(base);
	return retval;
}

int
remove_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl)
{
	int retval = NONE;
	char varient[HOST_S];
	dbdata_s *data = '\0';

	cbc_init_initial_dbdata(&data, VARIENT_ID_ON_VALIAS);
	if (strncmp(cvl->varient, "NULL", COMM_S) != 0) {
		snprintf(data->args.text, HOST_S, "%s", cvl->varient);
		retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VARIENT);
		if (retval != 1) {
			clean_dbdata_struct(data);
			return VARIENT_NOT_FOUND;
		}
	} else if (strncmp(cvl->valias, "NULL", COMM_S) != 0) {
		snprintf(data->args.text, HOST_S, "%s", cvl->valias);
		retval = cbc_run_search(cmc, data, VARIENT_ID_ON_VALIAS);
		if (retval != 1) {
			clean_dbdata_struct(data);
			return VARIENT_NOT_FOUND;
		}
	}
	snprintf(varient, HOST_S, "%s", data->args.text);
	data->args.number = data->fields.number;
	if ((retval = cbc_run_delete(cmc, data, VARI_DEL_VARI_ID)) != 1) {
		fprintf(stderr, "%d varients deleted for %s\n",
			retval, varient);
		retval = MULTIPLE_VARIENTS;
	} else {
		printf("Varient %s deleted\n", varient);
		retval = NONE;
	}
	free(data);
	return retval;
}
