/* 
 *
 *  cpc: Create Preseed Configuration
 *  Copyright (C) 2014 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cpc.c
 * 
 *  Main source file for cpc
 * 
 *  Part of the cpc program
 * 
 * 
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "cmdb.h"
#include "cmdb_cpc.h"
#include "cbc_data.h"
#include "checks.h"

int
main (int argc, char *argv[])
{
	int retval = NONE;
	cpc_config_s *cpc = '\0';
	cpc_comm_line_s *cl = '\0';

	if (!(cpc = malloc(sizeof(cpc_config_s))))
		report_error(MALLOC_FAIL, "cpc in main");
	if (!(cl = malloc(sizeof(cpc_comm_line_s))))
		report_error(MALLOC_FAIL, "cl in main");
	init_cpc_config(cpc);
	init_cpc_comm_line(cl);
	if ((retval = parse_cpc_comm_line(argc, argv, cl)) != 0) {
		clean_cpc_comm_line(cl);
		report_error(retval, "cpc command line");
	}
	output_preseed(cl, cpc);
	clean_cpc_comm_line(cl);
	clean_cpc_config(cpc);
	return retval;
}

int
parse_cpc_comm_line(int argc, char *argv[], cpc_comm_line_s *cl)
{
	int opt, retval = NONE;

	while ((opt = getopt(argc, argv, "f:n:")) != -1) {
		if (opt == 'f') {
			snprintf(cl->file, RBUFF_S, "%s", optarg);
		} else if (opt == 'n') {
			snprintf(cl->name, RBUFF_S, "%s", optarg);
		} else {
			fprintf(stderr, "Unknown option %c\n", opt);
			retval = DISPLAY_USAGE;
		}
	}
	return retval;
}

void
init_cpc_config(cpc_config_s *cpc)
{
	memset(cpc, 0, sizeof(cpc_config_s));
	if (!(cpc->name = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cpc->name init");
}

void
clean_cpc_config(cpc_config_s *cpc)
{
	if (cpc) {
		if (cpc->name)
			free(cpc->name);
	} else {
		return;
	}
	free(cpc);
}

void
init_cpc_comm_line(cpc_comm_line_s *cl)
{
	memset(cl, 0, sizeof(cpc_comm_line_s));
	if (!(cl->file = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cl->file init");
	if (!(cl->name = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "cl->name init");
}

void
clean_cpc_comm_line(cpc_comm_line_s *cl)
{
	if (cl) {
		if (cl->file)
			free(cl->file);
		if (cl->name)
			free(cl->name);
	} else {
		return;
	}
	free(cl);
}

void
output_preseed(cpc_comm_line_s *cl, cpc_config_s *cpc)
{
	string_len_s *output;
	FILE *outfile;

	if (!(output = malloc(sizeof(string_len_s))))
		report_error(MALLOC_FAIL, "output in output_preseed");
	init_string_len(output);
	clean_string_len(output);
}

