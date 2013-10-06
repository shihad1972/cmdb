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
 *  builddomain.h
 * 
 *  Header file for functions for cbcdomain program
 * 
 *  part of the cbcdomain program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */
#include "cbcdomain.h"

#ifndef __CBC_BUILD_DOMAIN_H__
# define __CBC_BUILD_DOMAIN_H__

int
display_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl);

int
list_cbc_build_domain(cbc_config_s *cbc);

int
add_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl);

int
remove_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl);

int
modify_cbc_build_domain(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl);

int
get_build_domain(cbcdomain_comm_line_s *cdl, cbc_s *base);

void
copy_build_domain_values(cbcdomain_comm_line_s *cdl, cbc_build_domain_s *bdom);

int
get_mod_ldap_bld_dom(cbcdomain_comm_line_s *cdl, int *query);

int
get_mod_app_bld_dom(cbcdomain_comm_line_s *cdl, int *query);

int
cbc_get_build_dom_id(cbc_config_s *cbc, cbcdomain_comm_line_s *cdl, dbdata_s *data);

void
cbc_fill_ldap_update_data(cbcdomain_comm_line_s *cdl, dbdata_s *data, int query);

void
cbc_fill_app_update_data(cbcdomain_comm_line_s *cdl, dbdata_s *data, int query);

void
list_build_dom_servers(cbc_config_s *cbc, unsigned long int id);

void
print_build_dom_servers(dbdata_s *data);

# ifdef HAVE_DNSA

# endif /* HAVE_DNSA */

#endif /* __CBC_BUILD_DOMAIN_H__ */
