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
 *  zones.c: functions that deal with zones and records for the dnsa program
 *
 * 
 */

#include "../config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
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
	printf("Name\t\t\t\tValid\tSerial\n");
	while (zone) {
		len = strlen(zone->name);
		if (len < 8)
			printf("%s\t\t\t\t%s\t%lu\n", zone->name, zone->valid, zone->serial);
		else if (len < 16)
			printf("%s\t\t\t%s\t%lu\n", zone->name, zone->valid, zone->serial);
		else if (len < 24)
			printf("%s\t\t%s\t%lu\n", zone->name, zone->valid, zone->serial);
		else if (len < 32)
			printf("%s\t%s\t%lu\n", zone->name, zone->valid, zone->serial);
		else
			printf("%s\n\t\t\t\t%s\t%lu\n", zone->name, zone->valid, zone->serial);
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
	if (i == 1)
		printf("\n%u record\n", i);
	else
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
check_fwd_zone(char *domain, dnsa_config_t *dc)
{
	char *command, syscom[RBUFF_S];
	int error, retval;
	
	command = &syscom[0];
	
	snprintf(command, RBUFF_S, "%s %s %s%s", dc->chkz, domain, dc->dir, domain);
	error = system(syscom);
	if (error != 0)
		retval = CHKZONE_FAIL;
	else
		retval = NONE;
	return retval;
}

int
check_rev_zone(char *domain, dnsa_config_t *dc)
{
	char *command, syscom[RBUFF_S];
	int error, retval;
	
	command = &syscom[0];
	snprintf(command, RBUFF_S, "%s %s %s%s", dc->chkz, domain, dc->dir, domain);
	error = system(syscom);
	if (error != 0)
		retval = CHKZONE_FAIL;
	else
		retval = NONE;
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
		check_for_updated_fwd_zone(dc, zone);
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
		else if ((retval = check_fwd_zone(zone->name, dc)) !=0)
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

void
check_for_updated_fwd_zone(dnsa_config_t *dc, zone_info_t *zone)
{
	int retval;
	unsigned long int serial;
	dbdata_t serial_data, id_data;

	retval = 0;
	if (strncmp("yes", zone->updated, COMM_S) == 0) {
		serial = get_zone_serial();
		if (serial > zone->serial)
			zone->serial = serial;
		else
			zone->serial++;
		init_dbdata_struct(&serial_data);
		init_dbdata_struct(&id_data);
		serial_data.args.number = zone->serial;
		id_data.args.number = zone->id;
		serial_data.next = &id_data;
		if ((retval = run_update(dc, &serial_data, ZONE_SERIAL)) != 0)
			fprintf(stderr, "Cannot update zone serial in database!\n");
		else
			fprintf(stderr, "Serial number updated\n");
		run_update(dc, &id_data, ZONE_UPDATED_NO);
	}
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
		else if ((retval = check_rev_zone(zone->net_range, dc)) != 0)
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
	
	if (!(buffer = calloc(RBUFF_S + COMM_S, sizeof(char))))
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
display_multi_a_records(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	dbdata_t *start;
	rev_zone_info_t *rzone;
	record_row_t *records;
	preferred_a_t *prefer;

	retval = 0;
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in disp_multi_a");
	if (!(rzone = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rzone in disp_multi_a");
	init_dnsa_struct(dnsa);
	init_rev_zone_struct(rzone);
	dnsa->rev_zones = rzone;
	if ((retval = run_multiple_query(
		dc, dnsa, DUPLICATE_A_RECORD | PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	init_initial_dbdata(&start, RECORDS_ON_DEST_AND_ID);
	if (strncmp(cm->dest, "NULL", COMM_S) != 0) {
		select_specific_ip(dnsa, cm);
		if (!(dnsa->records))
			fprintf(stderr, "No multiple A records for IP %s\n",
				cm->dest);
		else
			print_multiple_a_records(dc, start, dnsa);
		dnsa_clean_dbdata_list(start);
		dnsa_clean_list(dnsa);
		return NONE;
	}
	snprintf(rzone->net_range, RANGE_S, "%s", cm->domain);
	if ((retval = run_search(dc, dnsa, REV_ZONE_PREFIX)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	cm->prefix = rzone->prefix;
	fill_rev_zone_info(rzone, cm, dc);
	if (rzone->prefix == NONE) {
		printf("Net range %s does not exist in database\n", cm->domain);
		dnsa_clean_list(dnsa);
		dnsa_clean_dbdata_list(start);
		return NO_DOMAIN;
	}
	get_a_records_for_range(&(dnsa->records), dnsa->rev_zones);
	records = dnsa->records;
	if (!records)
		printf("No duplicate entries for range %s\n", cm->domain);
	while (records) {
		prefer = dnsa->prefer;
		printf("Destination %s has %lu records",
		       records->dest, records->id);
		while (prefer) {
			if (strncmp(prefer->ip, records->dest, RANGE_S) == 0)
				printf("; preferred PTR is %s", prefer->fqdn);
			prefer = prefer->next;
		}
		printf("\n");
		records = records->next;
	}
	records = dnsa->records;
	if (records) {
		printf("If you want to see the A records for a specific IP use the ");
		printf("-i option\nE.G. dnsa -m -i <IP-Address>\n");
	}
	dnsa_clean_dbdata_list(start);
	dnsa_clean_list(dnsa);
	return retval;
}

void
print_multiple_a_records(dnsa_config_t *dc, dbdata_t *start, dnsa_t *dnsa)
{
	int i, j, k;
	char name[RBUFF_S], *fqdn;
	dbdata_t *dlist;
	record_row_t *records = dnsa->records;
	preferred_a_t *prefer = dnsa->prefer;
	fqdn = &name[0];
	while (records) {
		dlist = start;
		printf("Destination %s has %lu records; * denotes preferred PTR record\n",
		records->dest, records->id);
		snprintf(dlist->args.text, RANGE_S, "%s", records->dest);
		i = run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
		for (j = 0; j < i; j++) {
			snprintf(fqdn, RBUFF_S, "%s.%s",
				 dlist->fields.text, dlist->next->fields.text);
			prefer = dnsa->prefer;
			k = 0;
			while (prefer) {
				if (strncmp(fqdn, prefer->fqdn, RBUFF_S) == 0) {
					printf("     *  %s\n", fqdn);
					k++;
				}
				prefer=prefer->next;
			}
			if (k == 0)
				printf("\t%s\n", fqdn);
			dlist = dlist->next->next->next;
		}
		printf("\n");
		records = records->next;
		dlist = start->next->next->next;
		dnsa_clean_dbdata_list(dlist);
		dlist = start->next->next;
		dlist->next = '\0';
	}
}

int
mark_preferred_a_record(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	zone_info_t *zone;

	retval = 0;
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in mark_preferred_a_record");
	if (!(zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "zone in mark_preferred_a_record");
	init_dnsa_struct(dnsa);
	init_zone_struct(zone);
	dnsa->zones = zone;
	if ((retval = run_query(dc, dnsa, DUPLICATE_A_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	select_specific_ip(dnsa, cm);
	if (!(dnsa->records)) {
		fprintf(stderr, "No multiple A records for IP %s\n",
			cm->dest);
		return CANNOT_ADD_A_RECORD;
	} else {
		retval = get_preferred_a_record(dc, cm, dnsa);
	}
	if (retval != 0)
		return retval;
	printf("IP: %s\tIP Addr: %lu\tRecord ID: %lu\n",
	       dnsa->prefer->ip, dnsa->prefer->ip_addr, dnsa->prefer->record_id);
	if ((retval = run_insert(dc, dnsa, PREFERRED_AS)) != 0)
		fprintf(stderr, "Cannot insert preferred A record\n");
	else
		printf("Database updated with preferred A record\n");
	dnsa_clean_list(dnsa);
	return retval;
}

int
get_preferred_a_record(dnsa_config_t *dc, comm_line_t *cm, dnsa_t *dnsa)
{
	char *name = cm->dest;
	char fqdn[RBUFF_S];
	int i = 0;
	uint32_t ip_addr;
	dbdata_t *start, *list;
	preferred_a_t *prefer;
	record_row_t *rec = dnsa->records;

	if (!(prefer = malloc(sizeof(preferred_a_t))))
		report_error(MALLOC_FAIL, "prefer in get_preferred_a_record");
	init_preferred_a_struct(prefer);
	dnsa->prefer = prefer;
	init_initial_dbdata(&start, RECORDS_ON_DEST_AND_ID);
	while (rec) {
		if (strncmp(name, rec->dest, RBUFF_S) == 0) {
			snprintf(prefer->ip, RANGE_S, "%s", cm->dest);
			inet_pton(AF_INET, rec->dest, &ip_addr);
			prefer->ip_addr = (unsigned long int) htonl(ip_addr);
			i++;
		}
		if (rec->next)
			rec = rec->next;
		else
			rec = '\0';
	}
	snprintf(start->args.text, RANGE_S, "%s", name);
	i = run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
	list = start;
	name = &fqdn[0];
	i = 0;
	while (list) {
		snprintf(name, RBUFF_S, "%s.%s",
list->fields.text, list->next->fields.text);
		if (strncmp(name, cm->domain, RBUFF_S) == 0) {
			i++;
			prefer->record_id = list->next->next->fields.number;
			snprintf(prefer->fqdn, RBUFF_S, "%s", name);
		}
		list = list->next->next->next;
	}
	if (i == 0) {
		fprintf(stderr,
"You FQDN is not associated with this IP address\n\
If you it associated with this IP address, please add it as an A record\n\
Curently you cannot add FQDN's not authoritative on this DNS server\n");
		return CANNOT_ADD_A_RECORD;
	}
	return NONE;
}

int
add_host(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	zone_info_t *zone;
	record_row_t *record;
	dbdata_t data;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in add_host");
	if (!(zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "zone in add_host");
	if (!(record = malloc(sizeof(record_row_t))))
		report_error(MALLOC_FAIL, "record in add_host");

	init_dnsa_struct(dnsa);
	init_dbdata_struct(&data);
	dnsa->zones = zone;
	dnsa->records = record;
	snprintf(zone->name, RBUFF_S, "%s", cm->domain);
	retval = 0;
	retval = run_search(dc, dnsa, ZONE_ID_ON_NAME);
	printf("Adding to zone %s, id %lu\n", zone->name, zone->id);
	snprintf(record->dest, RBUFF_S, "%s", cm->dest);
	snprintf(record->host, RBUFF_S, "%s", cm->host);
	snprintf(record->type, RANGE_S, "%s", cm->rtype);
	record->zone = data.args.number = zone->id;
	record->pri = cm->prefix;
	if ((retval = run_insert(dc, dnsa, RECORDS)) != 0)
		fprintf(stderr, "Cannot insert record\n");
	retval = run_update(dc, &data, ZONE_UPDATED_YES);
	dnsa_clean_list(dnsa);
	return retval;
}

int
add_fwd_zone(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	zone_info_t *zone;
	dbdata_t data;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in add_fwd_zone");
	if (!(zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "zone in add_fwd_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	init_zone_struct(zone);
	fill_fwd_zone_info(zone, cm, dc);
	dnsa->zones = zone;
	if ((retval = check_for_zone_in_db(dc, dnsa, FORWARD_ZONE)) != 0) {
		printf("Zone %s already exists in database\n", zone->name);
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = run_insert(dc, dnsa, ZONES)) != 0) {
		fprintf(stderr, "Unable to add zone %s\n", zone->name);
		dnsa_clean_list(dnsa);
		return CANNOT_INSERT_ZONE;
	} else {
		fprintf(stderr, "Added zone %s\n", zone->name);
	}
	if ((retval = validate_fwd_zone(dc, zone, dnsa)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	init_dbdata_struct(&data);
	data.args.number = zone->id;
	if ((retval = run_update(dc, &data, ZONE_VALID_YES)) != 0)
		printf("Unable to mark zone as valid in database\n");
	else
		printf("Zone marked as valid in the database\n");
	dnsa_clean_zones(zone);
	dnsa->zones = '\0';
	retval = create_and_write_fwd_config(dc, dnsa);
	dnsa_clean_list(dnsa);
	return retval;
}

int
add_rev_zone(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	rev_zone_info_t *zone;
	dbdata_t data;
	
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in add_fwd_zone");
	if (!(zone = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "zone in add_fwd_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	init_rev_zone_struct(zone);
	fill_rev_zone_info(zone, cm, dc);
	dnsa->rev_zones = zone;
	print_rev_zone_info(zone);
	if ((retval = check_for_zone_in_db(dc, dnsa, REVERSE_ZONE)) != 0) {
		printf("Zone %s already exists in database\n", zone->net_range);
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = run_insert(dc, dnsa, REV_ZONES)) != 0) {
		fprintf(stderr, "Unable to add zone %s\n", zone->net_range);
		dnsa_clean_list(dnsa);
		return CANNOT_INSERT_ZONE;
	} else {
		fprintf(stderr, "Added zone %s\n", zone->net_range);
	}
	if ((retval = validate_rev_zone(dc, zone, dnsa)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	init_dbdata_struct(&data);
	data.args.number = zone->rev_zone_id;
	if ((retval = run_update(dc, &data, REV_ZONE_VALID_YES)) != 0)
		printf("Unable to mark rev_zone %s as valid\n", zone->net_range);
	else
		printf("Rev zone %s marked as valid\n", zone->net_range);
	dnsa_clean_rev_zones(zone);
	dnsa->rev_zones = '\0';
	retval = create_and_write_rev_config(dc, dnsa);
	dnsa_clean_list(dnsa);
	return retval;
}

int
create_and_write_fwd_config(dnsa_config_t *dc, dnsa_t *dnsa)
{
	char *configfile, *buffer, filename[NAME_S];
	int retval;
	zone_info_t *zone;

	buffer = &filename[0];
	retval = 0;
	if ((retval = run_query(dc, dnsa, ZONE)) != 0)
		return retval;
	zone = dnsa->zones;
	if (!(configfile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "configfile in add_fwd_zone");
	while (zone) {
		if ((retval = create_fwd_config(dc, zone, configfile)) != 0) {
			printf("Buffer Full!\n");
			break;
		}
		zone = zone->next;
	}
	snprintf(buffer, NAME_S, "%s%s", dc->bind, dc->dnsa);
	if ((retval = write_file(filename, configfile)) != 0)
		fprintf(stderr, "Unable to write config file %s\n", filename);
	snprintf(buffer, NAME_S, "%s reload", dc->rndc);
	if ((retval = system(filename)) != 0)
		fprintf(stderr, "%s failed with %d\n", filename, retval);
	return retval;
}

int
create_and_write_rev_config(dnsa_config_t *dc, dnsa_t *dnsa)
{
	char *configfile, *buffer, filename[NAME_S];
	int retval;
	rev_zone_info_t *zone;
	
	buffer = &filename[0];
	retval = 0;
	if ((retval = run_query(dc, dnsa, REV_ZONE)) != 0)
		return retval;
	zone = dnsa->rev_zones;
	if (!(configfile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "configfile in add_fwd_zone");
	while (zone) {
		if ((retval = create_rev_config(dc, zone, configfile)) != 0) {
			printf("Buffer Full!\n");
			break;
		}
		zone = zone->next;
	}
	snprintf(buffer, NAME_S, "%s%s", dc->bind, dc->rev);
	if ((retval = write_file(filename, configfile)) != 0)
		fprintf(stderr, "Unable to write config file %s\n", filename);
	snprintf(buffer, NAME_S, "%s reload", dc->rndc);
	if ((retval = system(filename)) != 0)
		fprintf(stderr, "%s failed with %d\n", filename, retval);
	return retval;
}

int
validate_fwd_zone(dnsa_config_t *dc, zone_info_t *zone, dnsa_t *dnsa)
{
	char command[NAME_S], *buffer;
	int retval;

	retval = 0;
	buffer = &command[0];
	snprintf(zone->valid, COMM_S, "yes");
	if ((retval = add_trailing_dot(zone->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(zone->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	if ((retval = run_search(dc, dnsa, ZONE_ID_ON_NAME)) != 0) {
		printf("Unable to get ID of zone %s\n", zone->name);
		return ID_INVALID;
	}
	if ((retval = create_and_write_fwd_zone(dnsa, dc, zone)) != 0) {
		fprintf(stderr, "Unable to write the zonefile for %s\n",
			zone->name);
		return FILE_O_FAIL;
	}
	snprintf(buffer, NAME_S, "%s %s %s%s", 
		 dc->chkz, zone->name, dc->dir, zone->name);
	if ((retval = system(command)) != 0) {
		fprintf(stderr, "Checkzone of %s failed\n", zone->name);
		return CHKZONE_FAIL;
	}
	return retval;
}

int
validate_rev_zone(dnsa_config_t *dc, rev_zone_info_t *zone, dnsa_t *dnsa)
{
	char command[NAME_S], *buffer;
	int retval;
	
	retval = 0;
	buffer = &command[0];
	snprintf(zone->valid, COMM_S, "yes");
	if ((retval = add_trailing_dot(zone->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(zone->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	if ((retval = run_search(dc, dnsa, REV_ZONE_ID_ON_NET_RANGE)) != 0) {
		printf("Unable to get ID of zone %s\n", zone->net_range);
		return ID_INVALID;
	}
	if ((retval = create_and_write_rev_zone(dnsa, dc, zone)) != 0) {
		fprintf(stderr, "Unable to write the zonefile for %s\n",
			zone->net_range);
		return FILE_O_FAIL;
	}
	snprintf(buffer, NAME_S, "%s %s %s%s", 
		 dc->chkz, zone->net_range, dc->dir, zone->net_range);
	if ((retval = system(command)) != 0) {
		fprintf(stderr, "Checkzone of %s failed\n", zone->net_range);
		return CHKZONE_FAIL;
	}
	return retval;
}

int
build_reverse_zone(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval, a_rec, rev_rec;
	dnsa_t *dnsa;
	record_row_t *rec;
	rev_record_row_t *add, *delete;

	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in build_reverse_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = run_multiple_query(
	       dc, dnsa, DUPLICATE_A_RECORD | PREFERRED_A | REV_ZONE | ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = get_correct_rev_zone_and_preferred_records(cm, dnsa)) > 0) {
		dnsa_clean_list(dnsa);
		return retval;
	} else if (retval < 0) {
		/* No Duplicate records. Just convert all A records
		printf("\
Just using A records for net range %s\n", dnsa->rev_zones->net_range); */
		dnsa_clean_records(dnsa->records);
		dnsa_clean_prefer(dnsa->prefer);
		dnsa->records = '\0';
		dnsa->prefer = '\0';
	}
	rec = dnsa->records; /* Holds duplicate A records */
	dnsa->records = '\0';
	if ((retval = run_multiple_query(dc, dnsa, ALL_A_RECORD | REV_RECORD)) != 0) {
		dnsa_clean_records(rec);
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((a_rec = get_a_records_for_range(&(dnsa->records), dnsa->rev_zones)) == 0) {
		dnsa_clean_records(rec);
		dnsa_clean_list(dnsa);
		printf("\
No A records for net_range %s\n", dnsa->rev_zones->net_range);
		return NO_RECORDS;
	}
	rev_rec = get_rev_records_for_range(&(dnsa->rev_records), dnsa->rev_zones);
/*	printf("We have %d A records and %d rev records\n", a_rec, rev_rec); */
	add_int_ip_to_records(dnsa);
	retval = rev_records_to_add(dnsa, rec, &add);
	dnsa_clean_records(rec);
	dnsa_clean_list(dnsa);
	return NONE;
}

int
get_correct_rev_zone_and_preferred_records(comm_line_t *cm, dnsa_t *dnsa)
{
	int retval = 0;
	if ((retval = get_rev_zone(cm, dnsa)) != 0) {
		return retval;
	}
	if ((retval = get_a_records_for_range(&(dnsa->records), dnsa->rev_zones)) == 0) {
/*		printf("No Duplicate records for net range %s\n",
		      dnsa->rev_zones->net_range); */
		return -1;
	} else {
/*		printf("Got %d duplicate records\n", retval); */
		retval = NONE;
		if ((retval = get_pref_a_for_range(&(dnsa->prefer), dnsa->rev_zones)) == 0) {
/*			printf("No preferred A records\n"); */
			return retval;
		} else {
/*			printf("Got %d prefered records\n", retval); */
			retval = NONE;
		}
	}
	return retval;
}

int
get_rev_zone(comm_line_t *cm, dnsa_t *dnsa)
{
	rev_zone_info_t *rev, *list, *next;
	rev = dnsa->rev_zones;
	list = '\0';
	if (rev->next)
		next = rev->next;
	else
		next = '\0';
	while (rev) {
		if (strncmp(rev->net_range, cm->domain, RANGE_S) == 0) {
			list = rev;
			rev = next;
		} else {
			free(rev);
			rev = next;
		}
		if (next)
			next = rev->next;
	}
	dnsa->rev_zones = list;
	if (list) {
		list->next = '\0';
		return NONE;
	}
	fprintf(stderr, "Reverse domain %s not found\n", cm->domain);
	return NO_DOMAIN;
}

int
get_rev_records_for_range(rev_record_row_t **records, rev_zone_info_t *zone)
{
	int i = 0;
	rev_record_row_t *list, *prev, *next, *tmp, *rec;
	list = *records;
	rec = prev = list;
	if (rec)
		next = rec->next;
	else
		return NONE;
	while (rec) {
		tmp = rec;
		if (zone->rev_zone_id != rec->rev_zone) {
			free(rec);
			rec = next;
			if (rec)
				next = rec->next;
			if (tmp == list)
				list = prev = rec;
			else
				prev->next = rec;
		} else {
			i++;
			if (prev != rec)
				prev = prev->next;
			rec = next;
			if (rec)
				next = rec->next;
		}
	}
	if (*records != list)
		*records = list;
	return i;
}

/* Build a list of the reverse records we need to add to the database. */
int
rev_records_to_add(dnsa_t *dnsa, record_row_t *rec, rev_record_row_t **rev)
{
	int i, j, k;
	record_row_t *fwd = dnsa->records; /* List of A records */
	record_row_t *dups, *list, *tmp, *fwd_list, *prev;
	rev_record_row_t *rev_list;
	preferred_a_t *prefer;

	dups = '\0';
	fwd_list = prev = fwd;
	j = 0;
/* 
 * Run through the list of forward records for this network range. Delete
 * any duplicate forward records from this list that we find, but add them
 * to the 2nd list of forward records which is the list of duplicates.
 * Once we have a list of duplicates we then need to decide which of
 * these duplicates to use for the PTR record. Check the preferred_a list
 * and use that if it exists. If not, use the first one we come across. 
 */
	while (fwd) {
		i = k = 0;
		list = rec; /* List of duplicate IP address in the net range */
		tmp = fwd->next;
		while (list) {
			if (strncmp(list->dest, fwd->dest, RANGE_S) == 0) {
				if (check_for_duplicate(fwd->dest, dups)) {
					if (fwd_list == fwd)
						fwd_list = prev = tmp;
					else
						prev->next = tmp;
					free(fwd);
					fwd = '\0';
					break;
				}
				i++;
				add_a_to_duplicate(&dups, fwd);
				break;
			} else {
				list = list->next;
			}
		}
		if (!fwd) {
			fwd = tmp;
			continue;
		}
		j++; /* Count number of memebers we have in modified list */
		rev_list = dnsa->rev_records;
		while (dups) {
			i = 0;
			tmp = dups->next;
			prefer = dnsa->prefer;
			while (prefer) {
				if (strncmp(dups->dest, prefer->ip, RANGE_S) == 0) {
					i++;
					break;
				} else {
					prefer = prefer->next;
				}
			}
			if (i == 0)
				add_dup_to_prefer_list(dnsa, dups);
			free(dups);
			dups = tmp;
		}
		i = 0;
		while (rev_list) { /* check in rev list ?? */
			if (rev_list->ip_addr == fwd->ip_addr) {
				i++;
				break;
			} else {
				rev_list = rev_list->next;
			}
		}
		if (i == 0) {
			prefer = dnsa->prefer;
			while (prefer) {
				if (fwd->ip_addr == prefer->ip_addr)
					k++; /* there is a preferred A for this IP */
				if (strncmp(fwd->host, prefer->fqdn, RBUFF_S) == 0) {
					if ((insert_into_rev_add_list(dnsa, fwd, rev)) == 0) {
						return 1;
					} else {
						i++;
						break;
					}
				}
				prefer = prefer->next;
			}
			if (k == 0) { 
/* If no preferred A for this IP add the first one we see */
				printf("\
Adding record %s as PTR for %s. If you do not want to \n\
use this please setup a preferred A record for another host\n", fwd->host, fwd->dest);
				if ((insert_into_rev_add_list(dnsa, fwd, rev)) == 0) {
					return 1;
				}
			}
		}
		if (prev != fwd)
			prev = prev->next;
		fwd = fwd->next;
	}
/*	printf("Modified forward records now have %d members\n", j); */
	if (fwd_list != dnsa->records)
		dnsa->records = fwd_list;
	return NONE;
}

void
add_a_to_duplicate(record_row_t **dups, record_row_t *fwd)
{
	record_row_t *new, *tmp;

	if (!(new = malloc(sizeof(record_row_t))))
		report_error(MALLOC_FAIL, "new in add_a_to_duplicate");
	init_record_struct(new);
	snprintf(new->dest, RANGE_S, "%s", fwd->dest);
	snprintf(new->host, RBUFF_S, "%s", fwd->host);
	tmp = *dups;
	if (!tmp) {
		*dups = new;
	} else {
		while (tmp->next)
			tmp = tmp->next;
		tmp->next = new;
	}
}

int
insert_into_rev_add_list(dnsa_t *dnsa, record_row_t *fwd, rev_record_row_t **rev)
{
	char rev_host[RANGE_S];
	rev_record_row_t *new, *list;
	zone_info_t *zones = dnsa->zones;

	if (!(new = malloc(sizeof(rev_record_row_t))))
		report_error(MALLOC_FAIL, "new in insert_into_rev_add_list");
	list = *rev;
	init_rev_record_struct(new);
	new->rev_zone = dnsa->rev_zones->rev_zone_id;
	while (zones) {
		if (fwd->zone == zones->id)
			break;
		else
			zones = zones->next;
	}
	if (!zones) { /* fwd record belongs to non-existent zone */
		free(new);
		return 0;
	}
	snprintf(new->dest, RBUFF_S, "%s.", fwd->host);
	if (get_rev_host(dnsa->rev_zones->prefix, rev_host, fwd->dest) != 0)
		return 0;
	else
		snprintf(new->host, 11, "%s", rev_host);
	printf("Adding record for %s -> %s\n", new->dest, new->host);
	if (!list) {
		*rev = new;
	} else {
		while (list->next)
			list = list->next;
		list->next = new;
	}
	return 1;
}

void
add_dup_to_prefer_list(dnsa_t *dnsa, record_row_t *fwd)
{
	preferred_a_t *new, *list;

	if (!(new = malloc(sizeof(preferred_a_t))))
		report_error(MALLOC_FAIL, "new in add_dup_to_prefer_list");
	init_preferred_a_struct(new);
	list = dnsa->prefer;
	snprintf(new->ip, RANGE_S, "%s", fwd->dest);
	snprintf(new->fqdn, RBUFF_S, "%s", fwd->host);
	new->record_id = fwd->id;
	new->ip_addr = fwd->ip_addr;
	if (!list) {
		dnsa->prefer = new;
	} else {
		while (list->next)
			list = list->next;
		list->next = new;
	}
}

int
check_for_duplicate(char *destination, record_row_t *duplicates)
{
	while (duplicates) {
		if (strncmp(destination, duplicates->dest, RANGE_S) == 0)
			return 1;
		else
			duplicates = duplicates->next;
	}
	return 0;
}

int
get_rev_host(unsigned long int prefix, char *rev_host, char *host)
{
	char *tmp;
	int i;
	size_t len;

	*rev_host = '\0';
	if (prefix == 8) {
		for (i = 0; i < 3; i++) {
			tmp = strrchr(host, '.');
			tmp++;
			rev_host = strncat(rev_host, tmp, 4);
			rev_host = strncat(rev_host, ".", CH_S);
			tmp--;
			*tmp = '\0';
		}
		len = strlen(rev_host);
		rev_host[len - 1] = '\0';
	} else if (prefix == 16) {
		for (i = 0; i < 2; i++) {
			tmp = strrchr(host, '.');
			tmp++;
			rev_host = strncat(rev_host, tmp, 4);
			rev_host = strncat(rev_host, ".", CH_S);
			tmp--;
			*tmp = '\0';
		}
		len = strlen(rev_host);
		rev_host[len - 1] = '\0';
	} else if (prefix >= 24) {
		tmp = strrchr(host, '.');
		tmp++;
		strncpy(rev_host, tmp, 4);
	} else {
		printf("Prefix %lu invalid\n", prefix);
			return 1;
	}
	return 0;
}

void
add_int_ip_to_records(dnsa_t *dnsa)
{
	if (dnsa->records)
		add_int_ip_to_fwd_records(dnsa->records);
	if (dnsa->rev_records)
		add_int_ip_to_rev_records(dnsa);
}

void
add_int_ip_to_fwd_records(record_row_t *records)
{
	uint32_t ip;
	while (records) {
		if (inet_pton(AF_INET, records->dest, &ip))
			records->ip_addr = (unsigned long int) htonl(ip);
		else
			records->ip_addr = 0;
		records = records->next;
	}
}

int
add_int_ip_to_rev_records(dnsa_t *dnsa)
{
	char address[RANGE_S], host[RANGE_S], *ip_addr, *tmp;
	int i;
	uint32_t ip;
	unsigned long int prefix = dnsa->rev_zones->prefix;
	rev_record_row_t *rev = dnsa->rev_records;
	ip_addr = address;
	while (rev) {
		strncpy(ip_addr, dnsa->rev_zones->net_range, RANGE_S);
		strncpy(host, rev->host, RANGE_S);
		if (prefix == 8) {
			for (i = 0; i < 3; i++) {
				tmp = strrchr(ip_addr, '.');
				*tmp = '\0';
			}
			for (i = 0; i < 2; i++) {
				tmp = strrchr(host, '.');
				ip_addr = strncat(ip_addr, tmp, 4);
				*tmp = '\0';
			}
			ip_addr = strncat(ip_addr, ".", CH_S);
			strncat(ip_addr, host, 4);
		} else if (prefix == 16) {
			for (i = 0; i < 2; i++) {
				tmp = strrchr(ip_addr, '.');
				*tmp = '\0';
			}
			tmp = strrchr(host, '.');
			ip_addr = strncat(ip_addr, tmp, 4);
			*tmp = '\0';
			ip_addr = strncat(ip_addr, ".", CH_S);
			ip_addr = strncat(ip_addr, host, 4);
		} else if (prefix >= 24) {
			tmp= strrchr(ip_addr, '.');
			*tmp = '\0';
			ip_addr = strncat(ip_addr, ".", CH_S);
			ip_addr = strncat(ip_addr, host, 4);
		} else {
			return 1;
		}
		if (inet_pton(AF_INET, ip_addr, &ip)) {
			rev->ip_addr = (unsigned long int) htonl(ip);
		} else {
			rev->ip_addr = 0;
			return 1;
		}
		rev = rev->next;
	}
	return NONE;
}

unsigned long int
get_zone_serial(void)
{
	time_t now;
	struct tm *lctime;
	char sday[COMM_S], smonth[COMM_S], syear[COMM_S], sserial[RANGE_S];
	unsigned long int serial;

	now = time(0);
	lctime = localtime(&now);
	snprintf(syear, COMM_S, "%d", lctime->tm_year + 1900);
	if (lctime->tm_mon < 9)
		snprintf(smonth, COMM_S, "0%d", lctime->tm_mon + 1);
	else
		snprintf(smonth, COMM_S, "%d", lctime->tm_mon + 1);
	if (lctime->tm_mday < 10)
		snprintf(sday, COMM_S, "0%d", lctime->tm_mday);
	else
		snprintf(sday, COMM_S, "%d", lctime->tm_mday);
	snprintf(sserial, RANGE_S, "%s%s%s01",
		 syear,
		 smonth,
		 sday);
	serial = strtoul(sserial, NULL, 10);
	return serial;
}

int
check_for_zone_in_db(dnsa_config_t *dc, dnsa_t *dnsa, short int type)
{
	int retval;

	retval = 0;
	if (type == FORWARD_ZONE) {
		if ((retval = run_search(dc, dnsa, ZONE_ID_ON_NAME)) != 0)
			return retval;
		else if (dnsa->zones->id != 0)
			return ZONE_ALREADY_EXISTS;
	} else if (type == REVERSE_ZONE) {
		if ((retval = run_search(dc, dnsa, REV_ZONE_ID_ON_NET_RANGE)) !=0)
			return retval;
		else if (dnsa->rev_zones->rev_zone_id != 0)
			return ZONE_ALREADY_EXISTS;
	}
	return retval;
}
void
fill_fwd_zone_info(zone_info_t *zone, comm_line_t *cm, dnsa_config_t *dc)
{
	snprintf(zone->name, RBUFF_S, "%s", cm->domain);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
}

void
fill_rev_zone_info(rev_zone_info_t *zone, comm_line_t *cm, dnsa_config_t *dc)
{
	char address[RANGE_S], *addr;
	uint32_t ip_addr;
	unsigned long int range;

	addr = &(address[0]);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	snprintf(zone->net_range, RANGE_S, "%s", cm->domain);
	snprintf(zone->net_start, RANGE_S, "%s", cm->domain);
	zone->prefix = cm->prefix;
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
	inet_pton(AF_INET, zone->net_range, &ip_addr);
	ip_addr = htonl(ip_addr);
	zone->start_ip = ip_addr;
	range = get_net_range(cm->prefix);
	zone->end_ip = ip_addr + range - 1;
	ip_addr = htonl((uint32_t)zone->end_ip);
	inet_ntop(AF_INET, &ip_addr, addr, RANGE_S);
	snprintf(zone->net_finish, RANGE_S, "%s", addr);
	snprintf(zone->hostmaster, RBUFF_S, "%s", dc->hostmaster);
}

unsigned long int
get_net_range(unsigned long int prefix)
{
	unsigned long int range;
	range = (256ul * 256ul * 256ul * 256ul) - 1;
	range = (range >> prefix) + 1;
	return range;
}

void
select_specific_ip(dnsa_t *dnsa, comm_line_t *cm)
{
	record_row_t *records, *next, *ip;

	ip = '\0';
	records = dnsa->records;
	next = records->next;
	while (records) {
		if (strncmp(records->dest, cm->dest, RANGE_S) == 0) {
			ip = records;
			records = records->next;
			if (records)
				next = records->next;
		} else {
			free(records);
			records = next;
			if (records)
				next = records->next;
		}
	}
	dnsa->records = ip;
	if (ip)
		ip->next = '\0';
}

/*
 * In this function we need 4 counters; prev, next, tmp and list. This is so we
 * can remove entries from the linked list. list tracks the head node, and tmp
 * is only used to check if the head node is deleted so we can set list
 * accordingly. Once we have a head node, prev tracks the previous entry so
 * when we delete a member, the list can be updated (i.e. prev->next is set to
 * the member after the one deleted). Next tracks the next member so if we
 * free the current node we know where the next one is. Finally if the head
 * node is changed we set *records to list, the new head memeber
 */
int
get_a_records_for_range(record_row_t **records, rev_zone_info_t *zone)
{
	record_row_t *rec, *list, *tmp, *prev, *next;
	int i = 0;
	uint32_t ip_addr;
	unsigned long int ip;
	list = *records;
	rec = prev = list;
	if (rec)
		next = rec->next;
	else
		return NONE;
	while (rec) {
		tmp = rec;
		inet_pton(AF_INET, rec->dest, &ip_addr);
		ip = (unsigned long int) htonl(ip_addr);
		if (ip < zone->start_ip || ip > zone->end_ip) {
			free (rec);
			rec = next;
			if (rec)
				next = rec->next;
			if (tmp == list)
				list = prev = rec;
			else
				prev->next = rec;
		} else {
			i++;
			if (prev != rec)
				prev = prev->next;
			rec = next;
			if (rec)
				next = rec->next;
		}
	}
	if (*records != list)
		*records = list;
	return i;
}

int
get_pref_a_for_range(preferred_a_t **prefer, rev_zone_info_t *rev)
{
	preferred_a_t *pref, *list, *tmp, *prev, *next;
	int i = 0;
	list = *prefer;
	pref = prev = list;
	if (pref)
		next = pref->next;
	else
		return NONE;
	while (pref) {
		tmp = pref;
		if (pref->ip_addr < rev->start_ip || pref->ip_addr > rev->end_ip) {
			free(pref);
			pref = next;
			if (pref)
				next = pref->next;
			if (tmp == list)
				list = prev = pref;
			else
				prev->next = pref;
		} else {
			i++;
			if (prev != pref)
				prev = prev->next;
			pref = next;
			if (pref)
				next = pref->next;
		}
	}
	if (*prefer != list)
		*prefer = list;
	return i;
}

void
init_initial_dbdata(dbdata_t **list, int type)
{
	unsigned int i = 0;
	dbdata_t *data, *dlist;
	dlist = *list = '\0';
	for (i = 0; i < extended_search_fields[type]; i++) {
		if (!(data = malloc(sizeof(dbdata_t))))
			report_error(MALLOC_FAIL, "Data in disp_multi_a");
		init_dbdata_struct(data);
		if (!(*list)) {
			*list = dlist = data;
		} else {
			while (dlist->next)
				dlist = dlist->next;
			dlist->next = data;
		}
	}
}
