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
 *  build.h
 * 
 *  Header file for the build.c file.
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBC_BUILD_H__
# define __CBC_BUILD_H__
# include "cmdb_cbc.h"

int
display_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml);

int
cbc_get_server(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
create_build_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
cbc_get_build_details(cbc_s *cbc, cbc_s *details);

// New functions for new create_build_config

int
check_for_existing_build(cbc_config_s *cbc, cbc_build_s *build);

int
cbc_get_network_info(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_get_varient(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_get_os(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_get_locale(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_get_partition_scheme(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_get_ip_info(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_search_for_ip(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_check_in_use_ip(cbc_config_s *cbt, cbc_comm_line_s *cml, uli_t *ip);

int
cbc_find_build_ip(unsigned long int *ipinfo, dbdata_s *data);

int
cbc_get_build_dom_info(cbc_config_s *cbt, cbc_comm_line_s *cml, uli_t *bd);

int
cbc_add_disk(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
cbc_search_for_disk(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

// End of new functions

void
list_build_servers(cbc_config_s *cbt);

int
write_build_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_dhcp_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

void
fill_dhconf(char *name, dbdata_s *data, char *ip, cbc_dhcp_config_s *dhconf);

void
fill_dhcp_hosts(char *line, string_len_s *dhcp, cbc_dhcp_config_s *dhconf);

int
write_tftp_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_preseed_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_kickstart_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_pre_host_script(cbc_config_s *cmc, cbc_comm_line_s *cml);
/*
int
get_server_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *server_id); */

int
get_os_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *os_id);

int
get_build_id(cbc_config_s *cbc, uli_t id, char *name, uli_t *build_id);

int
get_modify_query(unsigned long int ids[]);

void
cbc_prep_update_dbdata(dbdata_s *data, int type, unsigned long int ids[]);
/*
int
get_server_name(cbc_config_s *cmc, char *name, uli_t server_id); */

void
print_build_config(cbc_s *details);

void
fill_tftp_output(cbc_comm_line_s *cml, dbdata_s *data, char *output);

void
fill_net_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

void
fill_mirror_output(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

int
fill_kernel(cbc_comm_line_s *cml, string_len_s *build);

void
fill_packages(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build, int i);

int
fill_kick_base(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build);

int
fill_kick_partitions(cbc_config_s *cbc, cbc_comm_line_s *cmc, string_len_s *build);

int
fill_kick_part_header(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build);

char *
get_kick_part_opts(cbc_config_s *cbc, cbc_comm_line_s *cml, char *mnt);

void
fill_kick_network_info(dbdata_s *data, string_len_s *build);

void
fill_kick_packages(dbdata_s *data, string_len_s *build);

void
add_kick_base_script(dbdata_s *data, string_len_s *build);

void
fill_build_scripts(cbc_config_s *cbc, dbdata_s *data, int no, string_len_s *build, cbc_comm_line_s *cml);

void
add_kick_final_config(string_len_s *build, char *url);

char *
add_pre_start_part(cbc_comm_line_s *cml, dbdata_s *data, char *disk);
/*
void
add_pre_lvm_part(dbdata_s *data, int retval, string_len_s *disk);

void
add_pre_part(dbdata_s *data, int retval, string_len_s *disk); */

int
add_pre_parts(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build, short int lvm);

int
get_pre_part_options(cbc_config_s *cbc, cbc_comm_line_s *cml, char *mnt, dbdata_s **opts);

void
add_pre_volume_group(cbc_comm_line_s *cml, string_len_s *disk);

int
fill_partition(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build);

int
fill_system_packages(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build);

void
add_system_package_line(cbc_config_s *cbc, uli_t server_id, string_len_s *build, dbdata_s *data);

char *
cbc_complete_arg(cbc_config_s *cbc, uli_t server_id, char *arg);

char *
get_replaced_syspack_arg(dbdata_s *data, int loop);

int
modify_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml);

int
remove_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml);

void
fill_dbdata_os_search(dbdata_s *data, cbc_comm_line_s *cml);

#endif /* __CBC_BUILD_H__ */
