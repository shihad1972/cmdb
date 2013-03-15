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
 *  builddomain.c
 * 
 *  Functions to get configuration values and also parse command line arguments
 * 
 *  part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_cbc.h"
#include "cbc_data.h"
#include "base_sql.h"
#include "cbc_base_sql.h"
#include "checks.h"
#include "cbcdomain.h"
#include "builddomain.h"

int
display_cbc_build_domain(cbc_config_t *cbc, cbcdomain_comm_line_s *cdl)
{
	int retval = NONE;
	cbc_t *base;

	if (!(base = malloc(sizeof(cbc_t))))
		report_error(MALLOC_FAIL, "base in display_cbc_build_domain");
	init_cbc_struct(base);
	if ((retval = run_query(cbc, base, BUILD_DOMAIN)) != 0) {
		fprintf(stderr, "build query failed\n");
		free(cbc);
		return retval;
	}
	if ((retval = get_build_domain(cdl, base)) != 0) {
		fprintf(stderr, "build domain %s not found\n", cdl->domain);
		free(cbc);
		return retval;
	}
	display_build_domain(base);
	clean_cbc_struct(base);
	return retval;
}

int
add_cbc_build_domain((cbc_config_t *cbc, cbcdomain_comm_line_s *cdl)
{
	int retval = NONE;

	return retval;
}

int
list_cbc_build_domain(cbc_config_t *cbc)
{
	int retval = NONE;
	cbc_t *base;
	cbc_build_domain_t *bdom;

	if (!(base = malloc(sizeof(cbc_t))))
		report_error(MALLOC_FAIL, "base in list_cbc_build_domain");
	init_cbc_struct(base);
	if ((retval = run_query(cbc, base, BUILD_DOMAIN)) != 0) {
		fprintf(stderr, "build query failed\n");
		free(cbc);
		return retval;
	}
	bdom = base->bdom;
	printf("Build Domains\n\n");
	while (bdom) {
		printf("%s\n", bdom->domain);
		bdom = bdom->next;
	}
	clean_cbc_struct(base);
	return retval;
}

int
get_build_domain(cbcdomain_comm_line_s *cdl, cbc_t *base)
{
	int retval = NONE;
	char *domain = cdl->domain;
	cbc_build_domain_t *bdom = base->bdom, *next;
	base->bdom = '\0';

	if (bdom)
		next = bdom->next;
	else
		return BUILD_DOMAIN_NOT_FOUND;
	while (bdom) {
		bdom->next = '\0';
		if (strncmp(bdom->domain, domain, RBUFF_S) == 0)
			base->bdom = bdom;
		else
			free(bdom);
		bdom = next;
		if (next)
			next = next->next;
	}
	if (!base->bdom)
		retval = BUILD_DOMAIN_NOT_FOUND;
	return retval;
}
