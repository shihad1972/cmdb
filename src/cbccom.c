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
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "PRESEED=%s", cbc->preseed);
		}
		rewind (cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "KICKSTART=%s", cbc->kickstart);
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
	if ((retval = add_trailing_slash(cbc->preseed)) != 0)
		retval = PRESEED_ERR;
	if ((retval = add_trailing_slash(cbc->kickstart)) !=0)
		retval = KICKSTART_ERR;
	
	return retval;
}

int parse_cbc_command_line(int argc, char *argv[], cbc_comm_line_t *cb, cbc_build_t *cbt)
{
	int retval, i;
	
	retval = NONE;
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_NAME;
			else
				strncpy(cb->name, argv[i], CONF_S);
		} else if ((strncmp(argv[i], "-u", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_UUID;
			else
				strncpy(cb->uuid, argv[i], CONF_S);
		} else if ((strncmp(argv[i], "-t", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_ID;
			else
				cb->server_id = strtoul(argv[i], NULL, 10);
		} else if ((strncmp(argv[i], "-k", COMM_S) == 0)) {
			cb->build_type = KICKSTART;
		} else if ((strncmp(argv[i], "-p", COMM_S) == 0)) {
			cb->build_type = PRESEED;
		} else if ((strncmp(argv[i], "-l", COMM_S) == 0)) {
			cb->action = WRITE_CONFIG;
		} else if ((strncmp(argv[i], "-y", COMM_S) == 0)) {
			cb->action = DISPLAY_CONFIG;
		} else {
			retval = DISPLAY_USAGE;
		}
	}
	
	if (cb->action == NONE)
		retval = NO_ACTION;
	else if (cb->build_type == NONE)
		retval = NO_TYPE;
	else if ((cb->server_id == NONE) || (strncmp(cb->uuid, "NONE", CONF_S) == 0)
		|| (strncmp(cb->name, "NONE", CONF_S) == 0))
		retval = NO_NAME_OR_ID;
	
	return retval;
	
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
		case PRESEED_ERR:
			fprintf(stderr, "Cannot add trailing / to PRESEED: > 79 characters\n");
			break;
		case KICKSTART_ERR:
			fprintf(stderr, "Cannot add trailing / to KICKSTART: > 79 characters\n");
			break;
		default:
			fprintf(stderr, "Unkown error code: %d\n", error);
			break;
	}
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

void init_cbc_comm_values(cbc_comm_line_t *cbt)
{
	cbt->action = NONE;
	cbt->build_type = NONE;
	cbt->server_id = NONE;
	cbt->usedb = 1;
	sprintf(cbt->name, "NULL");
	sprintf(cbt->uuid, "NULL");
	sprintf(cbt->config, "/etc/dnsa/dnsa.conf");	
}

void init_cbc_build_values(cbc_build_t *build_config)
{
	sprintf(build_config->ip_address, "NULL");
	sprintf(build_config->mac_address, "NULL");
	sprintf(build_config->hostname, "NULL");
	sprintf(build_config->domain, "NULL");
	sprintf(build_config->alias, "NULL");
	sprintf(build_config->varient, "NULL");
	sprintf(build_config->arch, "NULL");
	sprintf(build_config->boot, "NULL");
	sprintf(build_config->gateway, "NULL");
	sprintf(build_config->nameserver, "NULL");
	sprintf(build_config->netmask, "NULL");
}

void print_cbc_build_values(cbc_build_t *build_config)
{
	fprintf(stderr, "########\nBuild Values\n");
	fprintf(stderr, "IP: %s\n", build_config->ip_address);
	fprintf(stderr, "MAC: %s\n", build_config->mac_address);
	fprintf(stderr, "HOST: %s\n", build_config->hostname);
	fprintf(stderr, "DOMAIN: %s\n", build_config->domain);
	fprintf(stderr, "OS: %s\n", build_config->alias);
	fprintf(stderr, "BUILD VARIENT: %s\n", build_config->varient);
	fprintf(stderr, "ARCH: %s\n", build_config->arch);
	fprintf(stderr, "BOOT LINE: %s\n", build_config->boot);
	fprintf(stderr, "GW: %s\n", build_config->gateway);
	fprintf(stderr, "NS: %s\n", build_config->nameserver);
	fprintf(stderr, "NETMASK: %s\n", build_config->netmask);
	fprintf(stderr, "\n");
}

void print_cbc_command_line_values(cbc_comm_line_t *command_line)
{
	fprintf(stderr, "########\nCommand line Values\n");
	fprintf(stderr, "Action: %d\n", command_line->action);
	fprintf(stderr, "Usedb: %d\n", command_line->usedb);
	fprintf(stderr, "Build type: %d\n", command_line->build_type);
	fprintf(stderr, "Server ID: %ld\n", command_line->server_id);
	fprintf(stderr, "Config: %s\n", command_line->config);
	fprintf(stderr, "Name: %s\n", command_line->name);
	fprintf(stderr, "UUID: %s\n", command_line->uuid);
	fprintf(stderr, "\n");
}

void print_cbc_config(cbc_config_t *cbc)
{
	fprintf(stderr, "########\nConfig Values\n");
	fprintf(stderr, "DB: %s\n", cbc->db);
	fprintf(stderr, "USER: %s\n", cbc->user);
	fprintf(stderr, "PASS: %s\n", cbc->pass);
	fprintf(stderr, "HOST: %s\n", cbc->host);
	fprintf(stderr, "PORT: %d\n", cbc->port);
	fprintf(stderr, "SOCKET: %s\n", cbc->socket);
	fprintf(stderr, "TMPDIR: %s\n", cbc->tmpdir);
	fprintf(stderr, "TFTPDIR: %s\n", cbc->tftpdir);
	fprintf(stderr, "PXE: %s\n", cbc->pxe);
	fprintf(stderr, "TOPLEVELOS: %s\n", cbc->toplevelos);
	fprintf(stderr, "DHCPCONF: %s\n", cbc->dhcpconf);
	fprintf(stderr, "PRESEED: %s\n", cbc->preseed);
	fprintf(stderr, "KICKSTART: %s\n", cbc->kickstart);
	fprintf(stderr, "\n");
}