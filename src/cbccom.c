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
#include "checks.h"

int parse_cbc_config_file(cbc_config_t *cbc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	int retval;
	unsigned long int portno;

	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
	} else {
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PASS=%s", cbc->pass);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "HOST=%s", cbc->host);	
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "USER=%s", cbc->user);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DB=%s", cbc->db);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "SOCKET=%s", cbc->socket);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PORT=%s", port);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "TMPDIR=%s", cbc->tmpdir);
		}
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "TFTPDIR=%s", cbc->tftpdir);
		}
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PXE=%s", cbc->pxe);
		}
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "TOPLEVELOS=%s", cbc->toplevelos);
		}
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DHCPCONF=%s", cbc->dhcpconf);
		}
		rewind (cnf);
		retval = 0;
		fclose(cnf);
	}
	
	/* We need to check the value of portno before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
		return retval;
	} else {
		cbc->port = (unsigned int) portno;
	}
	
	if ((retval = add_trailing_slash(cbc->tmpdir)) != 0)
		retval = TMP_ERR;
	if ((retval = add_trailing_slash(cbc->tftpdir)) != 0)
		retval = TFTP_ERR;
	if ((retval = add_trailing_slash(cbc->pxe)) != 0)
		retval = PXE_ERR;
	if ((retval = add_trailing_slash(cbc->toplevelos)) != 0)
		retval = OS_ERR;
	
	return retval;
}

void init_cbc_config_values(cbc_config_t *cbc)
{
	sprintf(cbc->db, "cmdb");
	sprintf(cbc->user, "root");
	sprintf(cbc->host, "localhost");
	sprintf(cbc->pass, "%s", "");
	sprintf(cbc->socket, "%s", "");
	cbc->port = 3306;
	cbc->cliflag = 0;
}

void parse_cbc_config_error(int error)
{
	switch(error) {
		case PORT_ERR:
			fprintf(stderr, "Port higher than 65535!\n");
			break;
		case TMP_ERR:
			fprintf(stderr, "Cannot add trailing / to TMPDIR: > 79 characters\n");
			break;
		case TFTP_ERR:
			fprintf(stderr, "Cannot add trailing / to TFTPDIR: > 79 characters\n");
			break;
		case PXE_ERR:
			fprintf(stderr, "Cannot add trailing / to PXE: > 79 characters\n");
			break;
		case OS_ERR:
			fprintf(stderr, "Cannot add trailing / to TOPLEVELOS: > 79 characters\n");
			break;
	}
}

void print_cbc_config(cbc_config_t *cbc)
{
	fprintf(stderr, "DB: %s\n", cbc->db);
	fprintf(stderr, "USER: %s\n", cbc->user);
	fprintf(stderr, "PASS: %s\n", cbc->pass);
	fprintf(stderr, "HOST: %s\n", cbc->host);
	fprintf(stderr, "PORT: %d\n", cbc->port);
	fprintf(stderr, "SOCKER: %s\n", cbc->socket);
	fprintf(stderr, "TMPDIR: %s\n", cbc->tmpdir);
	fprintf(stderr, "TFTPDIR: %s\n", cbc->tftpdir);
	fprintf(stderr, "PXE: %s\n", cbc->pxe);
	fprintf(stderr, "TOPLEVELOS: %s\n", cbc->toplevelos);
	fprintf(stderr, "DHCPCONF: %s\n", cbc->dhcpconf);
}