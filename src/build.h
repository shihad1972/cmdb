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
	short int server;
	short int package;
	unsigned long int server_id;
	unsigned long int os_id;
	unsigned long int locale;
} cbc_comm_line_t;

int
display_build_config(cbc_config_t *cbt, cbc_comm_line_t *cml);

int
cbc_get_server(cbc_comm_line_t *cml, cbc_t *cbc, cbc_t *details);

int
cbc_get_build_details(cbc_t *cbc, cbc_t *details);

void
print_build_config(cbc_t *details);

void
init_cbc_comm_values(cbc_comm_line_t *cbt);

void
init_all_config(cbc_config_t *cct, cbc_comm_line_t *cclt/*, cbc_build_t *cbt*/);

void
print_cbc_command_line_values(cbc_comm_line_t *command_line);

int
parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_t *cb);

#endif /* __CBC_BUILD_H__ */