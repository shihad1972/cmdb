/* rev_zone.c:
 *
 * Contains functions that are needed to import and export information
 * from / to the database about the reverse zones, and to also write out
 * the reverse zone files and also the BIND reverse zone configuration file.
 * 
 * (C) Iain M Conochie 2012 <iain@ailsatech.net>
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"
#include "reverse.h"

/** Function to fill a struct with results from the DB query
 ** No error checking on fields
 */
rev_zone_info_t fill_rev_zone_data(MYSQL_ROW my_row)
{
	size_t len;
	const char *line, *delim, *hostmaster;
	char *tmp;
	rev_zone_info_t my_zone;
	
	delim = ".";
	hostmaster = "hostmaster";
	
	my_zone.rev_zone_id = atoi(my_row[0]);
	strncpy(my_zone.net_range, my_row[1], RANGE_S);
	my_zone.prefix = atoi(my_row[2]);
	strncpy(my_zone.net_start, my_row[3], RANGE_S);
	strncpy(my_zone.net_finish, my_row[4], RANGE_S);
	line = my_row[5];
	my_zone.start_ip = (unsigned int) strtoul(line, NULL, 10);
	line = my_row[6];
	my_zone.end_ip = (unsigned int) strtoul(line, NULL, 10);
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
	
	if (!(tmp = strstr(my_zone.pri_dns, delim))) {
		fprintf(stderr, "strstr in fill_rev_zone_data failed!\n");
		exit(NO_DELIM);
	} else {
		strncpy(my_zone.hostmaster, hostmaster, RANGE_S);
		len = strlen(tmp);
		strncat(my_zone.hostmaster, tmp, len);
	}
	return my_zone;
}

void create_rev_zone_header(rev_zone_info_t zone_info, char *rout)
{
	char *tmp;
	char ch;
	size_t offset;
	
	if (!(tmp = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp");

	offset = 0;

	sprintf(tmp, "$TTL %d\n@\t\tIN SOA\t%s.\t%s. (\n",
		zone_info.ttl, zone_info.pri_dns, zone_info.hostmaster);
	offset = strlen(tmp);
	strncpy(rout, tmp, offset);
	sprintf(tmp, "\t\t\t%d\t; Serial\n\t\t\t%d\t\t; Refresh\n\t\t\t%d\t\t; Retry\n",
		zone_info.serial, zone_info.refresh, zone_info.retry);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	sprintf(tmp, "\t\t\t%d\t\t; Expire\n\t\t\t%d)\t\t; Negative Cache TTL\n",
		zone_info.expire, zone_info.ttl);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	offset = strlen(zone_info.pri_dns);
	
	/* check for trainling . in NS record */
	ch = zone_info.pri_dns[offset - 1];
	if (ch == '.')
		sprintf(tmp, "\t\t\tNS\t%s\n", zone_info.pri_dns);
	else
		sprintf(tmp, "\t\t\tNS\t%s.\n", zone_info.pri_dns);
	
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	/* check for secondary DNS record */
	if ((strcmp(zone_info.sec_dns, "NULL")) == 0) {
		;	/* No secondary NS record so do nothing */
	} else {
		offset = strlen(zone_info.sec_dns);
		/* check for trainling . in NS record */
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
}

void check_rev_zone(char *filename, char *domain, dnsa_config_t *dc)
{
	char *command;
	const char *syscom;
	int error;
	
	if (!(command = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "command in check_rev_zone");
	syscom = command;
	
	sprintf(command, "%s %s %s", dc->chkz, filename, domain);
	error = system(syscom);
	if (error != 0)
		report_error(CHKZONE_FAIL, domain);
	else
		printf("check of zone %s ran successfully\n", domain);
	free(command);
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
	if (!(tmp = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in add_rev_records");
	sprintf(tmp, "%s\t\tPTR\t%s\n", my_row.host, my_row.dest);
	strncat(rout, tmp, RBUFF_S);
	free(tmp);
}

void create_rev_zone_filename (char *dom, const char *nr, dnsa_config_t *dc)
{
	size_t len;
	
	len = strlen(dc->dir);
	strncpy(dom, dc->dir, len);
	len = strlen(nr);
	strncat(dom, nr, len);
	
}
/** This function needs proper testing on range other than /24. This will most likely not
 ** be able to handle anything not on a /8, /16, or /24 boundary.
 */
void get_in_addr_string(char *in_addr, char range[])
{
	size_t len;
	char *tmp, *line;
	char louisa[] = ".in-addr.arpa";
	int c, i;
	
	c = '.';
	i = 0;
	tmp = 0;
	len = strlen(range);
	len++; /* Got to remember the terminating \0 :) */
	if (!(line = malloc((len) * sizeof(char))))
		report_error(MALLOC_FAIL, "line");
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
	sprintf(tmp, "%s", louisa);
	len = strlen(tmp);
	strncat(in_addr, tmp, len);
	free(line);
}
/** Get the reverse zone ID from the database. Return -1 on error
 ** Perhaps should add more error codes, but for now we assume invalid domain
 **/
int get_rev_id (char *domain, dnsa_config_t *dc)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	char *queryp, *error_code;
	const char *dquery, *unix_socket, *error_str;
	int retval, error;
	
	retval = 0;
	
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in get_rev_id");
	error_str = error_code; 
	if (!(queryp = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "queryp in get_rev_id");
	
	unix_socket = dc->socket;
	dquery = queryp;
	sprintf(queryp,
		"SELECT rev_zone_id FROM rev_zones WHERE net_range = '%s'", domain);

	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag )))
		report_error(MY_CONN_FAIL, mysql_error(&dnsa));
	
	error = mysql_query(&dnsa, dquery);
	snprintf(error_code, CONF_S, "%d", error);
	
	if ((error != 0))
		report_error(MY_QUERY_FAIL, error_str);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	
	snprintf(error_code, CONF_S, "%s", domain);
	/* Check for only 1 result */
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0))
		report_error(NO_DOMAIN, domain);
	else if (dnsa_rows > 1)
		report_error(MULTI_DOMAIN, domain);
	
	dnsa_row = mysql_fetch_row(dnsa_res);
	retval = atoi(dnsa_row[0]);
	return retval;
}
