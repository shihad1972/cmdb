/*
 *
 *  cbc: Create build config
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
 *  cbc_dnsa.h
 *
 *  Header file for the cbc program that contains the links to
 *  the dnsa functions and data structures
 *
 *
 *  Part of the CMDB program
 *
 *  (C) Iain M Conochie 2012 - 2013 */

#ifndef __CBC_DNSA_H__
# define __CBC_DNSA_H__
# include <config.h>
# ifdef HAVE_DNSA

#  include "cmdb_dnsa.h"
#  include "cmdb_cbc.h"
#  include "cbc_data.h"
#  include "build.h"

void
fill_cbc_fwd_zone(zone_info_s *zone, char *domain, ailsa_cmdb_s *dc);

void
copy_cbc_into_dnsa(ailsa_cmdb_s *dc, ailsa_cmdb_s *cbc);

int
get_dns_ip_list(ailsa_cmdb_s *cbt, uli_t *ip, dbdata_s *data);

void
prep_dnsa_ip_list(dbdata_s *data, dnsa_s *dnsa, uli_t *ip);

int
check_for_build_ip_in_dns(ailsa_cmdb_s *cbt, cbc_comm_line_s *cml, cbc_s *data);

void
setup_dnsa_build_ip_structs(zone_info_s *zone, dnsa_s *dnsa, ailsa_cmdb_s *cbt, record_row_s *rec);

void
fill_rec_with_build_info(record_row_s *rec, zone_info_s *zone, cbc_comm_line_s *cml, cbc_s *cbc);

int
do_build_ip_dns_check(cbc_build_ip_s *bip, dbdata_s *data);

int
add_build_host_to_dns(ailsa_cmdb_s *dc, dnsa_s *dnsa);

void
write_zone_and_reload_nameserver(cbc_comm_line_s *cml);

void
remove_ip_from_dns(ailsa_cmdb_s *cbc, cbc_comm_line_s *cml, dbdata_s *data);

# endif /* HAVE_DNSA */

#endif /* __CBC_DNSA_H__ */
