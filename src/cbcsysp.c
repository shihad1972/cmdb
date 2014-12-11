/* 
 *
 *  cbcsysp: Create Build Configuration Partition
 *  Copyright (C) 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcsysp.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  Part of the cbcsysp program
 * 
 *  (C) Iain M. Conochie 2014
 * 
 */
#define _GNU_SOURCE
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcsysp.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	int retval;
	cbc_config_s *cbc;
	cbc_sysp_s *cbs;

	if (!(cbc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cbc in main");
	if (!(cbs = malloc(sizeof(cbc_sysp_s))))
		report_error(MALLOC_FAIL, "cbs in main");
	init_cbcsysp_s(cbs);
	init_cbc_config_values(cbc);
	if ((retval = parse_cbc_sysp_comm_line(argc, argv, cbs)) != 0) {
		clean_cbcsysp_s(cbs);
		free(cbc);
		display_command_line_error(retval, argv[0]);
	}
	if ((retval = parse_cbc_config_file(cbc, config)) != 0) {
		clean_cbcsysp_s(cbs);
		free(cbc);
		parse_cbc_config_error(retval);
		exit(retval);
        }
	if (cbs->what == SPACKAGE) {
		if (cbs->action == LIST_CONFIG)
			retval = list_cbc_syspackage(cbc);
		else if (cbs->action == ADD_CONFIG)
			retval = add_cbc_syspackage(cbc, cbs);
		else
			retval = WRONG_ACTION;
	} else if (cbs->what == SPACKARG) {
		if (cbs->action == LIST_CONFIG)
			retval = list_cbc_syspackage_arg(cbc, cbs);
		else if (cbs->action == ADD_CONFIG)
			retval = add_cbc_syspackage_arg(cbc, cbs);
		else
			retval = WRONG_ACTION;
	} else if (cbs->what == SPACKCNF) {
		retval = WRONG_ACTION;
	}
	if (retval == WRONG_ACTION)
		fprintf(stderr, "Action not supported for type\n");
	clean_cbcsysp_s(cbs);
	free(cbc);
	return retval;
}

int
parse_cbc_sysp_comm_line(int argc, char *argv[], cbc_sysp_s *cbcs)
{
	int retval = 0, opt;

	while ((opt = getopt(argc, argv, "ab:df:g:lmn:oprt:vy")) != -1) {
		if (opt == 'a')
			cbcs->action = ADD_CONFIG;
		else if (opt == 'd')
			cbcs->action = DISPLAY_CONFIG;
		else if (opt == 'l')
			cbcs->action = LIST_CONFIG;
		else if (opt == 'm')
			cbcs->action = MOD_CONFIG;
		else if (opt == 'r')
			cbcs->action = RM_CONFIG;
		else if (opt == 'v')
			cbcs->action = CVERSION;
		else if (opt == 'o')
			cbcs->what = SPACKCNF;
		else if (opt == 'p')
			cbcs->what = SPACKAGE;
		else if (opt == 'y')
			cbcs->what = SPACKARG;
		else if (opt == 'b') {
			if (!(cbcs->domain = calloc(RBUFF_S, sizeof(char))))
				report_error(MALLOC_FAIL, "cbcs->domain in parse_cbc_sysp_comm_line");
			snprintf(cbcs->domain, RBUFF_S, "%s", optarg);
		} else if (opt == 'f') {
			if (!(cbcs->field = calloc(URL_S, sizeof(char))))
				report_error(MALLOC_FAIL, "cbcs->field in parse_cbc_sysp_comm_line");
			snprintf(cbcs->field, URL_S, "%s", optarg);
		} else if (opt == 'g') {
			if (!(cbcs->arg = calloc(RBUFF_S, sizeof(char))))
				report_error(MALLOC_FAIL, "cbcs->arg in parse_cbc_sysp_comm_line");
			snprintf(cbcs->arg, RBUFF_S, "%s", optarg);
		} else if (opt == 'n') {
			if (!(cbcs->name = calloc(URL_S, sizeof(char))))
				report_error(MALLOC_FAIL, "cbcs->name in parse_cbc_sysp_comm_line");
			snprintf(cbcs->name, URL_S, "%s", optarg);
		} else if (opt == 't') {
			if (!(cbcs->type = calloc(MAC_S, sizeof(char))))
				report_error(MALLOC_FAIL, "cbcs->type in parse_cbc_sysp_comm_line");
			snprintf(cbcs->type, MAC_S, "%s", optarg);
		} else
			retval = DISPLAY_USAGE;
	}
	if (cbcs->what == 0) 
		retval = DISPLAY_USAGE;
	else if (cbcs->what != SPACKAGE) {
		if (!(cbcs->name))
			retval = DISPLAY_USAGE;
		else if (cbcs->action != LIST_CONFIG) {
			if  ((cbcs->what == SPACKARG) && (!(cbcs->type) || !(cbcs->field)))
				retval = DISPLAY_USAGE;
			else if ((cbcs->what == SPACKCNF) && (!(cbcs->arg) || !(cbcs->domain)))
				retval = DISPLAY_USAGE;
		}
	}
	return retval;
}

void
init_cbcsysp_s(cbc_sysp_s *cbcs)
{
	memset(cbcs, 0, sizeof(cbc_sysp_s));
}

void
clean_cbcsysp_s(cbc_sysp_s *cbcs)
{
	if (!(cbcs))
		return;
	if (cbcs->arg)
		free(cbcs->arg);
	if (cbcs->domain)
		free(cbcs->domain);
	if (cbcs->field)
		free(cbcs->field);
	if (cbcs->name)
		free(cbcs->name);
	if (cbcs->type)
		free(cbcs->type);
	free(cbcs);
}

// List functions

int
list_cbc_syspackage(cbc_config_s *cbc)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_syspack_s *list;

	initialise_cbc_s(&cbs);
	if ((retval = cbc_run_query(cbc, cbs, SYSPACK)) == 0) {
		list = cbs->syspack;
		while (list) {
			printf("%s\n", list->name);
			list = list->next;
		}
	} else if (retval == 6)
		fprintf(stderr, "No system packages to display\n");
	clean_cbc_struct(cbs);
	return retval;
}

int
list_cbc_syspackage_arg(cbc_config_s *cbc, cbc_sysp_s *css)
{
	int retval = 0;
	dbdata_s *data = 0;
	cbc_s *cbs;
	cbc_syspack_arg_s *cspa, *list;

	initialise_cbc_s(&cbs);
	initialise_cbc_syspack_arg(&cspa);
	cbs->sysarg = cspa;
// First run query for arguments
	if ((retval = cbc_run_query(cbc, cbs, SYSARG)) == NO_RECORDS) {
		fprintf(stderr, "\
There are no arguments configured in the database for any package\n");
		goto cleanup;
	} else if (retval != 0)
		goto cleanup;
	init_multi_dbdata_struct(&data, 1);
	snprintf(data->args.text, URL_S, "%s", css->name);
	if ((retval = cbc_run_search(cbc, data, SYSPACK_ID_ON_NAME)) == 0) {
		fprintf(stderr, "\
Package %s does not have any configured arguments\n", css->name);
		retval = NO_RECORDS;
		goto cleanup;
	} else if (retval > 1) {
		fprintf(stderr, " Multiple id's for package %s??\n", css->name);
	}
	list = cspa;
	while (list) {
		if (list->syspack_id == data->fields.number)
			printf("%s\t%s\t%s\n", css->name, list->field, list->type);
		list = list->next;
	}
	goto cleanup;
	cleanup:
		clean_dbdata_struct(data);
		clean_cbc_struct(cbs);
		return retval;
}

// Add functions

int
add_cbc_syspackage(cbc_config_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_syspack_s *spack;

	initialise_cbc_s(&cbs);
	initialise_cbc_syspack(&spack);
	cbs->syspack = spack;
	pack_syspack(spack, cbcs);
	if ((retval = cbc_run_insert(cbc, cbs, SYSPACKS)) != 0)
		fprintf(stderr, "Cannot insert system package into DB\n");
	else
		printf("Package %s inserted into db\n", spack->name);
	clean_cbc_struct(cbs);
	return retval;
}

int
add_cbc_syspackage_arg(cbc_config_s *cbc, cbc_sysp_s *cbcs)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_syspack_arg_s *cpsa;
	dbdata_s *data = 0;

	init_multi_dbdata_struct(&data, 1);
	snprintf(data->args.text, URL_S, "%s", cbcs->name);
	if ((retval = cbc_run_search(cbc, data, SYSPACK_ID_ON_NAME)) == 0) {
		clean_dbdata_struct(data);
		fprintf(stderr, "No system package of the name %s\n", cbcs->name);
		return NO_RECORDS;
	}
	initialise_cbc_s(&cbs);
	initialise_cbc_syspack_arg(&cpsa);
	cbs->sysarg = cpsa;
	cpsa->syspack_id = data->fields.number;
	clean_dbdata_struct(data);
	pack_sysarg(cpsa, cbcs);
	if ((retval = cbc_run_insert(cbc, cbs, SYSARGS)) != 0)
		fprintf(stderr, "Cannot insert system package into DB\n");
	else
		printf("Package args for package %s inserted into DB\n", cbcs->name);
	clean_cbc_struct(cbs);
	return retval;
}

// Helper functions

void
pack_syspack(cbc_syspack_s *spack, cbc_sysp_s *cbs)
{
	snprintf(spack->name, URL_S, "%s", cbs->name);
	spack->cuser = spack->muser = (unsigned long int)getuid();
}

void
pack_sysarg(cbc_syspack_arg_s *cpsa, cbc_sysp_s *cbs)
{
	snprintf(cpsa->type, MAC_S, "%s", cbs->type);
	snprintf(cpsa->field, URL_S, "%s", cbs->field);
	cpsa->cuser = cpsa->muser = (unsigned long int)getuid();
}

