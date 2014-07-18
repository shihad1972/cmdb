/*
 *
 *  dnsa: Domain Name System Administration
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
 *  dnsa_net.h: DNSA network header file
 */

#ifndef __DNSA_NET_H
# define __DNSA_NET_H
# include "cmdb.h"
# include "cmdb_dnsa.h"

unsigned long int
get_net_range(unsigned long int prefix);

int
do_rev_lookup(char *ip, char *host, size_t len);

#endif /* __DNSA_NET_H */
