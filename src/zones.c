/* 
 * 
 *  dnsa: DNS Administration
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
 * dnsa.c: main() function for the dnsa program
 *
 * 
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "dnsa_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

void
list_zones (dnsa_config_t *dc)
{
	int retval;
	dnsa_t *dnsa;
	zone_info_t *zone;
	size_t len;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in list_zones");
	
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_query(dc, dnsa, ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	zone = dnsa->zones;
	printf("Listing zones from database %s on %s\n", dc->db, dc->dbtype);
	printf("Name\t\t\t\tValid\n");
	while (zone) {
		len = strlen(zone->name);
		if (len < 8)
			printf("%s\t\t\t\t%s\n", zone->name, zone->valid);
		else if (len < 16)
			printf("%s\t\t\t%s\n", zone->name, zone->valid);
		else if (len < 24)
			printf("%s\t\t%s\n", zone->name, zone->valid);
		else if (len < 32)
			printf("%s\t%s\n", zone->name, zone->valid);
		else
			printf("%s\n\t\t\t\t%s\n", zone->name, zone->valid);
		if (zone->next)
			zone = zone->next;
		else
			zone = '\0';
	}
	return;
}