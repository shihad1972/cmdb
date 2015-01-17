/*
 *
 *  cbc: Create Build Configuration
 *  Copyright (C) 2014 - 2015  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcscript.h
 *
 *  Header file for data and functions for cbcscript program
 *
 *  part of the cbcsysp program
 *
 */

#ifndef __CBCSCRIPT_H__
# define __CBCSCRIPT_H__

enum {
	CBCSCRIPT = 1,
	CBCSCRARG = 2
};

typedef struct cbc_syss_s {
	char *name;
	char *arg;
	char *domain;
	short int action;
	short int what;
	unsigned long int no;
} cbc_syss_s;

// Helper Functions

void
initialise_cbc_scr(cbc_syss_s **scr);

void
init_cbc_sys_script_s(cbc_syss_s *scr);

void
clean_cbc_syss_s(cbc_syss_s *scr);

int
parse_cbc_script_comm_line(int argc, char *argv[], cbc_syss_s *cbcs);

int
check_cbc_script_comm_line(cbc_syss_s *cbcs);

// Add functions

int
cbc_script_add_script(cbc_config_s *cbc, cbc_syss_s *scr);

// Remove functions

int
cbc_script_rm_script(cbc_config_s *cbc, cbc_syss_s *scr);

#endif // __CBCSCRIPT_H__

