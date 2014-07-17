/*
 *  cbc: Create Build Config
 *  (C) 2014 Iain M Conochie <iain-AT-thargoid.co.uk>
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
 *  cbcnet.h: Contains function definitions for cbcnet.c
 *
 */

#ifndef __CBC_NET_H
# define __CBC_NET_H
# include "../config.h"
# include "cbc_data.h"
# include "ifaddrs.h"

int
get_net_list_for_dhcp(cbc_build_domain_s *bd, cbc_dhcp_s **dh);

void
get_iface_info(cbc_iface_s **info);

int
fill_iface_info(struct ifaddrs *list, cbc_iface_s *info);

#endif /* __CBC_NET_H */

