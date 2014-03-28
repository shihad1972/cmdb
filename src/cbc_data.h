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
 *  cbc_data.h
 *
 *  Data header file for the cbc program. This contains the typedefs for the
 *  database tables
 *
 *  Part of the CMDB program
 *
 *  (C) Iain M Conochie 2012 - 2013 */

#ifndef __CBC_DATA_H__
# define __CBC_DATA_H__
# include "cmdb.h"

typedef struct cbc_boot_line_s {
	char os[MAC_S];
	char os_ver[MAC_S];
	char boot_line[RBUFF_S];
	unsigned long int boot_id;
	unsigned long int bt_id;
	struct cbc_boot_line_s *next;
} cbc_boot_line_s;

typedef struct cbc_build_s { 
	char mac_addr[MAC_S];
	char net_int[RANGE_S];
	unsigned long int build_id;
	unsigned long int varient_id;
	unsigned long int server_id;
	unsigned long int os_id;
	unsigned long int ip_id;
	unsigned long int locale_id;
	unsigned long int def_scheme_id;
	struct cbc_build_s *next;
} cbc_build_s;

typedef struct cbc_build_domain_s {
	char domain[RBUFF_S];
	char ntp_server[HOST_S];
	char ldap_dn[URL_S];
	char ldap_bind[URL_S];
	char ldap_host[URL_S];
	char ldap_server[URL_S];
	char log_server[CONF_S];
	char nfs_domain[CONF_S];
	char smtp_server[CONF_S];
	char xymon_server[CONF_S];
	short int config_ntp;
	short int ldap_ssl;
	short int config_ldap;
	short int config_log;
	short int config_email;
	short int config_xymon;
	unsigned long int bd_id;
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int netmask;
	unsigned long int gateway;
	unsigned long int ns;
	struct cbc_build_domain_s *next;
} cbc_build_domain_s;

typedef struct cbc_build_ip_s {
	char host[MAC_S];
	char domain[RBUFF_S];
	unsigned long int ip;
	unsigned long int ip_id;
	unsigned long int bd_id;
	unsigned long int server_id;
	struct cbc_build_ip_s *next;
} cbc_build_ip_s;

typedef struct cbc_build_os_s {
	char os[MAC_S];
	char version[MAC_S];
	char alias[MAC_S];
	char ver_alias[MAC_S];
	char arch[RANGE_S];
	unsigned long int os_id;
	unsigned long int bt_id;
	struct cbc_build_os_s *next;
} cbc_build_os_s;

typedef struct cbc_build_type_s {
	char alias[MAC_S];
	char build_type[MAC_S];
	char arg[RANGE_S];
	char url[CONF_S];
	char mirror[RBUFF_S];
	char boot_line[URL_S];
	unsigned long int bt_id;
	struct cbc_build_type_s *next;
} cbc_build_type_s;

typedef struct cbc_disk_dev_s {
	char device[HOST_S];
	short int lvm;
	unsigned long int disk_id;
	unsigned long int server_id;
	struct cbc_disk_dev_s *next;
} cbc_disk_dev_s;

typedef struct cbc_locale_s {
	char locale[MAC_S];
	char country[RANGE_S];
	char language[RANGE_S];
	char keymap[RANGE_S];
	char timezone[HOST_S];
	unsigned long int locale_id;
	unsigned long int os_id;
	unsigned long int bt_id;
	struct cbc_locale_s *next;
} cbc_locale_s;

typedef struct cbc_package_s {
	char package[HOST_S];
	unsigned long int pack_id;
	unsigned long int vari_id;
	unsigned long int os_id;
	struct cbc_package_s *next;
} cbc_package_s;

typedef union part_id_u {
	unsigned long int def_part_id;
	unsigned long int part_id;
} part_id_u;

typedef union scheme_id_u {
	unsigned long int def_scheme_id;
	unsigned long int server_id;
} scheme_id_u;

typedef struct cbc_pre_part_s {
	char mount[HOST_S];
	char fs[RANGE_S];
	char log_vol[MAC_S];
	unsigned long int min;
	unsigned long int max;
	unsigned long int pri;
	union part_id_u id;
	union scheme_id_u link_id;
	struct cbc_pre_part_s *next;
} cbc_pre_part_s;

typedef struct cbc_seed_scheme_s {
	char name[CONF_S];
	short int lvm;
	unsigned long int def_scheme_id;
	struct cbc_seed_scheme_s *next;
} cbc_seed_scheme_s;

typedef struct cbc_server_s {
	char vendor[CONF_S];
	char make[CONF_S];
	char model[CONF_S];
	char uuid[CONF_S];
	char name[HOST_S];
	unsigned long int server_id;
	unsigned long int cust_id;
	unsigned long int vm_server_id;
	struct cbc_server_s *next;
} cbc_server_s;

typedef struct cbc_varient_s {
	char varient[HOST_S];
	char valias[MAC_S];
	unsigned long int varient_id;
	struct cbc_varient_s *next;
} cbc_varient_s;

typedef struct cbc_vm_server_hosts_s {
	char vm_server[RBUFF_S];
	char type[HOST_S];
	unsigned long int vm_s_id;
	unsigned long int server_id;
	struct cbc_vm_server_hosts_s *next;
} cbc_vm_server_hosts_s;

typedef struct cbc_s {
	struct cbc_boot_line_s *bootl;
	struct cbc_build_s *build;
	struct cbc_build_domain_s *bdom;
	struct cbc_build_ip_s *bip;
	struct cbc_build_os_s *bos;
	struct cbc_build_type_s *btype;
	struct cbc_disk_dev_s *diskd;
	struct cbc_locale_s *locale;
	struct cbc_package_s *package;
	struct cbc_pre_part_s *dpart;
	struct cbc_pre_part_s *spart;
	struct cbc_seed_scheme_s *sscheme;
	struct cbc_server_s *server;
	struct cbc_varient_s *varient;
	struct cbc_vm_server_hosts_s *vmhost;
} cbc_s;

void
init_cbc_struct (cbc_s *cbc);

void
clean_cbc_struct (cbc_s *cbc);

void
init_boot_line(cbc_boot_line_s *boot);

void
clean_boot_line(cbc_boot_line_s *boot);

void
display_boot_line(cbc_s *base);

void
init_build_struct(cbc_build_s *build);

void
clean_build_struct(cbc_build_s *build);

void
display_build_struct(cbc_s *base);

void
init_build_domain(cbc_build_domain_s *dom);

void
clean_build_domain(cbc_build_domain_s *dom);

void
display_build_domain(cbc_build_domain_s *dom);

void
init_build_ip(cbc_build_ip_s *ip);

void
clean_build_ip(cbc_build_ip_s *ip);

void
display_build_ip(cbc_s *base);

void
init_build_os(cbc_build_os_s *os);

void
clean_build_os(cbc_build_os_s *os);

void
display_build_os(cbc_s *base);

void
init_build_type(cbc_build_type_s *type);

void
clean_build_type(cbc_build_type_s *type);

void
display_build_type(cbc_s *base);

void
init_pre_part(cbc_pre_part_s *prep);

void
clean_pre_part(cbc_pre_part_s *prep);

void
display_def_part(cbc_s *base);

void
display_seed_part(cbc_s *base);

void
init_disk_dev(cbc_disk_dev_s *disk);

void
clean_disk_dev(cbc_disk_dev_s *disk);

void
display_disk_dev(cbc_s *base);

void
init_locale(cbc_locale_s *locale);

void
clean_locale(cbc_locale_s *locale);

void
display_locale(cbc_s *base);

void
init_package(cbc_package_s *pack);

void
clean_package(cbc_package_s *pack);

void
display_package(cbc_s *base);

void
init_seed_scheme(cbc_seed_scheme_s *seed);

void
clean_seed_scheme(cbc_seed_scheme_s *seed);

void
display_seed_scheme(cbc_s *base);

void
init_cbc_server(cbc_server_s *server);

void
clean_cbc_server(cbc_server_s *server);

void
display_cbc_server(cbc_s *base);

void
init_varient(cbc_varient_s *vari);

void
clean_varient(cbc_varient_s *vari);

void
display_varient(cbc_s *base);

void
init_vm_hosts(cbc_vm_server_hosts_s *vm);

void
clean_vm_hosts(cbc_vm_server_hosts_s *vm);

void
display_vm_hosts(cbc_s *base);

#endif /* __CBC_DATA_H__ */
