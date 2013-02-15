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
#include <unistd.h>
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
check_fwd_zone(char *domain, char *filename, dnsa_config_t *dc)
{
	char *command;
	const char *syscom;
	int error, retval;
	
	if (!(command = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "command in check_fwd_zone");
	syscom = command;
	
	snprintf(command, RBUFF_S, "%s %s %s", dc->chkz, domain, filename);
	error = system(syscom);
	if (error != 0)
		retval = CHKZONE_FAIL;
	else
		retval = NONE;
	free(command);
	return retval;
}

int
check_rev_zone(char *domain, char *filename, dnsa_config_t *dc)
{
	char *command;
	const char *syscom;
	int error, retval;
	
	if (!(command = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "command in check_rev_zone");
	syscom = command;
	snprintf(command, RBUFF_S, "%s %s %s", dc->chkz, domain, filename);
	error = system(syscom);
	if (error != 0)
		retval = CHKZONE_FAIL;
	else
		retval = NONE;
	free(command);
	return retval;
}

int
commit_fwd_zones(dnsa_config_t *dc)
{
	char *configfile, *buffer, *filename;
	int retval;
	dnsa_t *dnsa;
	zone_info_t *zone;

	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in commit_fwd_zones");
	if (!(configfile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in commit_fwd_zones");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in commit_fwd_zones");
	filename = buffer;
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return MY_QUERY_FAIL;
	}
	zone = dnsa->zones;
	while (zone) {
		create_and_write_fwd_zone(dnsa, dc, zone);
		if ((retval = create_fwd_config(dc, zone, configfile)) != 0) {
			printf("Buffer Full!\n");
			break;
		}
		zone = zone->next;
	}
	snprintf(filename, NAME_S, "%s%s", dc->bind, dc->dnsa);
	if ((retval = write_file(filename, configfile)) != 0)
		fprintf(stderr, "Unable to write config file %s\n", filename);
	snprintf(buffer, NAME_S, "%s reload", dc->rndc);
	if ((retval = system(filename)) != 0)
		fprintf(stderr, "%s failed with %d\n", filename, retval);
	free(buffer);
	free(configfile);
	dnsa_clean_list(dnsa);
	return retval;
}

void
create_fwd_zone_header(dnsa_t *dnsa, char *hostm, unsigned long int id, char *zonefile)
{
	char *buffer;
	
	if (!(buffer = calloc(RBUFF_S + COMM_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_fwd_zone_header");
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

void
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
	free(buffer);
}

int
create_and_write_fwd_zone(dnsa_t *dnsa, dnsa_config_t *dc, zone_info_t *zone)
{
	int retval;
	char *zonefile, *buffer, *filename;
	
	if (!(zonefile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in create_and_write_fwd_zone");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_and_write_fwd_zone");
	filename = buffer;
	retval = 0;
	if (strncmp(zone->valid, "yes", COMM_S) == 0) {
		create_fwd_zone_header(
			dnsa, dc->hostmaster, zone->id, zonefile);
		add_records_to_fwd_zonefile(dnsa, zone->id, &zonefile);
		snprintf(filename, NAME_S, "%s%s",
			 dc->dir, zone->name);
		if ((retval = write_file(filename, zonefile)) != 0)
			printf("Unable to write %s zonefile\n",
			       zone->name);
		else if ((retval = check_fwd_zone(zone->name, filename, dc)) !=0)
			snprintf(zone->valid, COMM_S, "no");
	}
	free(zonefile);
	free(buffer);
	return retval;
}

int
create_fwd_config(dnsa_config_t *dc, zone_info_t *zone, char *configfile)
{
	int retval;
	char *buffer;
	size_t len;
	
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_fwd_config");
	len = BUILD_S;
	retval = 0;
	if (strncmp(zone->valid, "yes", COMM_S) == 0) {
			snprintf(buffer, TBUFF_S, "\
zone \"%s\" {\n\
\t\t\ttype master;\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n\n", zone->name, dc->dir, zone->name);
		if (strlen(configfile) + strlen(buffer) < len)
			strncat(configfile, buffer, strlen(buffer));
		else
			retval = BUFFER_FULL;
	}
	free(buffer);
	return retval;
}

int
commit_rev_zones(dnsa_config_t *dc)
{
	char *configfile, *buffer, *filename;
	int retval;
	dnsa_t *dnsa;
	rev_zone_info_t *zone;

	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in commit_rev_zones");
	if (!(configfile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in commit_rev_zones");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in commit_rev_zones");
	filename = buffer;
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(dc, dnsa, REV_ZONE | REV_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return MY_QUERY_FAIL;
	}
	zone = dnsa->rev_zones;
	while (zone) {
		create_and_write_rev_zone(dnsa, dc, zone);
		if ((retval = create_rev_config(dc, zone, configfile)) != 0) {
			fprintf(stderr, "Config file too big!\n");
			break;
		}
		zone = zone->next;
	}
	snprintf(filename, TBUFF_S, "%s%s", dc->bind, dc->rev);
	if ((retval = write_file(filename, configfile)) != 0)
		fprintf(stderr, "Writing %s failed with %d\n", buffer, retval);
	snprintf(buffer, NAME_S, "%s reload", dc->rndc);
	if ((retval = system(buffer)) != 0)
		fprintf(stderr, "%s failed with %d\n", buffer, retval);
	free(buffer);
	free(configfile);
	dnsa_clean_list(dnsa);
	return retval;
}

int
create_and_write_rev_zone(dnsa_t *dnsa, dnsa_config_t *dc, rev_zone_info_t *zone)
{
	int retval;
	char *zonefile, *buffer, *filename;
	unsigned long int id;
	
	if (!(zonefile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in create_rev_zones");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_rev_zones");
	filename = buffer;
	retval = 0;
	id = zone->rev_zone_id;
	if (strncmp(zone->valid, "yes", COMM_S) == 0) {
		create_rev_zone_header(
			dnsa, dc->hostmaster, id, zonefile);
		add_records_to_rev_zonefile(dnsa, id, &zonefile);
		snprintf(filename, NAME_S, "%s%s",
			 dc->dir, zone->net_range);
		if ((retval = write_file(filename, zonefile)) != 0)
			printf("Unable to write %s zonefile\n",
			       zone->net_range);
		else if ((retval = check_rev_zone(buffer, filename, dc)) != 0)
			snprintf(zone->valid, COMM_S, "no");
	}
	free(zonefile);
	free(buffer);
	return (retval);
}

void
create_rev_zone_header(dnsa_t *dnsa, char *hostm, unsigned long int id, char *zonefile)
{
	char *buffer;
	
	if (!(buffer = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer ins create_rev_zone_header");
	rev_zone_info_t *zone = dnsa->rev_zones;
	while (zone->rev_zone_id != id)
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
}

void
add_records_to_rev_zonefile(dnsa_t *dnsa, unsigned long int id, char **zonefile)
{
	char *buffer;
	size_t len, size;
	rev_record_row_t *record = dnsa->rev_records;
	len = BUILD_S;
	if (!(buffer = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in add_records_fwd");
	
	while (record) {
		if (record->rev_zone != id) {
			record = record->next;
		} else {
			snprintf(buffer, BUFF_S, "\
%s\tPTR\t%s\n", record->host, record->dest);
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
	free(buffer);
}

int
create_rev_config(dnsa_config_t *dc, rev_zone_info_t *zone, char *configfile)
{
	int retval;
	char *buffer, *in_addr;
	size_t len = BUILD_S;
	
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_rev_config");
	if (!(in_addr = calloc(MAC_S, sizeof(char))))
		report_error(MALLOC_FAIL, "in_addr in create_rev_config");
	retval = 0;
	get_in_addr_string(in_addr, zone->net_range, zone->prefix);
	if (strncmp(zone->valid, "yes", COMM_S) == 0) {
		snprintf(buffer, TBUFF_S, "\
zone \"%s\" {\n\
\t\t\ttype master;\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n\n", in_addr, dc->dir, zone->net_range);
		if (strlen(configfile) + strlen(buffer) < len)
			strncat(configfile, buffer, strlen(buffer));
		else
			retval = BUFFER_FULL;
	}
	free(buffer);
	free(in_addr);
	return retval;
}

int
add_host(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	zone_info_t *zone;
	record_row_t *record;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in add_host");
	if (!(zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "zone in add_host");
	if (!(record = malloc(sizeof(record_row_t))))
		report_error(MALLOC_FAIL, "record in add_host");

	init_dnsa_struct(dnsa);
	dnsa->zones = zone;
	dnsa->records = record;
	snprintf(zone->name, RBUFF_S, "%s", cm->domain);
	retval = 0;
	retval = run_search(dc, dnsa, ZONE_ID_ON_NAME);
	printf("Adding to zone %s, id %lu\n", zone->name, zone->id);
	snprintf(record->dest, RBUFF_S, "%s", cm->dest);
	snprintf(record->host, RBUFF_S, "%s", cm->host);
	snprintf(record->type, RBUFF_S, "%s", cm->rtype);
	record->zone = zone->id;
	record->pri = cm->prefix;
	run_insert(dc, dnsa, RECORDS);
	return retval;
}