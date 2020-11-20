/*
 *
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_dnsa.h: DNSA header file 
 */

#ifndef __CMDB_DNSA_H__
# define __CMDB_DNSA_H__
# include "cmdb.h"

typedef struct dnsa_comm_line_s { /* Hold parsed command line args */
	short int action;
	short int type;
	unsigned long int prefix;
	char *rtype;
	char *ztype;
	char *service;
	char *protocol;
	char *domain;
	char *toplevel;
	char *config;
	char *host;
	char *dest;
	char *master;
	char *glue_ip;
	char *glue_ns;
} dnsa_comm_line_s;

// Get command line args and pass them. Put actions into the struct
int
parse_dnsa_command_line(int argc, char **argv, dnsa_comm_line_s *comm);
// Grab config values from file
int
parse_dnsa_config_file(ailsa_cmdb_s *dc, char *config);
int
read_dnsa_config_values(ailsa_cmdb_s *dc, FILE *cnf);
void
parse_dnsa_config_error(int error);
int
validate_comm_line(dnsa_comm_line_s *comm);
int
validate_fwd_comm_line(dnsa_comm_line_s *comm);
int
validate_glue_comm_line(dnsa_comm_line_s *comm);
int
validate_rev_comm_line(dnsa_comm_line_s *comm);
// Zone action Functions
int
add_fwd_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
add_rev_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
add_glue_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
commit_fwd_zones(ailsa_cmdb_s *dc, char *zone);
int
commit_rev_zones(ailsa_cmdb_s *dc, char *name);
int
add_host(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
display_multi_a_records(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
mark_preferred_a_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
build_reverse_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* Added 06/03/2013 */
int
delete_reverse_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
delete_glue_zone (ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* End addition 06/03/2013 */
/* Added 07/03/2013 */
int
delete_fwd_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* End addition 07/03/2013 */
/* Zone display functions */
void
list_zones(ailsa_cmdb_s *dc);
void
list_rev_zones(ailsa_cmdb_s *dc);
void
display_zone(char *domain, ailsa_cmdb_s *dc);
void
display_rev_zone(char *domain, ailsa_cmdb_s *dc);
void
list_glue_zones(ailsa_cmdb_s *dc);
/* Various zone functions */
/* Added 05/03/2013 */
int
delete_preferred_a(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
delete_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* End add 05/03/2013 */
/* Forward zone functions */
int
check_zone(char *domain, ailsa_cmdb_s *dc);
/* 04/03/2013 functions add */
int
add_cname_to_root_domain(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);

#endif /* __CMDB_DNSA_H__ */
