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
 *  cbc.c
 * 
 *  main function for the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "build.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int
main(int argc, char *argv[])
{
	cbc_config_s *cmc;
	cbc_comm_line_s *cml;
	char sretval[MAC_S], conf[CONF_S];
	const char *config;
	int retval;
	
	retval = 0;
	get_config_file_location(conf);
	config = conf;
	if (!(cmc = malloc(sizeof(cbc_config_s))))
		report_error(MALLOC_FAIL, "cmc in cbc.c");
	if (!(cml = malloc(sizeof(cbc_comm_line_s))))
		report_error(MALLOC_FAIL, "cml in cbc.c");

	init_all_config(cmc, cml);
	retval = parse_cbc_command_line(argc, argv, cml);
	if (retval != 0) {
		free(cmc);
		free(cml);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	retval = parse_cbc_config_file(cmc, config);
	if (retval > 1) {
		parse_cbc_config_error(retval);
		free(cml);
		free(cmc);
		exit(retval);
	}
	if (cml->action == DISPLAY_CONFIG)
		retval = display_build_config(cmc, cml);
	else if (cml->action == LIST_CONFIG)
		retval = list_build_servers(cmc);
	else if (cml->action == WRITE_CONFIG)
		retval = write_build_config(cmc, cml);
	else if (cml->action == ADD_CONFIG)
		retval = create_build_config(cmc, cml);
	else if (cml->action == MOD_CONFIG)
		retval = modify_build_config(cmc, cml);
	else if (cml->action == RM_CONFIG)
		retval = remove_build_config(cmc, cml);
	else
		printf("Case %d not implemented yet\n", cml->action);
	free(cmc);
	free(cml);
	if (retval == DISPLAY_USAGE)
		retval = NONE;
	if (retval != NONE) {
		get_error_string(retval, sretval);
		report_error(CREATE_BUILD_FAILED, sretval);
	}
	exit(retval);
}
