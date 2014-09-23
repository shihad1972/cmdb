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

typedef struct cpc_comm_line_s {		/* Hold CMDB configuration values */
	char *name;
} cpc_comm_line_s;

#endif /* __CMDB_CPC_H__ */

