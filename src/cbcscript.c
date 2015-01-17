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
	free(scr);
}

int
parse_cbc_script_comm_line(int argc, char *argv[], cbc_syss_s *cbcs)
{
	int retval = 0, opt;

	while ((opt = getopt(argc, argv, "ag:ln:o:rst")) != -1) {
		if (opt == 'a')
			cbcs->action = ADD_CONFIG;
		else if (opt == 'l')
			cbcs->action = LIST_CONFIG;
		else if (opt == 'r')
			cbcs->action = RM_CONFIG;
		else if (opt == 's')
			cbcs->what = CBCSCRIPT;
		else if (opt == 't')
			cbcs->what = CBCSCRARG;
		else if (opt == 'o')
			cbcs->no = strtoul(optarg, NULL, 10);
		else if (opt == 'g') {
			if (!(cbcs->arg = strndup(optarg, (CONF_S - 1))))
				report_error(MALLOC_FAIL, "cbcs->arg");
		} else if (opt == 'n') {
			if (!(cbcs->name = strndup(optarg, (CONF_S - 1))))
				report_error(MALLOC_FAIL, "cbcs->arg");
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
	int retval;

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
	}
	return retval;
}
