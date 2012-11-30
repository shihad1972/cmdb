/* rev_zone.c:
 *
 * Contains functions that are needed to import and export information
 * from / to the database about the reverse zones, and to also write out
 * the reverse zone files and also the BIND reverse zone configuration file.
 * 
 * Part of the DNSA  program
 * 
 * (C) Iain M Conochie 2012
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "reverse.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"
#include "checks.h"

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
	size_t len;
	char *queryp, *error_code;
	const char *dquery, *error_str;
	int retval;
	
	retval = 0;
	
	/* 
	 * If domain is all, then we are listing all domains. Return with 0
	 * If domain is none, then we are writing config file. Return with 0 
	 */
	len = strlen(domain);
	if ((strncmp(domain, "all", len)) == 0 || (strncmp(domain, "none", len)) == 0 )
		return retval;
	retval = validate_user_input(domain, IP_REGEX);
	if (retval < 0) {
		printf("User input not valid!\n");
		return retval;
	}
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in get_rev_id");
	error_str = error_code; 
	if (!(queryp = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "queryp in get_rev_id");
	
	dquery = queryp;
	sprintf(queryp,
		"SELECT rev_zone_id FROM rev_zones WHERE net_range = '%s'", domain);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dquery);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	
	snprintf(error_code, CONF_S, "%s", domain);
	/* Check for only 1 result */
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		free(error_code);
		free(queryp);
		error_str = 0;
		dquery = 0;
		mysql_library_end();
		report_error(NO_DOMAIN, domain);
	} else if (dnsa_rows > 1) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		free(error_code);
		free(queryp);
		error_str = 0;
		dquery = 0;
		mysql_library_end();
		report_error(MULTI_DOMAIN, domain);
	}
	dnsa_row = mysql_fetch_row(dnsa_res);
	retval = atoi(dnsa_row[0]);
	mysql_free_result(dnsa_res);
	mysql_close(&dnsa);
	free(error_code);
	free(queryp);
	error_str = 0;
	dquery = 0;
	mysql_library_end();
	return retval;
}

int wrzf(int reverse, dnsa_config_t *dc)
{
	FILE *cnf;
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	rev_zone_info_t rev_zone_info, *rzi;
	rev_record_row_t rev_row;
	my_ulonglong dnsa_rows;
	int error, i;
	char *zonefn, *rout, *dquery, *domain;
	const char *dnsa_query, *net_range;
	
	
	if (!(dquery = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dquery in wrzf");
	if (!(zonefn = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefn in wrzf");
	if (!(rout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "rout in wrzf");
	if (!(domain = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "domain in wrzf");
	
	dnsa_query = dquery;
	rzi = &rev_zone_info;
	net_range = rzi->net_range;
	
	/* Initialise MYSQL connection and query */
	dnsa_mysql_init(dc, &dnsa);
	
	sprintf(dquery, "SELECT * FROM rev_zones WHERE rev_zone_id = '%d'", reverse);
	cmdb_mysql_query(&dnsa, dnsa_query);
	
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "Reverse zone id %d not found\n", reverse);
		return NO_DOMAIN;
	} else if (dnsa_rows > 1) {
		fprintf(stderr, "Multiple rows found for reverse zone id %d\n", reverse);
		return MULTI_DOMAIN;
	}
	
	/* Get the information for the reverse zone */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_zone_info = fill_rev_zone_data(dnsa_row);
	}
	mysql_free_result(dnsa_res);
	/* Start the output string with the zonefile header */
	create_rev_zone_header(rev_zone_info, rout);
	
	sprintf(dquery, "SELECT host, destination FROM rev_records WHERE rev_zone = '%d'", reverse);
	error = mysql_query(&dnsa, dnsa_query);
	
	if ((error != 0)) {
		fprintf(stderr, "Rev record query unsuccessful: error code %d\n", error);
		return MY_QUERY_FAIL;
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		fprintf(stderr, "Cannot store result set\n");
		return MY_STORE_FAIL;
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "No reverse records for zone %d\n", reverse);
		return NO_RECORDS;
	}
	
	/* Add the reverse zone records */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_row = get_rev_row(dnsa_row);
		add_rev_records(rout, rev_row);
	}
	mysql_free_result(dnsa_res);
	/* Build the config filename from config values */
	create_rev_zone_filename(domain, net_range, dc);
	
	/* Write out the reverse zone to the zonefile */
	if (!(cnf = fopen(domain, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing\n", domain);
		return FILE_O_FAIL;
	} else {
		fputs(rout, cnf);
		fclose(cnf);
	}
	
	for (i=0; i < FILE_S; i++)	/* zero rout file output buffer */
		*(rout + i) = '\0';
	
	/* Check if the zonefile is valid. If so update the DB to say zone
	 * is valid. */
	get_in_addr_string(zonefn, rzi->net_range);
	check_rev_zone(zonefn, domain, dc);

	sprintf(dquery, "UPDATE rev_zones SET valid = 'yes', updated = 'no' WHERE rev_zone_id = %d", reverse);
	error = mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if ((dnsa_rows == 1)) {
		fprintf(stderr, "Rev Zone id %d set to valid in DB\n", reverse);
	} else if ((dnsa_rows == 0)) {
		if ((error == 0)) {
			fprintf(stderr, "Rev zone id %d already valid in DB\n", reverse);
		} else {
			fprintf(stderr, "Rev Zone id %d not validated in DB\n", reverse);
			fprintf(stderr, "Error code from query is: %d\n", error);
		}
	} else {
		fprintf(stderr, "More than one zone update?? Multiple ID's %d??\n",
			reverse);
	}
	
	mysql_close(&dnsa);
	mysql_library_end();
	free(zonefn);
	free(rout);
	free(dquery);
	free(domain);
	return 0;
}

int wrcf(dnsa_config_t *dc)
{
	FILE *cnf;
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t len;
	my_ulonglong dnsa_rows;
	int error;
	char *rout, *dnsa_line, *zonefile, *tmp, *tmp2, *error_code;
	const char *dnsa_query, *syscom, *error_str, *domain;
	
	domain = "or reverse zone that is valid ";
	
	if (!(rout = calloc(FILE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "rout in wrcf");
	if (!(dnsa_line = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "dnsa_line in wrcf");
	if (!(tmp = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in wrcf");
	if (!(tmp2 = calloc(TBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp2 in wrcf");
	if (!(zonefile = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "zonefile in wrcf");
	if (!(error_code = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in wrcf");
	
	error_str = error_code;
	dnsa_query = dnsa_line;
	
	/* Initilaise MYSQL connection and query */
	sprintf(dnsa_line, "SELECT net_range FROM rev_zones");
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		report_error(NO_DOMAIN, domain);
	}
	
	/* From each DB row, create the config line for the reverse zone */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		get_in_addr_string(tmp2, dnsa_row[0]);
		sprintf(zonefile, "%s%s", dc->dir, dnsa_row[0]);
		if (!(cnf = fopen(zonefile, "r"))) {
			fprintf(stderr, "Cannot access zonefile %s\n", zonefile);
		} else {
			sprintf(tmp, "zone \"%s\" {\n\t\t\ttype master;\n\t\t\tfile \"%s%s\";\n\t\t};\n",
				tmp2, dc->dir, dnsa_row[0]);
			len = strlen(tmp);
			strncat(rout, tmp, len);
			fclose(cnf);
		}
	}
	mysql_free_result(dnsa_res);
	/* Write the config file.
	 * Check it and if successful reload bind */
	sprintf(zonefile, "%s%s", dc->bind, dc->rev);
	if (!(cnf = fopen(zonefile, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing!\n", zonefile);
		exit(FILE_O_FAIL);
	} else {
		fputs(rout, cnf);
		fclose(cnf);
	}
	sprintf(tmp, "%s %s", dc->chkc, zonefile);
	syscom = tmp;
	error = system(syscom);
	if ((error != 0)) {
		fprintf(stderr, "Check of config file failed. Error code %d\n",
			error);
	} else {
		sprintf(tmp, "%s reload", dc->rndc);
		error = system(syscom);
		if ((error != 0)) {
			fprintf(stderr, "Reload failed with error code %d\n",
				error);
		}
	}
	
	free(zonefile);
	free(tmp);
	free(tmp2);
	free(rout);
	free(dnsa_line);
	free(error_code);
	mysql_close(&dnsa);
	mysql_library_end();
	return 0;
}
