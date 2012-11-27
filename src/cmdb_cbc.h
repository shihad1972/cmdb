/* cmdb_cbc.h */

#ifndef __CMDB_CBC_H__
#define __CMDB_CBC_H__


typedef struct cbc_comm_line_t { /* Hold parsed command line args */
	short int action;
	char config[CONF_S];
	char name[CONF_S];
	char id[CONF_S];
} cbc_comm_line_t;

typedef struct cbc_config_t { /* Hold CMDB configuration values */
	char db[CONF_S];
	char user[CONF_S];
	char pass[CONF_S];
	char host[CONF_S];
	char socket[CONF_S];
	unsigned int port;
	unsigned long int cliflag;
	char tmpdir[CONF_S];
	char tftpdir[CONF_S];
	char pxe[CONF_S];
	char toplevelos[CONF_S];
	char dhcpconf[CONF_S];
	char kickstart[CONF_S];
	char preseed[CONF_S];
} cbc_config_t;


int
parse_cbc_config_file(cbc_config_t *dc, char *config);

void
init_cbc_config_values(cbc_config_t *dc);

void
parse_cbc_config_error(int error);

void
print_cbc_config(cbc_config_t *cbc);

#endif