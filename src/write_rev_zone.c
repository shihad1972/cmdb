/* write_rev_zone.c: DNSA module to write reverse zone file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"
#include "rev_zone.h"

/** Function to fill a struct with results from the DB query
 ** No error checking on fields
 */
rev_zone_info_t fill_rev_zone_data(MYSQL_ROW my_row)
{
	size_t len;
	const char *line, *ch, *hostmaster;
	char *tmp;
	rev_zone_info_t my_zone;
	ch = ".";
	hostmaster = "hostmaster";
	my_zone.rev_zone_id = atoi(my_row[0]);
	strncpy(my_zone.net_range, my_row[1], RANGE_S);
	my_zone.prefix = atoi(my_row[2]);
	strncpy(my_zone.net_start, my_row[3], RANGE_S);
	strncpy(my_zone.net_finish, my_row[4], RANGE_S);
	line = my_row[5];
	my_zone.start_ip = (unsigned int) strtol(line, NULL, 10);
	line = my_row[6];
	my_zone.end_ip = (unsigned int) strtol(line, NULL, 10);
	strncpy(my_zone.pri_dns, my_row[7], RBUFF_S);
	strncpy(my_zone.sec_dns, my_row[8] ? my_row[8] : "NULL", RBUFF_S);
	my_zone.serial = atoi(my_row[9]);
	my_zone.refresh = atoi(my_row[10]);
	my_zone.retry = atoi(my_row[11]);
	my_zone.expire = atoi(my_row[12]);
	my_zone.ttl = atoi(my_row[13]);
	strncpy(my_zone.valid, my_row[14], RBUFF_S);
	my_zone.owner = atoi(my_row[15]);
	strncpy(my_zone.updated, my_row[16], RBUFF_S);
	if (!(tmp = strstr(my_zone.pri_dns, ch))) {
		fprintf(stderr, "strstr in fill_rev_zone_data failed!\n");
		exit(NO_DELIM);
	} else {
		strncpy(my_zone.hostmaster, hostmaster, RANGE_S);
		len = strlen(tmp);
		strncat(my_zone.hostmaster, tmp, len);
	}
	return my_zone;
}

size_t create_rev_zone_header(rev_zone_info_t zone_info, char *rout)
{
	char *tmp;
	char ch;
	size_t offset;
	offset = 0;
	tmp = malloc(RBUFF_S * sizeof(char));
	sprintf(tmp, "$TTL %d\n@\t\tIN SOA\t%s.\t%s. (\n",
		zone_info.ttl, zone_info.pri_dns, zone_info.hostmaster);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	sprintf(tmp, "\t\t\t%d\t; Serial\n\t\t\t%d\t\t; Refresh\n\t\t\t%d\t\t; Retry\n",
		zone_info.serial, zone_info.refresh, zone_info.retry);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	sprintf(tmp, "\t\t\t%d\t\t; Expire\n\t\t\t%d)\t\t; Negative Cache TTL\n",
		zone_info.expire, zone_info.ttl);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	offset = strlen(zone_info.pri_dns);
	ch = zone_info.pri_dns[offset - 1];
	/* check for trainling . in NS record */
	if (ch == '.')
		sprintf(tmp, "\t\t\tNS\t%s\n", zone_info.pri_dns);
	else
		sprintf(tmp, "\t\t\tNS\t%s.\n", zone_info.pri_dns);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	/* check for secondary DNS record */
	if ((strcmp(zone_info.sec_dns, "NULL")) == 0) {
		;
	} else {
		offset = strlen(zone_info.sec_dns);
		ch = zone_info.sec_dns[offset - 1];
		if (ch == '.') {
			sprintf(tmp, "\t\t\tNS\t%s\n;\n", zone_info.sec_dns);
		} else {
			sprintf(tmp, "\t\t\tNS\t%s.\n;\n", zone_info.sec_dns);
		}
	}
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	offset = strlen(rout);
	free(tmp);
	return offset;
}

rev_record_row_t get_rev_row (MYSQL_ROW my_row)
{
	rev_record_row_t rev_row;
	
	strncpy(rev_row.host, my_row[0], RBUFF_S);
	strncpy(rev_row.dest, my_row[1], RBUFF_S);
	return rev_row;
}

void add_rev_records(char *rout, rev_record_row_t my_row)
{
	char *tmp;
	tmp = malloc(RBUFF_S * sizeof(char));
	sprintf(tmp, "%s\t\tPTR\t%s\n", my_row.host, my_row.dest);
	strncat(rout, tmp, RBUFF_S);
	free(tmp);
}
/** This function needs proper testing on range other than /24. This will most likely not
 ** be able to handle anything not on a /8, /16, or /24 boundary.
 */
void get_in_addr_string(char *in_addr, char range[])
{
	size_t len;
	char *tmp, *line;
	int c, i;
	
	c = '.';
	i = 0;
	len = strlen(range);
	line = malloc((len + 1) * sizeof(char));
	strncpy(line, range, len);
	tmp = strrchr(line, c);
	*tmp = '\0';		/* Get rid of training .0 */
	while ((tmp = strrchr(line, c))) {
		++tmp;
		len = strlen(tmp);
		strncat(in_addr, tmp, len);
		strncat(in_addr, ".", 1);
		--tmp;
		*tmp = '\0';
		i++;
	}
	if (i == 0) {
		strncat(in_addr, ".", 1);
	}
	len = strlen(line);
	strncat(in_addr, line, len);
	tmp = line;
	sprintf(tmp, ".in-addr.arpa");
	len = strlen(tmp);
	strncat(in_addr, tmp, len);
	free(line);
}
/** Get the reverse zone ID from the database. Return -1 on error
 ** Perhaps should add more error codes, but for now we assume invalid domain
 **/
int get_rev_id (char *domain)
{
	char *queryp;
	const char dquery[BUFF_S];
	int retval;
	
	queryp = dquery;
	sprintf(queryp, "SELECT * FROM rev_zones WHERE net_range = '%s'",
		domain);
	
	retval = -1;
	return retval;
}