#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dnsa.h"
#include "rev_zone.h"

int main(int argc, char *argv[])
{
	FILE *cnf;
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t len;
	rev_zone_info_t rev_zone_info;
	rev_record_row_t rev_row;
	my_ulonglong dnsa_rows;
	int error, reverse, i;
	unsigned int port;
	unsigned long int client_flag;
	char a;
	char *tmp, *rout, *tmp2, *dquery;
	const char *unix_socket, *dnsa_query, *syscom;
	char buff[CONF_S]="";
	char host[CONF_S]="somehost";
	char user[CONF_S]="someuser";
	char pass[CONF_S]="scribble";
	char db[CONF_S]="somedb";
	char dir[CONF_S]="./";
	char rev[CONF_S]="rev.conf";
	char search[CH_S] = "-r";
	char confile[CONF_S]="/etc/dnsa/dnsa.conf";
	char chkzone[CONF_S]="/usr/sbin/named-checkzone";	/* Should get from config file */
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	dquery = malloc(BUFF_S * sizeof(char));
	tmp = malloc(BUFF_S * sizeof(char));
	rout = malloc(FILE_S * sizeof(char));
	if (argc != 3) {
		printf("Usage: %s -r <rev-zone-id>\n", argv[0]);
		exit(ARGC_INVAL);
	}
	len = strlen(argv[1]);
	if ((strncmp(search, argv[1], len) == 0)) {
		len = strlen(argv[2]);
		if (!(tmp = strncpy(tmp, argv[2], ((len < CONF_S ? len : CONF_S - 1))))) {
			printf("Unable to parse input\n");
			exit(ARGV_INVAL);
		} else {
			reverse = atoi(tmp);
		}
	} else {
		printf("Usage: %s -r <rev-zone-id>\n", argv[0]);
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
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "REV=%s", rev);
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
	if (!(mysql_init(&dnsa))) {
		fprintf(stderr, "Cannot init. Out of memory?\n");
		exit(MY_INIT_FAIL);
	}
	if (!(mysql_real_connect(&dnsa,
		host, user, pass, db, port, unix_socket, client_flag ))) {
		fprintf(stderr, "Connect failed. Error: %s\n",
			mysql_error(&dnsa));
		exit(MY_CONN_FAIL);
	}
	sprintf(dquery, "SELECT * FROM rev_zones WHERE rev_zone_id = '%d'", reverse);
	dnsa_query = dquery;
	error = mysql_query(&dnsa, dnsa_query);
	if ((error != 0)) {
		fprintf(stderr, "Query not successful: error code %d\n", error);
		exit(MY_QUERY_FAIL);
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		fprintf(stderr, "Cannot store result set\n");
		exit(MY_STORE_FAIL);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "Reverse zone id %d not found\n", reverse);
		exit(NO_DOMAIN);
	} else if (dnsa_rows > 1) {
		fprintf(stderr, "Multiple rows found for reverse zone id %d\n", reverse);
		exit(MULTI_DOMAIN);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_zone_info = fill_rev_zone_data(dnsa_row);
	}
	len = create_rev_zone_header(rev_zone_info, rout);
	sprintf(dquery, "SELECT host, destination FROM rev_records WHERE rev_zone = '%d'", reverse);
	error = mysql_query(&dnsa, dnsa_query);
	if ((error != 0)) {
		fprintf(stderr, "Rev record query unsuccessful: error code %d\n", error);
		exit(MY_QUERY_FAIL);
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		fprintf(stderr, "Cannot store result set\n");
		exit(MY_STORE_FAIL);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "No reverse records for zone %d\n", reverse);
		exit(NO_RECORDS);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		rev_row = get_rev_row(dnsa_row);
		add_rev_records(rout, rev_row);
	}
	tmp2 = buff;
	for (i=0; i< CONF_S; i++)	/* zero buffer */
		*(tmp2 + i) = '\0';
	len = strlen(dir);
	strncpy(tmp2, dir, len);
	len = strlen(rev_zone_info.net_range);
	strncat(tmp2, rev_zone_info.net_range, len);
	len = strlen(rout);
	if (!(cnf = fopen(buff, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing\n", buff);
		exit(FILE_O_FAIL);
	} else {
		fputs(rout, cnf);
		fclose(cnf);
	}
	for (i=0; i< CONF_S; i++)	/* zero tmp buffer */
		*(tmp + i) = '\0';
	for (i=0; i < FILE_S; i++)	/* zero rout file output buffer */
		*(rout + i) = '\0';
	get_in_addr_string(tmp, rev_zone_info.net_range);
	sprintf(rout, "%s %s %s", chkzone, tmp, buff);
	syscom = rout;
	error = system(syscom);
	if (error == 0) {
		printf("check of zone %s ran successfully\n", buff);
		sprintf(dquery, "UPDATE rev_zones SET valid = 'yes', updated = 'no' WHERE rev_zone_id = %d",
			reverse);
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
	} else {
		printf("Check of zone %s failed. Error code %d\n", buff, error);
	}
	mysql_close(&dnsa);
	free(tmp);
	free(rout);
	free(dquery);
	exit(0);
}
