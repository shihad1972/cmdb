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
#include <mysql/mysql.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"

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
	
	mysql_close(&dnsa);
	free(dout);
	free(dnsa_line);
	free(zonefile);
	free(error_code);
	exit(0);
}
