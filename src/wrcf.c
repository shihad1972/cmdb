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
#include <mysql.h>
#include "dnsa.h"
#include "reverse.h"

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
	const char *dnsa_query, *unix_socket, *syscom, *error_str, *domain;
	domain = "or reverse zone that is valid ";
	unix_socket = dc->socket;
	rout = calloc(FILE_S, sizeof(char));
	dnsa_line = calloc(TBUFF_S, sizeof(char));
	tmp = calloc(TBUFF_S, sizeof(char));
	tmp2 = calloc(TBUFF_S, sizeof(char));
	zonefile = calloc(CONF_S, sizeof(char));
	error_code = calloc(RBUFF_S, sizeof(char));
	error_str = error_code;
	dnsa_query = dnsa_line;
	/* Initilaise MYSQL connection and query */
	if (!(mysql_init(&dnsa))) {
		report_error(MY_INIT_FAIL, error_str);
	}
	if (!(mysql_real_connect(&dnsa, dc->host, dc->user,
		dc->pass, dc->db, dc->port, unix_socket, dc->cliflag ))) {
		report_error(MY_CONN_FAIL, mysql_error(&dnsa));
	}
	sprintf(dnsa_line, "SELECT net_range FROM rev_zones");
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
	 * Check it and if successful write to real config file 
	 * and reload bind */
	sprintf(zonefile, "%s%s", dc->bind, dc->rev);
	if (!(cnf = fopen(zonefile, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing!\n", zonefile);
		exit(FILE_O_FAIL);
	} else {
		fputs(rout, cnf);
		fclose(cnf);
		sprintf(tmp, "%s %s", dc->chkc, zonefile);
		syscom = tmp;
		error = system(syscom);
		if ((error == 0)) {
			sprintf(tmp, "%s reload", dc->rndc);
			error = system(syscom);
			if ((error == 0)) {
				fprintf(stderr, "Reload successful\n");
			} else {
				fprintf(stderr, "Reload failed with error code %d\n",
					error);
			}
		} else {
			fprintf(stderr, "Check of config file failed. Error code %d\n",
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
