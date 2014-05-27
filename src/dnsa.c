/* 
 * 
 *  dnsa: DNS Administration
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
 * dnsa.c: main() function for the dnsa program
 *
 * 
 */
#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

int main(int argc, char *argv[])
{
	dnsa_comm_line_s *cm;
	dnsa_config_s *dc;
	char *domain;
	int retval, id;

	if (!(domain = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "domain in dnsa.c");
	if (!(dc = malloc(sizeof(dnsa_config_s))))
		report_error(MALLOC_FAIL, "dc in dnsa.c");
	if (!(cm = malloc(sizeof(dnsa_comm_line_s))))
		report_error(MALLOC_FAIL, "cm in dnsa.c");
	
	dnsa_init_all_config(dc, cm);
	retval = parse_dnsa_command_line(argc, argv, cm);
	if (retval != 0) {
		free(domain);
		free(dc);
		free(cm);
		display_command_line_error(retval, argv[0]);
	}
	/* Get config values from config file */
	id = parse_dnsa_config_file(dc, cm->config);
	if (id > 1) {
		parse_dnsa_config_error(retval);
		free(domain);
		free(dc);
		free(cm);
		exit(id);
	}
	retval = id = 0;
	strncpy(domain, cm->domain, CONF_S);
	if (cm->type == FORWARD_ZONE) {
#ifdef HAVE_LIBPCRE
		retval = validate_user_input(domain, DOMAIN_REGEX);
		if (retval < 0) {
			printf("User input not valid!\n");
			free(domain);
			free(dc);
			free(cm);
			exit (retval);
		}
#endif /* HAVE_LIBPCRE */
		if (cm->action == LIST_ZONES) {
			list_zones(dc);
		} else if (cm->action == DISPLAY_ZONE) {
			display_zone(domain, dc);
		} else if (cm->action == COMMIT_ZONES) {
			retval = commit_fwd_zones(dc, domain);
		} else if (cm->action == ADD_HOST) {
			retval = add_host(dc, cm);
		} else if (cm->action == ADD_ZONE) {
			retval = add_fwd_zone(dc, cm);
		} else if (cm->action == DELETE_RECORD) {
			retval = delete_record(dc, cm);
		} else if (cm->action == DELETE_ZONE) {
			retval = delete_fwd_zone(dc, cm);
		} else {
			printf("Action code %d not implemented\n", cm->action);
		}
	} else if (cm->type == REVERSE_ZONE) {
		if (cm->action == LIST_ZONES) {
			list_rev_zones(dc);
		} else if (cm->action == DISPLAY_ZONE) {
			display_rev_zone(domain, dc);
		} else if (cm->action == COMMIT_ZONES) {
			retval = commit_rev_zones(dc);
		} else if (cm->action == ADD_ZONE) {
			retval = add_rev_zone(dc, cm);
		} else if (cm->action == MULTIPLE_A) {
			retval = display_multi_a_records(dc, cm);
		} else if (cm->action == ADD_PREFER_A) {
			retval = mark_preferred_a_record(dc, cm);
		} else if (cm->action == BUILD_REV) {
			retval = build_reverse_zone(dc, cm);
		} else if (cm->action == DELETE_PREFERRED) {
			retval = delete_preferred_a(dc, cm);
		} else if (cm->action == DELETE_ZONE) {
			retval = delete_reverse_zone(dc, cm);
		} else {
			printf("Action code %d not implemented\n", cm->action);
		}
	} else if (cm->type == GLUE_ZONE) {
		if (cm->action == ADD_ZONE)
			retval = add_glue_zone(dc, cm);
		else if (cm->action == LIST_ZONES)
			list_glue_zones(dc);
		else if (cm->action == DELETE_ZONE)
			delete_glue_zone(dc, cm);
		else
			printf("Action code %d not implemented\n", cm->action);
	}

	free(domain);
	free(cm);
	free(dc);
	exit(retval);
}
