/* commline.c
 *
 * Contains the functions to deal with command line arguments and also to
 * read the values from the configuration file
 *
 * Part of the CMDB program
 *
 * (C) Iain M Conochie 2012
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"

int parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_t *comp)
{
	int i, retval;
	
	retval = NONE;
	
	comp->action = NONE;
	comp->type = NONE;
	strncpy(comp->config, "/etc/dnsa/dnsa.conf", CONF_S - 1);
	strncpy(comp->name, "NULL", CONF_S - 1);
	strncpy(comp->id, "NULL", RANGE_S - 1);
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-s", COMM_S) == 0)) {
			comp->type = SERVER;
		} else if ((strncmp(argv[i], "-c", COMM_S) == 0)) {
			comp->type = CUSTOMER;
		} else if ((strncmp(argv[i], "-t", COMM_S) == 0)) {
			comp->type = CONTACT;
		} else if ((strncmp(argv[i], "-d", COMM_S) == 0)) {
			comp->action = DISPLAY;
		} else if ((strncmp(argv[i], "-l", COMM_S) == 0)) {
			comp->action = LIST_OBJ;
			sprintf(comp->name, "all");
		} else if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_NAME;
			else if ((strncmp(argv[i], "-", 1) == 0))
				retval = NO_NAME;
			else
				strncpy(comp->name, argv[i], CONF_S);
		} else if ((strncmp(argv[i], "-i", COMM_S) == 0)) {
			i++;
			if (i >= argc)
				retval = NO_ID;
			else if ((strncmp(argv[i], "-", 1) == 0))
				retval = NO_ID;
			else
				strncpy(comp->id, argv[i], CONF_S);
		} else {
			retval = GENERIC_ERROR;
		}
	}
	
	if (comp->type == NONE)
		retval = NO_TYPE;
	else if (comp->action == NONE)
		retval = NO_ACTION;
	else if ((strncmp(comp->name, "NULL", CONF_S) == 0) &&
		strncmp(comp->id, "NULL", CONF_S) == 0)
		retval = NO_NAME_OR_ID;
	
	return retval;
}

int parse_cmdb_config_file(cmdb_config_t *dc, char *config)
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
	
	/* We need to check the value of portnop before we convert to int.
	 * Obviously we cannot have a port > 65535
	 */
	portno = strtoul(port, NULL, 10);
	if (portno > 65535) {
		retval = -1;
	} else {
		dc->port = (unsigned int) portno;
	}
	
	return retval;
}

void init_cmdb_config_values(cmdb_config_t *dc)
{
	sprintf(dc->db, "cmdb");
	sprintf(dc->user, "root");
	sprintf(dc->host, "localhost");
	sprintf(dc->pass, "%s", "");
	sprintf(dc->socket, "%s", "");
	dc->port = 3306;
	dc->cliflag = 0;
}