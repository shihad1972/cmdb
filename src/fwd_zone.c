/* fwd_zone:
 * 
 * Contains various funtions that are needed to import / export
 * forward zone information from the database, and to also write
 * the zone files and BIND configuration files.
 * 
 * Part of the DNSA program
 * 
 * (C) Iain M Conochie 2012
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mysql/mysql.h>
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
	int id;
	zone_info_t my_zone;
	id = atoi(my_row[0]);
	my_zone.id = id;
	strncpy(my_zone.name, my_row[1], RBUFF_S);
	strncpy(my_zone.pri_dns, my_row[2], RBUFF_S);
	strncpy(my_zone.sec_dns, my_row[3] ? my_row[3] : "NULL", RBUFF_S);
	id = atoi(my_row[4]);
	my_zone.serial = id;
	id = atoi(my_row[5]);
	my_zone.refresh = id;
	id = atoi(my_row[6]);
	my_zone.retry = id;
	id = atoi(my_row[7]);
	my_zone.expire = id;
	id = atoi(my_row[8]);
	my_zone.ttl = id;
	strncpy(my_zone.valid, my_row[9], RBUFF_S);
	id = atoi(my_row[10]);
	my_zone.owner = id;
	strncpy(my_zone.updated, my_row[11], RBUFF_S);
	return my_zone;
}

/* Write out zone file header */
void create_zone_header(char *zout, zone_info_t zone_info)
{
	zone_info_t *zi;
	char *tmp;
	size_t offset;
	offset = 0;
	if (!(tmp = malloc(TBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in create_zone_header");
	zi = &zone_info;
	sprintf(tmp, "$ORIGIN .\n$TTL %d\n", zi->ttl);
	offset = strlen(tmp);
	strncpy(zout, tmp, offset);
	if (strlen(zi->name) < 16) {
		sprintf(tmp, "%s\t\tIN SOA\t", zi->name);
		offset = strlen(tmp);
		strncat(zout, tmp , offset);
	} else {
		sprintf(tmp, "%s\tIN SOA\t", zi->name);
		offset = strlen(tmp);
		strncat(zout, tmp , offset);
	}
	sprintf(tmp, "%s. hostmaster.%s. (\n\t\t\t", zi->pri_dns, zi->name);
	offset = strlen(tmp);
	strncat(zout, tmp, offset);
	sprintf(tmp, "%d\t; Serial\n\t\t\t", zi->serial);
	offset = strlen(tmp);
	strncat(zout, tmp, offset);
	sprintf(tmp, "%d\t\t; Refresh\n\t\t\t", zi->refresh);
	offset = strlen(tmp);
	strncat(zout, tmp, offset);
	sprintf(tmp, "%d\t\t; Retry\n\t\t\t", zi->retry);
	offset = strlen(tmp);
	strncat(zout, tmp, offset);
	sprintf(tmp, "%d\t\t; Expire\n\t\t\t", zi->expire);
	offset = strlen(tmp);
	strncat(zout, tmp, offset);
	sprintf(tmp, "%d)\t\t; Negative Cache TTL\n\t\t\tNS\t\t%s.\n", zi->ttl, zi->pri_dns);
	offset = strlen(tmp);
	strncat(zout, tmp, offset);
	if (!(strcmp(zi->sec_dns, "NULL")) == 0) {
		sprintf(tmp, "\t\t\tNS\t\t%s.\n", zi->sec_dns);
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
	sprintf(tmp, "\t\t\t%s", results[3]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	sprintf(tmp, "\t%s", results[4]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	sprintf(tmp, "\t%s\n", results[5]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	free(tmp);
}

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

void init_dnsa_zone(dnsa_zone_t *dnsa_zone)
{
	snprintf(dnsa_zone->name, COMM_S, "NULL");
	snprintf(dnsa_zone->pri_dns, COMM_S, "NULL");
	snprintf(dnsa_zone->sec_dns, COMM_S, "NULL");
	snprintf(dnsa_zone->web_ip, COMM_S, "NULL");
	snprintf(dnsa_zone->ftp_ip, COMM_S, "NULL");
	snprintf(dnsa_zone->mail_ip, COMM_S, "NULL");
	dnsa_zone->serial = 0;
	dnsa_zone->refresh = 0;
	dnsa_zone->retry = 0;
	dnsa_zone->expire = 0;
	dnsa_zone->ttl = 0;
}

void print_fwd_zone_config(dnsa_zone_t *zone)
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

void fill_dnsa_config(MYSQL_ROW my_row, dnsa_zone_t *zone)
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

void insert_new_fwd_zone(dnsa_zone_t *zone, dnsa_config_t *config)
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
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		report_error(CANNOT_INSERT_ZONE, zone->name);
	}
	mysql_close(&dnsa);
	mysql_library_end();
	free(query);
}

void insert_new_fwd_zone_records(dnsa_zone_t *zone, dnsa_config_t *config)
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
	mysql_close(&dnsa);
	mysql_library_end();
	free(query);
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
		mysql_close(dnsa);
		mysql_library_end();
		free(query);
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
		report_error(NO_DOMAIN, clt->domain);
	} else if (dnsa_rows > 1) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		report_error(MULTI_DOMAIN, clt->domain);
	}
	dnsa_row = mysql_fetch_row(dnsa_res);
	zone_id = strtoul(dnsa_row[0], NULL, 10);
	mysql_free_result(dnsa_res);
	free(query);
	add_fwd_zone_record(&dnsa, zone_id, clt->host, clt->dest, clt->rtype);
}

size_t add_records(record_row_t *my_row, char *output, size_t offset)
{
	char *tmp;
	size_t len;
	if (!(tmp = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in add_records");
	len = strlen(my_row->host);
	if (len >= 8)
		sprintf(tmp, "%s\t", my_row->host);
	else
		sprintf(tmp, "%s\t\t", my_row->host);
	len = strlen(tmp);
	strncat(output, tmp, len);
	offset += len;
	sprintf(tmp, "%s\t", my_row->type);
	len = strlen(tmp);
	strncat(output, tmp, len);
	offset += len;
	sprintf(tmp, "%s\n", my_row->dest);
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
	
	sprintf(c, ".");
	error_string = c;
	sprintf(line, "%s", zi->pri_dns);
	if (!(thost = strtok(line, c)))
		report_error(NO_DELIM, error_string);
	/* Find and add 1st NS record */
	sprintf(dquery, "SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'", zi->id, thost);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa)))
		report_error(NO_RECORDS, zi->name);
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(dnsa_row);
		len = add_records(&row_data, out, len);
	}
	/* Now check for second NS record */
	if (!(strcmp(zi->sec_dns, "NULL")) == 0) {
		mysql_free_result(dnsa_res);
		sprintf(line, "%s", zi->sec_dns);
		if (!(thost = strtok(line, c)))
			report_error(NO_DELIM, error_string);
		sprintf(dquery, "SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'", zi->id, thost);
		cmdb_mysql_query(&dnsa, dnsa_query);
		if (!(dnsa_res = mysql_store_result(&dnsa)))
			report_error(NO_RECORDS, zi->name);
		while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
			row_data = fill_record_data(dnsa_row);
			len = add_records(&row_data, out, len);
		}
	}
	mysql_free_result(dnsa_res);
	mysql_close(&dnsa);
	free(dquery);
	free(c);
	free(line);
}

int add_MX_A_records_to_header(zone_info_t *zi, dnsa_config_t *dc, char *out)
{
	MYSQL dnsa, dnsa2;
	MYSQL_RES *dnsa_res, *dnsa_res2;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	size_t len;
	record_row_t row_data, row_data2;
	char *line, *dquery, *c, *thost;
	const char *dnsa_query, *error_string;
	int mx;
	
	if (!(line = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "line in add_MX_A_records");
	if (!(c = malloc(CH_S * sizeof(char))))
		report_error(MALLOC_FAIL, "c in add_MX_A_records");
	if (!(dquery = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dquery in add_MX_A_records");
	
	dnsa_query = dquery;
	error_string = c;
	len = 0;
	mx = 0;
	
	sprintf(c, ".");
	sprintf(dquery, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zi->id);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa)))
		report_error(NO_RECORDS, zi->name);
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "No MX records for domain %s\n", zi->name);
		mx = -1;
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		free(dquery);
		free(c);
		free(line);
		return mx;
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		row_data = fill_record_data(dnsa_row);
		sprintf(line, "%s", row_data.dest);
		if (!(thost = strtok(line, c)))
			report_error(NO_DELIM, error_string);
		sprintf(dquery, "SELECT * FROM records WHERE zone = %d AND host = '%s'",
				zi->id, thost);
		dnsa_mysql_init(dc, &dnsa2);
		cmdb_mysql_query(&dnsa2, dquery);
		if (!(dnsa_res2 = mysql_store_result(&dnsa2))) {
			fprintf(stderr, "No result set?\n");
			if (mx == 0)
				mx = -1;
			mysql_free_result(dnsa_res);
			mysql_close(&dnsa);
			free(dquery);
			free(c);
			free(line);
			return mx;
		}
		while ((dnsa_row = mysql_fetch_row(dnsa_res2))) {
			row_data2 = fill_record_data(dnsa_row);
			len = add_records(&row_data2, out, len);
		}
		mysql_free_result(dnsa_res2);
		mysql_close(&dnsa2);
		mx++;
	}
	mysql_free_result(dnsa_res);
	mysql_close(&dnsa);
	free(dquery);
	free(c);
	free(line);
	return mx;
}

void check_fwd_zone(char *filename, char *domain, dnsa_config_t *dc)
{
	char *command;
	const char *syscom;
	int error;
	
	if (!(command = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "command in check_rev_zone");
	syscom = command;
	
	sprintf(command, "%s %s %s", dc->chkz, domain, filename);
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
	dnsa_zone_t *new_zone;
	char *query;
	const char *dnsa_query;

	if (!(new_zone = malloc(sizeof(dnsa_zone_t))))
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
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		free(new_zone);
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		free(new_zone);
		report_error(NO_ZONE_CONFIGURATION, dnsa_query);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res)))
		fill_dnsa_config(dnsa_row, new_zone);
	
	mysql_free_result(dnsa_res);
	mysql_close(&dnsa);
	mysql_library_end();
	
	snprintf(new_zone->name, RBUFF_S, "%s", domain);
	update_fwd_zone_serial(new_zone);
	print_fwd_zone_config(new_zone);
	insert_new_fwd_zone(new_zone, dc);
	insert_new_fwd_zone_records(new_zone, dc);
	free(query);
	free(new_zone);
}

int wzf (char *domain, dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res, *my_res;
	MYSQL_ROW my_row;
	zone_info_t zone_info, *zi;
	record_row_t row_data;
	size_t offset, len;
	my_ulonglong dnsa_rows;
	int error;
	char *zout, *zout2, *tmp, *zonefilename;
	const char *dnsa_query;

	if (!(zout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zout in wzf");
	if (!(zout2 = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zout2 in wzf");
	if (!(tmp = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in wzf");
	if (!(zonefilename = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefilename in wrz");
	
	dnsa_query = tmp;
	zi = &zone_info;
	
	/* Initialise MYSQL connection and query */
	dnsa_mysql_init(dc, &dnsa);
	sprintf(tmp, "SELECT * FROM zones WHERE name = '%s'", domain);
	cmdb_mysql_query(&dnsa, dnsa_query);

	if (!(dnsa_res = mysql_store_result(&dnsa)))
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0))
		report_error(NO_DOMAIN, domain);
	
	my_row = mysql_fetch_row(dnsa_res);
	zone_info = fill_zone_data(my_row);
	create_zone_header(zout, zone_info);
	mysql_free_result(dnsa_res);
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zi->id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	
	if (!(dnsa_res = mysql_store_result(&dnsa)))
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	if (!(dnsa_rows = mysql_num_rows(dnsa_res)) == 0) {
		while ((my_row = mysql_fetch_row(dnsa_res))) {
			add_mx_to_header(zout, my_row);
		}
	}
	mysql_free_result(dnsa_res);
	sprintf(tmp, "$ORIGIN\t%s.\n", zi->name);
	len = strlen(tmp);
	strncat(zout, tmp, len);
	
	/** zout2 will be used to write the real zone file.
	 ** Cannot have A records for the NS and MX added twice
	 ** so save the buffer into zout2 and use that
	 */
	len = strlen(zout);
	strncpy(zout2, zout, len);
	
	add_ns_A_records_to_header(zi, dc, zout);
	error = add_MX_A_records_to_header(zi, dc, zout);
	if (error < 0) {
		; /* No MX records */
	}

	/* Check zonefile */
	sprintf(zonefilename, "%sdb1.%s", dc->dir, zi->name);
	write_fwd_zonefile(zonefilename, zout);
	check_fwd_zone(zonefilename, zi->name, dc);
	remove(zonefilename);
	
	/* Add the rest of the records */
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'A' OR zone = %d AND type = 'CNAME'", zi->id, zi->id);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(my_res = mysql_store_result(&dnsa)))
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	while ((my_row = mysql_fetch_row(my_res))) {
		row_data = fill_record_data(my_row);
		offset = add_records(&row_data, zout2, offset);	
	}
	mysql_free_result(my_res);
	sprintf(zonefilename, "%sdb.%s", dc->dir, zi->name);
	write_fwd_zonefile(zonefilename, zout2);
	check_fwd_zone(zonefilename, zi->name, dc);
	sprintf(tmp, "UPDATE zones SET updated = 'no', valid = 'yes' WHERE name = '%s'", zi->name);
	error = mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if (dnsa_rows == 1)
		fprintf(stderr, "DB updated as zone validated\n");
	mysql_close(&dnsa);
	mysql_library_end();
	free(zout2);
	free(zout);
	free(tmp);
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
	char *dout, *dnsa_line, *zonefile, *error_code;
	const char *dnsa_query, *check_comm, *reload_comm, *error_str, *domain;
	
	dnsa_query = "SELECT name FROM zones WHERE valid = 'yes'";
	domain = "that is valid ";

	if (!(dout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dout in wcf");
	if (!(dnsa_line = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dnsa_line in wcf");
	if (!(zonefile = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in wcf");
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in wcf");
	
	error_str = error_code;

	/* Initialise MYSQL Connection and query*/
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		report_error(NO_DOMAIN, domain);
	}
	
	/* From each DB row create the config lines
	 * also check if the zone file exists on the filesystem*/
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		sprintf(zonefile, "%sdb.%s", dc->dir, dnsa_row[0]);
		if ((cnf = fopen(zonefile, "r"))){
			sprintf(dnsa_line, "zone \"%s\" {\n\t\t\ttype master;\n\t\t\tfile \"%s\";\n\t\t};\n\n", dnsa_row[0], zonefile);
			len = strlen(dnsa_line);
			strncat(dout, dnsa_line, len);
			fclose(cnf);
		} else {
			printf("Not adding for zonefile %s\n", dnsa_row[0]);
		}
	}
	
	/* Write the BIND config file */
	sprintf(dnsa_line, "%s%s", dc->bind, dc->dnsa);
	
	if (!(cnf = fopen(dnsa_line, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing!\n", dnsa_line);
		exit(1);
	} else {
		fputs(dout, cnf);
		fclose(cnf);
	}
	
	/* Check the file and if OK reload BIND */
	check_comm = dnsa_line;
	sprintf(dnsa_line, "%s %s%s", dc->chkc, dc->bind, dc->dnsa);
	error = system(check_comm);
	if (error != 0) {
		fprintf(stderr, "Bind config check failed! Error code was %d\n", error);
	} else {
		reload_comm = dnsa_line;
		sprintf(dnsa_line, "%s reload", dc->rndc);
		error = system(reload_comm);
		if (error != 0) {
			fprintf(stderr, "Bind reload failed with error code %d\n", error);
		}
	}
	mysql_free_result(dnsa_res);
	mysql_close(&dnsa);
	mysql_library_end();
	free(dout);
	free(dnsa_line);
	free(zonefile);
	free(error_code);
	return 0;
}

void update_fwd_zone_serial(dnsa_zone_t *zone)
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