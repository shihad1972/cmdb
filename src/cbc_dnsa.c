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
 *  cbc_dnsa.c
 * 
 *  Functions to likn into dnsa functions for cbc
 * 
 *  part of the cbc program
 * 
 *  (C) Iain M. Conochie 2012 - 2013
 * 
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../config.h"
#include "cmdb.h"
#include "cmdb_cbc.h"

#ifdef HAVE_LIBPCRE

# include "checks.h"

#endif /* HAVE_LIBPCRE */

#ifdef HAVE_DNSA

# include "cmdb_dnsa.h"
# include "cbc_dnsa.h"


void
fill_cbc_fwd_zone(zone_info_s *zone, char *domain, dnsa_config_s *dc)
{
	snprintf(zone->name, RBUFF_S, "%s", domain);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
}

#endif /* HAVE_DNSA */
