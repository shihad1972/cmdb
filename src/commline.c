#include "dnsa.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_command_line(int argc, char **argv, comm_line_t *comp)
{
	int i, retval;
	
	retval = 0;
	strncpy(comp->action, "NULL", COMM_S);
	strncpy(comp->type, "NULL", COMM_S);
	strncpy(comp->domain, "NULL", CONF_S);
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-d", COMM_S) == 0)) {
			strncpy(comp->action, "display", COMM_S);
		} else if ((strncmp(argv[i], "-w", COMM_S) == 0)) {
			strncpy(comp->action, "write", COMM_S);
		} else if ((strncmp(argv[i], "-f", COMM_S) == 0)) {
			strncpy(comp->type, "forward", COMM_S);
		} else if ((strncmp(argv[i], "-r", COMM_S) == 0)) {
			strncpy(comp->type, "reverse", COMM_S);
		} else if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc) 
				retval = -1;
			else
				strncpy(comp->domain, argv[i], CONF_S);
		} else {
			retval = -1;
		}
	}
	
	if ((strncmp(comp->action, "NULL", COMM_S) == 0))
		retval = -1;
	else if ((strncmp(comp->type, "NULL", COMM_S) == 0))
		retval = -1;
	else if ((strncmp(comp->domain, "NULL", CONF_S) == 0))
		retval = -1;
	
	return retval;
}

int parse_config_file(char config[][CONF_S])
{
	FILE *cnf;	/* File handle for config file */
	char confile[CONF_S]="/etc/dnsa/dnsa.conf";	/* File name */
	char buff[CONF_S] = "";
	int retval;
	sprintf(config[DB], "bind");
	sprintf(config[USER], "root");
	sprintf(config[HOST], "localhost");
	sprintf(config[DIR], "/var/named/");
	sprintf(config[BIND], "/var/named/");
	sprintf(config[DNSA], "dnsa.conf");
	sprintf(config[REV], "dnsa-rev.conf");
	sprintf(config[RNDC], "/usr/sbin/rndc");
	sprintf(config[CHKC], "/usr/sbin/named-checkconf");
	sprintf(config[CHKZ], "/usr/sbin/named-checkzone");
	if (!(cnf = fopen(confile, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", confile);
		fprintf(stderr, "Using default values\n");
		retval = -1;
	} else {
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "PASS=%s", config[PASS]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "HOST=%s", config[HOST]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "USER=%s", config[USER]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DB=%s", config[DB]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DIR=%s", config[DIR]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "REV=%s", config[REV]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "DNSA=%s", config[DNSA]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "RNDC=%s", config[RNDC]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "CHKZ=%s", config[CHKZ]);
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf)))
			sscanf(buff, "CHKC=%s", config[CHKC]);
		retval = 0;
		fclose(cnf);
	}
	
	return retval;
}
