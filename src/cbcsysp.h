/* 
 *
 *  cbc: Create Build Configuration
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
 *  cbcsysp.h
 * 
 *  Header file for data and functions for cbcsysp program
 * 
 *  part of the cbcsysp program
 * 
 *  (C) Iain M. Conochie 2014
 * 
 */

#ifndef __CBCSYSP_H__
# define __CBCSYSP_H__

enum {
	SPACKAGE = 1,
	SPACKARG = 2,
	SPACKCNF = 3
};

typedef struct cbc_sysp_s {
	char *arg;
	char *domain;
	char *field;
	char *name;
	char *type;
	short int action;
	short int what;
} cbc_sysp_s;

void
init_cbcsysp_s(cbc_sysp_s *cbcs);

void
clean_cbcsysp_s(cbc_sysp_s *cbcs);

int
parse_cbc_sysp_comm_line(int argc, char *argv[], cbc_sysp_s *cbcs);

// List functions

int
list_cbc_sys_package(cbc_config_s *cbc);

int
list_cbc_sys_packacg_arg(cbc_config_s *cbc, cbc_sysp_s *css);

// Add functions

int
add_cbc_sys_package(cbc_config_s *cbc, cbc_sysp_s *cbs);

// Helper funtions

void
pack_syspack(cbc_sys_pack_s *spack, cbc_sysp_s *cbs);

#endif // __CBCSYSP_H__
