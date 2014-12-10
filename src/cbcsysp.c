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
	int retval;
	cbc_sysp_s *cbs;

	if (!(cbs = malloc(sizeof(cbc_sysp_s))))
		report_error(MALLOC_FAIL, "cbs in main");
	init_cbcsysp_s(cbs);
	if ((retval = parse_cbc_sysp_comm_line(argc, argv, cbs)) != 0) {
		clean_cbcsysp_s(cbs);
		display_command_line_error(retval, argv[0]);
	}

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
			if (!(cbcs->name = calloc(MAC_S, sizeof(char))))
				report_error(MALLOC_FAIL, "cbcs->type in parse_cbc_sysp_comm_line");
			snprintf(cbcs->type, MAC_S, "%s", optarg);
		} else
			retval = DISPLAY_USAGE;
	}
	if ((cbcs->what == 0) || !(cbcs->name))
		retval = DISPLAY_USAGE;
	else if ((cbcs->what == SPACKARG) && (!(cbcs->type) || !(cbcs->field)))
		retval = DISPLAY_USAGE;
	else if ((cbcs->what == SPACKCNF) && (!(cbcs->arg) || !(cbcs->domain)))
		retval = DISPLAY_USAGE;
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


