/* wzf.c:
 * 
 * Contains function to write out the forward zone file from information
 * stored in the database.
 * 
 * Contains some basic error checking, and will update the zone in the
 * database as valid if these checks are passed.
 * 
 * You still need to write out the config file to have the zones implemented
 * on the name server
 * 
 * (C) Iain M Conochie 2012 <iain@ailsatech.net>
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "forward.h"

int wzf (char *domain, dnsa_config_t *dc)
{
	FILE *zonefile;
	MYSQL shihad, shihad2, shihad3;
	MYSQL_RES *shihad_res, *my_res;
	MYSQL_ROW my_row, my_row2, my_row3;
	zone_info_t zone_info;
	record_row_t row_data, row_data2, row_data3;
	size_t offset, len;
	my_ulonglong shihad_rows;
	int error;
	char *zout, *zout2, *tmp, *tmp2, *c, *zonefilename, *thost, *error_code;
	const char *unix_socket, *shiquery, *system_command, *error_str;

	if (!(zout = malloc(FILE_S * sizeof(char))))
		report_error(MALLOC_FAIL, "zout in wzf");
	if (!(zout2 = malloc(FILE_S * sizeof(char))))
		report_error(MALLOC_FAIL, "zout2 in wzf");
	if (!(tmp = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp in wzf");
	if (!(tmp2 = malloc(BUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "tmp2 in wzf");
	if (!(zonefilename = malloc(TBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "zonefilename in wrz");
	if (!(c = malloc(CH_S * sizeof(char))))
		report_error(MALLOC_FAIL, "c in wzf");
	if (!(error_code = malloc(RBUFF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "error_code in wrz");
	
	shiquery = tmp;
	unix_socket = dc->socket;
	error_str = error_code;
	
	/* Initialise MYSQL connection and query */
	if (!(mysql_init(&shihad))) {
		report_error(MY_INIT_FAIL, mysql_error(&shihad));
	}
	if (!(mysql_real_connect(&shihad, dc->host, dc->user, dc->pass,
		dc->db, dc->port, unix_socket, dc->cliflag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&shihad));
	}
	
	sprintf(tmp, "SELECT * FROM zones WHERE name = '%s'", domain);
	error = mysql_query(&shihad, shiquery);
	snprintf(error_code, CONF_S, "%d", error);
	
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(shihad_res = mysql_store_result(&shihad))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&shihad));
		report_error(MY_STORE_FAIL, error_str);
	}
	
	if (((shihad_rows = mysql_num_rows(shihad_res)) == 0)) {
		report_error(NO_DOMAIN, domain);
	} else if (shihad_rows > 1) {
		report_error(MULTI_DOMAIN, domain);
	}
	
	/* Get the zone info from the DB */
	while ((my_row = mysql_fetch_row(shihad_res))) {
		zone_info = fill_zone_data(my_row);
	}
	
	/* Create the zone file header */
	offset = create_zone_header(zout, zone_info);
	
	/* Check for MX records and add them to the zone */
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zone_info.id);
	error = mysql_query(&shihad, shiquery);
	snprintf(error_code, CONF_S, "%d", error);
	
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(shihad_res = mysql_store_result(&shihad)))
		fprintf(stderr, "No result set?\n");
	if (((shihad_rows = mysql_num_rows(shihad_res)) == 0)) {
		fprintf(stderr, "No MX records for domain %s\n", domain);
	} else {
		while ((my_row = mysql_fetch_row(shihad_res))) {
			offset = add_mx_to_header(zout, offset, my_row);
		}
	}
	sprintf(tmp, "$ORIGIN\t%s.\n", zone_info.name);
	
	/** zout2 will be used to write the real zone file.
	 ** Cannot have A records for the NS and MX added twice
	 ** so save the buffer into zout2 and use that
	 */
	len = strlen(tmp);
	strncat(zout, tmp, len);
	offset += len;
	strncpy(zout2, zout, offset);
	/** We need to add the A records for the DNS servers and MX servers
	 ** Cannot do this in the previous 2 functions as we need a new query
	 ** This is so we can can check the zone header for errors. We can use
	 ** the first part of the FQDN of MX and NS records
	 */
	sprintf(c, ".");
	sprintf(tmp2, "%s", zone_info.pri_dns);
	/* Search for A records for the NS servers and add to zonefile */
	if (!(thost = strtok(tmp2, c)))	{/* Grab up to first . */
		printf("record does not seem to contain %s\n", c);
		return NO_DELIM;
	} else {
		sprintf	(tmp, "SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'",
			 zone_info.id, thost);
		shiquery = tmp;
		error = mysql_query(&shihad, shiquery);
		if ((error != 0)) {
			fprintf(stderr, "Query not successful: error code %d\n",
				error);
			return MY_QUERY_FAIL;
		}
		if (!(shihad_res = mysql_store_result(&shihad)))
			fprintf(stderr, "No result set?\n");
		while ((my_row = mysql_fetch_row(shihad_res))) {
			row_data = fill_record_data(my_row);
			offset = add_records(row_data, zout, offset);	
		}
	}
	if ((strcmp(zone_info.sec_dns, "NULL")) == 0) { /* Do we have a secondary DNS server? */
		;
	} else {
		sprintf(c, ".");
		sprintf(tmp2, "%s", zone_info.sec_dns);
		strtok(tmp2, c); /* Grab up to first . */
		sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'", zone_info.id, tmp2);
		shiquery = tmp;
		error = mysql_query(&shihad, shiquery);
		snprintf(error_code, CONF_S, "%d", error);
		if ((error != 0)) {
			report_error(MY_QUERY_FAIL, error_str);
		}
		if (!(shihad_res = mysql_store_result(&shihad)))
			fprintf(stderr, "No result set?\n");
		while ((my_row = mysql_fetch_row(shihad_res))) {
			row_data = fill_record_data(my_row);
			offset = add_records(row_data, zout, offset);
		}
	}
	/* Search for any MX records in the domain */
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zone_info.id);
	shiquery = tmp;
	error = mysql_query(&shihad, tmp);
	snprintf(error_code, CONF_S, "%d", error);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(shihad_res = mysql_store_result(&shihad)))
		fprintf(stderr, "Store failed\n");
	if (((shihad_rows = mysql_num_rows(shihad_res)) == 0)) {
		fprintf(stderr, "No MX records for domain %s\n", domain);
	} else { 
		while ((my_row = mysql_fetch_row(shihad_res))) {
			row_data = fill_record_data(my_row);
			sprintf(c, "."); /* Grab up to first . */
			sprintf(tmp, "%s", row_data.dest);
			strtok(tmp, c);
			sprintf(tmp2, "SELECT * FROM records WHERE zone = %d AND host = '%s'",
				zone_info.id, tmp);
			shiquery = tmp2;
			if (!(mysql_init(&shihad2))) {
				report_error(MY_INIT_FAIL, error_code);
			}
			if (!(mysql_real_connect(&shihad2,
				dc->host, dc->user, dc->pass, dc->db, dc->port, unix_socket, dc->cliflag ))) {
				report_error(MY_CONN_FAIL, mysql_error(&shihad));
			}
			error = mysql_query(&shihad2, shiquery);
			snprintf(error_code, CONF_S, "%d", error);
			if ((error != 0)) {
				report_error(MY_QUERY_FAIL, error_str);
			}
			if (!(my_res = mysql_store_result(&shihad2)))
				fprintf(stderr, "No result set?\n");
			while ((my_row2 = mysql_fetch_row(my_res))) {
				row_data2 = fill_record_data(my_row2);
				offset = add_records(row_data2, zout, offset);
			}
			mysql_close(&shihad2);
		}
	}
	/** Now we have to test the zonefile for errors
	 ** Output zonefile and check then remove.
	 **/
	sprintf(zonefilename, "%sdb1.%s", dc->dir, zone_info.name);
 	if (!(zonefile = fopen(zonefilename, "w"))) {
		report_error(FILE_O_FAIL, zonefilename);
	} else {
		fputs(zout, zonefile);
		fclose(zonefile);
	}
	sprintf(tmp, "%s %s %s", dc->chkz, zone_info.name, zonefilename);
	system_command = tmp;
	error = system(system_command);
	if (error == 0) {
		printf("Checkzone ran successfully\n");
	} else {
		printf("Checkzone exitied with status %d\n", error);
		return error;
	}
	remove(zonefilename);
	/** Get the rest of the A and CNAME records
	 ** We will need to add the other record types
	 ** Can check this from the DB
	 **/
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'A' OR zone = %d AND type = 'CNAME'", zone_info.id, zone_info.id);
	shiquery = tmp;
	if (!(mysql_init(&shihad3))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&shihad3,
		dc->host, dc->user, dc->pass, dc->db, dc->port, unix_socket, dc->cliflag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&shihad));
	}
	error = mysql_query(&shihad3, shiquery);
	snprintf(error_code, CONF_S, "%d", error);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(my_res = mysql_store_result(&shihad3)))
		fprintf(stderr, "No result set?\n");
	while ((my_row3 = mysql_fetch_row(my_res))) {
		row_data3 = fill_record_data(my_row3);
		offset = add_records(row_data3, zout2, offset);	
	}
	mysql_close(&shihad3); 
	/* Output full zonefile */
	sprintf(zonefilename, "%sdb.%s", dc->dir, zone_info.name);
 	if (!(zonefile = fopen(zonefilename, "w"))) {
		report_error(FILE_O_FAIL, zonefilename);
	} else {
		fputs(zout2, zonefile);
		fclose(zonefile);
	}
	/** Check the zone for errors. If none, update DB with the to say that 
	 ** zone is valid */
	sprintf(tmp, "%s %s %s",dc->chkz, zone_info.name, zonefilename);
	system_command = tmp;
	error = system(system_command);
	if (error == 0) {
		printf("Checkzone ran successfully\n");
	} else {
		printf("Checkzone exitied with status %d\n", error);
		return error;
	}
	sprintf(tmp, "UPDATE zones SET updated = 'no', valid = 'yes' WHERE name = '%s'", zone_info.name);
	shiquery = tmp;
	error = mysql_query(&shihad, shiquery);
	shihad_rows = mysql_affected_rows(&shihad);
	if (shihad_rows == 1)
		fprintf(stderr, "DB updated as zone validated\n");
	
	mysql_close(&shihad);
	free(c);
	free(zout2);
	free(zout);
	free(tmp);
	free(tmp2);
	free(zonefilename);
	free(error_code);
	return 0;
}
