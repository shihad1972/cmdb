/* wcf.c: main function for write config file */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql.h>
#include "dnsa.h"

int main(void)
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
	char a;
	char *dout, *dnsa_line, *zonefile;
	char confile[]="/etc/dnsa/dnsa.conf";
	const char *dnsa_query, *unix_socket, *check_comm, *reload_comm;
	char buff[CONF_S]="";
	char host[CONF_S]="somehost";
	char user[CONF_S]="someuser";
	char pass[CONF_S]="scribble";
	char db[CONF_S]="somedb";
	char dir[CONF_S]="./";
	char bind[CONF_S]="/var/named/";
	char dnsav[CONF_S]="dnsa.conf";
	dnsa_query = "SELECT name FROM zones WHERE valid = 'yes'";
	check_comm = "/usr/sbin/named-checkconf";
	reload_comm = "/usr/sbin/rndc reload";
	unix_socket = "";
	port = 3306;
	client_flag = 0;
	dout = malloc(FILE_S * sizeof(char));
	dnsa_line = malloc(TBUFF_S * sizeof(char));
	zonefile = malloc(CONF_S * sizeof(char));
	
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
			sscanf(buff, "DNSA=%s", dnsav);
		fclose(cnf);
	}
	len = strlen(dir);
	a = dir[len - 1];
	if (!(a == '/')) {
		fprintf(stderr, "dir value missing trailing /\n");
		dir[len] = '/';
		dir[len + 1] = '\0';
		len = strlen(dir);
	}
	len = strlen(bind);
	a = bind[len - 1];
	if (!(a == '/')) {
		fprintf(stderr, "bind value missing trailing /\n");
		bind[len] = '/';
		bind[len + 1] = '\0';
		len = strlen(bind);
	}
	
	printf("%s\t%s\n", dir, bind);
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
		fprintf(stderr, "No valid domains found\n");
		exit(NO_DOMAIN);
	}
	
	while ((dnsa_row = mysql_fetch_row(dnsa_res))) {
		sprintf(zonefile, "%sdb.%s", dir, dnsa_row[0]);
		if ((cnf = fopen(zonefile, "r"))){
			sprintf(dnsa_line, "zone \"%s\" {\n\t\t\ttype master;\n\t\t\tfile \"%s\";\n\t\t};\n\n", dnsa_row[0], zonefile);
			len = strlen(dnsa_line);
			strncat(dout, dnsa_line, len);
			fclose(cnf);
		} else {
			printf("Not adding for zonefile %s\n", dnsa_row[0]);
		}
	}
	len = strlen(dnsav);
	strncat(bind, dnsav, len);
	if (!(cnf = fopen(bind, "w"))) {
		fprintf(stderr, "Cannot open config file %s for writing!\n", bind);
		exit(1);
	} else {
		fputs(dout, cnf);
		fclose(cnf);
	}
	mysql_close(&dnsa);
	
	error = system(check_comm);
	if (!(error == 0)) {
		fprintf(stderr, "Bind config check failed! Error code was %d\n", error);
	} else {
		error = system(reload_comm);
		if (!(error == 0)) {
			fprintf(stderr, "Bind reload failed with error code %d\n", error);
		}
	}
	free(dout);
	free(dnsa_line);
	free(zonefile);
	exit(0);
}