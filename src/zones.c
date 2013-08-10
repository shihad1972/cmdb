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
/* For freeBSD ?? */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
/* End freeBSD */
#include <arpa/inet.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "base_sql.h"
#include "dnsa_base_sql.h"
#ifdef HAVE_LIBPCRE
# include "checks.h"
#endif /* HAVE_LIBPCRE */

void
list_zones (dnsa_config_s *dc)
{
	int retval;
	dnsa_s *dnsa;
	zone_info_s *zone;
	size_t len;
	
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in list_zones");
	
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_query(dc, dnsa, ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	zone = dnsa->zones;
	printf("Listing zones from database %s on %s\n", dc->db, dc->dbtype);
	printf("Name\t\t\t\tValid\tSerial\t\tID\tType\tMaster\n");
	while (zone) {
		len = strlen(zone->name);
		if ((strncmp(zone->master, "(null)", COMM_S)) == 0)
			snprintf(zone->master, RANGE_S, "N/A");
		if (len < 8)
			printf("%s\t\t\t\t%s\t%lu\t%lu\t%s\t%s\n", 
zone->name, zone->valid, zone->serial, zone->id, zone->type, zone->master);
		else if (len < 16)
			printf("%s\t\t\t%s\t%lu\t%lu\t%s\t%s\n",
zone->name, zone->valid, zone->serial, zone->id, zone->type, zone->master);
		else if (len < 24)
			printf("%s\t\t%s\t%lu\t%lu\t%s\t%s\n",
zone->name, zone->valid, zone->serial, zone->id, zone->type, zone->master);
		else if (len < 32)
			printf("%s\t%s\t%lu\t%lu\t%s\t%s\n",
zone->name, zone->valid, zone->serial, zone->id, zone->type, zone->master);
		else
			printf("%s\n\t\t\t\t%s\t%lu\t%lu\t%s\t%s\n",
zone->name, zone->valid, zone->serial, zone->id, zone->type, zone->master);
		if (zone->next)
			zone = zone->next;
		else
			zone = '\0';
	}
	dnsa_clean_list(dnsa);
}

void
list_rev_zones(dnsa_config_s *dc)
{
	int retval;
	dnsa_s *dnsa;
	rev_zone_info_s *rev;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in list_zones");

	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_query(dc, dnsa, REV_ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	rev = dnsa->rev_zones;
	printf("Listing reverse zones from database %s on %s\n", dc->db, dc->dbtype);
	printf("Range\t\tprefix\tvalid\tType\tMaster\n");
	while (rev) {
		if ((strncmp(rev->master, "(null)", COMM_S)) == 0)
			snprintf(rev->master, RANGE_S, "N/A");
		printf("%s\t/%lu\t%s\t%s\t%s\n",
rev->net_range, rev->prefix, rev->valid, rev->type, rev->master);
		if (rev->next)
			rev = rev->next;
		else
			rev = '\0';
	}
	dnsa_clean_list(dnsa);
}

void
display_zone(char *domain, dnsa_config_s *dc)
{
	int retval;
	dnsa_s *dnsa;
	zone_info_s *zone;
	
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in display_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		if (retval == 1)
			printf("There are either no zones or records in the database\n");
		dnsa_clean_list(dnsa);
		return;
	}
	zone = dnsa->zones;
	while (zone) {
		if ((strncmp(zone->name, domain, RBUFF_S)) == 0)
			break;
		else
			zone = zone->next;
	}
	if (zone) {
		if ((strncmp(zone->type, "master", RANGE_S)) == 0)
			print_zone(dnsa, domain);
		else
			printf("This is a slave zone. No records to display\n");
	} else {
		fprintf(stderr, "Zone %s not found\n", domain);
	}
	dnsa_clean_list(dnsa);
}

void
print_zone(dnsa_s *dnsa, char *domain)
{
	unsigned int i, j;
	record_row_s *records = dnsa->records;
	zone_info_s *zone = dnsa->zones;
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
display_rev_zone(char *domain, dnsa_config_s *dc)
{
	int retval;
	dnsa_s *dnsa;
	rev_zone_info_s *rev;
	
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in display_rev_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_multiple_query(dc, dnsa, REV_ZONE | REV_RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return;
	}
	rev = dnsa->rev_zones;
	while (rev) {
		if ((strncmp(rev->net_range, domain, RBUFF_S)) == 0)
			break;
		else
			rev = rev->next;
	}
	if (rev) {
		if ((strncmp(rev->type, "master", RANGE_S)) == 0)
			print_rev_zone(dnsa, domain);
		else
			printf("This is a slave reverse zone. No records to display\n");
	} else {
		fprintf(stderr, "Reverse zone %s not found\n", domain);
	}
	dnsa_clean_list(dnsa);
}

void
print_rev_zone(dnsa_s *dnsa, char *domain)
{
	char *in_addr;
	unsigned int i, j;
	rev_record_row_s *records = dnsa->rev_records;
	rev_zone_info_s *zone = dnsa->rev_zones;
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
check_fwd_zone(char *domain, dnsa_config_s *dc)
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
check_rev_zone(char *domain, dnsa_config_s *dc)
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
commit_fwd_zones(dnsa_config_s *dc)
{
	char *configfile, *buffer, *filename;
	int retval;
	dnsa_s *dnsa;
	zone_info_s *zone;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in commit_fwd_zones");
	if (!(configfile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in commit_fwd_zones");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in commit_fwd_zones");
	filename = buffer;
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		dnsa_clean_list(dnsa);
		return MY_QUERY_FAIL;
	}
	zone = dnsa->zones;
	while (zone) {
		if ((strncmp(zone->type, "slave", COMM_S)) != 0) {
			check_for_updated_fwd_zone(dc, zone);
/*			create_and_write_fwd_zone(dnsa, dc, zone); */
			if ((retval = validate_fwd_zone(dc, zone, dnsa)) != 0) {
				free(configfile);
				free(buffer);
				dnsa_clean_list(dnsa);
				return retval;
			}
		}
		if ((retval = create_fwd_config(dc, zone, configfile)) != 0) {
			printf("Configuration Buffer Full!\n");
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
create_fwd_zone_header(dnsa_s *dnsa, char *hostm, unsigned long int id, char *zonefile)
{
	char *buffer;
	
	if (!(buffer = calloc(RBUFF_S + COMM_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_fwd_zone_header");
	zone_info_s *zone = dnsa->zones;
	record_row_s *record = dnsa->records;
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
add_records_to_fwd_zonefile(dnsa_s *dnsa, unsigned long int id, char **zonefile)
{
	char *buffer;
	size_t len, size;
	record_row_s *record = dnsa->records;
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
create_and_write_fwd_zone(dnsa_s *dnsa, dnsa_config_s *dc, zone_info_s *zone)
{
	int retval;
	char *zonefile, *buffer, *filename;
	
	if (!(zonefile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in create_and_write_fwd_zone");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_and_write_fwd_zone");
	filename = buffer;
	retval = 0;
	create_fwd_zone_header(dnsa, dc->hostmaster, zone->id, zonefile);
	add_records_to_fwd_zonefile(dnsa, zone->id, &zonefile);
	snprintf(filename, NAME_S, "%s%s",
		 dc->dir, zone->name);
	if ((retval = write_file(filename, zonefile)) != 0)
		printf("Unable to write %s zonefile\n",
		       zone->name);
/*	if ((retval = check_fwd_zone(zone->name, dc)) !=0)
		snprintf(zone->valid, COMM_S, "no"); */
	free(zonefile);
	free(buffer);
	return retval;
}

int
create_fwd_config(dnsa_config_s *dc, zone_info_s *zone, char *configfile)
{
	int retval;
	char *buffer;
	size_t len;
	
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in create_fwd_config");
	len = BUILD_S;
	retval = 0;
	if (strncmp(zone->type, "master", COMM_S) == 0) {
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
	} else if (strncmp(zone->type, "slave", COMM_S) == 0) {
		if (strncmp(zone->valid, "yes", COMM_S) == 0) {
			snprintf(buffer, TBUFF_S, "\
zone \"%s\" {\n\
\t\t\ttype slave;\n\
\t\t\tmasters { %s; };\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n\n", zone->name, zone->master, dc->dir, zone->name);
			if (strlen(configfile) + strlen(buffer) < len)
				strncat(configfile, buffer, strlen(buffer));
			else
				retval = BUFFER_FULL;
		}
	}
	free(buffer);
	return retval;
}

void
check_for_updated_fwd_zone(dnsa_config_s *dc, zone_info_s *zone)
{
	int retval;
	unsigned long int serial;
	dbdata_s serial_data, id_data;

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
		if ((retval = dnsa_run_update(dc, &serial_data, ZONE_SERIAL)) != 0)
			fprintf(stderr, "Cannot update zone serial in database!\n");
		else
			fprintf(stderr, "Serial number updated\n");
		if ((retval = dnsa_run_update(dc, &id_data, ZONE_UPDATED_NO)) != 0)
			fprintf(stderr, "Cannot set zone as not updated in database!\n");
	}
}

int
commit_rev_zones(dnsa_config_s *dc)
{
	char *configfile, *buffer, *filename;
	int retval;
	dnsa_s *dnsa;
	rev_zone_info_s *zone;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in commit_rev_zones");
	if (!(configfile = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in commit_rev_zones");
	if (!(buffer = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer in commit_rev_zones");
	filename = buffer;
	retval = 0;
	init_dnsa_struct(dnsa);
	if ((retval = dnsa_run_multiple_query(dc, dnsa, REV_ZONE | REV_RECORD)) != 0) {
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
create_and_write_rev_zone(dnsa_s *dnsa, dnsa_config_s *dc, rev_zone_info_s *zone)
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
create_rev_zone_header(dnsa_s *dnsa, char *hostm, unsigned long int id, char *zonefile)
{
	char *buffer;
	
	if (!(buffer = calloc(RBUFF_S + COMM_S, sizeof(char))))
		report_error(MALLOC_FAIL, "buffer ins create_rev_zone_header");
	rev_zone_info_s *zone = dnsa->rev_zones;
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
add_records_to_rev_zonefile(dnsa_s *dnsa, unsigned long int id, char **zonefile)
{
	char *buffer;
	size_t len, size;
	rev_record_row_s *record = dnsa->rev_records;
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
create_rev_config(dnsa_config_s *dc, rev_zone_info_s *zone, char *configfile)
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
		if ((strncmp(zone->type, "slave", COMM_S)) != 0) {
			snprintf(buffer, TBUFF_S, "\
zone \"%s\" {\n\
\t\t\ttype master;\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n\n", in_addr, dc->dir, zone->net_range);
		} else {
			snprintf(buffer, TBUFF_S, "\
zone \"%s\" {\n\
\t\t\ttype slave;\n\
\t\t\tmasters { %s; };\n\
\t\t\tfile \"%s%s\";\n\
\t\t};\n\n", in_addr, zone->master, dc->dir, zone->net_range);
		}
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
display_multi_a_records(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval;
	dnsa_s *dnsa;
	dbdata_s *start;
	rev_zone_info_s *rzone;
	record_row_s *records;
	preferred_a_s *prefer;

	retval = 0;
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in disp_multi_a");
	if (!(rzone = malloc(sizeof(rev_zone_info_s))))
		report_error(MALLOC_FAIL, "rzone in disp_multi_a");
	init_dnsa_struct(dnsa);
	init_rev_zone_struct(rzone);
	dnsa->rev_zones = rzone;
	if ((retval = dnsa_run_multiple_query(
		dc, dnsa, DUPLICATE_A_RECORD | PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	dnsa_init_initial_dbdata(&start, RECORDS_ON_DEST_AND_ID);
	if (strncmp(cm->dest, "NULL", COMM_S) != 0) {
		select_specific_ip(dnsa, cm);
		if (!(dnsa->records))
			fprintf(stderr, "No multiple A records for IP %s\n",
				cm->dest);
		else
			print_multiple_a_records(dc, start, dnsa);
		clean_dbdata_struct(start);
		dnsa_clean_list(dnsa);
		return NONE;
	}
	snprintf(rzone->net_range, RANGE_S, "%s", cm->domain);
	if ((retval = dnsa_run_search(dc, dnsa, REV_ZONE_PREFIX)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	cm->prefix = rzone->prefix;
	fill_rev_zone_info(rzone, cm, dc);
	if (rzone->prefix == NONE) {
		printf("Net range %s does not exist in database\n", cm->domain);
		dnsa_clean_list(dnsa);
		clean_dbdata_struct(start);
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
		printf("-i option\nE.G. dnsa -u -i <IP-Address>\n");
	}
	clean_dbdata_struct(start);
	dnsa_clean_list(dnsa);
	return retval;
}

void
print_multiple_a_records(dnsa_config_s *dc, dbdata_s *start, dnsa_s *dnsa)
{
	int i, j, k;
	char name[RBUFF_S], *fqdn;
	dbdata_s *dlist;
	record_row_s *records = dnsa->records;
	preferred_a_s *prefer = dnsa->prefer;
	fqdn = &name[0];
	while (records) {
		dlist = start;
		printf("Destination %s has %lu records; * denotes preferred PTR record\n",
		records->dest, records->id);
		snprintf(dlist->args.text, RANGE_S, "%s", records->dest);
		i = dnsa_run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
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
		clean_dbdata_struct(dlist);
		dlist = start->next->next;
		dlist->next = '\0';
	}
}

int
mark_preferred_a_record(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval;
	uint32_t ip_addr;
	unsigned long int ip;
	dnsa_s *dnsa;
	zone_info_s *zone;
	preferred_a_s *prefer;

	retval = 0;
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in mark_preferred_a_record");
	if (!(zone = malloc(sizeof(zone_info_s))))
		report_error(MALLOC_FAIL, "zone in mark_preferred_a_record");
	init_dnsa_struct(dnsa);
	init_zone_struct(zone);
	dnsa->zones = zone;
	if ((retval = dnsa_run_multiple_query(dc, dnsa,
		 DUPLICATE_A_RECORD | PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	select_specific_ip(dnsa, cm);
	prefer = dnsa->prefer;
	if (inet_pton(AF_INET, cm->dest, &ip_addr))
		ip = (unsigned long int)htonl(ip_addr);
	else
		return USER_INPUT_INVALID;
	while (prefer) {
		if (prefer->ip_addr == ip) {
			printf("IP %s already has preferred A record %s\n",
			       cm->dest, prefer->fqdn);
			dnsa_clean_list(dnsa);
			return NONE;
		} else {
			prefer = prefer->next;
		}
	}
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
	if ((retval = dnsa_run_insert(dc, dnsa, PREFERRED_AS)) != 0)
		fprintf(stderr, "Cannot insert preferred A record\n");
	else
		printf("Database updated with preferred A record\n");
	dnsa_clean_list(dnsa);
	return retval;
}

int
get_preferred_a_record(dnsa_config_s *dc, dnsa_comm_line_s *cm, dnsa_s *dnsa)
{
	char *name = cm->dest;
	char fqdn[RBUFF_S], cl_fqdn[RBUFF_S], *cl_name;
	int i = 0;
	uint32_t ip_addr;
	dbdata_s *start, *list;
	preferred_a_s *prefer;
	record_row_s *rec = dnsa->records;

	if (!(prefer = malloc(sizeof(preferred_a_s))))
		report_error(MALLOC_FAIL, "prefer in get_preferred_a_record");
	init_preferred_a_struct(prefer);
	dnsa->prefer = prefer;
	dnsa_init_initial_dbdata(&start, RECORDS_ON_DEST_AND_ID);
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
	i = dnsa_run_extended_search(dc, start, RECORDS_ON_DEST_AND_ID);
	list = start;
	name = fqdn;
	cl_name = cl_fqdn;
	i = 0;
	while (list) {
		snprintf(name, RBUFF_S, "%s.%s",
list->fields.text, list->next->fields.text);
		snprintf(cl_name, RBUFF_S, "%s.%s",
cm->host, cm->domain);
		if (strncmp(name, cl_name, RBUFF_S) == 0) {
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
delete_preferred_a(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	char *ip_addr = cm->dest;
	int retval = 0;
	int i = 0;
	dnsa_s *dnsa;
	dbdata_s data;
	preferred_a_s *prefer;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in delete_preferred_a");
	init_dnsa_struct(dnsa);
	init_dbdata_struct(&data);
	if ((retval = dnsa_run_query(dc, dnsa, PREFERRED_A)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	prefer = dnsa->prefer;
	while (prefer) {
		if (strncmp(ip_addr, prefer->ip, RANGE_S) == 0) {
			printf("Found IP address %s in preferred list\n",
			       ip_addr);
			i++;
			break;
		}
		prefer = prefer->next;
	}
	if (i == 0) {
		fprintf(stderr, "IP %s does not have a preferred A record\n",
			ip_addr);
		dnsa_clean_list(dnsa);
		return USER_INPUT_INVALID;
	}
	data.args.number = prefer->prefa_id;
	if ((retval = dnsa_run_delete(dc, &data, PREFERRED_AS)) != 1)
		printf("Unable to delete IP %s from preferred list\n", ip_addr);
	else
		printf("Deleted IP %s from preferred list\n", ip_addr);
	dnsa_clean_list(dnsa);
	return retval;
}

int
add_host(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval;
	dnsa_s *dnsa;
	zone_info_s *zone;
	record_row_s *record;
	dbdata_s data;
	
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in add_host");
	if (!(zone = malloc(sizeof(zone_info_s))))
		report_error(MALLOC_FAIL, "zone in add_host");
	if (!(record = malloc(sizeof(record_row_s))))
		report_error(MALLOC_FAIL, "record in add_host");

	init_dnsa_struct(dnsa);
	init_dbdata_struct(&data);
	dnsa->zones = zone;
	dnsa->records = record;
	snprintf(zone->name, RBUFF_S, "%s", cm->domain);
	if (strncmp(cm->rtype, "CNAME", COMM_S) == 0)
		add_trailing_dot(cm->dest);
	retval = 0;
	retval = dnsa_run_search(dc, dnsa, ZONE_ID_ON_NAME);
	printf("Adding to zone %s, id %lu\n", zone->name, zone->id);
	snprintf(record->dest, RBUFF_S, "%s", cm->dest);
	snprintf(record->host, RBUFF_S, "%s", cm->host);
	snprintf(record->type, RANGE_S, "%s", cm->rtype);
	record->zone = data.args.number = zone->id;
	record->pri = cm->prefix;
	if ((retval = dnsa_run_insert(dc, dnsa, RECORDS)) != 0)
		fprintf(stderr, "Cannot insert record\n");
	retval = dnsa_run_update(dc, &data, ZONE_UPDATED_YES);
	dnsa_clean_list(dnsa);
	return retval;
}

int
delete_record(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	char fqdn[RBUFF_S], *name;
	int retval = 0;
	dnsa_s *dnsa;
	dbdata_s data;
	zone_info_s *zone;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in delete_record");
	init_dnsa_struct(dnsa);
	init_dbdata_struct(&data);
	if ((retval = dnsa_run_multiple_query(dc, dnsa, RECORD | ZONE)) != 0) {
		printf("DB search failed with %d\n", retval);
		dnsa_clean_list(dnsa);
		return NO_RECORDS;
	}
	name = fqdn;
	snprintf(name, RBUFF_S, "%s.%s.", cm->host, cm->domain);
	zone = dnsa->zones;
	while (zone) {
		if ((strncmp(zone->name, cm->domain, RBUFF_S)) == 0)
			break;
		else
			zone = zone->next;
	}
	if (zone)
		data.args.number = zone->id;
	if ((retval = check_for_fwd_record_use(dnsa, name)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = get_record_id_and_delete(dc, dnsa, cm)) == 0) {
		dnsa_clean_list(dnsa);
		printf("Cannot find a record ID for %s.%s\n",
		       cm->host, cm->domain);
		return CANNOT_FIND_RECORD_ID;
	}
	printf("%d record(s) deleted\n", retval);
	retval = dnsa_run_update(dc, &data, ZONE_UPDATED_YES);
	dnsa_clean_list(dnsa);
	return NONE;
}

int
add_fwd_zone(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval;
	dnsa_s *dnsa;
	zone_info_s *zone;
	dbdata_s data;
	
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in add_fwd_zone");
	if (!(zone = malloc(sizeof(zone_info_s))))
		report_error(MALLOC_FAIL, "zone in add_fwd_zone");
	retval = 0;
	init_dnsa_struct(dnsa);
	init_zone_struct(zone);
	if ((strncmp(cm->ztype, "NULL", COMM_S)) == 0)
		snprintf(cm->ztype, RANGE_S, "master");
	fill_fwd_zone_info(zone, cm, dc);
	dnsa->zones = zone;
	if ((retval = check_for_zone_in_db(dc, dnsa, FORWARD_ZONE)) != 0) {
		printf("Zone %s already exists in database\n", zone->name);
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = dnsa_run_insert(dc, dnsa, ZONES)) != 0) {
		fprintf(stderr, "Unable to add zone %s\n", zone->name);
		dnsa_clean_list(dnsa);
		return CANNOT_INSERT_ZONE;
	} else {
		fprintf(stderr, "Added zone %s\n", zone->name);
	}
	if ((strncmp(zone->type, "slave", COMM_S)) != 0) {
		if ((retval = validate_fwd_zone(dc, zone, dnsa)) != 0) {
			dnsa_clean_list(dnsa);
			return retval;
		}
	} else {
		if ((retval = dnsa_run_search(dc, dnsa, ZONE_ID_ON_NAME)) != 0) {
		printf("Unable to get ID of zone %s\n", zone->name);
		return ID_INVALID;
		}
	}
	init_dbdata_struct(&data);
	data.args.number = zone->id;
	if ((retval = dnsa_run_update(dc, &data, ZONE_VALID_YES)) != 0)
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
delete_fwd_zone(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	char fqdn[RBUFF_S], *name;
	int retval, i = 0;
	dnsa_s *dnsa;
	dbdata_s *data;
	record_row_s *fwd, *other, *list;
	zone_info_s *zone;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in delete_fwd_zone");
	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in delete_fwd_zone");
	init_dnsa_struct(dnsa);
	init_dbdata_struct(data);
	if ((retval = dnsa_run_multiple_query(dc, dnsa, ZONE | RECORD)) != 0) {
		printf("Database query for zones and records failed\n");
		dnsa_clean_list(dnsa);
		return retval;
	}
	zone = dnsa->zones;
	fwd = other = '\0';
	name = fqdn;
	while (zone) {
		if (strncmp(cm->domain, zone->name, RBUFF_S) == 0)
			break;
		else
			zone = zone->next;
	}
	if (!dnsa->records) {
		printf("There are no forward records for this domain\n");
	} else if (!zone) {
		printf("The domain %s does not exist\n", cm->domain);
		return NO_DOMAIN;
	} else {
		split_fwd_record_list(zone, dnsa->records, &fwd, &other);
		dnsa->records = other;
		list = fwd;
		while (list) {
			snprintf(name, RBUFF_S, "%s.%s.", list->host, cm->domain);
			if ((retval = check_for_fwd_record_use(dnsa, name)) != 0)
				i++;
			list = list->next;
		}
		if (i != 0) {
			printf("\n\
You seem to have some forward records for the domain %s\n\
that are used elsewhere\n\
These records have been displayed. \n\
Please delete them and then try to delete the zone again.\n", cm->domain);
			return retval;
		}
	}
	data->args.number = zone->id;
	if ((retval = dnsa_run_delete(dc, data, RECORDS_ON_FWD_ZONE)) == 0) {
		printf("zone %s was empty\n", cm->domain);;
	} else {
		printf("%d Records for zone %s deleted\n", retval, cm->domain);
	}
	if ((retval = dnsa_run_delete(dc, data, ZONES)) == 0) {
		retval = CANNOT_DELETE_ZONE;
		printf("Unable to delete forward zone %s in database\n", cm->domain);
	} else if (retval == 1) {
		printf("Zone %s deleted\n", cm->domain);
		retval = NONE;
	} else {
		printf("More than one zone deleted??\n");
		retval = MULTIPLE_ZONE_DELETED;
	}
	dnsa_clean_list(dnsa);
	clean_dbdata_struct(data);
	return retval;
}

int
add_rev_zone(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval;
	dnsa_s *dnsa;
	rev_zone_info_s *zone;
	dbdata_s data;
	
	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in add_fwd_zone");
	if (!(zone = malloc(sizeof(rev_zone_info_s))))
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
	if ((retval = dnsa_run_insert(dc, dnsa, REV_ZONES)) != 0) {
		fprintf(stderr, "Unable to add zone %s\n", zone->net_range);
		dnsa_clean_list(dnsa);
		return CANNOT_INSERT_ZONE;
	} else {
		fprintf(stderr, "Added zone %s\n", zone->net_range);
	}
	if ((strncmp(zone->type, "slave", COMM_S)) != 0) {
		if ((retval = validate_rev_zone(dc, zone, dnsa)) != 0) {
			dnsa_clean_list(dnsa);
			return retval;
		}
	} else {
		if ((retval = dnsa_run_search(dc, dnsa, REV_ZONE_ID_ON_NET_RANGE)) != 0) {
		printf("Unable to get ID of zone %s\n", zone->net_range);
		return ID_INVALID;
		}
	}
	init_dbdata_struct(&data);
	data.args.number = zone->rev_zone_id;
	if ((retval = dnsa_run_update(dc, &data, REV_ZONE_VALID_YES)) != 0)
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
delete_reverse_zone(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval;
	dnsa_s *dnsa;
	dbdata_s *data;
	rev_zone_info_s *rev;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in delete_reverse_zone");
	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in delete_reverse_zone");
	init_dnsa_struct(dnsa);
	init_dbdata_struct(data);
	if ((retval = dnsa_run_query(dc, dnsa, REV_ZONE)) != 0) {
		printf("Query to get reverse zones from DB failed\n");
		dnsa_clean_list(dnsa);
		return NO_DOMAIN;
	}
	rev = dnsa->rev_zones;
	while (rev) {
		if (strncmp(cm->domain, rev->net_range, RANGE_S) == 0) {
			data->args.number = rev->rev_zone_id;
		}
		rev = rev->next;
	}
	printf("Deleting record from reverse zone %s\n", cm->domain);
	retval = dnsa_run_delete(dc, data, REV_RECORDS_ON_REV_ZONE);
	printf("%d records deleted\n\n", retval);
	printf("Deleting reverse zone %s\n", cm->domain);
	retval = dnsa_run_delete(dc, data, REV_ZONES);
	printf("%d zone(s) deleted\n", retval);

	return NONE;
}

int
create_and_write_fwd_config(dnsa_config_s *dc, dnsa_s *dnsa)
{
	char *configfile, *buffer, filename[NAME_S];
	int retval;
	zone_info_s *zone;

	buffer = &filename[0];
	retval = 0;
	if ((retval = dnsa_run_query(dc, dnsa, ZONE)) != 0)
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
create_and_write_rev_config(dnsa_config_s *dc, dnsa_s *dnsa)
{
	char *configfile, *buffer, filename[NAME_S];
	int retval;
	rev_zone_info_s *zone;
	
	buffer = &filename[0];
	retval = 0;
	if ((retval = dnsa_run_query(dc, dnsa, REV_ZONE)) != 0)
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
validate_fwd_zone(dnsa_config_s *dc, zone_info_s *zone, dnsa_s *dnsa)
{
	char command[NAME_S], *buffer;
	int retval;
	dbdata_s *data;

	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in validate_fwd_zone");
	retval = 0;
	buffer = &command[0];
	snprintf(zone->valid, COMM_S, "yes");
	if ((retval = add_trailing_dot(zone->pri_dns)) != 0)
		fprintf(stderr, "Unable to add trailing dot to PRI_NS\n");
	if (strncmp(zone->sec_dns, "(null)", COMM_S) != 0)
		if ((retval = add_trailing_dot(zone->sec_dns)) != 0)
			fprintf(stderr, "Unable to add trailing dot to SEC_NS\n");
	if ((retval = dnsa_run_search(dc, dnsa, ZONE_ID_ON_NAME)) != 0) {
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
		data->args.number = zone->id;
		if ((retval = dnsa_run_update(dc, data, ZONE_VALID_NO)) != 0)
			fprintf(stderr, "Set zone not valid in DB failed\n");
		free(data);
		return CHKZONE_FAIL;
	} else {
		data->args.number = zone->id;
		if ((retval = dnsa_run_update(dc, data, ZONE_VALID_YES)) != 0) {
			free(data);
			fprintf(stderr, "Set zone valid in DB failed\n");
			return retval;
		}
		free(data);
	}
	return retval;
}

int
validate_rev_zone(dnsa_config_s *dc, rev_zone_info_s *zone, dnsa_s *dnsa)
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
	if ((retval = dnsa_run_search(dc, dnsa, REV_ZONE_ID_ON_NET_RANGE)) != 0) {
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
check_for_fwd_record_use(dnsa_s *dnsa, char *name)
{
	int retval;
/**
 * If the record we want to delete is marked as an NS record for any zone
 * then refuse to delete.
 */
	if ((retval = compare_fwd_ns_records_with_host(dnsa, name)) != 0) {
		return retval;
	}
/**
 * If the record is a destination for any other record then refuse to delete
 */
	if ((retval = compare_host_with_record_destination(dnsa, name)) != 0) {
		return retval;
	}
/**
 * CNAMES that the destination is not a Fully Qualified Domain Name will have
 * been missed in the above search. So we need to search for them. If they
 * exist, refuse to delete
 */
	if ((retval = compare_host_with_fqdn_cname(dnsa, name)) != 0) {
		return retval;
	}
	return NONE;
}

int
build_reverse_zone(dnsa_config_s *dc, dnsa_comm_line_s *cm)
{
	int retval, a_rec;
	unsigned long int serial;
	dnsa_s *dnsa;
	dbdata_s serial_d, zone_info_d, *data;
	record_row_s *rec;
	rev_record_row_s *add, *delete, *list;

	if (!(dnsa = malloc(sizeof(dnsa_s))))
		report_error(MALLOC_FAIL, "dnsa in build_reverse_zone");
	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in build_reverse_zone");
	retval = 0;
/* Set to NULL so we can check if there are no records to add / delete */
	add = delete = '\0';
	init_dnsa_struct(dnsa);
	init_dbdata_struct(data);
	if ((retval = dnsa_run_multiple_query(
	       dc, dnsa, DUPLICATE_A_RECORD | PREFERRED_A | REV_ZONE | ZONE)) != 0) {
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((retval = get_correct_rev_zone_and_preferred_records(dnsa, cm)) > 0) {
		dnsa_clean_list(dnsa);
		return retval;
	} else if (retval < 0) {
/* No Duplicate records. Just convert all A records */
		dnsa_clean_records(dnsa->records);
		dnsa_clean_prefer(dnsa->prefer);
		dnsa->records = '\0';
		dnsa->prefer = '\0';
	}
	rec = dnsa->records; /* Holds duplicate A records */
	dnsa->records = '\0';
	if ((retval = dnsa_run_multiple_query(dc, dnsa, ALL_A_RECORD | REV_RECORD)) != 0) {
		dnsa_clean_records(rec);
		dnsa_clean_list(dnsa);
		return retval;
	}
	if ((a_rec = get_a_records_for_range(&(dnsa->records), dnsa->rev_zones)) == 0) {
		dnsa_clean_records(rec);
		dnsa_clean_list(dnsa);
		printf("No A records for net_range %s\n", cm->domain);
		return NO_RECORDS;
	}
	get_rev_records_for_range(&(dnsa->rev_records), dnsa->rev_zones);
	add_int_ip_to_records(dnsa);
	trim_forward_record_list(dnsa, rec);
	retval = rev_records_to_add(dnsa, &add);
	retval = rev_records_to_delete(dnsa, &delete);
	dnsa_clean_rev_records (dnsa->rev_records);
	list = delete;
	while (list) {
		data->args.number = list->record_id;
		printf("Deleting record %lu\n", data->args.number);
		dnsa_run_delete(dc, data, REV_RECORDS);
		list = list->next;
	}
	dnsa->rev_records = list = add;
	retval = 0;
	while (dnsa->rev_records) {
		printf("Adding PTR record for %s\n", dnsa->rev_records->host);
		retval += dnsa_run_insert(dc, dnsa, REV_RECORDS);
		dnsa->rev_records = dnsa->rev_records->next;
	}
	if (add || delete) {
		serial = get_zone_serial();
		if (serial > dnsa->rev_zones->serial)
			dnsa->rev_zones->serial = serial;
		else
			dnsa->rev_zones->serial++;
		init_dbdata_struct(&serial_d);
		init_dbdata_struct(&zone_info_d);
		serial_d.args.number = dnsa->rev_zones->serial;
		zone_info_d.args.number = dnsa->rev_zones->rev_zone_id;
		serial_d.next = &zone_info_d;
		if ((retval = dnsa_run_update(dc, &serial_d, REV_ZONE_SERIAL)) != 0)
			fprintf(stderr, "Cannot update rev zone serial!\n");
		else
			fprintf(stderr, "Rev zone serial number updated\n");
	} else {
		printf("No rev records to add / delete for range %s\n",
		       dnsa->rev_zones->net_range);
	}
	dnsa->rev_records = add;
	clean_dbdata_struct(data);
	dnsa_clean_rev_records(delete);
	dnsa_clean_records(rec);
	dnsa_clean_list(dnsa);
	return NONE;
}

int
get_correct_rev_zone_and_preferred_records(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	int retval = 0;
	if ((retval = get_rev_zone(dnsa, cm)) != 0) {
		return retval;
	}
	if ((retval = get_a_records_for_range(&(dnsa->records), dnsa->rev_zones)) == 0) {
		return -1;
	} else {
		retval = NONE;
		if ((retval = get_pref_a_for_range(&(dnsa->prefer), dnsa->rev_zones)) == 0) {
			return retval;
		} else {
			retval = NONE;
		}
	}
	return retval;
}

int
get_fwd_zone(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	zone_info_s *fwd, *list, *next;
	fwd = dnsa->zones;
	list = '\0';
	if (fwd->next)
		next = fwd->next;
	else
		next = '\0';
	while (fwd) {
		if (strncmp(fwd->name, cm->domain, RBUFF_S) == 0) {
			list = fwd;
			fwd = next;
		} else {
			free (fwd);
			fwd = next;
		}
		if (next)
			next = fwd->next;
	}
	dnsa->zones = list;
	if (list) {
		list->next = '\0';
		return NONE;
	}
	fprintf(stderr, "Forward domain %s not found\n", cm->domain);
	return NO_DOMAIN;
}

int
get_rev_zone(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	rev_zone_info_s *rev, *list, *next;
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
get_rev_records_for_range(rev_record_row_s **records, rev_zone_info_s *zone)
{
	int i = 0;
	rev_record_row_s *list, *prev, *next, *tmp, *rec;
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

void
trim_forward_record_list(dnsa_s *dnsa, record_row_s *rec)
{
	char host[RBUFF_S], *newhost;
	int retval;
	record_row_s *fwd = dnsa->records; /* List of A records */
	record_row_s *list, *tmp, *fwd_list, *prev;
	fwd_list = prev = fwd;
	while (fwd) {
/* Get rid of @. in the start of these types of A records */
		newhost = host;
		snprintf(newhost, RBUFF_S, "%s", fwd->host);
		if (host[0] == '@') {
			newhost = strchr(host, '.');
			newhost++;
			snprintf(fwd->host, RBUFF_S, "%s", newhost);
		}
		list = rec; /* List of duplicate IP address in the net range */
		tmp = fwd->next;
/* Check if in duplicate and prefer list */
		if ((retval = check_dup_and_pref_list(list, fwd, dnsa)) > 0) {
			if (fwd_list == fwd) {
				fwd_list = prev = tmp;
				free(fwd);
			} else {
				free(fwd);
				prev->next = tmp;
			}
/* As we are deleting this record, then no more checks should be performed.
 * Therefore, we should continue the loop */
			if ((prev->next != fwd) && (prev->next != tmp) && (prev != tmp))
				prev = prev->next;
			fwd = tmp;
			continue;
		}
		if (prev != fwd)
			prev = prev->next;
		fwd = fwd->next;
	}
	if (fwd_list != dnsa->records)
		dnsa->records = fwd_list;
}

/* Build a list of the reverse records we need to add to the database. */
int
rev_records_to_add(dnsa_s *dnsa, rev_record_row_s **rev)
{
	int i;
	size_t len;
	record_row_s *fwd = dnsa->records; /* List of A records */
	rev_record_row_s *rev_list;

	while (fwd) {
/* Now check if the forward record is in the reverse list */
		i = 0;
		rev_list = dnsa->rev_records;
		while (rev_list) {
			if (rev_list->ip_addr == fwd->ip_addr) {
				len = strlen(fwd->host);
				if (strncmp(fwd->host, rev_list->dest, len) == 0)
					i++;
				break;
			} else {
				rev_list = rev_list->next;
			}
		}
		if (i == 0)
			if ((insert_into_rev_add_list(dnsa, fwd, rev)) == 0)
				return 1;
		fwd = fwd->next;
	}
	return NONE;
}

int
check_dup_and_pref_list(record_row_s *list, record_row_s *fwd, dnsa_s *dnsa)
{
	preferred_a_s *prefer;
	while (list) {
		prefer = dnsa->prefer;
/* Check if this is IP is in the duplicate list */
		if (strncmp(list->dest, fwd->dest, RANGE_S) == 0) {
/* Yes is is, so check if this has a preferred A record */
			while (prefer) {
/* Move preferred list to correct IP address for forward A record */
				if (fwd->ip_addr != prefer->ip_addr) {
					prefer = prefer->next;
					continue;
				}
/* If this is the correct record, return none and we will keep it.
 * If this is not the correct record, return 1 and we will delete it from
 * the forward list
 */
				if (fwd->id != prefer->record_id) {
					return 1;
				} else {
					return NONE;
				}
			}
/* We did not find a preferred record for this duplicate IP address so add
 * it to the preferred list.
 */
			add_dup_to_prefer_list(dnsa, fwd);
		}
		list = list->next;
	}
/* Not in the duplicate list, so return NONE and use this A record */
	return NONE;
}

int
insert_into_rev_add_list(dnsa_s *dnsa, record_row_s *fwd, rev_record_row_s **rev)
{
	char rev_host[RANGE_S];
	rev_record_row_s *new, *list;
	zone_info_s *zones = dnsa->zones;

	if (!(new = malloc(sizeof(rev_record_row_s))))
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
add_dup_to_prefer_list(dnsa_s *dnsa, record_row_s *fwd)
{
	preferred_a_s *new, *list;

	if (!(new = malloc(sizeof(preferred_a_s))))
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
	fprintf(stderr, "\
***\nDuplicate IP %s does not have a preferred A record\n", fwd->dest);
	fprintf(stderr, "\
Using %s for this PTR.\n\
If this is not what you want, please set up a preferred record for this PTR\n",
		fwd->host);
	fprintf(stderr, "***\n");
}

/* Build list of the reverse records we need to delete from the database */
int
rev_records_to_delete(dnsa_s *dnsa, rev_record_row_s **rev)
{
	int i = 0;
	size_t len;
	rev_record_row_s *list = dnsa->rev_records;
	record_row_s *fwd;
/* Check for new preferred A records */
	while (list) {
		fwd = dnsa->records;
		while (fwd) {
			if (list->ip_addr == fwd->ip_addr) {
				len = strlen(fwd->host);
				if (strncmp(fwd->host, list->dest, len) != 0) {
					insert_into_rev_del_list(list, rev);
					i++;
				}
			}
			fwd = fwd->next;
		}
		list = list->next;
	}
/* Check for deleted A records */
	list = dnsa->rev_records;
	while (list) {
		fwd = dnsa->records;
		while (fwd) {
			if (list->ip_addr == fwd->ip_addr)
				break;
			else
				fwd = fwd->next;
		}
		if (!fwd) {
			insert_into_rev_del_list(list, rev);
			i++;
		}
		list = list->next;
	}
	return i;
}

void
insert_into_rev_del_list(rev_record_row_s *record, rev_record_row_s **rev)
{
	rev_record_row_s *new, *list;

	if (!(new = malloc(sizeof(rev_record_row_s))))
		report_error(MALLOC_FAIL, "new in insert_into_rev_add_list");
	list = *rev;
	init_rev_record_struct(new);
	new->record_id = record->record_id;
	printf("Deleting record id %lu; %s\n", new->record_id, record->dest);
	if (!list) {
		*rev = new;
	} else {
		while (list->next)
			list = list->next;
		list->next = new;
	}
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
add_int_ip_to_records(dnsa_s *dnsa)
{
	if (dnsa->records)
		add_int_ip_to_fwd_records(dnsa->records);
	if (dnsa->rev_records)
		add_int_ip_to_rev_records(dnsa);
}

void
add_int_ip_to_fwd_records(record_row_s *records)
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
add_int_ip_to_rev_records(dnsa_s *dnsa)
{
	char address[RANGE_S], host[RANGE_S], *ip_addr, *tmp;
	int i;
	uint32_t ip;
	unsigned long int prefix = dnsa->rev_zones->prefix;
	rev_record_row_s *rev = dnsa->rev_records;
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

int
compare_fwd_ns_records_with_host(dnsa_s *dnsa, char *name)
{
	int retval = NONE;
	zone_info_s *list;

	if (dnsa->zones)
		list = dnsa->zones;
	else
		return DOMAIN_LIST_FAIL;
	while (list) {
		if (strncmp(name, list->pri_dns, RBUFF_S) == 0) {
			printf("\
Zone %s has primary NS server of %s You want to delete %s\n",
			  list->name, list->pri_dns, name);
			retval = REFUSE_TO_DELETE_NS_RECORD;
		}
		if (strncmp(name, list->sec_dns, RBUFF_S) == 0) {
			printf("\
Zone %s has secondary NS server of %s You want to delete %s\n",
			  list->name, list->sec_dns, name);
			retval = REFUSE_TO_DELETE_NS_RECORD;
		}
		list = list->next;
	}
	return retval;
}

int
compare_host_with_record_destination(dnsa_s *dnsa, char *name)
{
	record_row_s *list;

	if (dnsa->records)
		list = dnsa->records;
	else
		return NO_RECORDS;
	while (list) {
		if (strncmp(name, list->dest, RBUFF_S) == 0) {
			printf("\
We have a record type %s in zone ID %lu with destination %s\n",
			       list->type, list->zone, name);
			return REFUSE_TO_DELETE_A_RECORD_DEST;
		}
		list = list->next;
	}
	return NONE;
}

int
compare_host_with_fqdn_cname(dnsa_s *dnsa, char *hname)
{
	char rfqdn[RBUFF_S], *rname;
	record_row_s *list;

	rname = rfqdn;
	if (dnsa->records)
		list = dnsa->records;
	else
		return NO_RECORDS;
	while (list) {
		get_fqdn_for_record_dest(dnsa, list, rname);
		if (strncmp(rfqdn, hname, RBUFF_S) == 0) {
			printf("\
We have a record in zone id %lu whose destination FQDN is %s\n", list->zone, rname);
			return REFUSE_TO_DELETE_A_RECORD_DEST;
		}
		list = list->next;
	}
	return NONE;
}

void
get_fqdn_for_record_dest(dnsa_s *dnsa, record_row_s *fwd, char *fqdn)
{
	char *tmp;
	int i;
	zone_info_s *zone;

	tmp = fqdn;
	for (i = 0; i < RBUFF_S; i++) {
		*tmp = '\0';
		tmp++;
	}
	if (dnsa->zones)
		zone = dnsa->zones;
	else
		return;
	while (zone) {
		if (zone->id == fwd->zone) {
			snprintf(fqdn, RBUFF_S, "%s.%s.", fwd->dest, zone->name);
			return;
		}
		zone = zone->next;
	}
}

void
get_fqdn_for_record_host(dnsa_s *dnsa, record_row_s *fwd, char *fqdn)
{
	char *tmp;
	int i;
	zone_info_s *zone;

	tmp = fqdn;
	for (i = 0; i < RBUFF_S; i++) {
		*tmp = '\0';
		tmp++;
	}
	if (dnsa->zones)
		zone = dnsa->zones;
	else
		return;
	while (zone) {
		if (zone->id == fwd->zone) {
			snprintf(fqdn, RBUFF_S, "%s.%s.", fwd->host, zone->name);
			return;
		}
		zone = zone->next;
	}
}

void
split_fwd_record_list(zone_info_s *zone, record_row_s *rec, record_row_s **fwd, record_row_s **other)
{
	record_row_s *list, *next;

	while (rec) {
		next = rec->next;
		if (rec->zone == zone->id) {
			if (!(*fwd)) {
				*fwd = rec;
				rec->next = '\0';
			} else {
				list = *fwd;
				while (list->next)
					list = list->next;
				list->next = rec;
				rec->next = '\0';
			}
		} else {
			if (!(*other)) {
				*other = rec;
				rec->next = '\0';
			} else {
				list = *other;
				while (list->next)
					list = list->next;
				list->next = rec;
				rec->next = '\0';
			}
		}
		rec = next;
	}
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
check_for_zone_in_db(dnsa_config_s *dc, dnsa_s *dnsa, short int type)
{
	int retval;

	retval = 0;
	if (type == FORWARD_ZONE) {
		if ((retval = dnsa_run_search(dc, dnsa, ZONE_ID_ON_NAME)) != 0)
			return retval;
		else if (dnsa->zones->id != 0)
			return ZONE_ALREADY_EXISTS;
	} else if (type == REVERSE_ZONE) {
		if ((retval = dnsa_run_search(dc, dnsa, REV_ZONE_ID_ON_NET_RANGE)) !=0)
			return retval;
		else if (dnsa->rev_zones->rev_zone_id != 0)
			return ZONE_ALREADY_EXISTS;
	}
	return retval;
}

void
fill_fwd_zone_info(zone_info_s *zone, dnsa_comm_line_s *cm, dnsa_config_s *dc)
{
	memset(zone, 0, sizeof(zone));
	snprintf(zone->name, RBUFF_S, "%s", cm->domain);
	snprintf(zone->pri_dns, RBUFF_S, "%s", dc->prins);
	snprintf(zone->sec_dns, RBUFF_S, "%s", dc->secns);
	snprintf(zone->type, RANGE_S, "%s", cm->ztype);
	snprintf(zone->master, RBUFF_S, "%s", cm->master);
	zone->serial = get_zone_serial();
	zone->refresh = dc->refresh;
	zone->retry = dc->retry;
	zone->expire = dc->expire;
	zone->ttl = dc->ttl;
}

void
fill_rev_zone_info(rev_zone_info_s *zone, dnsa_comm_line_s *cm, dnsa_config_s *dc)
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
	snprintf(zone->master, RBUFF_S, "%s", cm->master);
	if ((strncmp(cm->ztype, "slave", COMM_S)) == 0)
		snprintf(zone->type, RANGE_S, "%s", cm->ztype);
	else
		snprintf(zone->type, COMM_S, "master");
}

unsigned long int
get_net_range(unsigned long int prefix)
{
	unsigned long int range;
	range = (256ul * 256ul * 256ul * 256ul) - 1;
	range = (range >> prefix) + 1;
	return range;
}

int
get_record_id_and_delete(dnsa_config_s *dc, dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	char hfqdn[RBUFF_S], rfqdn[RBUFF_S], *hname, *rname;
	int retval = 0;
	dbdata_s *data;
	record_row_s *list;

	if (!(data = malloc(sizeof(dbdata_s))))
		report_error(MALLOC_FAIL, "data in get_record_id_and_delete");
	init_dbdata_struct(data);
	hname = hfqdn;
	rname = rfqdn;
	snprintf(hname, RBUFF_S, "%s.%s.", cm->host, cm->domain);
	if (dnsa->records)
		list = dnsa->records;
	else
		return NO_RECORDS;
	while (list) {
		get_fqdn_for_record_host(dnsa, list, rname);
		if (strncmp(rfqdn, hfqdn, RBUFF_S) == 0) {
			printf("Deleting record ID %lu, type %s\n",
			       list->id, list->type);
			data->args.number = list->id;
			retval += dnsa_run_delete(dc, data, RECORDS);
		}
		list = list->next;
	}
	return retval;
}

void
select_specific_ip(dnsa_s *dnsa, dnsa_comm_line_s *cm)
{
	record_row_s *records, *next, *ip;

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
get_a_records_for_range(record_row_s **records, rev_zone_info_s *zone)
{
	record_row_s *rec, *list, *tmp, *prev, *next;
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
get_pref_a_for_range(preferred_a_s **prefer, rev_zone_info_s *rev)
{
	preferred_a_s *pref, *list, *tmp, *prev, *next;
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
