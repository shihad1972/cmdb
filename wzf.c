#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "write_zone.h"

int main (int argc, char *argv[])
{
	FILE *cnf, *zonefile;
	MYSQL shihad, shihad2, shihad3;
	MYSQL_RES *shihad_res, *my_res;
	MYSQL_ROW my_row, my_row2, my_row3;
	zone_info_t zone_info;
	record_row_t row_data, row_data2, row_data3;
	size_t offset, len;
	my_ulonglong shihad_rows;
	int error;
	unsigned int port;
	unsigned long int client_flag;
	char a;
	char *zout, *zout2, *tmp, *tmp2, *c, *zonefilename, *domain, *thost;
	char buff[CONF_S]="";
	char host[CONF_S]="somehost";
	char user[CONF_S]="someuser";
	char pass[CONF_S]="scribble";
	char db[CONF_S]="somedb";
	char dir[CONF_S]="./";
	char search[] = "-d";
	const char *unix_socket, *shiquery, *system_command;
	char confile[]="/etc/dnsa/dnsa.conf";
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	zout = malloc(FILE_S * sizeof(char));
	zout2 = malloc(FILE_S * sizeof(char));
	tmp = malloc(BUFF_S * sizeof(char));
	tmp2 = malloc(BUFF_S * sizeof(char));
	zonefilename = malloc(TBUFF_S * sizeof(char));
	domain = malloc(TBUFF_S * sizeof(char));
	c = malloc(CH_S * sizeof(char));
	
	if (argc != 3) {
		printf("Usage: %s -d <domain>\n", argv[0]);
		exit(ARGC_INVAL);
	}
	len = strlen(argv[1]);
	if ((strncmp(search, argv[1], len) == 0)) {
		len = strlen(argv[2]);
		if (!(domain = strncpy(domain, argv[2], ((len < CONF_S ? len : CONF_S - 1))))) {
			printf("Unable to parse input\n");
			exit(ARGV_INVAL);
		}
	} else {
		printf("Usage: %s -d <domain>\n", argv[0]);
		exit(ARGV_INVAL);
	}
	if (!(cnf = fopen(confile, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", confile);
		fprintf(stderr, "Using default values\n");
	} else {
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "PASS=%s", pass);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "HOST=%s", host);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "USER=%s", user);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DB=%s", db);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DIR=%s", dir);
		fclose(cnf);
	}
	len = strlen(dir);
	a = dir[len -1];
	if (!(a == '/')) {
		fprintf(stderr, "dir value missing trailing /\n");
		dir[len] = '/';
		dir[len + 1] = '\0';
		len = strlen(dir);
	}
	if (!(mysql_init(&shihad))) {
		fprintf(stderr, "Cannot init. Out of memory?\n");
		exit(MY_INIT_FAIL);
	}
	if (!(mysql_real_connect(&shihad,
		host, user, pass, db, port, unix_socket, client_flag ))) {
		fprintf(stderr, "Connect failed. Error: %s\n",
			mysql_error(&shihad));
		exit(MY_CONN_FAIL);
	}
	sprintf(tmp, "SELECT * FROM zones WHERE name = '%s'", domain);
	shiquery = tmp;
	error = mysql_query(&shihad, shiquery);
	if ((error != 0)) {
		fprintf(stderr, "Query not successful: error code %d\n", error);
		exit(MY_QUERY_FAIL);
	}
	if (!(shihad_res = mysql_store_result(&shihad))) {
		fprintf(stderr, "Cannot store result set\n");
		exit(MY_STORE_FAIL);
	}
	if (((shihad_rows = mysql_num_rows(shihad_res)) == 0)) {
		fprintf(stderr, "Domain %s not found\n", domain);
		exit(NO_DOMAIN);
	} else if (shihad_rows > 1) {
		fprintf(stderr, "Multiple rows found for domain %s\n", domain);
		exit(MULTI_DOMAIN);
	}
	while ((my_row = mysql_fetch_row(shihad_res))) {
		zone_info = fill_zone_data(my_row);
	}
	offset = create_zone_header(zout, zone_info);
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zone_info.id);
	shiquery = tmp;
	error = mysql_query(&shihad, shiquery);
	if ((error != 0)) {
		fprintf(stderr, "Query not successful: error code %d\n", error);
		exit(MY_QUERY_FAIL);
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
	/** We need to add the A records for the DNS servers and MX servers
	 ** Cannot do this in the previous 2 functions as we need a new query
	 ** This is so we can can check the zone header for errors.
	 */
	len = strlen(tmp);
	strncat(zout, tmp, len);
	offset += len;
	strncpy(zout2, zout, offset); 
	/** zout2 will be used to write the real zone file.
	 ** Cannot have A records for the NS and MX added twice
	 ** so save the buffer into zout2 and use that
	 */
	sprintf(c, ".");
	sprintf(tmp2, "%s", zone_info.pri_dns);
	if (!(thost = strtok(tmp2, c)))	{/* Grab up to first . */
		printf("record does not seem to contain %s\n", c);
		exit(NO_DELIM);
	} else {
		sprintf	(tmp, "SELECT * FROM records WHERE zone = %d AND host like '%s' AND type = 'A'", zone_info.id, thost);
		shiquery = tmp;
		error = mysql_query(&shihad, shiquery);
		if ((error != 0)) {
			fprintf(stderr, "Query not successful: error code %d\n", error);
			exit(MY_QUERY_FAIL);
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
		if ((error != 0)) {
			fprintf(stderr, "Query not successful: error code %d\n", error);
			exit(MY_QUERY_FAIL);
		}
		if (!(shihad_res = mysql_store_result(&shihad)))
			fprintf(stderr, "No result set?\n");
		while ((my_row = mysql_fetch_row(shihad_res))) {
			row_data = fill_record_data(my_row);
			offset = add_records(row_data, zout, offset);
		}
	}
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'MX'", zone_info.id);
	shiquery = tmp;
	error = mysql_query(&shihad, tmp);
	if ((error != 0)) {
		fprintf(stderr, "Query not successful: error code %d\n", error);
		exit(MY_QUERY_FAIL);
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
			sprintf(tmp2, "SELECT * FROM records WHERE zone = %d AND host = '%s'", zone_info.id, tmp);
			shiquery = tmp2;
			if (!(mysql_init(&shihad2))) {
				fprintf(stderr, "Cannot init. Out of memory?\n");
				exit(MY_INIT_FAIL);
			}
			if (!(mysql_real_connect(&shihad2,
				host, user, pass, db, port, unix_socket, client_flag ))) {
				fprintf(stderr, "Connect failed. Error: %s\n",
					mysql_error(&shihad));
				exit(MY_CONN_FAIL);
			}
			error = mysql_query(&shihad2, shiquery);
			if ((error != 0)) {
				fprintf(stderr, "Query not successful: error code %d\n", error);
				exit(MY_QUERY_FAIL);
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
	 ** Best to output to CWD and test.
	 **/
	sprintf(zonefilename, "%sdb1.%s", dir, zone_info.name);
 	if (!(zonefile = fopen(zonefilename, "w"))) {
		fprintf(stderr, "Cannot open zonefile file %s\n", zonefilename);
		exit(FILE_O_FAIL);
	} else {
		fputs(zout, zonefile);
		fclose(zonefile);
	}
	sprintf(tmp, "/usr/sbin/named-checkzone %s %s", zone_info.name, zonefilename);
	system_command = tmp;
	error = system(system_command);
	if (error == 0) {
		printf("Checkzone ran successfully\n");
	} else {
		printf("Checkzone exitied with status %d\n", error);
		exit(error);
	}
	remove(zonefilename);
	sprintf(tmp, "SELECT * FROM records WHERE zone = %d AND type = 'A' OR zone = %d AND type = 'CNAME'", zone_info.id, zone_info.id);
	shiquery = tmp;
	if (!(mysql_init(&shihad3))) {
		fprintf(stderr, "Cannot init. Out of memory?\n");
		exit(MY_INIT_FAIL);
	}
	if (!(mysql_real_connect(&shihad3,
		host, user, pass, db, port, unix_socket, client_flag ))) {
		fprintf(stderr, "Connect failed. Error: %s\n",
			mysql_error(&shihad));
		exit(MY_CONN_FAIL);
	}
	error = mysql_query(&shihad3, shiquery);
	if ((error != 0)) {
		fprintf(stderr, "Query 3 not successful: error code %d\n", error);
		exit(MY_QUERY_FAIL);
	}
	if (!(my_res = mysql_store_result(&shihad3)))
		fprintf(stderr, "No result set?\n");
	while ((my_row3 = mysql_fetch_row(my_res))) {
		row_data3 = fill_record_data(my_row3);
		offset = add_records(row_data3, zout2, offset);	
	}
	mysql_close(&shihad3); 
	sprintf(zonefilename, "%sdb.%s", dir, zone_info.name);
 	if (!(zonefile = fopen(zonefilename, "w"))) {
		fprintf(stderr, "Cannot open zonefile file %s\n", zonefilename);
		exit(FILE_O_FAIL);
	} else {
		fputs(zout2, zonefile);
		fclose(zonefile);
	}
	sprintf(tmp, "/usr/sbin/named-checkzone %s %s", zone_info.name, zonefilename);
	system_command = tmp;
	error = system(system_command);
	if (error == 0) {
		printf("Checkzone ran successfully\n");
	} else {
		printf("Checkzone exitied with status %d\n", error);
		exit(error);
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
	free(domain);
	exit(0);
}
