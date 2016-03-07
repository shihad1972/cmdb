/*
 *
 *  cbc: Create build config
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
 *  cmdb_cbc.h
 *
 *  Main header file for the cbc program
 *
 */

#ifndef __CMDB_CBC_H__
# define __CMDB_CBC_H__
# include <config.h>

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
	char harddisk[HOST_S];
	char locale[NAME_S];
	short int action;
	short int server;
	short int removeip;
	short int lvm;
	unsigned long int server_id;
	unsigned long int os_id;
	unsigned long int locale_id;
} cbc_comm_line_s;

typedef struct cbc_dhcp_config_s { /* Hold information about dhcp config */
	char file[CONF_S];
	char name[CONF_S];
	char eth[MAC_S];
	char ip[MAC_S];
	char domain[RBUFF_S];
} cbc_dhcp_config_s;

void
init_cbc_comm_values(cbc_comm_line_s *cbt);

void
init_all_config(cbc_config_s *cct, cbc_comm_line_s *cclt/*, cbc_build_s *cbt*/);

void
print_cbc_command_line_values(cbc_comm_line_s *command_line);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb);

# ifdef HAVE_LIBPCRE
void
validate_cbc_comm_line(cbc_comm_line_s *cml);
# endif /* HAVE_LIBPCRE */

int
parse_cbc_config_file(cbc_config_s *dc, const char *config);

int
read_cbc_config_values(cbc_config_s *dc, FILE *cnf);

void
init_cbc_config_values(cbc_config_s *dc);

void
parse_cbc_config_error(int error);

void
print_cbc_config(cbc_config_s *cbc);

int
query_config();


#endif
