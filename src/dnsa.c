/* 
 * 
 *  dnsa: DNS Administration
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
 *  dnsa.c: main() function for the dnsa program
 *
 * 
 */
#include <config.h>
#include <configmake.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ailsacmdb.h>
#include "cmdb.h"
#include "dnsa_data.h"
#include "cmdb_dnsa.h"

static void
clean_dnsa_comm_line(void *dcl);

int main(int argc, char *argv[])
{
	char *domain = NULL;
	int retval;
	dnsa_comm_line_s *cm = ailsa_calloc(sizeof(dnsa_comm_line_s), "cm in main");
	ailsa_cmdb_s *dc = ailsa_calloc(sizeof(ailsa_cmdb_s), "dc in main");

	if ((retval = parse_dnsa_command_line(argc, argv, cm)) != 0) {
		ailsa_clean_cmdb(dc);
		clean_dnsa_comm_line(cm);
		display_command_line_error(retval, argv[0]);
	}
	parse_cmdb_config(dc);
	if (cm->domain)
		domain = cm->domain;
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
		} else if (cm->action == ADD_CNAME_ON_ROOT) {
			retval = add_cname_to_root_domain(dc, cm);
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

	clean_dnsa_comm_line(cm);
	ailsa_clean_cmdb(dc);
	exit(retval);
}

static void
clean_dnsa_comm_line(void *comm)
{
	if (!(comm))
		return;
	dnsa_comm_line_s *dcl = comm;
        if (dcl->rtype)
		my_free(dcl->rtype);
        if (dcl->ztype)
		my_free(dcl->ztype);
        if (dcl->service)
		my_free(dcl->service);
        if (dcl->protocol)
		my_free(dcl->protocol);
        if (dcl->domain)
		my_free(dcl->domain);
	if (dcl->config)
		my_free(dcl->config);
        if (dcl->host)
		my_free(dcl->host);
        if (dcl->dest)
		my_free(dcl->dest);
        if (dcl->master)
		my_free(dcl->master);
        if (dcl->glue_ip)
		my_free(dcl->glue_ip);
        if (dcl->glue_ns)
		my_free(dcl->glue_ns);
	if (dcl->toplevel)
		my_free(dcl->toplevel);
	my_free(dcl);
}
