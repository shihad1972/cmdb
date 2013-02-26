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

int
display_multi_a_records(dnsa_config_t *dc, comm_line_t *cm)
{
	int retval;
	dnsa_t *dnsa;
	dbdata_t *start;
	rev_zone_info_t *rzone;
	record_row_t *records;

	retval = 0;
	if (!(dnsa = malloc(sizeof(dnsa_t))))
		report_error(MALLOC_FAIL, "dnsa in display_multi_a_records");
	if (!(rzone = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rzone in display_multi_a_records");
	init_dnsa_struct(dnsa);
	init_rev_zone_struct(rzone);
	start = '\0';
	init_initial_dbdata(&start, RECORDS_ON_DEST_AND_ID);
	dnsa->rev_zones = rzone;
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
	if ((retval = run_query(dc, dnsa, DUPLICATE_A_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	get_a_records_for_range(&(dnsa->records), dnsa->rev_zones);
	records = dnsa->records;
	if (!records)
		printf("No duplicate entries for range %s\n", cm->domain);
	print_multiple_a_records(dc, start, records);
	dnsa_clean_dbdata_list(start);
	dnsa_clean_list(dnsa);
	return retval;
}

void
print_multiple_a_records(dnsa_config_t *dc, dbdata_t *start, record_row_t *records)
{
	int i, j;
	dbdata_t *dlist;
	while (records) {
		dlist = start;
		printf("Destination %s has %lu records\n",
		records->dest, records->id);
		snprintf(dlist->args.text, RANGE_S, "%s", records->dest);
		i = run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
		for (j = 0; j < i; j++) {
			printf("%s.", dlist->fields.text);
			dlist = dlist->next;
			printf("%s\n", dlist->fields.text);
			dlist = dlist->next;
			dlist = dlist->next;
		}
		printf("\n");
		records = records->next;
		dlist = start->next;
		dlist = dlist->next;
		dlist = dlist->next;
		dnsa_clean_dbdata_list(dlist);
		dlist = start->next;
		dlist = dlist->next;
		dlist->next = '\0';
	}
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
	snprintf(record->type, RBUFF_S, "%s", cm->rtype);
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
	next = rec->next;
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

void
init_initial_dbdata(dbdata_t **list, int type)
{
	unsigned int i = 0;
	dbdata_t *data, *dlist;
	dlist = *list;
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
