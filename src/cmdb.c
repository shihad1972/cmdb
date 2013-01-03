/* cmdb.c
 *
 * Contains main() function for cmdb program
 *
 * Command line arguments:
 *
 * -s: Choose a server
 * -c: Choose a customer
 * -t: Choose a contact
 * -d: Display details
 * -l: List <customers|contacts|servers>
 * -n <name>: Name of customer / contact / server
 * -i <id>: UUID's of servers OR COID of customer OR CONID of contact
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmdb.h"
#include "cmdb_cmdb.h"
#include "checks.h"

int main(int argc, char *argv[])
{
	cmdb_comm_line_t command, *cm;
	cmdb_config_t cmdb_c, *cmc;
	int retval;
	char *cmdb_config, *name, *uuid;
	
	cm = &command;
	cmc = &cmdb_c;
	
	name = cm->name;
	uuid = cm->id;
	
	if (!(cmdb_config = malloc(CONF_S * sizeof(char))))
		report_error(MALLOC_FAIL, "cmdb_config in cmdb.c");
	
	init_cmdb_config_values(cmc);
	retval = parse_cmdb_command_line(argc, argv, cm);
	if (retval < 0) {
		free(cmdb_config);
		display_cmdb_command_line_error(retval, argv[0]);
	}
	sprintf(cmdb_config, "%s", cm->config);
	retval = parse_cmdb_config_file(cmc, cmdb_config);
	if (retval == -2) {
		printf("Port value higher that 65535!\n");
		free(cmdb_config);
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
				free(cmdb_config);
				exit (1);
			}
			if (retval < 0) {
				printf("User input not valid\n");
				free(cmdb_config);
				exit (1);
			}
			if (cm->action == DISPLAY) {
				display_server_info(name, uuid, cmc);
			} else if (cm->action == LIST_OBJ) {
				display_all_servers(cmc);
			} else if (cm->action == ADD_TO_DB) {
				retval = add_server_to_database(cmc);
				if (retval > 0) {
					printf("Error adding to database\n");
					free(cmdb_config);
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
				free(cmdb_config);
				exit (1);
			}
			if (retval < 0) {
				printf("User input not valid\n");
				free(cmdb_config);
				exit (1);
			}
			if (cm->action == DISPLAY)
				display_customer_info(name, uuid, cmc);
			else if (cm->action == LIST_OBJ)
				display_all_customers(cmc);
			break;
		default:
			printf("Not implemented yet :(\n");
			break;
	}
	free(cmdb_config);
	exit (0);
}