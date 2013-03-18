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
 *  cbcos.h
 * 
 *  Header file for data and functions for cbcos program
 * 
 *  part of the cbcos program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBCOS_H__
# define __CBCOS_H__

typedef struct cbcos_comm_line_s {
	char alias[MAC_S];
	char arch[RANGE_S];
	char os[MAC_S];
	char ver_alias[MAC_S];
	char version[MAC_S];
	short int action;
	unsigned long int id;
} cbcos_comm_line_s;

void
init_cbcos_config(cbc_config_s *cmc, cbcos_comm_line_s *col);

void
init_cbcos_comm_line(cbcos_comm_line_s *col);

int
parse_cbcos_comm_line(int argc, char *argv[], cbcos_comm_line_s *col);

int
list_cbc_build_os(cbc_config_s *cmc);

int
display_cbc_build_os(cbc_config_s *cmc, cbcos_comm_line_s *col);

#endif /* __CBCOS_H__ */