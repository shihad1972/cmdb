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
 *  build.c
 * 
 *  Contains functions to display / add / modify / delete build details in the
 *  database for the main cbc program.
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "build.h"


int
create_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml)
{
	int retval = NONE, query = NONE;
	cbc_s *cbc, *details;
	cbc_build_s *build;
	
	if (!(cbc = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "cbc in display_build_config");
	if (!(details = malloc(sizeof(cbc_s))))
		report_error(MALLOC_FAIL, "details in display_build_config");
	if (!(build = malloc(sizeof(cbc_build_s))))
		report_error(MALLOC_FAIL, "build in display_build_config");
	init_cbc_struct(cbc);
	init_cbc_struct(details);
	init_build_struct(build);
	cbc->build = build;
	query = BUILD_DOMAIN | BUILD_IP | BUILD_TYPE | BUILD_OS | 
	  CSERVER | LOCALE | SPART | VARIENT | SSCHEME;
	if ((retval = run_multiple_query(cbt, cbc, query)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return MY_QUERY_FAIL;
	}
	if ((retval = cbc_get_server(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return SERVER_NOT_FOUND;
	}
	clean_cbc_struct(cbc);
	free(details);

	return retval;
}