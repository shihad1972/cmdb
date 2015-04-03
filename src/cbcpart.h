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
 *  cbcpart.h
 * 
 *  Header file for data and functions for cbcpart program
 * 
 *  part of the cbcpart program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBCPART_H__
# define __CBCPART_H__
# include "../config.h"
# include "cmdb.h"
# include "cbc_data.h"

enum {
	PARTITION = 1,
	SCHEME = 2,
	OPTION = 3
};

typedef struct cbcpart_comm_line_s {
	unsigned long int min;
	unsigned long int max;
	unsigned long int pri;
	unsigned long int scheme_id;
	char *fs;
	char *log_vol;
	char *option;
	char *partition;
	char *scheme;
	short int action;
	short int lvm;
	short int type;
} cbcpart_comm_line_s;

void
init_cbcpart_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

void
init_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

void
clean_cbcpart_comm_line(cbcpart_comm_line_s *cpl);

int
parse_cbcpart_comm_line(int argc, char *argv[], cbcpart_comm_line_s *cpl);

# ifdef HAVE_LIBPCRE
int
validate_cbcpart_comm_line(cbcpart_comm_line_s *cpl);
# endif // HAVE_LIBPCRE

int
validate_cbcpart_user_input(cbcpart_comm_line_s *cpl, int argc);

int
list_seed_schemes(cbc_config_s *cbc);

int
display_full_seed_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
add_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
add_partition_to_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
check_cbcpart_lvm(cbcpart_comm_line_s *cpl, short int lvm);

int
check_cbcpart_names(cbc_pre_part_s *dpart, cbc_pre_part_s *part, cbcpart_comm_line_s *cpl);

int
add_new_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
add_part_info(cbcpart_comm_line_s *cpl, cbc_pre_part_s *part);

void
cbcpart_add_part_option(cbc_config_s *cbc, cbc_s *base, cbcpart_comm_line_s *cpl);

int
add_new_partition_option(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
remove_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
remove_partition_from_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
remove_scheme(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
remove_part_option(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
mod_scheme_part(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
modify_partition_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

int
modify_scheme_config(cbc_config_s *cbc, cbcpart_comm_line_s *cpl);

#endif /* __CBCPART_H__ */

