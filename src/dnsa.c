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
	int retval;

	domain = cmdb_malloc(CONF_S, "domain in main");
	dc = cmdb_malloc(sizeof(dnsa_config_s), "dc in main");
	cm = cmdb_malloc(sizeof(dnsa_comm_line_s), "cm in main");
	dnsa_init_all_config(dc, cm);
	if ((retval = parse_dnsa_command_line(argc, argv, cm)) != 0) {
		cmdb_free(domain, CONF_S);
		cmdb_free(dc, sizeof(dnsa_config_s));
		cmdb_free(cm, sizeof(dnsa_comm_line_s));
		display_command_line_error(retval, argv[0]);
	}
	/* Get config values from config file */
	if ((retval = parse_dnsa_config_file(dc, cm->config)) != 0) {
		parse_dnsa_config_error(retval);
		goto cleanup;
	}
	retval = 0;
	strncpy(domain, cm->domain, CONF_S);
	if (cm->type == FORWARD_ZONE) {
		if (cm->action == LIST_ZONES) {
			list_zones(dc);
			retval = 0;
		} else if (cm->action == DISPLAY_ZONE) {
			display_zone(domain, dc);
			retval = 0;
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
			retval = 0;
		} else if (cm->action == DISPLAY_ZONE) {
			display_rev_zone(domain, dc);
			retval = 0;
		} else if (cm->action == COMMIT_ZONES) {
			retval = commit_rev_zones(dc, domain);
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

	cleanup:
		cmdb_free(domain, CONF_S);
		cmdb_free(cm, sizeof(dnsa_comm_line_s));
		cmdb_free(dc, sizeof(dnsa_config_s));
		exit(retval);
}
