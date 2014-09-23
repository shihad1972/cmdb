/*
 *
 *  cpc: Create preseed config
 *  Copyright (C) 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cmdb_cpc.h
 *
 *  Main header file for the cpc program
 *
 */

#ifndef __CMDB_CPC_H__
# define __CMDB_CPC_H__
# include "../config.h"

typedef struct cpc_comm_line_s {		/* Hold cpc command line */
	char *file;
	char *name;
} cpc_comm_line_s;

typedef struct cpc_config_s {
	char *name;
} cpc_config_s;

int
parse_cpc_comm_line(int argc, char *argv[], cpc_comm_line_s *cl);

void
init_cpc_config(cpc_config_s *cpc);

void
clean_cpc_config(cpc_config_s *cpc);

void
init_cpc_comm_line(cpc_comm_line_s *cl);

void
clean_cpc_comm_line(cpc_comm_line_s *cl);

void
output_preseed(cpc_comm_line_s *cl, cpc_config_s *cpc);

#endif /* __CMDB_CPC_H__ */

