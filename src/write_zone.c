#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "forward.h"


/** Function to fill a struct with results from the DB query
 ** No error checking on fields
 */
zone_info_t fill_zone_data(MYSQL_ROW my_row)
{
	int id;
	zone_info_t my_zone;
	id = atoi(my_row[0]);
	my_zone.id = id;
	strncpy(my_zone.name, my_row[1], 256);
	strncpy(my_zone.pri_dns, my_row[2], 256);
	strncpy(my_zone.sec_dns, my_row[3] ? my_row[3] : "NULL", 256);
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
	strncpy(my_zone.valid, my_row[9], 256);
	id = atoi(my_row[10]);
	my_zone.owner = id;
	strncpy(my_zone.updated, my_row[11], 256);
	return my_zone;
}

/* Write out zone file header */
size_t create_zone_header(char *zout, zone_info_t zone_info)
{
	char *tmp;
	size_t offset, soffset;
	offset = 0;
	tmp = malloc(256 * sizeof(char));
	sprintf(tmp, "$ORIGIN .\n$TTL %d\n", zone_info.ttl);
	soffset = strlen(tmp);
	strncpy(zout, tmp, soffset);
	offset += soffset;
	if (strlen(zone_info.name) < 16) {
		sprintf(tmp, "%s\t\tIN SOA\t", zone_info.name);
		soffset = strlen(tmp);
		strncat(zout, tmp , soffset);
		offset += soffset;
	} else {
		sprintf(tmp, "%s\tIN SOA\t", zone_info.name);
		soffset = strlen(tmp);
		strncat(zout, tmp , soffset);
		offset += soffset;
	}
	sprintf(tmp, "%s. hostmaster.%s. (\n\t\t\t", zone_info.pri_dns, zone_info.name);
	soffset = strlen(tmp);
	strncat(zout, tmp, soffset);
	offset +=soffset;
	sprintf(tmp, "%d\t; Serial\n\t\t\t", zone_info.serial);
	soffset = strlen(tmp);
	strncat(zout, tmp, soffset);
	offset += soffset;
	sprintf(tmp, "%d\t\t; Refresh\n\t\t\t", zone_info.refresh);
	soffset = strlen(tmp);
	strncat(zout, tmp, soffset);
	offset += soffset;
	sprintf(tmp, "%d\t\t; Retry\n\t\t\t", zone_info.retry);
	soffset = strlen(tmp);
	strncat(zout, tmp, soffset);
	offset += soffset;
	sprintf(tmp, "%d\t\t; Expire\n\t\t\t", zone_info.expire);
	soffset = strlen(tmp);
	strncat(zout, tmp, soffset);
	offset += soffset;
	sprintf(tmp, "%d)\t\t; Negative Cache TTL\n\t\t\tNS\t\t%s.\n", zone_info.ttl, zone_info.pri_dns);
	soffset = strlen(tmp);
	strncat(zout, tmp, soffset);
	offset += soffset;
	if ((strcmp(zone_info.sec_dns, "NULL")) == 0) {
		;
	} else {
		sprintf(tmp, "\t\t\tNS\t\t%s.\n", zone_info.sec_dns);
		soffset = strlen(tmp);
		strncat(zout, tmp, soffset);
		offset += soffset;
	}
	return offset;
}
/* add MX records to zone file header */
size_t add_mx_to_header(char *output, size_t offset, MYSQL_ROW results)
{
	size_t count;
	char *tmp;
	tmp = malloc(1024 * sizeof(char));
	sprintf(tmp, "\t\t\t%s", results[3]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	offset += count;
	sprintf(tmp, "\t%s", results[4]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	offset += count;
	sprintf(tmp, "\t%s\n", results[5]);
	count = strlen(tmp);
	strncat(output, tmp, count);
	offset += count;
	return offset;
}

record_row_t fill_record_data(MYSQL_ROW my_row)
{
	record_row_t records;
	records.id = atoi(my_row[0]);
	records.zone = atoi(my_row[1]);
	strncpy(records.host, my_row[2], 256);
	strncpy(records.type, my_row[3], 256);
	records.pri = atoi(my_row[4]);
	strncpy(records.dest, my_row[5], 256);
	strncpy(records.valid, my_row[6], 256);
	return records;
}


size_t add_records(record_row_t my_row, char *output, size_t offset)
{
	char *tmp;
	size_t len;
	tmp = malloc(sizeof(char) * 256);
	len = strlen(my_row.host);
	if (len >= 8)
		sprintf(tmp, "%s\t", my_row.host);
	else
		sprintf(tmp, "%s\t\t", my_row.host);
	len = strlen(tmp);
	strncat(output, tmp, len);
	offset += len;
	sprintf(tmp, "%s\t", my_row.type);
	len = strlen(tmp);
	strncat(output, tmp, len);
	offset += len;
	sprintf(tmp, "%s\n", my_row.dest);
	len = strlen(tmp);
	strncat(output, tmp, len);
	offset += len;
	return offset;
}
