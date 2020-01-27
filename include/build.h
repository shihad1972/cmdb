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

int
write_tftp_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_preseed_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_kickstart_build_file(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
write_pre_host_script(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
get_build_id(cbc_config_s *cbc, uli_t id, char *name, uli_t *build_id);

int
get_modify_query(unsigned long int ids[]);

void
cbc_prep_update_dbdata(dbdata_s *data, int type, unsigned long int ids[]);

void
print_build_config(cbc_s *details);

char *
get_kick_part_opts(cbc_config_s *cbc, cbc_comm_line_s *cml, char *mnt);

void
add_kick_base_script(dbdata_s *data, string_len_s *build);

void
add_kick_final_config(cbc_comm_line_s *cml, string_len_s *build, char *url);

char *
add_pre_start_part(cbc_comm_line_s *cml, dbdata_s *data, char *disk);

int
add_pre_parts(cbc_config_s *cbc, cbc_comm_line_s *cml, string_len_s *build, short int lvm);

int
get_pre_part_options(cbc_config_s *cbc, cbc_comm_line_s *cml, char *mnt, dbdata_s **opts);

void
add_pre_volume_group(cbc_comm_line_s *cml, string_len_s *disk);

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

#endif /* __CBC_BUILD_H__ */
