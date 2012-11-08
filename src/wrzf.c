/* wrzf.c:
 * 
 * Contains the function to write out the reverse zonefile. 
 * 
 * This has some basic error checking, and if these checks are passed
 * then the zone is updated as valid in the database.
 * 
 * You still need to write out the configuration file for the zones
 * to be implemented in BIND
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
#include "mysqlfunc.h"

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
	error = mysql_query(&dnsa, dnsa_query);
	
	if ((error != 0)) {
		fprintf(stderr, "Query not successful: error code %d\n", error);
		return MY_QUERY_FAIL;
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		fprintf(stderr, "Cannot store result set\n");
		return MY_STORE_FAIL;
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
	free(zonefn);
	free(rout);
	free(dquery);
	return 0;
}
