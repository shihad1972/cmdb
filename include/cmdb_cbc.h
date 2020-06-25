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
# include <ailsacmdb.h>
# include "cbc_data.h"

typedef struct cbc_comm_line_s {	/* Hold parsed command line args */
	char *config;
	char *name;
	char *uuid;
	char *partition;
	char *varient;
	char *os;
	char *os_version;
	char *build_domain;
	char *action_type;
	char *arch;
	char *netcard;
	char *harddisk;
	char *locale;
	short int action;
	short int server;
	short int removeip;
	short int lvm;
	short int gui;
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
clean_cbc_comm_line(cbc_comm_line_s *cbt);

void
print_cbc_command_line_values(cbc_comm_line_s *command_line);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_s *cb);

int
query_config();


#endif
