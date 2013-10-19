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

int
parse_cbc_config_file(cbc_config_s *dc, const char *config);

void
init_cbc_config_values(cbc_config_s *dc);

void
parse_cbc_config_error(int error);

void
print_cbc_config(cbc_config_s *cbc);

int
query_config();


#endif
