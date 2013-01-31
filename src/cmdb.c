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
 *  cmdb.c
 *
 *  Contains main() function for cmdb program
 *
 *  Command line arguments:
 *
 *  -s: Choose a server
 *  -c: Choose a customer
 *  -t: Choose a contact
 *  -d: Display details
 *  -l: List <customers|contacts|servers>
 *  -n <name>: Name of customer / contact / server
 *  -i <id>: UUID's of servers OR COID of customer OR CONID of contact
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "checks.h"

int main(int argc, char *argv[])
{
	cmdb_comm_line_t *cm;
	cmdb_config_t *cmc;
	char *cmdb_config;
	int retval;

	if (!(cmc = malloc(sizeof(cmdb_config_t))))
		report_error(MALLOC_FAIL, "cmc in cmdb.c");
	if (!(cm = malloc(sizeof(cmdb_comm_line_t))))
		report_error(MALLOC_FAIL, "cm in cmdb.c");
	if (!(cmdb_config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "cmdb_config in cmdb.c");
	
	init_cmdb_config_values(cmc);
	retval = parse_cmdb_command_line(argc, argv, cm);
	if (retval < 0) {
		cmdb_main_free(cm, cmc, cmdb_config);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	sprintf(cmdb_config, "%s", cm->config);
	retval = parse_cmdb_config_file(cmc, cmdb_config);
	if (retval == -2) {
		printf("Port value higher that 65535!\n");
		cmdb_main_free(cm, cmc, cmdb_config);
		exit (1);
	}
	
	switch (cm->type) {
		case SERVER:
			if ((strncmp(cm->name, "NULL", CONF_S) == 0)) {
				retval = validate_user_input(cm->id, UUID_REGEX);
				if (retval < 0)
					retval = validate_user_input(cm->id, NAME_REGEX);
			} else if ((strncmp(cm->id, "NULL", CONF_S) == 0)) {
				retval = validate_user_input(cm->name, NAME_REGEX);
			} else {
				printf("Both name and uuid set to NULL??\n");
				cmdb_main_free(cm, cmc, cmdb_config);
				exit (1);
			}
			if (retval < 0) {
				printf("User input not valid\n");
				cmdb_main_free(cm, cmc, cmdb_config);
				exit (1);
			}
			if (cm->action == DISPLAY) {
				display_server_info(cm->name, cm->id, cmc);
			} else if (cm->action == LIST_OBJ) {
				display_all_servers(cmc);
			} else if (cm->action == ADD_TO_DB) {
				retval = add_server_to_database(cmc);
				if (retval > 0) {
					printf("Error adding to database\n");
					cmdb_main_free(cm, cmc, cmdb_config);
					exit(1);
				} else {
					printf("Added into database\n");
				}
			}
			break;
		case CUSTOMER:
			if ((strncmp(cm->name, "NULL", CONF_S) == 0)) {
				retval = validate_user_input(cm->id, COID_REGEX);
			} else if ((strncmp(cm->id, "NULL", CONF_S) == 0)) {
				retval = validate_user_input(cm->name, CUSTOMER_REGEX);
			} else {
				printf("Both name and coid set to NULL??\n");
				cmdb_main_free(cm, cmc, cmdb_config);
				exit (1);
			}
			if (retval < 0) {
				printf("User input not valid\n");
				cmdb_main_free(cm, cmc, cmdb_config);
				exit (1);
			}
			if (cm->action == DISPLAY)
				display_customer_info(cm->name, cm->id, cmc);
			else if (cm->action == LIST_OBJ)
				display_all_customers(cmc);
			break;
		default:
			printf("Not implemented yet :(\n");
			break;
	}
	cmdb_main_free(cm, cmc, cmdb_config);
	exit (0);
}