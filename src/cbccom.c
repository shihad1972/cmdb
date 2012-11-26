/* cbccom.c
 * 
 * Functions to get configuration values and also parse command line arguments
 * 
 * part of the cbc program
 * 
 * (C) 2012 Iain M. Conochie
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_cbc.h"

int parse_cbc_config_file(cbc_config_t *dc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	int retval;
	unsigned long int portno;

	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = -1;
	} else {
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PASS=%s", dc->pass);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "HOST=%s", dc->host);	
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "USER=%s", dc->user);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DB=%s", dc->db);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "SOCKET=%s", dc->socket);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PORT=%s", port);
		}
		retval = 0;
		fclose(cnf);
	}
	
	/* We need to check the value of portno before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = -2;
	} else {
		dc->port = (unsigned int) portno;
	}
	
	return retval;
}

void init_cbc_config_values(cbc_config_t *dc)
{
	sprintf(dc->db, "cmdb");
	sprintf(dc->user, "root");
	sprintf(dc->host, "localhost");
	sprintf(dc->pass, "%s", "");
	sprintf(dc->socket, "%s", "");
	dc->port = 3306;
	dc->cliflag = 0;
}