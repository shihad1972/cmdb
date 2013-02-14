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
 *  zones.c: functions that deal with zones for the dnsa program
 *
 * 
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> /* For sleep() */
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
	dnsa_clean_list(dnsa);
}

void
list_rev_zones(dnsa_config_t *dc)
{
	int retval;
	dnsa_t *dnsa;
	rev_zone_info_t *rev;

	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in list_zones");

	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_query(dc, dnsa, REV_ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	rev = dnsa->rev_zones;
	printf("Listing reverse zones from database %s on %s\n", dc->db, dc->dbtype);
	printf("Range\t\tprefix\tvalid\n");
	while (rev) {
		printf("%s\t/%lu\t%s\n", rev->net_range, rev->prefix, rev->valid);
		if (rev->next)
			rev = rev->next;
		else
			rev = '\0';
	}
	dnsa_clean_list(dnsa);
}

void
display_zone(char *domain, dnsa_config_t *dc)
{
	int retval;
	dnsa_t *dnsa;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in display_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	print_zone(dnsa, domain);
	dnsa_clean_list(dnsa);
}

void
print_zone(dnsa_t *dnsa, char *domain)
{
	unsigned int i, j;
	record_row_t *records = dnsa->records;
	zone_info_t *zone = dnsa->zones;
	i = j = 0;
	while (zone) {
		if (strncmp(zone->name, domain, RBUFF_S) == 0) {
			printf("%s.\t%s\thostmaster.%s\t%lu\n",
zone->name, zone->pri_dns, zone->name, zone->serial);
			j++;
			break;
		} else {
			zone = zone->next;
		}
	}
	if (j == 0) {
		printf("No zone %s found\n", domain);
		return;
	}
	while (records) {
		if (zone->id == records->zone) {
			if (strlen(records->host) < 8)
				printf("%s\t\t\tIN\t%s\t%s\n",
records->host, records->type, records->dest);
			else if (strlen(records->host) < 16)
				printf("%s\t\tIN\t%s\t%s\n",
records->host, records->type, records->dest);
			else if (strlen(records->host) < 24)
				printf("%s\tIN\t%s\t%s\n",
records->host, records->type, records->dest);
			else
				printf("%s\n\t\t\tIN\t%s\t%s\n",
records->host, records->type, records->dest);
			i++;
			records = records->next;
		} else {
			records = records->next;
		}
	}
	printf("\n%u records\n", i);
}

void
display_rev_zone(char *domain, dnsa_config_t *dc)
{
	int retval;
	dnsa_t *dnsa;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in display_rev_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(dc, dnsa, REV_ZONE | REV_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	print_rev_zone(dnsa, domain);
	dnsa_clean_list(dnsa);
}

void
print_rev_zone(dnsa_t *dnsa, char *domain)
{
	char *in_addr;
	unsigned int i, j;
	rev_record_row_t *records = dnsa->rev_records;
	rev_zone_info_t *zone = dnsa->rev_zones;
	if (!(in_addr = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "in_addr in print rev zone");
	i = j = 0;
	while (zone) {
		if (strncmp(zone->net_range, domain, RBUFF_S) == 0) {
			printf("%s\t%s\t%lu\n",
zone->net_range, zone->pri_dns, zone->serial);
			j++;
			break;
		} else {
			zone = zone->next;
		}
	}
	if (j == 0) {
		printf("Zone %s not found\n", domain);
		return;
	}
	get_in_addr_string(in_addr, zone->net_range, zone->prefix);
	while (records) {
		if (records->rev_zone == zone->rev_zone_id) {
			printf("%s.%s\t%s\n", records->host, in_addr, records->dest);
			records = records->next;
			i++;
		} else {
			records = records->next;
		}
	}
	if (i == 0)
		printf("No reverse records for range %s\n", zone->net_range);
}

int
commit_fwd_zones(dnsa_config_t *dc)
{
	char *zonefile, *filename;
	int retval;
	size_t len;
	dnsa_t *dnsa;
	zone_info_t *zone;

	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in commit_fwd_zones");
	if (!(zonefile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in commit_fwd_zones");
	if (!(filename = calloc(NAME_S, sizeof(char))))
		report_error(MALLOC_FAIL, "filename in commit_fwd_zones");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return MY_QUERY_FAIL;
	}
	if (dnsa->zones)
		zone = dnsa->zones;
	else
		return DOMAIN_LIST_FAIL;
	while (zone) {
		if (strncmp(zone->valid, "yes", COMM_S) == 0) {
			create_fwd_zone_header(dnsa, dc->hostmaster, zone->id, zonefile);
			len = add_records_to_fwd_zonefile(dnsa, zone->id, &zonefile);
			fprintf(stderr, "%s file is up to %zd\n", zone->name, len);
			snprintf(filename, NAME_S, "%s%s", dc->dir, zone->name);
			if ((retval = write_file(filename, zonefile)) != 0)
				printf("Unable to write %s zonefile\n", zone->name);
			zone = zone->next;
		} else {
			zone = zone->next;
		}
	}
	free(zonefile);
	dnsa_clean_list(dnsa);
	return retval;
}

int
commit_rev_zones(dnsa_config_t *dc)
{
	int retval;
	dnsa_t *dnsa;

	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in display_rev_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(dc, dnsa, REV_ZONE | REV_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return MY_QUERY_FAIL;
	}
	dnsa_clean_list(dnsa);
	return retval;
}

void
create_fwd_zone_header(dnsa_t *dnsa, char *hostm, unsigned long int id, char *zonefile)
{
	char *buffer;
	
	if (!(buffer = calloc(RBUFF_S + COMM_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_zone_header");
	zone_info_t *zone = dnsa->zones;
	record_row_t *record = dnsa->records;
	while (zone->id != id)
		zone = zone->next;
	snprintf(zonefile, BUILD_S, "\
$TTL %lu\n\
@\tIN\tSOA\t%s\t%s (\n\
\t\t\t\t%lu\t; Serial\n\
\t\t\t\t%lu\t\t; Refresh\n\
\t\t\t\t%lu\t\t; Retry\n\
\t\t\t\t%lu\t\t; Expire\n\
\t\t\t\t%lu\t\t); Cache TTL\n\
;\n\
\t\tNS\t%s\n",
	zone->ttl, zone->pri_dns, hostm, zone->serial, zone->refresh,
	zone->retry, zone->expire, zone->ttl, zone->pri_dns);
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0) {
		snprintf(buffer, RBUFF_S + COMM_S, "\t\tNS\t%s\n",
			 zone->sec_dns);
		strncat(zonefile, buffer, strlen(buffer));
	}
	while (record) {
		if ((record->zone == id) && 
			(strncmp(record->type, "MX", COMM_S) == 0)) {
			snprintf(buffer, RBUFF_S + COMM_S, "\
\t\tMX %lu\t%s\n", record->pri, record->dest);
			strncat(zonefile, buffer, strlen(buffer));
			record = record->next;
		} else {
			record = record->next;
		}
	}
	free(buffer);
}

size_t
add_records_to_fwd_zonefile(dnsa_t *dnsa, unsigned long int id, char **zonefile)
{
	char *buffer;
	size_t len, size;
	record_row_t *record = dnsa->records;
	len = BUILD_S;
	if (!(buffer = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in add_records_fwd");
	
	while (record) {
		if (record->zone != id) {
			record = record->next;
		} else if (strncmp(record->type, "MX", COMM_S) == 0) {
			record = record->next;
		} else if (strncmp(record->type, "NS", COMM_S) == 0) {
			record = record->next;
		} else {
			snprintf(buffer, BUFF_S, "\
%s\t%s\t%s\n", record->host, record->type, record->dest);
			size = strlen(*zonefile);
			if (strlen(buffer) + size > len) {
				len = len + BUILD_S;
				if (!(realloc(*zonefile, len))) {
					report_error(MALLOC_FAIL, "realloc of zonefile");
				}
			}
			strncat(*zonefile, buffer, strlen(buffer));
			record = record->next;
		}
	}
	return len;
}
