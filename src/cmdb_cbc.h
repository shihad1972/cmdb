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
	short int action;
	short int usedb;
	short int server;
	unsigned long int server_id;
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

typedef struct cbc_build_t {		/* Hold build configuration values */
	char ip_address[RANGE_S];
	char mac_address[MAC_S];
	char hostname[CONF_S];
	char domain[RBUFF_S];
	char alias[CONF_S];
	char ver_alias[CONF_S];
	char version[CONF_S];
	char varient[CONF_S];
	char arch[CONF_S];
	char boot[RBUFF_S];
	char gateway[RANGE_S];
	char nameserver[RANGE_S];
	char netmask[RANGE_S];
	char build_type[RANGE_S];
	char arg[RANGE_S];
	char url[RBUFF_S];
	char country[RANGE_S];
	char locale[RANGE_S];
	char language[RANGE_S];
	char keymap[RANGE_S];
	char netdev[RANGE_S];
	char diskdev[MAC_S];
	char mirror[CONF_S];
	char ntpserver[CONF_S];
	int config_ntp;
	int use_lvm;
	unsigned long int server_id;
	unsigned long int bd_id;
} cbc_build_t;

typedef struct pre_disk_part_t {		/* Linked list for disk partitions */
	char mount_point[HOST_S + 1];
	char filesystem[RANGE_S + 1];
	char log_vol[RANGE_S + 1];
	unsigned long int min;
	unsigned long int pri;
	unsigned long int max;
	unsigned long int part_id;
	struct pre_disk_part_t *nextpart;
} pre_disk_part_t;

typedef struct pre_app_config_t {
	char ldap_url[URL_S];
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
print_cbc_command_line_values(cbc_comm_line_t *command_line);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_t *cb);

void
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

#endif