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

int parse_command_line(int argc, char **argv, cmdb_comm_line_t *comp)
{
	int i, retval;
	
	retval = NONE;
	
	comp->action = NONE;
	comp->type = NONE;
	strncpy(comp->config, "/etc/conman/conman.conf", CONF_S - 1);
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
				strncpy(comp->id, argv[i], RANGE_S);
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
