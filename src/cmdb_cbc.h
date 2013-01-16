/* cmdb_cbc.h */

#ifndef __CMDB_CBC_H__
#define __CMDB_CBC_H__


typedef struct cbc_comm_line_t {	/* Hold parsed command line args */
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
	short int action;
	short int usedb;
	short int server;
	unsigned long int server_id;
	unsigned long int os_id;
	unsigned long int locale;
} cbc_comm_line_t;

typedef struct cbc_config_t {		/* Hold CMDB configuration values */
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
} cbc_config_t;

typedef struct cbc_domain_ip_t {
	unsigned long int ip;
	char hostname[CONF_S];
	struct cbc_domain_ip_t *next;
} cbc_domain_ip_t;

typedef struct cbc_build_domain_t {		/* Hold net info for build domain */
	unsigned long int start_ip;
	unsigned long int end_ip;
	unsigned long int netmask;
	unsigned long int gateway;
	unsigned long int ns;
	cbc_domain_ip_t *iplist;
} cbc_build_domain_t;

typedef struct cbc_build_t {		/* Hold build configuration values */
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
	cbc_build_domain_t *build_dom;
} cbc_build_t;

typedef struct pre_disk_part_t {	/* Linked list for disk partitions */
	char mount_point[HOST_S + 1];
	char filesystem[RANGE_S + 1];
	char log_vol[RANGE_S + 1];
	unsigned long int min;
	unsigned long int pri;
	unsigned long int max;
	unsigned long int part_id;
	struct pre_disk_part_t *nextpart;
} pre_disk_part_t;

typedef struct partition_schemes_t {	/* Linked list for partition schemes */
	unsigned long int id;
	char name[CONF_S];
	unsigned long int lvm;
	struct partition_schemes_t *next;
} partition_schemes_t;

typedef struct pre_app_config_t {	/* Linked list for preseed extra */
	char ldap_url[URL_S];		/* application config */
	char ldap_dn[URL_S];
	char ldap_bind[URL_S];
	char log_server[CONF_S];
	char smtp_server[CONF_S];
	char xymon_server[CONF_S];
	unsigned long int config_ldap;
	unsigned long int config_log;
	unsigned long int config_email;
	unsigned long int config_xymon;
} pre_app_config_t;

int
parse_cbc_config_file(cbc_config_t *dc, char *config);

void
init_all_config(cbc_config_t *cct, cbc_comm_line_t *cclt, cbc_build_t *cbt);

void
init_cbc_config_values(cbc_config_t *dc);

void
init_cbc_comm_values(cbc_comm_line_t *cbt);

void
init_cbc_build_values(cbc_build_t *build_config);

void
parse_cbc_config_error(int error);

void
print_cbc_config(cbc_config_t *cbc);

void
print_cbc_build_values(cbc_build_t *build_config);

void
print_cbc_build_ids(cbc_build_t *build_config);

void
print_cbc_command_line_values(cbc_comm_line_t *command_line);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_t *cb);

int
get_server_name(cbc_comm_line_t *info, cbc_config_t *config);

int
get_build_info(cbc_config_t *config, cbc_build_t *build_info, unsigned long int server_id);

void
write_tftp_config(cbc_config_t *cct, cbc_build_t *cbt);

void
write_dhcp_config(cbc_config_t *cct, cbc_build_t *cbt);

int
write_build_config(cbc_config_t *cmc, cbc_build_t *cbt);

int
add_partition_scheme(cbc_config_t *config);

void
display_partition_schemes(cbc_config_t *config);

void
display_build_operating_systems(cbc_config_t *config);

void
display_build_os_versions(cbc_config_t *config);

void
display_build_domains(cbc_config_t *config);

void
display_build_varients(cbc_config_t *config);

void
display_build_locales(cbc_config_t *config);

int
create_build_config(cbc_config_t *cbc, cbc_comm_line_t *cml, cbc_build_t *cbt);

int
get_os_from_user(cbc_config_t *cbc, cbc_comm_line_t *cml);

int
get_os_version_from_user(cbc_config_t *config, cbc_comm_line_t *cml);

int
get_os_arch_from_user(cbc_config_t *config, cbc_comm_line_t *cml);

int
get_build_os_id(cbc_config_t *config, cbc_comm_line_t *cml);

int
get_build_domain_from_user(cbc_config_t *config, cbc_comm_line_t *cml);

int
get_build_varient_from_user(cbc_config_t *config, cbc_comm_line_t *cml);

int
get_locale_from_user(cbc_config_t *config, cbc_comm_line_t *cml);

int
get_disk_scheme_from_user(cbc_config_t *config, cbc_comm_line_t *cml);

int
copy_build_values(cbc_comm_line_t *cml, cbc_build_t *cbt);

void
copy_initial_build_values(cbc_comm_line_t *cml, cbc_build_t *cbt);

int
get_build_hardware(cbc_config_t *config, cbc_build_t *cbt);

unsigned long int
get_hard_type_id(cbc_config_t *config, char *htype, char *hclass);

int
get_build_hardware_device(cbc_config_t *config, unsigned long int id, unsigned long int sid, char *device, char *detail);

int
get_build_varient_id(cbc_config_t *config, cbc_build_t *cbt);

int
get_build_partition_id(cbc_config_t *config, cbc_build_t *cbt);

void
get_base_os_version(cbc_build_t *cbt);

int
get_build_boot_line_id(cbc_config_t *config, cbc_build_t *cbt);

int
get_build_domain_id(cbc_config_t *config, cbc_build_t *cbt);

int
insert_build_into_database(cbc_config_t *config, cbc_build_t *cbt);

int
get_build_domain_info_on_id(cbc_config_t *config, cbc_build_domain_t *cbt, unsigned long int id);

int
get_build_domain_ip_list(cbc_config_t *config, cbc_build_domain_t *bd);

void
convert_build_ip_address(cbc_build_t *cbt);

#endif
