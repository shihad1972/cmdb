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
 *  fwd_zone:
 * 
 *  Contains various funtions that are needed to import / export
 *  forward zone information from the database, and to also write
 *  the zone files and BIND configuration files.
 * 
 *  Part of the DNSA program
 * 
 *  (C) Iain M Conochie 2012 - 2013
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "forward.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"


/** Function to fill a struct with results from the DB query
 ** No error checking on fields
 */
zone_info_t fill_zone_data(MYSQL_ROW my_row)
{
	zone_info_t my_zone;
	my_zone.id = atoi(my_row[0]);
	strncpy(my_zone.name, my_row[1], RBUFF_S);
	strncpy(my_zone.pri_dns, my_row[2], RBUFF_S);
	strncpy(my_zone.sec_dns, my_row[3] ? my_row[3] : "NULL", RBUFF_S);
	my_zone.serial = strtoul(my_row[4], NULL, 10);
	my_zone.refresh = strtoul(my_row[5], NULL, 10);
	my_zone.retry = strtoul(my_row[6], NULL, 10);
	my_zone.expire = strtoul(my_row[7], NULL, 10);
	my_zone.ttl = strtoul(my_row[8], NULL, 10);
	strncpy(my_zone.valid, my_row[9], RBUFF_S);
	my_zone.owner = atoi(my_row[10]);
	strncpy(my_zone.updated, my_row[11], RBUFF_S);
	return my_zone;
}

/* Write out zone file header */
void create_zone_header(char *zout, zone_info_t *zi, size_t len)
{
	char *tmp;
	size_t offset;
	offset = 0;
	if (!(tmp = calloc(BUFF_S,  sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in create_zone_header");
	if (strlen(zi->name) < 16) {
		snprintf(tmp, BUFF_S, "\
$ORIGIN .\n$TTL %ld\n\
%s\t\tIN SOA\t", zi->ttl, zi->name);
		offset = strlen(tmp);
		strncat(zout, tmp , offset);
	} else {
		snprintf(tmp, BUFF_S, "\
$ORIGIN .\n$TTL %ld\n\
%s\tIN SOA\t", zi->ttl, zi->name);
		offset = strlen(tmp);
		strncat(zout, tmp , offset);
	}
	snprintf(tmp, BUFF_S, "\
%s. hostmaster.%s. (\n\
\t\t\t%ld\t; Serial\n\
\t\t\t%ld\t\t; Refresh\n\
\t\t\t%ld\t\t; Retry\n",
		 zi->pri_dns, zi->name,
		 zi->serial, zi->refresh, zi->retry);
	offset = strlen(tmp);
	if ((offset + strlen(zout)) < len)
		strncat(zout, tmp, offset);
	snprintf(tmp, BUFF_S, "\
\t\t\t%ld\t\t; Expire\n\
\t\t\t%ld)\t\t; Negative Cache TTL\n\
\t\t\tNS\t\t%s.\n",
		 zi->expire, zi->ttl, zi->pri_dns);
	offset = strlen(tmp);
	if ((offset + strlen(zout)) < len)
		strncat(zout, tmp, offset);
	if (!(strcmp(zi->sec_dns, "NULL")) == 0) {
		snprintf(tmp, BUFF_S, "\t\t\tNS\t\t%s.\n", zi->sec_dns);
		offset = strlen(tmp);
		strncat(zout, tmp, offset);
	}
	free(tmp);
}
/* add MX records to zone file header */
void add_mx_to_header(char *output,  MYSQL_ROW results)
{
	size_t count;
	char *tmp;
	if (!(tmp = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in add_mx_to_header");
	snprintf(tmp, BUFF_S, "\
\t\t\t%s\t%s\t%s\n", results[3], results[4], results[5]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	free(tmp);
}

/* Hopefully soon to be obselete */
record_row_t fill_record_data(MYSQL_ROW my_row)
{
	record_row_t records;
	records.id = atoi(my_row[0]);
	records.zone = atoi(my_row[1]);
	strncpy(records.host, my_row[2], RBUFF_S);
	strncpy(records.type, my_row[3], RBUFF_S);
	records.pri = atoi(my_row[4]);
	strncpy(records.dest, my_row[5], RBUFF_S);
	strncpy(records.valid, my_row[6], RBUFF_S);
	return records;
}

void init_dnsa_zone(zone_info_t *zone)
{
	snprintf(zone->name, COMM_S, "NULL");
	snprintf(zone->pri_dns, COMM_S, "NULL");
	snprintf(zone->sec_dns, COMM_S, "NULL");
	snprintf(zone->web_ip, COMM_S, "NULL");
	snprintf(zone->ftp_ip, COMM_S, "NULL");
	snprintf(zone->mail_ip, COMM_S, "NULL");
	snprintf(zone->valid, COMM_S, "NULL");
	snprintf(zone->updated, COMM_S, "NULL");
	zone->serial = 0;
	zone->refresh = 0;
	zone->retry = 0;
	zone->expire = 0;
	zone->ttl = 0;
	zone->id = 0;
	zone->owner = 0;
}

void print_fwd_zone_config(zone_info_t *zone)
{
	printf("Zone %s information\n", zone->name);
	printf("Serial: %ld\n", zone->serial);
	printf("Refresh: %ld\n", zone->refresh);
	printf("Retry: %ld\n", zone->retry);
	printf("Expire: %ld\n", zone->expire);
	printf("TTL: %ld\n", zone->ttl);
	printf("Primary DNS: %s\n", zone->pri_dns);
	printf("Secondary DNS: %s\n", zone->sec_dns);
	printf("Web IP address: %s\n", zone->web_ip);
	printf("FTP IP address: %s\n", zone->ftp_ip);
	printf("MX IP address: %s\n", zone->mail_ip);
}

void fill_dnsa_config(MYSQL_ROW my_row, zone_info_t *zone)
{
	if ((strncmp(my_row[0], "dnsa_pri_dns", RBUFF_S)) == 0)
		snprintf(zone->pri_dns, RBUFF_S, "%s",  my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_sec_dns", RBUFF_S)) == 0)
		snprintf(zone->sec_dns, RBUFF_S, "%s",  my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_web_ip", RANGE_S)) == 0)
		snprintf(zone->web_ip, RANGE_S, "%s", my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_ftp_ip", RANGE_S)) == 0)
		snprintf(zone->ftp_ip, RANGE_S, "%s", my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_mail_ip", RANGE_S)) == 0)
		snprintf(zone->mail_ip, RANGE_S, "%s", my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_refresh", RBUFF_S)) == 0)
		zone->refresh = strtoul(my_row[1], NULL, 10);
	else if ((strncmp(my_row[0], "dnsa_retry", RBUFF_S)) == 0)
		zone->retry = strtoul(my_row[1], NULL, 10);
	else if ((strncmp(my_row[0], "dnsa_expire", RBUFF_S)) == 0)
		zone->expire = strtoul(my_row[1], NULL, 10);
	else if ((strncmp(my_row[0], "dnsa_ttl", RBUFF_S)) == 0)
		zone->ttl = strtoul(my_row[1], NULL, 10);
}

void insert_new_fwd_zone(zone_info_t *zone, dnsa_config_t *config)
{
	MYSQL dnsa;
	my_ulonglong dnsa_rows;
	char *query;
	const char *dnsa_query;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_new_fwd_zone");
	
	snprintf(query, RBUFF_S,
"INSERT INTO zones (name, pri_dns, sec_dns, serial, refresh, retry, expire, \
ttl) VALUES ('%s', 'ns1.%s', 'ns2.%s', %ld, %ld, %ld, %ld, %ld)",
		 zone->name,
		 zone->name,
		 zone->name,
		 zone->serial,
		 zone->refresh,
		 zone->retry,
		 zone->expire,
		 zone->ttl);
	dnsa_query = query;
	dnsa_mysql_init(config, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if (dnsa_rows == 1) {
		fprintf(stderr, "New zone %s added\n", zone->name);
	} else {
		cmdb_mysql_clean(&dnsa, query);
		report_error(CANNOT_INSERT_ZONE, zone->name);
	}
	cmdb_mysql_clean(&dnsa, query);
}

void insert_new_fwd_zone_records(zone_info_t *zone, dnsa_config_t *config)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	char *query;
	const char *dnsa_query;
	unsigned long int zone_id;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_new_fwd_zone_records");
	snprintf(query, RBUFF_S,
"SELECT id FROM zones WHERE name = '%s'", zone->name);
	dnsa_query = query;
	dnsa_mysql_init(config, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		report_error(NO_DOMAIN, zone->name);
	} else if (dnsa_rows > 1) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		report_error(MULTI_DOMAIN, zone->name);
	}
	dnsa_row = mysql_fetch_row(dnsa_res);
	zone_id = strtoul(dnsa_row[0], NULL, 10);
	mysql_free_result(dnsa_res);
	
	add_fwd_zone_record(&dnsa, zone_id, "ftp", zone->ftp_ip, "A");
	add_fwd_zone_record(&dnsa, zone_id, "www", zone->web_ip, "A");
	add_fwd_zone_record(&dnsa, zone_id, "mail01", zone->mail_ip, "A");
	add_fwd_zone_record(&dnsa, zone_id, "ns1", zone->pri_dns, "A");
	add_fwd_zone_record(&dnsa, zone_id, "ns2", zone->sec_dns, "A");
	cmdb_mysql_clean(&dnsa, query);
}

void add_fwd_zone_record(MYSQL *dnsa, unsigned long int zone_id, const char *name, char *dest, const char *type)
{
	char *query;
	const char *dnsa_query;
	my_ulonglong dnsa_rows;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_fwd_zone_record");
	dnsa_query = query;
	snprintf(query, RBUFF_S,
"INSERT INTO records (zone, host, type, destination) VALUES \
(%ld, '%s', '%s', '%s')", zone_id, name, type, dest);
	cmdb_mysql_query(dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(dnsa);
	if (dnsa_rows == 1) {
		fprintf(stderr, "New record %s added\n", name);
	} else {
		cmdb_mysql_clean(dnsa, query);
		report_error(CANNOT_INSERT_RECORD, name);
	}
	free(query);
}

void add_fwd_host(dnsa_config_t *dct, comm_line_t *clt)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	char *query;
	const char *dnsa_query;
	unsigned long int zone_id;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_new_fwd_zone_records");
	snprintf(query, RBUFF_S,
"SELECT id FROM zones WHERE name = '%s'", clt->domain);
	dnsa_query = query;
	dnsa_mysql_init(dct, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, query);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
		report_error(NO_DOMAIN, clt->domain);
	} else if (dnsa_rows > 1) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
		report_error(MULTI_DOMAIN, clt->domain);
	}
	dnsa_row = mysql_fetch_row(dnsa_res);
	zone_id = strtoul(dnsa_row[0], NULL, 10);
	add_fwd_zone_record(&dnsa, zone_id, clt->host, clt->dest, clt->rtype);
	cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
}

size_t add_records(record_row_t *my_row, char *output, size_t offset)
{
	char *tmp;
	size_t len;
	if (!(tmp = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in add_records");
	len = strlen(my_row->host);
	if (len >= 8)
		snprintf(tmp, RBUFF_S, "\
%s\t%s\t%s\n", my_row->host, my_row->type, my_row->dest);
	else
		snprintf(tmp, RBUFF_S, "\
%s\t\t%s\t%s\n", my_row->host, my_row->type, my_row->dest);
	len = strlen(tmp);
	strncat(output, tmp, len);
	offset += len;
	free(tmp);
	return offset;
}

void add_ns_A_records_to_header(zone_info_t *zi, dnsa_config_t *dc, char *out)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t len;
	record_row_t row_data;
	char *line, *dquery, *c, *thost;
	const char *dnsa_query, *error_string;
	
	if (!(line = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "line in add_ns_A_records");
	if (!(c = malloc(CH_S * sizeof(char))))
		report_error(MALLOC_FAIL, "c in add_ns_A_records");
	if (!(dquery = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dquery in add_ns_A_records");
	
	dnsa_query = dquery;
	len = 0;
	
	snprintf(c, CH_S, ".");
	error_string = c;
	snprintf(line, BUFF_S, "%s", zi->pri_dns);
	if (!(thost = strtok(line, c)))
		report_error(NO_DELIM, error_string);
	/* Find and add 1st NS record */
	snprintf(dquery, RBUFF_S, "\
SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'",
		 zi->id, thost);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		free(line);
		free(c);
		cmdb_mysql_clean(&dnsa, dquery);
		report_error(NO_RECORDS, zi->name);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(dnsa_row);
		len = add_records(&row_data, out, len);
	}
	/* Now check for second NS record */
	if (!(strcmp(zi->sec_dns, "NULL")) == 0) {
		mysql_free_result(dnsa_res);
		snprintf(line, BUFF_S, "%s", zi->sec_dns);
		if (!(thost = strtok(line, c)))
			report_error(NO_DELIM, error_string);
		snprintf(dquery, RBUFF_S, "\
SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'",
			zi->id, thost);
		cmdb_mysql_query(&dnsa, dnsa_query);
		if (!(dnsa_res = mysql_store_result(&dnsa)))
			report_error(NO_RECORDS, zi->name);
		while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
			row_data = fill_record_data(dnsa_row);
			len = add_records(&row_data, out, len);
		}
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
	free(c);
	free(line);
}

void add_MX_A_records_to_header(zone_info_t *zi, dnsa_config_t *dc, char *out)
{
	MYSQL dnsa, dnsa2;
	MYSQL_RES *dnsa_res, *dnsa_res2;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	size_t len;
	record_row_t row_data, row_data2;
	char *line, *dquery, *c, *thost;
	const char *dnsa_query, *error_string;
	
	if (!(line = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "line in add_MX_A_records");
	if (!(c = malloc(CH_S * sizeof(char))))
		report_error(MALLOC_FAIL, "c in add_MX_A_records");
	if (!(dquery = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dquery in add_MX_A_records");
	
	dnsa_query = dquery;
	error_string = c;
	len = 0;
	
	snprintf(c, CH_S, ".");
	snprintf(dquery, RBUFF_S, "\
SELECT * FROM records WHERE zone = %d AND type = 'MX'", zi->id);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, dquery);
		report_error(NO_RECORDS, zi->name);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "No MX records for domain %s\n", zi->name);
		cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
		free(c);
		free(line);
		return;
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(dnsa_row);
		snprintf(line, BUFF_S, "%s", row_data.dest);
		if (!(thost = strtok(line, c)))
			report_error(NO_DELIM, error_string);
		snprintf(dquery, RBUFF_S, "\
SELECT * FROM records WHERE zone = %d AND host = '%s'",
				zi->id, thost);
		dnsa_mysql_init(dc, &dnsa2);
		cmdb_mysql_query(&dnsa2, dquery);
		if (!(dnsa_res2 = mysql_store_result(&dnsa2))) {
			fprintf(stderr, "No result set?\n");
			cmdb_mysql_clean_full(dnsa_res2, &dnsa2, dquery);
			cmdb_mysql_clean_full(dnsa_res, &dnsa, line);
			free(c);
			return;
		}
		while ((dnsa_row = mysql_fetch_row(dnsa_res2))) {
			row_data2 = fill_record_data(dnsa_row);
			len = add_records(&row_data2, out, len);
		}
		mysql_free_result(dnsa_res2);
		mysql_close(&dnsa2);
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, dquery);
	free(c);
	free(line);
}

void check_fwd_zone(char *filename, char *domain, dnsa_config_t *dc)
{
	char *command;
	const char *syscom;
	int error;
	
	if (!(command = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "command in check_rev_zone");
	syscom = command;
	
	snprintf(command, RBUFF_S, "%s %s %s", dc->chkz, domain, filename);
	error = system(syscom);
	if (error != 0)
		report_error(CHKZONE_FAIL, domain);
	else
		printf("check of zone %s ran successfully\n", domain);
	free(command);
}

void write_fwd_zonefile(char *filename, char *output)
{
	FILE *zonefile;
	if (!(zonefile = fopen(filename, "w"))) {
		report_error(FILE_O_FAIL, filename);
	} else {
		fputs(output, zonefile);
		fclose(zonefile);
	}
}

void add_fwd_zone(char *domain, dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	zone_info_t *new_zone;
	char *query;
	const char *dnsa_query;

	if (!(new_zone = malloc(sizeof(zone_info_t))))
		report_error(MALLOC_FAIL, "new_zone in add_fwd_zone");
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_fwd_zone");
	
	init_dnsa_zone(new_zone);
	snprintf(query, BUFF_S,\
"SELECT config, value FROM configuration WHERE config LIKE 'dnsa_%%'");
	dnsa_query = query;
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, query);
		free(new_zone);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
		free(new_zone);
		report_error(NO_ZONE_CONFIGURATION, dnsa_query);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res)))
		fill_dnsa_config(dnsa_row, new_zone);

	cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
	snprintf(new_zone->name, RBUFF_S, "%s", domain);
	update_fwd_zone_serial(new_zone);
	print_fwd_zone_config(new_zone);
	insert_new_fwd_zone(new_zone, dc);
	insert_new_fwd_zone_records(new_zone, dc);
	free(new_zone);
}

int wzf (char *domain, dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW my_row;
	my_ulonglong dnsa_rows;
	record_row_t row_data;
	size_t len, offset;
	zone_info_t zone_info, *zi;
	int error;
	char *zout, *zout2, *tmp, *zonefilename;
	const char *dnsa_query;

	if (!(tmp = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in wzf");
	
	dnsa_query = tmp;
	zi = &zone_info;
	len = offset = 0;
	
	dnsa_mysql_init(dc, &dnsa);
	snprintf(tmp, BUFF_S,
"SELECT * FROM zones WHERE name = '%s'", domain);
	cmdb_mysql_query(&dnsa, dnsa_query);

	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, tmp);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
		report_error(NO_DOMAIN, domain);
	} else if (dnsa_rows > 1) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, tmp);
		report_error(MULTI_DOMAIN, domain);
	}
	my_row = mysql_fetch_row(dnsa_res);
	zone_info = fill_zone_data(my_row);
	mysql_free_result(dnsa_res);
	update_fwd_zone_serial(zi);
	snprintf(tmp, BUFF_S,
"UPDATE zones SET serial = %ld WHERE name = '%s'", zi->serial, domain);
	cmdb_mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if (dnsa_rows != 1)
		fprintf(stderr, "Unable to update serial for zone '%s'", domain);
	if (!(zout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zout in wzf");
	create_zone_header(zout, zi, FILE_S);
	snprintf(tmp, BUFF_S,
"SELECT * FROM records WHERE zone = %d AND type = 'MX'", zi->id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		free(zout);
		cmdb_mysql_clean(&dnsa, tmp);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (!(dnsa_rows = mysql_num_rows(dnsa_res)) == 0) {
		while ((my_row = mysql_fetch_row(dnsa_res))) {
			add_mx_to_header(zout, my_row);
		}
	}
	mysql_free_result(dnsa_res);
	snprintf(tmp, BUFF_S, "$ORIGIN\t%s.\n", zi->name);
	len = strlen(tmp);
	strncat(zout, tmp, len);
	
	/** zout2 will be used to write the real zone file.
	 ** Cannot have A records for the NS and MX added twice
	 ** so save the buffer into zout2 and use that
	 */
	if (!(zout2 = calloc(BUILD_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zout2 in wzf");
	len = strlen(zout);
	strncpy(zout2, zout, len);
	
	add_ns_A_records_to_header(zi, dc, zout);
	add_MX_A_records_to_header(zi, dc, zout);
	if (!(zonefilename = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefilename in wrz");
	/* Check zonefile */
	snprintf(zonefilename, TBUFF_S, "%sdb1.%s", dc->dir, zi->name);
	write_fwd_zonefile(zonefilename, zout);
	check_fwd_zone(zonefilename, zi->name, dc);
	remove(zonefilename);
	free(zout);
	
	/* Add the rest of the records */
	snprintf(tmp, BUFF_S,
"SELECT * FROM records WHERE zone = %d AND type = 'A' \
OR zone = %d AND type = 'CNAME'", zi->id, zi->id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, tmp);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	while ((my_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(my_row);
		offset = add_records(&row_data, zout2, offset);	
	}
	mysql_free_result(dnsa_res);
	snprintf(zonefilename, TBUFF_S, "%sdb.%s", dc->dir, zi->name);
	write_fwd_zonefile(zonefilename, zout2);
	check_fwd_zone(zonefilename, zi->name, dc);
	snprintf(tmp, BUFF_S,
"UPDATE zones SET updated = 'no', valid = 'yes' WHERE name = '%s'", zi->name);
	error = mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if (dnsa_rows == 1)
		fprintf(stderr, "DB updated as zone validated\n");
	cmdb_mysql_clean(&dnsa, tmp);
	free(zout2);
	free(zonefilename);
	return 0;
}

int wcf(dnsa_config_t *dc)
{
	FILE *cnf;
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t len;
	my_ulonglong dnsa_rows;
	int error;
	char *dout, *dnsa_line, *zonefile;
	const char *dnsa_query, *check_comm, *reload_comm, *domain;
	
	dnsa_query = "SELECT name FROM zones WHERE valid = 'yes'";
	domain = "that is valid ";

	if (!(dnsa_line = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dnsa_line in wcf");

	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, dnsa_line);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, dnsa_line);
		report_error(NO_DOMAIN, domain);
	}
	/* From each DB row create the config lines
	 * also check if the zone file exists on the filesystem*/
	if (!(dout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dout in wcf");
	if (!(zonefile = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in wcf");
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		snprintf(zonefile, CONF_S, "%sdb.%s", dc->dir, dnsa_row[0]);
		if ((cnf = fopen(zonefile, "r"))){
			snprintf(dnsa_line, TBUFF_S, "\
zone \"%s\" {\n\t\t\ttype master;\n\t\t\tfile \"%s\";\n\t\t};\n\n", 
				 dnsa_row[0], zonefile);
			len = strlen(dnsa_line);
			strncat(dout, dnsa_line, len);
			fclose(cnf);
		} else {
			printf("Not adding for zonefile %s\n", dnsa_row[0]);
		}
	}
	/* Write the BIND config file */
	snprintf(dnsa_line, TBUFF_S, "%s%s", dc->bind, dc->dnsa);
	if (!(cnf = fopen(dnsa_line, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing!\n", dnsa_line);
		exit(1);
	} else {
		fputs(dout, cnf);
		fclose(cnf);
	}
	/* Check the file and if OK reload BIND */
	check_comm = dnsa_line;
	snprintf(dnsa_line, TBUFF_S, "%s %s%s", dc->chkc, dc->bind, dc->dnsa);
	error = system(check_comm);
	if (error != 0) {
		fprintf(stderr, "Bind config check failed! Error code was %d\n", error);
	} else {
		reload_comm = dnsa_line;
		snprintf(dnsa_line, TBUFF_S, "%s reload", dc->rndc);
		error = system(reload_comm);
		if (error != 0) {
			fprintf(stderr, "Bind reload failed with error code %d\n", error);
		}
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, dout);
	free(dnsa_line);
	free(zonefile);
	return 0;
}

void update_fwd_zone_serial(zone_info_t *zone)
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
	if (zone->serial < serial)
		zone->serial = serial;
	else
		zone->serial++;
}