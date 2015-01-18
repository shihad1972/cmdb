/*
 *
 *  cbcscript: Create Build Configuration Scripts
 *  Copyright (C) 2014 - 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcscript.c
 *
 *  Functions to get configuration values and also parse command line arguments
 *
 *  Part of the cbcscript program
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
#include "cbc_common.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcscript.h"

int
main(int argc, char *argv[])
{
	const char *config = "/etc/dnsa/dnsa.conf";
	int retval = 0;
	cbc_syss_s *scr = 0;
	cbc_config_s *cbc;

	if (!(cbc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cbc in main");
	init_cbc_config_values(cbc);
	initialise_cbc_scr(&scr);
	if ((retval = parse_cbc_script_comm_line(argc, argv, scr)) != 0) {
		clean_cbc_syss_s(scr);
		free(cbc);
		display_command_line_error(retval, argv[0]);
	}
	if ((retval = parse_cbc_config_file(cbc, config)) != 0) {
		clean_cbc_syss_s(scr);
		free(cbc);
		parse_cbc_config_error(retval);
		exit(retval);
	}
	if (scr->action == ADD_CONFIG) {
		if (scr->what == CBCSCRIPT)
			retval = cbc_script_add_script(cbc, scr);
		else if (scr->what == CBCSCRARG)
			retval = cbc_script_add_script_arg(cbc, scr);
		else
			retval = WRONG_TYPE;
	} else if (scr->action == RM_CONFIG) {
		if (scr->what == CBCSCRIPT)
			retval = cbc_script_rm_script(cbc, scr);
		else
			retval = WRONG_TYPE;
	} else if (scr->action == LIST_CONFIG) {
			retval = cbc_script_list_script(cbc);
	} else {
		retval = WRONG_ACTION;
	}
	free(cbc);
	clean_cbc_syss_s(scr);
	if ((retval != 0) && (retval != NO_RECORDS))
		report_error(retval, "");
	return retval;
}

// Helper functions

void
initialise_cbc_scr(cbc_syss_s **scr)
{
	if (!(*scr = malloc(sizeof(cbc_syss_s))))
		report_error(MALLOC_FAIL, "scr in main");
	init_cbc_sys_script_s(*scr);
}

void
init_cbc_sys_script_s(cbc_syss_s *scr)
{
	memset(scr, 0, sizeof(cbc_syss_s));
}

void
clean_cbc_syss_s(cbc_syss_s *scr)
{
	if (!(scr))
		return;
	if (scr->name)
		free(scr->name);
	if (scr->arg)
		free(scr->arg);
	if (scr->domain)
		free(scr->domain);
	if (scr->type)
		free(scr->type);
	free(scr);
}

int
parse_cbc_script_comm_line(int argc, char *argv[], cbc_syss_s *cbcs)
{
	int retval = 0, opt;

	while ((opt = getopt(argc, argv, "ab:fg:ln:o:rst:")) != -1) {
		if (opt == 'a')
			cbcs->action = ADD_CONFIG;
		else if (opt == 'l')
			cbcs->action = LIST_CONFIG;
		else if (opt == 'r')
			cbcs->action = RM_CONFIG;
		else if (opt == 's')
			cbcs->what = CBCSCRIPT;
		else if (opt == 'f')
			cbcs->what = CBCSCRARG;
		else if (opt == 'o')
			cbcs->no = strtoul(optarg, NULL, 10);
		else if (opt == 'b') {
			if (!(cbcs->domain = strndup(optarg, (CONF_S -1))))
				report_error(MALLOC_FAIL, "cbcs->domain");
		} else if (opt == 'g') {
			if (!(cbcs->arg = strndup(optarg, (CONF_S - 1))))
				report_error(MALLOC_FAIL, "cbcs->arg");
		} else if (opt == 'n') {
			if (!(cbcs->name = strndup(optarg, (CONF_S - 1))))
				report_error(MALLOC_FAIL, "cbcs->arg");
		} else if (opt == 't') {
			if (!(cbcs->type = strndup(optarg, (MAC_S - 1))))
				report_error(MALLOC_FAIL, "cbcs->type");
		} else
			retval = DISPLAY_USAGE;
	}
	if (argc == 1)
		retval = DISPLAY_USAGE;
	else
		retval = check_cbc_script_comm_line(cbcs);
	return retval;
}

int
check_cbc_script_comm_line(cbc_syss_s *cbcs)
{
	int retval = 0;

	if (cbcs->action == 0)
		retval = NO_ACTION;
	else if (cbcs->what == 0)
		retval = NO_TYPE;
	else if (cbcs->action != LIST_CONFIG) {
		if (!(cbcs->name))
			retval = NO_NAME;
		else if (cbcs->what == CBCSCRARG) {
			if  (!(cbcs->arg))
				retval = NO_ARG;
			else if (cbcs->no == 0)
				retval = NO_NUMBER;
		}
	} else if (cbcs->what == CBCSCRARG) {
		if (!(cbcs->name))
			retval = NO_NAME;
		else if (!(cbcs->arg))
			retval = NO_ARG;
		else if (!(cbcs->domain) || !(cbcs->type))
			retval = DISPLAY_USAGE;
	}
	return retval;
}

int
pack_script_arg(cbc_config_s *cbc, cbc_script_arg_s *arg, cbc_syss_s *scr)
{
	int retval = 0;
	if (scr->name) {
		if ((retval = get_system_script_id(cbc, scr->name, &(arg->systscr_id))) != 0)
			return retval;
	} else {
		return NO_NAME;
	}
	if (scr->domain) {
		if ((retval = get_build_domain_id(cbc, scr->domain, &(arg->bd_id))) != 0)
			return retval;
	} else {
		return NO_DOMAIN;
	}
	if (scr->type) {
		if ((retval = get_build_type_id(cbc, scr->type, &(arg->bt_id))) != 0)
			return retval;
	} else {
		return NO_DATA;
	}
	if (scr->arg)
		snprintf(arg->arg, URL_S, "%s", scr->arg);
	else
		return NO_DATA;
	if (scr->no > 0)
		arg->no = scr->no;
	else
		return NO_DATA;
	arg->cuser = arg->muser = (unsigned long int)getuid();
	return 0;
}

// Add functions

int
cbc_script_add_script(cbc_config_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_script_s *cbcscr;

	initialise_cbc_s(&cbs);
	initialise_cbc_scripts(&cbcscr);
	cbs->scripts = cbcscr;
	cbcscr->cuser = cbcscr->muser = (unsigned long int)getuid();
	snprintf(cbcscr->name, URL_S, "%s", scr->name);
	if ((retval = cbc_run_insert(cbc, cbs, SCRIPTS)) != 0)
		fprintf(stderr, "Unable to add script to database\n");
	else
		printf("Script %s added to database\n", scr->name);
	clean_cbc_struct(cbs);
	return retval;
}

int
cbc_script_add_script_arg(cbc_config_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_script_arg_s *arg;

	initialise_cbc_s(&cbs);
	initialise_cbc_script_args(&arg);
	cbs->script_arg = arg;
	if ((retval = pack_script_arg(cbc, arg, scr)) != 0)
		goto cleanup;
	if ((retval = cbc_run_insert(cbc, cbs, SCRIPTAS)) != 0)
		fprintf(stderr, "Unable to add args for script to db\n");
	else
		printf("Script args added into db\n");
	goto cleanup;

	cleanup:
		clean_cbc_struct(cbs);
		return retval;
}

// Remove functions

int
cbc_script_rm_script(cbc_config_s *cbc, cbc_syss_s *scr)
{
	int retval = 0;
	dbdata_s *data;

	init_multi_dbdata_struct(&data, 1);
	if ((retval = get_system_script_id(cbc, scr->name, &(data->args.number))) != 0) {
		clean_dbdata_struct(data);
		return retval;
	}
	if ((retval = cbc_run_delete(cbc, data, CBCSCR_ON_ID)) == 0) {
		fprintf(stderr, "Unable to remove script %s\n", scr->name);
		retval = DB_DELETE_FAILED;
	} else if (retval > 1) {
		fprintf(stderr, "Multiple scripts removed for %s\n", scr->name);
		retval = 0;
	} else {
		printf("Script %s removed from database\n", scr->name);
		retval = 0;
	}
	clean_dbdata_struct(data);
	return retval;
}

// List functions

int
cbc_script_list_script(cbc_config_s *cbc)
{
	int retval = 0;
	cbc_s *cbs;
	cbc_script_s *script;
	size_t len;
	time_t create;

	initialise_cbc_s(&cbs);
	if ((retval = cbc_run_query(cbc, cbs, SCRIPT)) != 0) {
		clean_cbc_struct(cbs);
		return retval;
	}
	script = cbs->scripts;
	if (script)
		printf("Script\t\t\tCreation User\tCreation Time\n");
	else
		printf("No scripts in the database\n");
	while (script) {
		create = (time_t)script->ctime;
		printf("%s", script->name);
		len = strlen(script->name);
		if (len < 8)
			printf("\t\t\t");
		else if (len < 16)
			printf("\t\t");
		else
			printf("\t");
		len = strlen(get_uname(script->cuser));
		printf("%s", get_uname(script->cuser));
		if (len < 8)
			printf("\t\t");
		else
			printf("\t");
		printf("%s", ctime(&create));
		script = script->next;
	}
	clean_cbc_struct(cbs);
	return retval;
}

