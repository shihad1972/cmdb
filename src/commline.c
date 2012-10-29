#include "dnsa.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int parse_command_line(int argc, char **argv, comm_line_t *comp)
{
	int i, retval;
	
	retval = 0;
	strncpy(comp->action, "NULL", COMM_S);
	strncpy(comp->type, "NULL", COMM_S);
	strncpy(comp->domain, "NULL", CONF_S);
	
	for (i = 1; i < argc; i++) {
		if ((strncmp(argv[i], "-d", COMM_S) == 0)) {
			strncpy(comp->action, "display", COMM_S);
		} else if ((strncmp(argv[i], "-w", COMM_S) == 0)) {
			strncpy(comp->action, "write", COMM_S);
		} else if ((strncmp(argv[i], "-f", COMM_S) == 0)) {
			strncpy(comp->type, "forward", COMM_S);
		} else if ((strncmp(argv[i], "-r", COMM_S) == 0)) {
			strncpy(comp->type, "reverse", COMM_S);
		} else if ((strncmp(argv[i], "-n", COMM_S) == 0)) {
			i++;
			if (i >= argc) 
				retval = -1;
			else
				strncpy(comp->domain, argv[i], CONF_S);
		} else {
			retval = -1;
		}
	}
	
	if ((strncmp(comp->action, "NULL", COMM_S) == 0))
		retval = -1;
	else if ((strncmp(comp->type, "NULL", COMM_S) == 0))
		retval = -1;
	else if ((strncmp(comp->domain, "NULL", CONF_S) == 0))
		retval = -1;
	
	return retval;
}