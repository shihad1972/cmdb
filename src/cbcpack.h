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
 *  cbcpack.h
 * 
 *  Header file for data and functions for cbcpack program
 * 
 *  part of the cbcpack program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBCPACK_H__
# define __CBCPACK_H__

enum {
	ARCH = 1,
	VER = 2,
	BOTH = 3
};

typedef struct cbcpack_comm_line_s {
	char alias[MAC_S];
	char arch[RANGE_S];
	char os[MAC_S];
	char ver_alias[MAC_S];
	char version[MAC_S];
	char varient[HOST_S];
	char valias[MAC_S];
	char package[HOST_S];
	short int action;
} cbcpack_comm_line_s;

void
init_cbcpack_config(cbc_config_s *cbc, cbcpack_comm_line_s *cpl);

void
init_cbcpack_comm_line(cbcpack_comm_line_s *cpl);

int
parse_cbcpack_comm_line(int argc, char *argv[], cbcpack_comm_line_s *cpl);

int
add_package(cbc_config_s *cmc, cbcpack_comm_line_s *cpl);

int
get_os_list_count(cbcpack_comm_line_s *cpl, cbc_s *cbc);

int
get_comm_line_os_details(cbcpack_comm_line_s *cpl);

int
get_os_list(cbcpack_comm_line_s *cpl, cbc_s *cbc, unsigned long int *id, int num);

int
get_vari_list(cbcpack_comm_line_s *cpl, cbc_s *cbc, unsigned long int *id, int num);

int
get_vari_list_count(cbcpack_comm_line_s *cpl, cbc_s *cbc);

int
remove_package(cbc_config_s *cmc, cbcpack_comm_line_s *cpl);

#endif /* __CBCPACK_H__ */
