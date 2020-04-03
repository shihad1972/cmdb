/*
 *
 *  cbc: Create build config
 *  Copyright (C) 2012 - 2014  Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbc_common.h
 *
 *  Common header file for the cbc program suite
 *
 */

#ifndef __CBC_COMMON_H__
# define __CBC_COMMON_H__
# include <config.h>
# include "cbc_data.h"

int
get_varient_id(ailsa_cmdb_s *cmc, char *vari, unsigned long int *varient_id);

void 
cbc_set_varient_updated(ailsa_cmdb_s *cbc, unsigned long int vid);

int
check_for_package(ailsa_cmdb_s *cbc, unsigned long int osid, unsigned long int vid, char *pack);

void
check_ip_in_dns(unsigned long int *ip_addr, char *name, char *domain);

void
set_build_domain_updated(ailsa_cmdb_s *cbt, char *domain);

int
get_default_id(ailsa_cmdb_s *cbc, int query, char *name, unsigned long int *id);

int
get_server_id(ailsa_cmdb_s *cbc, char *server, unsigned long int *id);

int
get_os_id(ailsa_cmdb_s *cmc, char *os[], unsigned long int *os_id);

int
get_build_domain_id(ailsa_cmdb_s *cbc, char *domain, uli_t *id);

int
get_system_package_id(ailsa_cmdb_s *cbc, char *domain, uli_t *id);

int
get_syspack_arg_id(ailsa_cmdb_s *cbc, char *field, uli_t sp_id, uli_t *id);

int
get_system_script_id(ailsa_cmdb_s *cbc, char *package, uli_t *id);

int
get_build_type_id(ailsa_cmdb_s *cbc, char *os, uli_t *id);

int
get_partition_id(ailsa_cmdb_s *cbc, char *name, char *mount, uli_t *id);

int
get_scheme_id(ailsa_cmdb_s *cbc, char *name, uli_t *id);

int
get_locale_id(ailsa_cmdb_s *cbc, char *name, uli_t *id);

int
get_scheme_id_from_build(ailsa_cmdb_s *cbc, uli_t server_id, uli_t *id);

int
get_os_alias(ailsa_cmdb_s *cbc, char *os, char *alias);

int
get_scheme_name(ailsa_cmdb_s *cbc, uli_t server_id, char *name);

int
get_part_opt_id(ailsa_cmdb_s *cbc, char *name, char *part, char *opt, uli_t *id);

int
set_scheme_updated(ailsa_cmdb_s *cbc, char *scheme);

void
fill_dbdata_os_search(dbdata_s *data, char *os[]);

void
check_for_alias(char **what, char *name, char *alias);

int
cbc_add_server(ailsa_cmdb_s *cbc, char *name, long unsigned int *server_id);

#endif /* CBC_COMMON_H */
