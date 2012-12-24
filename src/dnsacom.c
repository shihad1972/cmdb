/* commline.c:
 * 
 * Contains functions to deal with command line arguments and also
 * to read the values from the configuration file.
 *
 * Part of the DNSA  program
 * 
 * (C) Iain M Conochie 2012
 * 
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_dnsa.h"
#include "checks.h"

int parse_dnsa_command_line(int argc, char **argv, comm_line_t *comp)
{
	int i, retval;
	
	retval = 0;

	comp->action = NONE;
	comp->type = NONE;
	comp->prefix = NONE;
	strncpy(comp->domain, "NULL", CONF_S);
	strncpy(comp->dest, "NULL", RANGE_S);
	strncpy(comp->rtype, "NULL", RANGE_S);
	strncpy(comp->host, "NULL", RANGE_S);
	strncpy(comp->config, "/etc/dnsa/dnsa.conf", CONF_S);
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-d", COMM_S) == 0)) {
			comp->action = DISPLAY_ZONE;
		} else if ((strncmp(argv[i], "-w", COMM_S) == 0)) {
			comp->action = WRITE_ZONE;
		} else if ((strncmp(argv[i], "-z", COMM_S) == 0)) {
			comp->action = ADD_ZONE;
		} else if ((strncmp(argv[i], "-c", COMM_S) == 0)) {
			comp->action = CONFIGURE_ZONE;
			strncpy(comp->domain, "none", CONF_S);
		} else if ((strncmp(argv[i], "-a", COMM_S) == 0)) {
			comp->action = ADD_HOST;
		} else if ((strncmp(argv[i], "-l", COMM_S) == 0)) {
			comp->action = LIST_ZONES;
			strncpy(comp->domain, "all", CONF_S);
		} else if ((strncmp(argv[i], "-f", COMM_S) == 0)) {
			comp->type = FORWARD_ZONE;
		} else if ((strncmp(argv[i], "-r", COMM_S) == 0)) {
			comp->type = REVERSE_ZONE;
		} else if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc) 
				retval = NO_DOMAIN_NAME;
			else
				strncpy(comp->domain, argv[i], CONF_S);
		} else if ((strncmp(argv[i], "-i", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_IP_ADDRESS;
			else
				strncpy(comp->dest, argv[i], RBUFF_S);
		} else if ((strncmp(argv[i], "-h", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_HOST_NAME;
			else
				strncpy(comp->host, argv[i], RBUFF_S);
		} else if ((strncmp(argv[i], "-t", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_RECORD_TYPE;
			else
				strncpy(comp->rtype, argv[i], RANGE_S);
		} else if ((strncmp(argv[i], "-p", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_PREFIX;
			else
				comp->prefix = strtoul(argv[i], NULL, 10);
		} else {
			retval = DISPLAY_USAGE;
		}
	}
	if (comp->action == NONE && comp->type == NONE && strncmp(comp->domain, "NULL", CONF_S) == 0)
		retval = DISPLAY_USAGE;
	else if (comp->action == NONE)
		retval = NO_ACTION;
	else if (comp->type == NONE)
		retval = NO_TYPE;
	else if ((strncmp(comp->domain, "NULL", CONF_S) == 0))
		retval = NO_DOMAIN_NAME;
	else if ((comp->action == ADD_HOST && strncmp(comp->dest, "NULL", RANGE_S) == 0))
		retval = NO_IP_ADDRESS;
	else if ((comp->action == ADD_HOST && strncmp(comp->host, "NULL", RBUFF_S) == 0))
		retval = NO_HOST_NAME;
	else if ((comp->action == ADD_HOST && strncmp(comp->rtype, "NULL", RANGE_S) == 0))
		retval = NO_RECORD_TYPE;
	else if ((comp->action == ADD_ZONE && comp->type == REVERSE_ZONE && comp->prefix == 0))
		retval = NO_PREFIX;
	return retval;
}

int parse_dnsa_config_file(dnsa_config_t *dc, char *config)
{
	FILE *cnf;	/* File handle for config file */
	int retval;
	unsigned long int portno;

	char buff[CONF_S] = "";
	char port[CONF_S] = "";

	dc->port = 3306;
	dc->cliflag = 0;

	if (!(cnf = fopen(config, "r"))) {
		fprintf(stderr, "Cannot open config file %s\n", config);
		fprintf(stderr, "Using default values\n");
		retval = CONF_ERR;
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
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DIR=%s", dc->dir);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "BIND=%s", dc->bind);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "REV=%s", dc->rev);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "DNSA=%s", dc->dnsa);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "RNDC=%s", dc->rndc);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "CHKZ=%s", dc->chkz);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S, cnf))) {
			sscanf(buff, "CHKC=%s", dc->chkc);
		}
		retval = NONE;
		fclose(cnf);
	}
	
	/* We need to check the value of portnop before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = PORT_ERR;
	} else {
		dc->port = (unsigned int) portno;
	}
	
	/* The next 2 values need to be checked for a trailing /
	 * If there is not one then add it
	 */
	
	if ((retval = add_trailing_slash(dc->dir)) != 0)
		retval = DIR_ERR;
	if ((retval = add_trailing_slash(dc->bind)) != 0)
		retval = BIND_ERR;
	
	return retval;
}

void parse_dnsa_config_error(int error)
{
	switch(error) {
		case PORT_ERR:
			fprintf(stderr, "Port higher than 65535!\n");
			break;
		case DIR_ERR:
			fprintf(stderr, "Cannot add trailing / to DIR: > 79 characters\n");
			break;
		case BIND_ERR:
			fprintf(stderr, "Cannot add trailing / to BIND: > 79 characters\n");
			break;
		}
}

void init_config_values(dnsa_config_t *dc)
{
	char *buff;
	buff = dc->socket;
	sprintf(dc->db, "bind");
	sprintf(dc->user, "root");
	sprintf(dc->host, "localhost");
	sprintf(dc->pass, "%s", "");
	sprintf(dc->dir, "/var/named/");
	sprintf(dc->bind, "/var/named/");
	sprintf(dc->dnsa, "dnsa.conf");
	sprintf(dc->rev, "dnsa-rev.conf");
	sprintf(dc->rndc, "/usr/sbin/rndc");
	sprintf(dc->chkz, "/usr/sbin/named-checkzone");
	sprintf(dc->chkc, "/usr/sbin/named-checkconf");
	sprintf(buff, "%s", "");
}

int validate_comm_line(comm_line_t *comm)
{
	int retval;
	
	retval = 0;
	
	retval = validate_user_input(comm->host, NAME_REGEX);
	if (retval < 0)
		return retval;
	retval = validate_user_input(comm->dest, IP_REGEX);
	if (retval < 0)
		return retval;
	return retval;
}