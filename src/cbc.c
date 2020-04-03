/* 
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2012 - 2020  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ailsacmdb.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "build.h"

int
main(int argc, char *argv[])
{
	ailsa_cmdb_s *cmc;
	cbc_comm_line_s *cml;
	char sretval[CONF_S], conf[CONF_S];
	int retval = NONE;
	
	if (!(cmc = malloc(sizeof(ailsa_cmdb_s))))
		report_error(MALLOC_FAIL, "cmc in cbc.c");
	if (!(cml = malloc(sizeof(cbc_comm_line_s))))
		report_error(MALLOC_FAIL, "cml in cbc.c");

	init_all_config(cmc, cml);
	if ((retval = parse_cbc_command_line(argc, argv, cml)) != 0) {
		free(cmc);
		free(cml);
		display_command_line_error(retval, argv[0]);
	}
	if (cml->action == QUERY_CONFIG)
		retval = query_config();
	else {
		get_config_file_location(conf);
		if ((retval = parse_cbc_config_file(cmc, conf)) > 1) {
			parse_cbc_config_error(retval);
			free(cml);
			free(cmc);
			exit(retval);
		}
	}
	if (cml->action == DISPLAY_CONFIG)
		retval = display_build_config(cmc, cml);
	else if (cml->action == LIST_CONFIG)
		list_build_servers(cmc);
	else if (cml->action == WRITE_CONFIG)
		retval = write_build_config(cmc, cml);
	else if (cml->action == ADD_CONFIG)
		retval = create_build_config(cmc, cml);
	else if (cml->action == MOD_CONFIG)
		retval = modify_build_config(cmc, cml);
	else if (cml->action == RM_CONFIG)
		retval = remove_build_config(cmc, cml);
	else if (cml->action == QUERY_CONFIG)
		;
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

int
query_config()
{
	int retval = NONE;
#if defined HAVE_MYSQL
# if defined HAVE_SQLITE3
	printf("both\n");
# else
	printf("mysql\n");
# endif
#elif defined HAVE_SQLITE3
	printf("sqlite\n");
#else
	printf("none\n");
#endif
	return retval;
}
