/* wrcf.c: Write the reverse zone configuration file 
 *
 * This function creates and writes the BIND reverse zone
 * configuration file from all the zones that are valid in
 * the database. 
 * 
 * Contains some basic error checking, such as does the reverse
 * zonefile exist on the filesystem, and is the written config
 * file correct before reloading the name server.
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
#include "reverse.h"
#include "mysqlfunc.h"
#include "dnsa_mysql.h"

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
	exit(0);
}
