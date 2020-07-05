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
 *  build.h
 * 
 *  Header file for the build.c file.
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#ifndef __CBC_BUILD_H__
# define __CBC_BUILD_H__
# include "cmdb_cbc.h"

int
display_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml);

int
create_build_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

void
list_build_servers(ailsa_cmdb_s *cbt);

int
write_build_config(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

int
write_pre_host_script(ailsa_cmdb_s *cmc, cbc_comm_line_s *cml);

int
get_build_id(ailsa_cmdb_s *cbc, uli_t id, char *name, uli_t *build_id);

int
get_modify_query(unsigned long int ids[]);

void
cbc_prep_update_dbdata(dbdata_s *data, int type, unsigned long int ids[]);

char *
get_kick_part_opts(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, char *mnt);

void
add_kick_base_script(dbdata_s *data, string_len_s *build);

void
add_kick_final_config(cbc_comm_line_s *cml, string_len_s *build, char *url);

int
get_pre_part_options(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, char *mnt, dbdata_s **opts);

char *
cbc_complete_arg(ailsa_cmdb_s *cbc, uli_t server_id, char *arg);

char *
get_replaced_syspack_arg(dbdata_s *data, int loop);

int
modify_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml);

int
remove_build_config(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml);

#endif /* __CBC_BUILD_H__ */
