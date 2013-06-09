/*
 *
 *  cbc: Create build config
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
 *  cmdb_cbc.h
 *
 *  Main header file for the cbc program
 *
 *
 *  Part of the CMDB program
 *
 *  (C) Iain M Conochie 2012 - 2013 */

#ifndef __CMDB_CBC_H__
# define __CMDB_CBC_H__
# include "../config.h"
# ifdef HAVE_DNSA

#  include "cmdb_dnsa.h"

# endif /* HAVE_DNSA */

typedef struct cbc_config_s {		/* Hold CMDB configuration values */
	char dbtype[RANGE_S];
	char file[CONF_S];
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char socket[CONF_S];
	char tmpdir[CONF_S];
	char tftpdir[CONF_S];
	char pxe[CONF_S];
	char toplevelos[CONF_S];
	char dhcpconf[CONF_S];
	char kickstart[CONF_S];
	char preseed[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
} cbc_config_s;
/*
typedef struct cbc_build_dom_s {
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int netmask;
	unsigned long int gateway;
	unsigned long int ns;
	struct cbc_domain_ip_t *iplist;
} cbc_build_dom_s;

typedef struct cbc_build_s {
	char ip_address[RANGE_S];
	char gateway[RANGE_S];
	char nameserver[RANGE_S];
	char netmask[RANGE_S];
	char mac_address[MAC_S];
	char netdev[RANGE_S];
	char hostname[CONF_S];
	char domain[RBUFF_S];
	char alias[CONF_S];
	char ver_alias[CONF_S];
	char version[CONF_S];
	char base_ver[MAC_S];
	char arch[CONF_S];
	char varient[CONF_S];
	char boot[RBUFF_S];
	char build_type[RANGE_S];
	char arg[RANGE_S];
	char url[RBUFF_S];
	char mirror[CONF_S];
	char country[RANGE_S];
	char locale[RANGE_S];
	char language[RANGE_S];
	char keymap[RANGE_S];
	char timezone[HOST_S];
	char diskdev[MAC_S];
	char ntpserver[CONF_S];
	char part_scheme_name[CONF_S];
	int config_ntp;
	int use_lvm;
	unsigned long int server_id;
	unsigned long int bd_id;
	unsigned long int def_scheme_id;
	unsigned long int os_id;
	unsigned long int varient_id;
	unsigned long int boot_id;
	unsigned long int locale_id;
	unsigned long int ip_id;
	struct cbc_build_dom_s *build_dom;
} cbc_build_s;

typedef struct pre_disk_part_s {
	char mount_point[HOST_S + 1];
	char filesystem[RANGE_S + 1];
	char log_vol[RANGE_S + 1];
	unsigned long int min;
	unsigned long int pri;
	unsigned long int max;
	unsigned long int part_id;
	struct pre_disk_part_s *nextpart;
} pre_disk_part_s;

typedef struct partition_schemes_s {
	unsigned long int id;
	char name[CONF_S];
	unsigned long int lvm;
	struct partition_schemes_s *next;
} partition_schemes_s;
*/
int
parse_cbc_config_file(cbc_config_s *dc, const char *config);

void
init_cbc_config_values(cbc_config_s *dc);
/*
void
init_cbc_build_values(cbc_build_s *build_config);
*/
void
parse_cbc_config_error(int error);

void
print_cbc_config(cbc_config_s *cbc);

# ifdef HAVE_DNSA

void
fill_cbc_fwd_zone(zone_info_s *zone, char *domain, dnsa_config_s *dc);

# endif /* HAVE_DNSA */
/*
void
print_cbc_build_values(cbc_build_s *build_config);

void
print_cbc_build_ids(cbc_build_s *build_config);

int
get_server_name(cbc_comm_line_s *info, cbc_config_s *config);

int
get_build_info(cbc_config_s *config, cbc_build_s *build_info, unsigned long int server_id);

void
write_tftp_config(cbc_config_s *cct, cbc_build_s *cbt);

void
write_dhcp_config(cbc_config_s *cct, cbc_build_s *cbt);

int
write_build_config(cbc_config_s *cmc, cbc_build_s *cbt);

int
delete_build_if_exists(cbc_config_s *cmc, cbc_build_s *cbt);

int
add_partition_scheme(cbc_config_s *config);

void
display_partition_schemes(cbc_config_s *config);

void
display_build_operating_systems(cbc_config_s *config);

void
display_build_os_versions(cbc_config_s *config);

void
display_build_domains(cbc_config_s *config);

void
display_build_varients(cbc_config_s *config);

void
display_build_locales(cbc_config_s *config);

int
create_build_config(cbc_config_s *cbc, cbc_comm_line_s *cml, cbc_build_s *cbt);

int
get_os_from_user(cbc_config_s *cbc, cbc_comm_line_s *cml);

int
get_os_version_from_user(cbc_config_s *config, cbc_comm_line_s *cml);

int
get_os_arch_from_user(cbc_config_s *config, cbc_comm_line_s *cml);

int
get_build_os_id(cbc_config_s *config, cbc_comm_line_s *cml);

int
get_build_domain_from_user(cbc_config_s *config, cbc_comm_line_s *cml);

int
get_build_varient_from_user(cbc_config_s *config, cbc_comm_line_s *cml);

int
get_locale_from_user(cbc_config_s *config, cbc_comm_line_s *cml);

int
get_disk_scheme_from_user(cbc_config_s *config, cbc_comm_line_s *cml);

int
copy_build_values(cbc_comm_line_s *cml, cbc_build_s *cbt);

void
copy_initial_build_values(cbc_comm_line_s *cml, cbc_build_s *cbt);

int
get_build_hardware(cbc_config_s *config, cbc_build_s *cbt);

unsigned long int
get_hard_type_id(cbc_config_s *config, char *htype, char *hclass);

int
get_build_hardware_device(cbc_config_s *config, unsigned long int id, unsigned long int sid, char *device, char *detail);

int
get_build_varient_id(cbc_config_s *config, cbc_build_s *cbt);

int
get_build_partition_id(cbc_config_s *config, cbc_build_s *cbt);

void
get_base_os_version(cbc_build_s *cbt);

int
get_build_boot_line_id(cbc_config_s *config, cbc_build_s *cbt);

int
get_build_domain_id(cbc_config_s *config, cbc_build_s *cbt);

int
insert_build_into_database(cbc_config_s *config, cbc_build_s *cbt);

int
get_build_domain_info_on_id(cbc_config_s *config, cbc_build_dom_s *cbt, unsigned long int id);

int
get_build_ip(cbc_config_s *config, cbc_build_dom_s *bd);

void
convert_build_ip_address(cbc_build_s *cbt);

int
insert_ip_into_db(cbc_config_s *config, cbc_build_s *cbt);

int
insert_into_build_table(cbc_config_s *config, cbc_build_s *cbt);

int
insert_build_partitions(cbc_config_s *config, cbc_build_s *cbt);

int
insert_disk_device(cbc_config_s *config, cbc_build_s *cbt);
*/
#endif
