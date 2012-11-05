/* wcf.c: main function for write config file
 * 
 * This function creates and writes the BIND config file
 * for all the DNS zones that are held in the database. 
 * 
 * Contains some error checking (is config file valid, do the
 * zone files exist)
 * 
 * (C) Iain M Conochie 2012 <iain@ailsatech.net>
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"

int wcf(char config[][CONF_S])
{
	FILE *cnf;
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t len;
	my_ulonglong dnsa_rows;
	int error;
	unsigned int port;
	unsigned long int client_flag;
	char *dout, *dnsa_line, *zonefile, *error_code;
	const char *dnsa_query, *unix_socket, *check_comm, *reload_comm, *error_str, *domain;
	dnsa_query = "SELECT name FROM zones WHERE valid = 'yes'";
	domain = "that is valid ";
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	dout = calloc(FILE_S, sizeof(char));
	dnsa_line = calloc(TBUFF_S, sizeof(char));
	zonefile = calloc(CONF_S, sizeof(char));
	error_code = malloc(RBUFF_S * sizeof(char));
	error_str = error_code;

	/* Initialise MYSQL Connection and query*/
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa, config[HOST], config[USER],
		config[PASS], config[DB], port, unix_socket, client_flag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&dnsa));
	}
	error = mysql_query(&dnsa, dnsa_query);
	if ((error != 0)) {
		report_error(MY_QUERY_FAIL, error_str);
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		snprintf(error_code, CONF_S, "%s", mysql_error(&dnsa));
		report_error(MY_STORE_FAIL, error_str);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		report_error(NO_DOMAIN, domain);
	}
	/* From each DB row create the config liens */
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		sprintf(zonefile, "%sdb.%s", config[DIR], dnsa_row[0]);
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
	sprintf(dnsa_line, "%s%s", config[BIND], config[DNSA]);
	
	if (!(cnf = fopen(dnsa_line, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing!\n", dnsa_line);
		exit(1);
	} else {
		fputs(dout, cnf);
		fclose(cnf);
	}
	/* Check the file and if OK reload BIND */
	check_comm = dnsa_line;
	sprintf(dnsa_line, "%s %s%s", config[CHKC], config[BIND], config[DNSA]);
	error = system(check_comm);
	if (!(error == 0)) {
		fprintf(stderr, "Bind config check failed! Error code was %d\n", error);
	} else {
		reload_comm = dnsa_line;
		sprintf(dnsa_line, "%s reload", config[RNDC]);
		error = system(reload_comm);
		if (!(error == 0)) {
			fprintf(stderr, "Bind reload failed with error code %d\n", error);
		}
	}
	
	mysql_close(&dnsa);
	free(dout);
	free(dnsa_line);
	free(zonefile);
	free(error_code);
	exit(0);
}
