/* 
 * 
 *  cmdb: Configuration Management Database
 *  Copyright (C) 2012 - 2013  Iain M Conochie <iain-AT-thargoid.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *  commline.c
 *
 *  Contains the functions to deal with command line arguments and also to
 *  read the values from the configuration file
 *
 *  Part of the CMDB program
 *
 *  (C) Iain M Conochie 2012 - 2013
 *
 */

#include "../config.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "cmdb_base_sql.h"

int
parse_cmdb_command_line(int argc, char **argv, cmdb_comm_line_t *comp)
{
	int opt, retval;
	
	retval = NONE;
	
	comp->action = NONE;
	comp->type = NONE;
	strncpy(comp->config, "/etc/dnsa/dnsa.conf", CONF_S - 1);
	strncpy(comp->name, "NULL", CONF_S - 1);
	strncpy(comp->id, "NULL", RANGE_S - 1);
	
	while ((opt = getopt(argc, argv, "n:i:dlatsc")) != -1) {
		switch (opt) {
			case 'n':
				snprintf(comp->name, CONF_S, "%s", optarg);
				break;
			case 'i':
				snprintf(comp->id, CONF_S, "%s", optarg);
				break;
			case 's':
				comp->type = SERVER;
				break;
			case 'c':
				comp->type = CUSTOMER;
				break;
			case 'd':
				comp->action = DISPLAY;
				break;
			case 'l':
				comp->action = LIST_OBJ;
				snprintf(comp->name, MAC_S, "all");
				break;
			case 'a':
				comp->action = ADD_TO_DB;
				snprintf(comp->name, MAC_S, "none");
				break;
			case 't':
				comp->type = CONTACT;
				break;
			default:
				printf("Unknown option: %c\n", opt);
				retval = DISPLAY_USAGE;
				break;
		}
	}

	if ((comp->type == NONE) && (comp->action != NONE))
		retval = NO_TYPE;
	else if ((comp->action == NONE) && (comp->type != NONE))
		retval = NO_ACTION;
	else if ((strncmp(comp->name, "NULL", CONF_S) == 0) &&
		(strncmp(comp->id, "NULL", CONF_S) == 0) &&
		(comp->type != NONE || comp->action != NONE))
		retval = NO_NAME_OR_ID;
	else if ((strncmp(comp->name, "NULL", CONF_S) == 0) &&
		(strncmp(comp->id, "NULL", CONF_S) == 0) &&
		(comp->type == NONE && comp->action == NONE))
		retval = DISPLAY_USAGE;
	return retval;
}

int
parse_cmdb_config_file(cmdb_config_t *dc, char *config)
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
		while ((fgets(buff, RANGE_S - 1, cnf))) {
			sscanf(buff, "DBTYPE=%s", dc->dbtype);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "FILE=%s", dc->file);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "PASS=%s", dc->pass);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "HOST=%s", dc->host);	
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "USER=%s", dc->user);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "DB=%s", dc->db);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
			sscanf(buff, "SOCKET=%s", dc->socket);
		}
		rewind(cnf);
		while ((fgets(buff, CONF_S - 1, cnf))) {
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

void
init_cmdb_config_values(cmdb_config_t *dc)
{
	snprintf(dc->dbtype, RANGE_S, "none");
	snprintf(dc->file, CONF_S, "none");
	snprintf(dc->db, CONF_S, "cmdb");
	snprintf(dc->user, CONF_S, "root");
	snprintf(dc->host, CONF_S, "localhost");
	snprintf(dc->pass, CONF_S, "%s", "");
	snprintf(dc->socket, CONF_S, "%s", "");
	dc->port = 3306;
	dc->cliflag = 0;
}

void
cmdb_main_free(cmdb_comm_line_t *cm, cmdb_config_t *cmc, char *cmdb_config)
{
	free(cm);
	free(cmc);
	free(cmdb_config);
}

