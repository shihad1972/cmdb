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
# include "base_sql.h"
# include "cmdb.h"
# include "dnsa_data.h"

enum {			/* zone types; use NONE from action codes */
	FORWARD_ZONE = 1,
	REVERSE_ZONE = 2,
	GLUE_ZONE = 3
};

enum {			/* record types; use NONE from action codes */
	A = 1,
	CNAME = 2,
	SRV = 3,
	NS = 4,
	MX = 5,
	TXT = 6
};

/* Get command line args and pass them. Put actions into the struct */
int
parse_dnsa_command_line(int argc, char **argv, dnsa_comm_line_s *comm);
/* Grab config values from file */
int
parse_dnsa_config_file(ailsa_cmdb_s *dc, char *config);
int
read_dnsa_config_values(ailsa_cmdb_s *dc, FILE *cnf);
void
parse_dnsa_config_error(int error);
int
validate_comm_line(dnsa_comm_line_s *comm);
void
validate_fwd_comm_line(dnsa_comm_line_s *comm);
void
validate_glue_comm_line(dnsa_comm_line_s *comm);
void
validate_rev_comm_line(dnsa_comm_line_s *comm);
/* Zone action Functions */
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
int
get_correct_rev_zone_and_preferred_records(dnsa_s *dnsa, dnsa_comm_line_s *cm);
/* Added 06/03/2013 */
int
compare_fwd_ns_records_with_host(dnsa_s *dnsa, char *name, dnsa_comm_line_s *cm);
int
compare_host_with_record_destination(dnsa_s *dnsa, char *name);
int
compare_host_with_fqdn_cname(dnsa_s *dnsa, char *name);
void
get_fqdn_for_record_host(dnsa_s *dnsa, record_row_s *fwd, char *fqdn);
void
get_fqdn_for_record_dest(dnsa_s *dnsa, record_row_s *fwd, char *fqdn);
int
get_fwd_zone(dnsa_s *dnsa, dnsa_comm_line_s *cm);
int
get_record_id_and_delete(ailsa_cmdb_s *dc, dnsa_s *dnsa, dnsa_comm_line_s *cm);
int
delete_reverse_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
delete_glue_zone (ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* End addition 06/03/2013 */
/* Added 07/03/2013 */
int
check_for_fwd_record_use(dnsa_s *dnsa, char *name, dnsa_comm_line_s *cm);
int
delete_fwd_zone(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
void
split_fwd_record_list(zone_info_s *zone, record_row_s *list, record_row_s **fwd, record_row_s **other);
/* End addition 07/03/2013 */
int
get_rev_zone(dnsa_s *dnsa, dnsa_comm_line_s *cm);
void
get_duplicate_a_records(dnsa_comm_line_s *cm, dnsa_s *dnsa);
int
get_rev_records_for_range(rev_record_row_s **rev, rev_zone_info_s *zone);
/* Zone display functions */
void
list_zones(ailsa_cmdb_s *dc);
void
list_rev_zones(ailsa_cmdb_s *dc);
void
display_zone(char *domain, ailsa_cmdb_s *dc);
void
print_zone(dnsa_s *dnsa, char *domain);
void
print_record(record_row_s *rec, char *zname);
/*int
get_port_number(record_row_s *rec, char *name, unsigned short int *port); */
void
display_rev_zone(char *domain, ailsa_cmdb_s *dc);
void
print_rev_zone(dnsa_s *dnsa, char *domain);
void
print_multiple_a_records(ailsa_cmdb_s *dc, dbdata_s *data, dnsa_s *dnsa);
int
get_preferred_a_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm, dnsa_s *dnsa);
void
list_glue_zones(ailsa_cmdb_s *dc);
/* Various zone functions */
void
get_in_addr_string(char *in_addr, char range[], unsigned long int prefix);
unsigned long int
get_zone_serial(void);
int
check_for_zone_in_db(ailsa_cmdb_s *dc, dnsa_s *dnsa, short int type);
void
select_specific_ip(dnsa_s *dnsa, dnsa_comm_line_s *cm);
int
get_a_records_for_range(record_row_s **records, rev_zone_info_s *zone);
int
get_pref_a_for_range(preferred_a_s **prefer, rev_zone_info_s *rev);
void
add_int_ip_to_records(dnsa_s *dnsa);
void
add_int_ip_to_fwd_records(record_row_s *records);
int
add_int_ip_to_rev_records(dnsa_s *dnsa);
int
check_notify_ip(zone_info_s *zone, char **host);
int
check_parent_for_a_record(char *dns, char *parent, dnsa_s *dnsa);
void
add_mx_record(string_len_s *zone, record_row_s *rec);
void
add_ns_record(string_len_s *zone, record_row_s *rec);
void
add_srv_record(string_len_s *zone, record_row_s *rec, zone_info_s *zinfo);
/* Added 05/03/2013 */
int
delete_preferred_a(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
int
delete_record(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* End add 05/03/2013 */
/* Forward zone functions */
int
check_zone(char *domain, ailsa_cmdb_s *dc);
int
create_and_write_fwd_zone(dnsa_s *dnsa, ailsa_cmdb_s *dc, zone_info_s *zone);
int
create_fwd_config(ailsa_cmdb_s *dc, zone_info_s *zone, string_len_s *config);
void
create_fwd_zone_header(record_row_s *record, char *hostm, zone_info_s *zone, string_len_s *zonefile);
void
add_records_to_fwd_zonefile(dnsa_s *dnsa, unsigned long int id, string_len_s *zonefile);
void
check_a_record_for_ns(string_len_s *zonefile, glue_zone_info_s *glue, char *parent, dnsa_s *dnsa);
int
create_and_write_fwd_config(ailsa_cmdb_s *dc, dnsa_s *dnsa);
void
check_for_updated_fwd_zone(ailsa_cmdb_s *dc, zone_info_s *zone);
int
validate_fwd_zone(ailsa_cmdb_s *dc, zone_info_s *zone, dnsa_s *dnsa);
void
fill_fwd_zone_info(zone_info_s *zone, dnsa_comm_line_s *cm, ailsa_cmdb_s *dc);
/* Reverse zone functions */
void 
print_rev_zone_info(rev_zone_info_s *zone);
int
create_and_write_rev_zone(dnsa_s *dnsa, ailsa_cmdb_s *dc, rev_zone_info_s *zone);
void
create_rev_zone_header(dnsa_s *dnsa, char *hostm, unsigned long int id, string_len_s *zonefile);
void
add_records_to_rev_zonefile(dnsa_s *dnsa, unsigned long int id, string_len_s *zonefile);
int
create_rev_config(ailsa_cmdb_s *dc, rev_zone_info_s *zone, string_len_s *config);
void
fill_rev_zone_info(rev_zone_info_s *zone, dnsa_comm_line_s *cm, ailsa_cmdb_s *dc);
int
validate_rev_zone(ailsa_cmdb_s *dc, rev_zone_info_s *zone, dnsa_s *dnsa);
int
create_and_write_rev_config(ailsa_cmdb_s *dc, dnsa_s *dnsa);
int
set_slave_name_servers(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm, dbdata_s *data);
/* 04/03/2013 functions add */
void
trim_forward_record_list(dnsa_s *dnsa, record_row_s *rec);
int
check_dup_and_pref_list(record_row_s *list, record_row_s *fwd, dnsa_s *dnsa);
int
rev_records_to_delete(dnsa_s *dnsa, rev_record_row_s **rev);
void
insert_into_rev_del_list(rev_record_row_s *record, rev_record_row_s **rev);
int
add_cname_to_root_domain(ailsa_cmdb_s *dc, dnsa_comm_line_s *cm);
/* Updated / modified functions */
int
rev_records_to_add(dnsa_s *dnsa, rev_record_row_s **rev);
/* End 04/03/2013 */
int
insert_into_rev_add_list(dnsa_s *dnsa, record_row_s *fwd, rev_record_row_s **rev);
void
add_dup_to_prefer_list(dnsa_s *dnsa, record_row_s *fwd);
int
get_rev_host(unsigned long int prefix, char *rev_dest, char *dest);
/* Glue zone functins */
void
split_glue_ns(char *pri_ns, glue_zone_info_s *glue);
void
split_glue_ip(char *pri_ip, glue_zone_info_s *glue);
void
setup_glue_struct(dnsa_s *dnsa, zone_info_s *zone, glue_zone_info_s *glue);
int
get_glue_zone_parent(ailsa_cmdb_s *dc, dnsa_s *dnsa);
void
check_glue_zone_input(glue_zone_info_s *glue);
void
print_glue_zone(glue_zone_info_s *glue, zone_info_s *zone);
char *
get_zone_fqdn_name(zone_info_s *zone, glue_zone_info_s *glue, int ns);
void
glue_sort_fqdn(glue_zone_info_s *glue);

#endif /* __CMDB_DNSA_H__ */
