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
# include "../config.h"
# include "cbc_data.h"

int
get_varient_id(cbc_config_s *cmc, char *vari, unsigned long int *varient_id);

char *
cbc_get_varient_name(char *varient, char *valias);

unsigned long int
cbc_get_varient_id(cbc_varient_s *vari, char *name);

unsigned long int
search_for_vid(cbc_varient_s *vari, char *varient, char *valias);

int
check_for_package(cbc_config_s *cbc, unsigned long int osid, unsigned long int vid, char *pack);

void
check_ip_in_dns(unsigned long int *ip_addr, char *name, char *domain);

void
set_build_domain_updated(cbc_config_s *cbt, char *domain, uli_t id);

int
get_build_domain_id(cbc_config_s *cbc, char *domain, uli_t *id);

int
get_system_package_id(cbc_config_s *cbc, char *domain, uli_t *id);

#endif /* CBC_COMMON_H */
