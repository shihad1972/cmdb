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
#include <time.h>
#include <arpa/inet.h>
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
void fill_rev_zone_data(MYSQL_ROW my_row, rev_zone_info_t *my_zone)
{
	size_t len;
	const char *delim, *hostmaster;
	char *tmp;
	
	delim = ".";
	hostmaster = "hostmaster";
	
	my_zone->rev_zone_id = atoi(my_row[0]);
	strncpy(my_zone->net_range, my_row[1], RANGE_S);
	my_zone->prefix = strtoul(my_row[2], NULL, 10);
	strncpy(my_zone->net_start, my_row[3], RANGE_S);
	strncpy(my_zone->net_finish, my_row[4], RANGE_S);
	my_zone->start_ip = strtoul(my_row[5], NULL, 10);
	my_zone->end_ip = strtoul(my_row[6], NULL, 10);
	strncpy(my_zone->pri_dns, my_row[7], RBUFF_S);
	strncpy(my_zone->sec_dns, my_row[8] ? my_row[8] : "NULL", RBUFF_S);
	my_zone->serial = strtoul(my_row[9], NULL, 10);
	my_zone->refresh = strtoul(my_row[10], NULL, 10);
	my_zone->retry = strtoul(my_row[11], NULL, 10);
	my_zone->expire = strtoul(my_row[12], NULL, 10);
	my_zone->ttl = strtoul(my_row[13], NULL, 10);
	strncpy(my_zone->valid, my_row[14], RBUFF_S);
	my_zone->owner = atoi(my_row[15]);
	strncpy(my_zone->updated, my_row[16], RBUFF_S);
	
	if (!(tmp = strstr(my_zone->pri_dns, delim))) {
		fprintf(stderr, "strstr in fill_rev_zone_data failed!\n");
		exit(NO_DELIM);
	} else {
		strncpy(my_zone->hostmaster, hostmaster, RANGE_S);
		len = strlen(tmp);
		strncat(my_zone->hostmaster, tmp, len);
	}
}

void create_rev_zone_header(rev_zone_info_t *zone_info, char *rout)
{
	char *tmp;
	char ch;
	size_t offset;
	
	if (!(tmp = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "tmp");

	offset = 0;

	sprintf(tmp, "$TTL %ld\n@\t\tIN SOA\t%s.\t%s. (\n",
		zone_info->ttl, zone_info->pri_dns, zone_info->hostmaster);
	offset = strlen(tmp);
	strncpy(rout, tmp, offset);
	sprintf(tmp, "\t\t\t%ld\t; Serial\n\t\t\t%ld\t\t; Refresh\n\t\t\t%ld\t\t; Retry\n",
		zone_info->serial, zone_info->refresh, zone_info->retry);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	sprintf(tmp, "\t\t\t%ld\t\t; Expire\n\t\t\t%ld)\t\t; Negative Cache TTL\n",
		zone_info->expire, zone_info->ttl);
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	offset = strlen(zone_info->pri_dns);
	
	/* check for trainling . in NS record */
	ch = zone_info->pri_dns[offset - 1];
	if (ch == '.')
		sprintf(tmp, "\t\t\tNS\t%s\n", zone_info->pri_dns);
	else
		sprintf(tmp, "\t\t\tNS\t%s.\n", zone_info->pri_dns);
	
	offset = strlen(tmp);
	strncat(rout, tmp, offset);
	/* check for secondary DNS record */
	if ((strcmp(zone_info->sec_dns, "NULL")) == 0) {
		;	/* No secondary NS record so do nothing */
	} else {
		offset = strlen(zone_info->sec_dns);
		/* check for trainling . in NS record */
		ch = zone_info->sec_dns[offset - 1];
		if (ch == '.') {
			sprintf(tmp, "\t\t\tNS\t%s\n;\n", zone_info->sec_dns);
		} else {
			sprintf(tmp, "\t\t\tNS\t%s.\n;\n", zone_info->sec_dns);
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
	len++;/* Got to remember the terminating \0 :) */
	if (!(line = malloc((len) * sizeof(char))))
		report_error(MALLOC_FAIL, "line");
	snprintf(line, len, "%s", range);
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
	len = strlen(line);
	strncat(in_addr, line, len);
	len = strlen(louisa);
	strncat(in_addr, louisa, len);
	free(line);
}

void get_in_addr_string2(char *in_addr, char range[], unsigned long int prefix)
{
	size_t len;
	char *tmp, *line, *classless;
	char louisa[] = ".in-addr.arpa";
	int c, i;
	
	c = '.';
	i = 0;
	tmp = 0;
	len = strlen(range);
	len++;/* Got to remember the terminating \0 :) */
	if (!(line = malloc((len) * sizeof(char))))
		report_error(MALLOC_FAIL, "line in get_in_addr_string2");
	if (!(classless = calloc(CONF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "classless in get_in_addr_string2");
	
	snprintf(line, len, "%s", range);
/*	tmp = strrchr(line, c);
	*tmp = '\0';		 Get rid of training .0 */
	switch (prefix) {
		case 24:
			tmp = strrchr(line, c);
			*tmp = '\0';
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		case 16:
			tmp = strrchr(line, c);
			*tmp = '\0';
			tmp = strrchr(line, c);
			*tmp = '\0';
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		case 8:
			tmp = strrchr(line, c);
			*tmp = '\0';
			tmp = strrchr(line, c);
			*tmp = '\0';
			tmp = strrchr(line, c);
			*tmp = '\0';
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		case 25: case 26: case 27: case 28: case 29: case 30:
		case 31: case 32:
			tmp = strrchr(line, c);
			++tmp;
			len = strlen(tmp);
			strncat(in_addr, tmp, len);
			strncat(in_addr, ".", 1);
			--tmp;
			*tmp = '\0';
			snprintf(classless, CONF_S, "/%lu.", prefix);
			len = strlen(classless);
			strncat(in_addr, classless, len);
			while ((tmp = strrchr(line, c))) {
				++tmp;
				len = strlen(tmp);
				strncat(in_addr, tmp, len);
				strncat(in_addr, ".", 1);
				--tmp;
				*tmp = '\0';
				i++;
			}
			break;
		default:
			break;
	}
	len = strlen(line);
	strncat(in_addr, line, len);
	len = strlen(louisa);
	strncat(in_addr, louisa, len);
	free(line);
}
/** Get the reverse zone ID from the database. Return -1 on error
 ** Perhaps should add more error codes, but for now we assume invalid domain
 **/
int get_rev_id (char *domain, dnsa_config_t *dc, short int action)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	size_t len;
	char *queryp;
	const char *dquery;
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
	if (action == ADD_ZONE)
		return retval;	/* No Need to get ID to add a zone */
	if (!(queryp = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "queryp in get_rev_id");
	
	dquery = queryp;
	sprintf(queryp,
		"SELECT rev_zone_id FROM rev_zones WHERE net_range = '%s'", domain);
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dquery);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	
	/* Check for only 1 result */
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		free(queryp);
		dquery = 0;
		mysql_library_end();
		report_error(NO_DOMAIN, domain);
	} else if (dnsa_rows > 1) {
		mysql_free_result(dnsa_res);
		mysql_close(&dnsa);
		free(queryp);
		dquery = 0;
		mysql_library_end();
		report_error(MULTI_DOMAIN, domain);
	}
	dnsa_row = mysql_fetch_row(dnsa_res);
	retval = atoi(dnsa_row[0]);
	mysql_free_result(dnsa_res);
	mysql_close(&dnsa);
	free(queryp);
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
	rev_zone_info_t *rzi;
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
	if (!(rzi = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rzi in wrzf");
	
	dnsa_query = dquery;
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
		fill_rev_zone_data(dnsa_row, rzi);
	}
	mysql_free_result(dnsa_res);
	/* Start the output string with the zonefile header */
	create_rev_zone_header(rzi, rout);
	
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
	free(rzi);
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
	unsigned long int prefix;
	char *rout, *dnsa_line, *zonefile, *tmp, *tmp2;
	const char *dnsa_query, *syscom, *domain;
	
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
	
	dnsa_query = dnsa_line;
	
	/* Initilaise MYSQL connection and query */
	sprintf(dnsa_line, "SELECT net_range, prefix FROM rev_zones");
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		report_error(NO_DOMAIN, domain);
	}
	
	/* From each DB row, create the config line for the reverse zone */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		snprintf(tmp2, CH_S, "%s", "");
		prefix = strtoul(dnsa_row[1], NULL, 10);
		get_in_addr_string2(tmp2, dnsa_row[0], prefix);
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
	mysql_close(&dnsa);
	mysql_library_end();
	return 0;
}

/** This function needs proper testing on range other than /24. This will most likely not
 ** be able to handle anything not on a /8, /16, or /24 boundary.
 */
void add_rev_zone(char *domain, dnsa_config_t *dc, unsigned long int prefix)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	rev_zone_info_t *rzi;
	char *query;
	const char *dnsa_query;
	
	if (!(rzi = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rzi in add_rev_zone");
	if (!(query = calloc(BUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in add_rev_zone");
	
	init_dnsa_rev_zone(rzi);
	rzi->prefix = prefix;
	snprintf(rzi->net_range, RANGE_S, "%s", domain);
	fill_range_info(rzi);
	dnsa_query = query;
	
	snprintf(query, BUFF_S,
"SELECT config, value FROM configuration WHERE config LIKE 'dnsa_%%'");
	dnsa_mysql_init(dc, &dnsa);
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		report_error(NO_ZONE_CONFIGURATION, domain);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		add_rev_config(dnsa_row, rzi);
	}
	mysql_free_result(dnsa_res);
	snprintf(query, BUFF_S,
"SELECT config, value FROM configuration WHERE config = 'hostmaster'");
	cmdb_mysql_query(&dnsa, dnsa_query);
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		report_error(MY_STORE_FAIL, mysql_error(&dnsa));
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) != 1)) {
		report_error(NO_ZONE_CONFIGURATION, domain);
	}
	dnsa_row = mysql_fetch_row(dnsa_res);
	snprintf(rzi->hostmaster, RBUFF_S, "%s", dnsa_row[1]);
	mysql_free_result(dnsa_res);
	update_rev_zone_serial(rzi);
	snprintf(query, BUFF_S,
"INSERT INTO rev_zones (net_range, prefix, net_start, net_finish, start_ip, \
finish_ip, pri_dns, sec_dns, serial, refresh, retry, expire, ttl) VALUES ('%s'\
, '%lu', '%s', '%s', %lu, %lu, '%s', '%s', %lu, %lu, %lu, %lu, %lu)", \
rzi->net_range, rzi->prefix, rzi->net_start, rzi->net_finish, rzi->start_ip, \
rzi->end_ip, rzi->pri_dns, rzi->sec_dns, rzi->serial, rzi->refresh, \
rzi->retry, rzi->expire, rzi->ttl);
	cmdb_mysql_query(&dnsa, dnsa_query);
	dnsa_rows = mysql_affected_rows(&dnsa);
	if (dnsa_rows == 1) {
		fprintf(stderr, "New zone %s added\n", rzi->net_range);
	} else {
		mysql_close(&dnsa);
		mysql_library_end();
		free(query);
		report_error(CANNOT_INSERT_ZONE, rzi->net_range);
	}
	mysql_close(&dnsa);
	mysql_library_end();
	free(rzi);
	free(query);
}

void add_rev_config(MYSQL_ROW my_row, rev_zone_info_t *zone)
{
	if ((strncmp(my_row[0], "dnsa_pri_ns", RBUFF_S)) == 0)
		snprintf(zone->pri_dns, RBUFF_S, "%s",  my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_sec_ns", RBUFF_S)) == 0)
		snprintf(zone->sec_dns, RBUFF_S, "%s",  my_row[1]);
	else if ((strncmp(my_row[0], "dnsa_refresh", RBUFF_S)) == 0)
		zone->refresh = strtoul(my_row[1], NULL, 10);
	else if ((strncmp(my_row[0], "dnsa_retry", RBUFF_S)) == 0)
		zone->retry = strtoul(my_row[1], NULL, 10);
	else if ((strncmp(my_row[0], "dnsa_expire", RBUFF_S)) == 0)
		zone->expire = strtoul(my_row[1], NULL, 10);
	else if ((strncmp(my_row[0], "dnsa_ttl", RBUFF_S)) == 0)
		zone->ttl = strtoul(my_row[1], NULL, 10);
}

void init_dnsa_rev_zone(rev_zone_info_t *rev_zone)
{
	rev_zone->rev_zone_id = 0;
	rev_zone->prefix = 0;
	rev_zone->owner = 0;
	rev_zone->start_ip = 0;
	rev_zone->end_ip = 0;
	rev_zone->serial = 0;
	rev_zone->refresh = 0;
	rev_zone->retry = 0;
	rev_zone->expire = 0;
	rev_zone->ttl = 0;
	snprintf(rev_zone->net_range, COMM_S, "NULL");
	snprintf(rev_zone->net_start, COMM_S, "NULL");
	snprintf(rev_zone->net_finish, COMM_S, "NULL");
	snprintf(rev_zone->pri_dns, COMM_S, "NULL");
	snprintf(rev_zone->sec_dns, COMM_S, "NULL");
	snprintf(rev_zone->valid, COMM_S, "NULL");
	snprintf(rev_zone->updated, COMM_S, "NULL");
	snprintf(rev_zone->hostmaster, COMM_S, "NULL");
}
void init_dnsa_rev_record(rev_record_row_t *rev_record)
{
	snprintf(rev_record->host, COMM_S, "NULL");
	snprintf(rev_record->dest, COMM_S, "NULL");
	rev_record->next = 0;
}

unsigned long int get_net_range(unsigned long int prefix)
{
	unsigned long int range;
	range = 256ul * 256ul * 256ul * 256ul;
	range = range >> prefix;
	return range;
}

void fill_range_info(rev_zone_info_t *rzi)
{
	uint32_t ip_addr;
	unsigned long int range;
	char *address;
	
	if (!(address = calloc(RANGE_S, sizeof(char))))
		report_error(MALLOC_FAIL, "Address in fill_dnsa_rev_zone");
	
	snprintf(rzi->net_start, RANGE_S, "%s", rzi->net_range);
	inet_pton(AF_INET, rzi->net_range, &ip_addr);
	ip_addr = htonl(ip_addr);
	rzi->start_ip = ip_addr;
	range = get_net_range(rzi->prefix);
	range--;
	rzi->end_ip = ip_addr + range;
	ip_addr = htonl((uint32_t)rzi->end_ip);
	inet_ntop(AF_INET, &ip_addr, address, RANGE_S);
	snprintf(rzi->net_finish, RANGE_S, "%s", address);
}

void print_rev_zone_info(rev_zone_info_t *rzi)
{
	printf("rev_zone-id: %d\n", rzi->rev_zone_id);
	printf("prefix: %lu\n", rzi->prefix);
	printf("owner: %d\n", rzi->owner);
	printf("start_ip: %lu\n", rzi->start_ip);
	printf("end_ip: %lu\n", rzi->end_ip);
	printf("serial: %lu\n", rzi->serial);
	printf("refresh: %lu\n", rzi->refresh);
	printf("retry: %lu\n", rzi->retry);
	printf("expire: %lu\n", rzi->expire);
	printf("ttl: %lu\n", rzi->ttl);
	printf("net_range: %s\n", rzi->net_range);
	printf("net_start: %s\n", rzi->net_start);
	printf("net_finish: %s\n", rzi->net_finish);
	printf("pri_dns: %s\n", rzi->pri_dns);
	printf("sec_dns: %s\n", rzi->sec_dns);
	printf("valid: %s\n", rzi->valid);
	printf("updated: %s\n", rzi->updated);
	printf("hostmaster: %s\n", rzi->hostmaster);
}

void update_rev_zone_serial(rev_zone_info_t *zone)
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

int build_rev_zone(dnsa_config_t *dc, char *domain)
{
	int retval;
	rev_zone_info_t *rev_zone;
	rev_record_row_t *records;

	if (!(rev_zone = malloc(sizeof(rev_zone_info_t))))
		report_error(MALLOC_FAIL, "rev_zone in build_rev_zone");
	if (!(records = malloc(sizeof(rev_record_row_t))))
		report_error(MALLOC_FAIL, "records in build_rev_zone");

	retval = 0;
	init_dnsa_rev_zone(rev_zone);
	init_dnsa_rev_record(records);
	rev_zone->rev_zone_id = get_rev_id(domain, dc, BUILD_REV);
	snprintf(rev_zone->net_range, RANGE_S, "%s", domain);
	if ((retval = get_rev_zone_info(dc, rev_zone)) > 0) {
		free(rev_zone);
		return retval;
	}
	if ((retval = get_rev_zone_records(dc, rev_zone, records)) > 0) {
		delete_A_records(records);
		free(rev_zone);
		return retval;
	}
	if ((retval = convert_rev_records(records, rev_zone->prefix)) > 0) {
		delete_A_records(records);
		free(rev_zone);
		return retval;
	}
	if ((retval = insert_rev_records(dc, records, rev_zone)) > 0) {
		delete_A_records(records);
		free(rev_zone);
		return retval;
	}
	printf("Reverse zone %s added into database\n", domain);
	delete_A_records(records);
	free(rev_zone);
	return retval;
}

int get_rev_zone_info(dnsa_config_t *dc, rev_zone_info_t *rev)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	char *query;
	const char *dnsa_query;
	int retval;

	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in build_rev_zone");
	dnsa_query = query;
	dnsa_mysql_init(dc, &dnsa);
	snprintf(query, RBUFF_S,
"SELECT * FROM rev_zones WHERE rev_zone_id = '%d'", rev->rev_zone_id);
	cmdb_mysql_query(&dnsa, dnsa_query);

	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, query);
		return MY_STORE_FAIL;
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
		return NO_DOMAIN;
	} else if (dnsa_rows > 1) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
		return MULTI_DOMAIN;
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		fill_rev_zone_data(dnsa_row, rev);
	}
	cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
	return retval;
}

int get_rev_zone_records(dnsa_config_t *dc, rev_zone_info_t *rev, rev_record_row_t *records)
{
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	my_ulonglong dnsa_rows;
	char *query;
	const char *dnsa_query;
	int retval;
	dnsa_config_and_reverse *config;

	retval = 0;
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in get_rev_zone_records");
	if (!(config = malloc(sizeof(dnsa_config_and_reverse))))
		report_error(MALLOC_FAIL, "config in get_rev_zone_records");
	config->dc = dc;
	config->record = records;
	config->zone = rev;
	dnsa_query = query;
	dnsa_mysql_init(dc, &dnsa);
	snprintf(query, RBUFF_S, "SELECT name, host, destination FROM \
records r, zones z WHERE z.id = r.zone AND type = 'A' ORDER BY destination");
	cmdb_mysql_query(&dnsa, dnsa_query);

	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		cmdb_mysql_clean(&dnsa, query);
		free(config);
		return MY_STORE_FAIL;
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
		free(config);
		return NO_FORWARD_RECORDS;
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		if ((retval = store_valid_a_record(config, dnsa_row)) != 0) {
			retval++;
			continue;
		}
	}
	if (retval > 0)
		printf("There were %d records not added\n", retval);
	free(config);
	cmdb_mysql_clean_full(dnsa_res, &dnsa, query);
	return retval;
}
int store_valid_a_record(dnsa_config_and_reverse *config, MYSQL_ROW row)
{
	int retval;
	uint32_t ip_addr;
	rev_record_row_t *record = config->record;
	rev_zone_info_t *zone = config->zone;

	retval = 0;
	inet_pton(AF_INET, row[2], &ip_addr);
	ip_addr = htonl(ip_addr);
	if (ip_addr < zone->start_ip || ip_addr > zone->end_ip)
		return retval;
	if ((check_if_a_record_exists(record, row[2])) != 0) {
		return retval;
	}
	if ((retval = store_a_record(record, row)) != 0) {
		return retval;
	}
	return retval;
}

int store_a_record(rev_record_row_t *record, MYSQL_ROW row)
{
	rev_record_row_t *new, *saved;

	saved = record;
	if (!(new = malloc(sizeof(rev_record_row_t))))
		report_error(MALLOC_FAIL, "new in store_a_record");
	/* Check for head node */
	if ((strncmp(record->dest, "NULL", COMM_S)) == 0) {
		free(new);
		new = record;
	}
	if ((strncmp(row[1], "@", CH_S)) == 0)
		snprintf(new->dest, RBUFF_S, "%s.", row[0]);
	else
		snprintf(new->dest, RBUFF_S, "%s.%s.", row[1], row[0]);
	snprintf(new->host, RBUFF_S, "%s", row[2]);
	new->next = 0;
	if (new == record) {
		return 0;
	} else {
		while (saved->next) {
			saved = saved->next;
		}
		saved->next = new;
	}

	return 0;
}

int check_if_a_record_exists(rev_record_row_t *record, char *ip)
{
	rev_record_row_t *saved;

	saved = record;
	while(saved) {
		if ((strncmp(saved->host, ip, RBUFF_S)) == 0) {
			return 1;
		} else {
			saved = saved->next;
		}
	}
	return 0;
}

int convert_rev_records(rev_record_row_t *records, unsigned long int prefix)
{
	rev_record_row_t *saved;
	char *host;
	
	saved = records;
	while (saved) {
		if (prefix == 8) {
			host = strchr(saved->host, '.');
		} else if (prefix == 16) {
			host = strchr(saved->host, '.');
			host = strchr(saved->host, '.');
		} else if (prefix>= 24) {
			host = strrchr(saved->host, '.');
		} else {
			printf("Prefix %lu invalid\n", prefix);
			return 1;
		}
		host++;
		snprintf(saved->host, RBUFF_S, "%s", host);
		saved = saved->next;
	}
	return 0;
}
int insert_rev_records(dnsa_config_t *dc, rev_record_row_t *records, rev_zone_info_t *zone)
{
	MYSQL dnsa;
	char *query;
	const char *dnsa_query;
	int retval;
	rev_record_row_t *saved;
	
	if (!(query = calloc(RBUFF_S, sizeof(char))))
		report_error(MALLOC_FAIL, "query in insert_rev_records");
	saved = records;
	dnsa_query = query;
	dnsa_mysql_init(dc, &dnsa);
	snprintf(query, RBUFF_S,
"START TRANSACTION");
	if ((retval = cmdb_mysql_query_with_checks(&dnsa, dnsa_query)) > 0) {
		mysql_rollback(&dnsa);
		cmdb_mysql_clean(&dnsa, query);
		return retval;
	}
	snprintf(query, RBUFF_S,
"DELETE FROM rev_records WHERE rev_zone = %d", zone->rev_zone_id);
	if ((retval = cmdb_mysql_query_with_checks(&dnsa, dnsa_query)) > 0) {
		mysql_rollback(&dnsa);
		cmdb_mysql_clean(&dnsa, query);
		return retval;
	}
	update_rev_zone_serial(zone);
	snprintf(query, RBUFF_S,
"UPDATE rev_zones SET updated = 'yes', valid = 'unknown', \
serial = %lu WHERE rev_zone_id = %d", zone->serial, zone->rev_zone_id);
	if ((retval = cmdb_mysql_query_with_checks(&dnsa, dnsa_query)) > 0) {
		mysql_rollback(&dnsa);
		cmdb_mysql_clean(&dnsa, query);
		return retval;
	}
	while (saved) {
		snprintf(query, RBUFF_S,
"INSERT INTO rev_records (rev_zone, host, destination) VALUES (%d, '%s', '%s')",
		 zone->rev_zone_id, saved->host, saved->dest);
		if ((retval = cmdb_mysql_query_with_checks(&dnsa, dnsa_query)) > 0) {
			mysql_rollback(&dnsa);
			cmdb_mysql_clean(&dnsa, query);
			return retval;
		}
		saved = saved->next;
	}
	mysql_commit(&dnsa);
	cmdb_mysql_clean(&dnsa, query);
	
	return 0;
}

void delete_A_records(rev_record_row_t *records)
{
	rev_record_row_t *saved, *next;

	saved = next = records;
	while (saved) {
		next = saved->next;
		free(saved);
		saved = next;
	}
}