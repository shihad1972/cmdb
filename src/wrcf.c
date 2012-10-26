/* wrcf.c: Write the reverse zone configuration file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"
#include "rev_zone.h"

int main(void)
{
	FILE *cnf;
	MYSQL dnsa;
	MYSQL_RES *dnsa_res;
	MYSQL_ROW dnsa_row;
	size_t len;
	my_ulonglong dnsa_rows;
	int error, i;
	unsigned int port;
	unsigned long int client_flag;
	char a;
	char *rout, *dnsa_line, *zonefile, *tmp, *tmp2;
	const char *dnsa_query, *unix_socket, *check_comm, *syscom;
	char confile[]="/etc/dnsa/dnsa.conf";
	char buff[CONF_S]="";
	char host[CONF_S]="somehost";
	char user[CONF_S]="someuser";
	char pass[CONF_S]="scribble";
	char db[CONF_S]="somedb";
	char dir[CONF_S]="./";
	char bind[CONF_S]="/var/named/";
	char dnsav[CONF_S]="rev.conf";
	char reload_comm[CONF_S] = "/usr/sbin/rndc";
	const char tmpdir[CONF_S] = "/tmp/";
	check_comm = "/usr/sbin/named-checkconf";	/* Should add to config file */
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	rout = malloc(FILE_S * sizeof(char));
	dnsa_line = malloc(TBUFF_S * sizeof(char));
	tmp = malloc(TBUFF_S * sizeof(char));
	tmp2 = malloc(TBUFF_S * sizeof(char));
	zonefile = malloc(CONF_S * sizeof(char));
	dnsa_query = dnsa_line;
	
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
			sscanf(buff, "BIND=%s", bind);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "REV=%s", dnsav);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "RNDC=%s", reload_comm);
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
	sprintf(dnsa_line, "SELECT net_range FROM rev_zones");
	error = mysql_query(&dnsa, dnsa_query);
	if ((error != 0)) {
		fprintf(stderr, "net_range query failed: error code %d\n", error);
		exit(MY_QUERY_FAIL);
	}
	if (!(dnsa_res = mysql_store_result(&dnsa))) {
		fprintf(stderr, "Cannot store result set of net range\n");
		exit(MY_STORE_FAIL);
	}
	if (((dnsa_rows = mysql_num_rows(dnsa_res)) == 0)) {
		fprintf(stderr, "No reverse zones??\n");
		exit(NO_RECORDS);
	}
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		for (i=0; i< RBUFF_S; i++)	/* zero tmp buffer */
			*(tmp2 + i) = '\0';
		get_in_addr_string(tmp2, dnsa_row[0]);
		sprintf(zonefile, "%s%s", dir, dnsa_row[0]);
		if (!(cnf = fopen(zonefile, "r"))) {
			fprintf(stderr, "Cannot access zonefile %s\n", zonefile);
		} else {
			sprintf(tmp, "zone \"%s\" {\n\t\t\ttype master;\n\t\t\tfile \"%s%s\";\n\t\t};\n",
				tmp2, dir, dnsa_row[0]);
			len = strlen(tmp);
			strncat(rout, tmp, len);
			fclose(cnf);
		}
	}
	sprintf(zonefile, "%s%s", tmpdir, dnsav);
	if (!(cnf = fopen(zonefile, "w"))) {
		fprintf(stderr, "Cannot open config files %s for writing!\n", zonefile);
		exit(FILE_O_FAIL);
	} else {
		fputs(rout, cnf);
		fclose(cnf);
		sprintf(tmp, "%s %s", check_comm, zonefile);
		syscom = tmp;
		error = system(syscom);
		if ((error == 0)) {
			sprintf(zonefile, "%s%s", bind, dnsav);
			if (!(cnf = fopen(zonefile, "w"))) {
				fprintf(stderr, "Cannot open config file %s for writing!\n",
					zonefile);
				exit(FILE_O_FAIL);
			} else {
				fputs(rout, cnf);
				fclose(cnf);
				sprintf(tmp, "%s reload", reload_comm);
				error = system(syscom);
				if ((error == 0)) {
					fprintf(stderr, "Reload successful\n");
				} else {
					fprintf(stderr, "Reload failed with error code %d\n",
						error);
				}
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
	mysql_close(&dnsa);
	exit(0);
}