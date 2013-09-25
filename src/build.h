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

typedef struct cbc_comm_line_s {	/* Hold parsed command line args */
	char config[CONF_S];
	char name[CONF_S];
	char uuid[CONF_S];
	char partition[CONF_S];
	char varient[CONF_S];
	char os[CONF_S];
	char os_version[MAC_S];
	char build_domain[RBUFF_S];
	char action_type[MAC_S];
	char arch[MAC_S];
	char netcard[HOST_S];
	short int action;
	short int server;
	short int removeip;
	unsigned long int server_id;
	unsigned long int os_id;
	unsigned long int locale;
} cbc_comm_line_s;

typedef struct cbc_dhcp_config_s { /* Hold information about dhcp config */
	char file[CONF_S];
	char name[CONF_S];
	char eth[MAC_S];
	char ip[MAC_S];
	char domain[RBUFF_S];
} cbc_dhcp_config_s;

int
display_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml);

int
cbc_get_server(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
cbc_get_os(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
cbc_get_build_domain(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
cbc_get_build_ip(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_s *details);

int
cbc_find_build_ip(unsigned long int *ip_addr, cbc_s *details, dbdata_s *data, dbdata_s *list);

int
cbc_get_varient(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
cbc_get_seed_scheme(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
cbc_get_locale(cbc_comm_line_s *cml, cbc_s *cbc, cbc_s *details);

int
cbc_get_build_config(cbc_s *cbc, cbc_s *details, cbc_build_s *build);

int
cbc_get_build_partitons(cbc_s *cbc, cbc_s *details);

int
cbc_get_build_details(cbc_s *cbc, cbc_s *details);

int
cbc_get_network_info(cbc_config_s *cbt, cbc_comm_line_s *cml, cbc_build_s *build);

int
check_for_disk_device(cbc_config_s *cbc, cbc_s *details);

void
cbc_fill_build_ip(cbc_build_ip_s *ip, cbc_comm_line_s *cml, cbc_build_domain_s *bdom, unsigned long int ip_addr, cbc_server_s *server);

int
list_build_servers(cbc_config_s *cbt);

int
write_build_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

int
create_build_config(cbc_config_s *cmc, cbc_comm_line_s *cml);

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
get_server_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *server_id);

int
get_varient_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *varient_id);

int
get_os_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *os_id);

int
get_def_scheme_id(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int *def_scheme_id);

int
get_build_id(cbc_config_s *cbc, cbc_comm_line_s *cml, unsigned long int *build_id);

int
get_modify_query(unsigned long int ids[]);

void
cbc_prep_update_dbdata(dbdata_s *data, int type, unsigned long int ids[]);

int
get_server_name(cbc_config_s *cmc, cbc_comm_line_s *cml, unsigned long int server_id);

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

void
fill_kick_base(dbdata_s *data, string_len_s *build);

void
fill_kick_partitions(cbc_comm_line_s *cmc, dbdata_s *data, string_len_s *build);

void
fill_kick_network_info(dbdata_s *data, string_len_s *build);

void
fill_kick_packages(dbdata_s *data, string_len_s *build);

void
add_kick_ntp_config(dbdata_s *data, string_len_s *build, char *url);

void
add_kick_ldap_config(dbdata_s *data, string_len_s *build, char *url);

void
add_kick_smtp_config(dbdata_s *data, string_len_s *build, string_l *conf);

void
add_kick_base_script(dbdata_s *data, string_len_s *build);

void
add_kick_log_config(dbdata_s *data, string_len_s *build, char *url);

char *
add_pre_start_part(cbc_comm_line_s *cml, dbdata_s *data, char *disk);

void
add_pre_lvm_part(dbdata_s *data, int retval, string_len_s *disk);

void
add_pre_part(dbdata_s *data, int retval, string_len_s *disk);

void
add_pre_volume_group(cbc_comm_line_s *cml, string_len_s *disk);

int
fill_partition(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build);

int
fill_app_config(cbc_config_s *cmc, cbc_comm_line_s *cml, string_len_s *build);

void
fill_ldap_config(dbdata_s *data, string_len_s *build);

void
fill_xymon_config(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

void
fill_smtp_config(cbc_comm_line_s *cml, dbdata_s *data, string_len_s *build);

void
init_cbc_comm_values(cbc_comm_line_s *cbt);

void
init_all_config(cbc_config_s *cct, cbc_comm_line_s *cclt/*, cbc_build_s *cbt*/);

void
print_cbc_command_line_values(cbc_comm_line_s *command_line);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb);

int
modify_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml);

int
remove_build_config(cbc_config_s *cbt, cbc_comm_line_s *cml);

void
fill_dbdata_os_search(dbdata_s *data, cbc_comm_line_s *cml);

void
resize_string_buff(string_len_s *build);

#endif /* __CBC_BUILD_H__ */
