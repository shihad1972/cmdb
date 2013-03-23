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
 *  cbcvarient.h
 * 
 *  Header file for data and functions for cbcvarient program
 * 
 *  part of the cbcvarient program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBCVARI_H__
# define __CBCVARI_H__

typedef struct cbcvari_comm_line_s {
	char alias[MAC_S];
	char arch[RANGE_S];
	char os[MAC_S];
	char ver_alias[MAC_S];
	char version[MAC_S];
	char varient[HOST_S];
	char valias[MAC_S];
	short int action;
	unsigned long int id;
	unsigned long int os_id;
} cbcvari_comm_line_s;

void
init_cbcvari_config(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

void
init_cbcvari_comm_line(cbcvari_comm_line_s *cvl);

int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl);

int
list_cbc_build_varient(cbc_config_s *cmc);

#endif /* __CBCVARI_H__ */