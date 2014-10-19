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

enum {
	CVARIENT = 1,
	CPACKAGE = 2
};
typedef struct cbcvari_comm_line_s {
	char alias[MAC_S];
	char arch[RANGE_S];
	char os[MAC_S];
	char ver_alias[MAC_S];
	char version[MAC_S];
	char varient[HOST_S];
	char valias[MAC_S];
	char package[HOST_S];
	short int action;
	short int type;
} cbcvari_comm_line_s;

void
init_cbcvari_config(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

void
init_cbcvari_comm_line(cbcvari_comm_line_s *cvl);

int
parse_cbcvarient_comm_line(int argc, char *argv[], cbcvari_comm_line_s *cvl);

int
list_cbc_build_varient(cbc_config_s *cmc);

int
display_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

int
add_cbc_build_varient(cbc_config_s *cbc, cbcvari_comm_line_s *cvl);

int
add_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl);

int
remove_cbc_package(cbc_config_s *cbc, cbcvari_comm_line_s *cvl);

int
remove_cbc_build_varient(cbc_config_s *cmc, cbcvari_comm_line_s *cvl);

int
display_all_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl);

int
display_one_os_packages(cbc_s *base, unsigned long int id, cbcvari_comm_line_s *cvl);

int
display_specific_os_packages(cbc_s *base, unsigned long int id, unsigned long int osid);

int
get_os_alias(cbc_s *base, cbcvari_comm_line_s *cvl);

unsigned long int
get_single_os_id(cbc_s *base, cbcvari_comm_line_s *cvl);

int
cbc_get_os(cbc_build_os_s *os, char *name, char *alias, char *arch, char *ver, unsigned long int **id);

int
cbc_get_os_list(cbc_build_os_s *os, char *name, char *alias, char *arch, char *ver, unsigned long int *id);

cbc_package_s *
build_package_list(cbc_config_s *cbc, unsigned long int *os, int nos, char *pack);

dbdata_s *
build_rem_pack_list(cbc_config_s *cbc, unsigned long int *ids, int noids, char *pack);

#endif /* __CBCVARI_H__ */
