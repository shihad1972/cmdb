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

int main(int argc, char *argv[])
{
	cmdb_comm_line_t command, *cm;
	cmdb_config_t cmdb_c, *cmc;
	int retval;
	
	cm = &command;
	cmc = &cmdb_c;
	
	retval = parse_command_line(argc, argv, cm);
	if (retval < 0) {
		switch (retval){
			case NO_NAME:
				fprintf(stderr, "No name specified with -n\n");
				break;
			case NO_ID:
				fprintf(stderr, "No ID specified with -i\n");
				break;
			case NO_TYPE:
				fprintf(stderr, "No type specified on command line\n");
				break;
			case NO_ACTION:
				fprintf(stderr, "No action specified on command line\n");
				break;
			case NO_NAME_OR_ID:
				fprintf(stderr, "No name or ID specified on command line\n");
				break;
			case GENERIC_ERROR:
				fprintf(stderr, "Unknown command line option\n");
				break;
			default:
				fprintf(stderr, "Unknown error code!\n");
				break;
		}
		
		printf("Usage: %s [-s | -c | -t ] [-d | -l ] [-n <name> | -i <id> ]\n",
		       argv[0]);
		exit (retval);
	}
	
	printf("This is what we got in the config struct\n");
	printf("Action: ");
	if (cm->action == DISPLAY)
		printf("Display\n");
	else if (cm->action == LIST_OBJ)
		printf("List\n");
	else if (cm->action == NONE)
		printf("No Action\n");
	else
		printf("Unknown action type\n");
	
	printf("Object type: ");
	if (cm->type == SERVER)
		printf("Server\n");
	else if (cm->type == CUSTOMER)
		printf("Customer\n");
	else if (cm->type == CONTACT)
		printf("Contact\n");
	else if (cm->type == NONE)
		printf("No object type\n");
	else
		printf("Unknown object type");
	
	printf("Name: %s\n", cm->name);
	printf("ID: %s\n", cm->id);
	printf("Config file: %s\n", cm->config);
	
	exit (0);
}