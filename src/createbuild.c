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
 *  createbuild.c
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
	  CSERVER | LOCALE | DPART | VARIENT | SSCHEME;
	if ((retval = cbc_run_multiple_query(cbt, cbc, query)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return MY_QUERY_FAIL;
	}
	if ((retval = cbc_get_server(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return SERVER_NOT_FOUND;
	}
	if ((retval = cbc_get_os(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return OS_NOT_FOUND;
	}
	if ((retval = cbc_get_build_domain(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return BUILD_DOMAIN_NOT_FOUND;
	}
	if ((retval = cbc_get_seed_scheme(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return SCHEME_NOT_FOUND;
	}
	if ((retval = cbc_get_varient(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return VARIENT_NOT_FOUND;
	}
	if ((retval = cbc_get_locale(cml, cbc, details)) != 0) {
		clean_cbc_struct(cbc);
		free(details);
		return LOCALE_NOT_FOUND;
	}
	cbc_get_build_config(cbc, details, build);
	clean_cbc_struct(cbc);
	free(details);

	return retval;
}

int
cbc_get_os(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_build_os_s *list = cbc->bos;
	while (list) {
		if (((strncmp(list->os, cml->os, MAC_S) == 0) ||
		     (strncmp(list->alias, cml->os, MAC_S) == 0)) &&
		     (strncmp(list->version, cml->os_version, MAC_S) == 0) &&
		     (strncmp(list->arch, cml->arch, RANGE_S) == 0))
			details->bos = list;
		list = list->next;
	}
	if (!details->bos)
		retval = OS_NOT_FOUND;
	return retval;
}

int
cbc_get_build_domain(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_build_domain_s *list = cbc->bdom;
	while (list) {
		if (strncmp(list->domain, cml->build_domain, RBUFF_S) == 0)
			details->bdom = list;
		list = list->next;
	}
	if (!details->bdom)
		retval = BUILD_DOMAIN_NOT_FOUND;
	return retval;
}

int
cbc_get_varient(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_varient_s *list = cbc->varient;
	while (list) {
		if ((strncmp(list->varient, cml->varient, CONF_S) == 0) ||
		    (strncmp(list->valias, cml->varient, MAC_S) == 0))
			details->varient = list;
		list = list->next;
	}
	if (!details->varient)
		retval = VARIENT_NOT_FOUND;
	return retval;
}

int
cbc_get_seed_scheme(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_seed_scheme_s *list = cbc->sscheme;
	while (list) {
		if (strncmp(list->name, cml->partition, CONF_S) == 0)
			details->sscheme = list;
		list = list->next;
	}
	if (!details->sscheme)
		retval = SCHEME_NOT_FOUND;
	return retval;
}

int
cbc_get_locale(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	cbc_locale_s *list = cbc->locale;
	if (cml->locale != 0) {
		while (list) {
			if (list->locale_id == cml->locale)
				details->locale = list;
			list = list->next;
		}
	} else {
		while (list) {
			if (list->os_id == details->bos->os_id)
				details->locale = list;
			list = list->next;
		}
	}
	if (!details->locale)
		retval = LOCALE_NOT_FOUND;
	return retval;
}

int
cbc_get_build_config(cbc_s *cbc, cbc_s *details, cbc_build_s *build)
{
	int retval = NONE;

	if ((retval = cbc_get_build_partitons(cbc, details)) != 0)
		return retval;
	build->varient_id = details->varient->varient_id;
	build->server_id = details->server->server_id;
	build->os_id = details->bos->os_id;
	build->locale_id = details->locale->locale_id;
	build->def_scheme_id = details->sscheme->def_scheme_id;

	return retval;
}

int
cbc_get_build_partitons(cbc_s *cbc, cbc_s *details)
{
	int retval = NONE;
	unsigned long int scheme_id = details->sscheme->def_scheme_id;
	cbc_pre_part_s *part, *new, *old, *next, *list = cbc->dpart;

	part = new = old = next = '\0';
	while (list) {
		if (list->link_id.def_scheme_id == scheme_id) {
			if (!(new = malloc(sizeof(cbc_pre_part_s))))
				report_error(MALLOC_FAIL, "new in get build part");
			memcpy(new, list, sizeof(cbc_pre_part_s));
			new->next = '\0';
			if (!part) {
				part = new;
			} else if (!part->next) {
				part->next = new;
			} else {
				next = part->next;
				old = part;
				while (next) {
					old = next;
					next = next->next;
				}
				old->next = new;
			}
		}
		list = list->next;
	}
	if (!part)
		retval = PARTITIONS_NOT_FOUND;
	return retval;
}
